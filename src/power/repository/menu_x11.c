#include "menu_x11.h"
#include "../model/menu_nav.h"
#include "../../shared/logging/logger.h"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_FONT "monospace:size=12"
#define BORDER_WIDTH 2
#define GRAB_ATTEMPTS 1000

/* Fallbacks used only when neither power.conf nor X resources give a colour. */
#define FALLBACK_BG "#1e1e1e"
#define FALLBACK_FG "#d4d4d4"
#define FALLBACK_SEL_BG "#3b6ea5"
#define FALLBACK_SEL_FG "#ffffff"

typedef enum {
    MENU_RESULT_PENDING,
    MENU_RESULT_CHOSEN,
    MENU_RESULT_CANCELLED
} menu_result_t;

typedef struct {
    Display *dpy;
    int screen;
    Window root;
    Window win;
    Pixmap buffer;
    XftFont *font;
    XftDraw *draw;
    XftColor bg;
    XftColor fg;
    XftColor sel_bg;
    XftColor sel_fg;
    XftColor border;
    int has_colors;
    int width;
    int height;
    int row_height;
    int pad;
} menu_t;

/* Private function prototypes */
static int load_color(menu_t *menu, const char *const candidates[],
                      int candidate_count, XftColor *out);
static int load_palette(menu_t *menu, const PowerConfig *config);
static void free_palette(menu_t *menu);
static void compute_geometry(menu_t *menu, const char *const options[],
                             int count);
static void place_window(menu_t *menu, int *x, int *y);
static int grab_input(menu_t *menu);
static void redraw(menu_t *menu, const char *const options[], int count,
                   int selected);
static menu_result_t handle_key(XKeyEvent *event, int count, int *selected);
static menu_result_t handle_button(menu_t *menu, XButtonEvent *event,
                                   int count, int *selected);
static menu_result_t event_loop(menu_t *menu, const char *const options[],
                                int count, int *selected);

/* Public function definitions */
PowerStatus menu_x11_run(const char *const options[], int count,
                         const PowerConfig *config, char *selection,
                         size_t selection_size) {
    if (options == NULL || selection == NULL || selection_size == 0) {
        return POWER_STATUS_ERR_NULL_POINTER;
    }
    selection[0] = '\0';
    if (count <= 0) {
        return POWER_STATUS_ERR_NO_LAUNCHER;
    }

    /* Under a native Wayland session an X window would run through XWayland,
     * where keyboard grabs are unreliable; external launchers do better. */
    if (getenv("WAYLAND_DISPLAY") != NULL) {
        LOG_DEBUG("Wayland session detected; skipping built-in X11 menu");
        return POWER_STATUS_ERR_NO_LAUNCHER;
    }

    menu_t menu;
    memset(&menu, 0, sizeof(menu));
    PowerStatus status = POWER_STATUS_ERR_NO_LAUNCHER;

    menu.dpy = XOpenDisplay(NULL);
    if (menu.dpy == NULL) {
        LOG_DEBUG("Built-in menu: cannot open X display");
        return status;
    }
    menu.screen = DefaultScreen(menu.dpy);
    menu.root = RootWindow(menu.dpy, menu.screen);

    const char *font_name =
        (config != NULL && config->menu_font != NULL) ? config->menu_font
                                                      : DEFAULT_FONT;
    menu.font = XftFontOpenName(menu.dpy, menu.screen, font_name);
    if (menu.font == NULL) {
        LOG_WARN("Built-in menu: cannot load font '%s'", font_name);
        goto cleanup;
    }
    if (!load_palette(&menu, config)) {
        LOG_WARN("Built-in menu: cannot allocate colours");
        goto cleanup;
    }

    compute_geometry(&menu, options, count);
    int x = 0;
    int y = 0;
    place_window(&menu, &x, &y);

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.border_pixel = menu.border.pixel;
    attrs.background_pixel = menu.bg.pixel;
    attrs.event_mask = ExposureMask | KeyPressMask | ButtonPressMask |
                       PointerMotionMask | VisibilityChangeMask;
    menu.win = XCreateWindow(
        menu.dpy, menu.root, x, y, (unsigned int)menu.width,
        (unsigned int)menu.height, BORDER_WIDTH,
        DefaultDepth(menu.dpy, menu.screen), InputOutput,
        DefaultVisual(menu.dpy, menu.screen),
        CWOverrideRedirect | CWBorderPixel | CWBackPixel | CWEventMask,
        &attrs);

    /* The class lets compositors (picom) target the menu with rules. */
    XClassHint class_hint = {"power", "Power"};
    XSetClassHint(menu.dpy, menu.win, &class_hint);

    menu.buffer = XCreatePixmap(menu.dpy, menu.win, (unsigned int)menu.width,
                                (unsigned int)menu.height,
                                (unsigned int)DefaultDepth(menu.dpy,
                                                           menu.screen));
    menu.draw = XftDrawCreate(menu.dpy, menu.buffer,
                              DefaultVisual(menu.dpy, menu.screen),
                              DefaultColormap(menu.dpy, menu.screen));

    XMapRaised(menu.dpy, menu.win);
    if (!grab_input(&menu)) {
        LOG_WARN("Built-in menu: cannot grab the keyboard");
        goto cleanup;
    }

    int selected = 0;
    menu_result_t result = event_loop(&menu, options, count, &selected);
    if (result == MENU_RESULT_CHOSEN) {
        strncpy(selection, options[selected], selection_size - 1);
        selection[selection_size - 1] = '\0';
    }
    status = POWER_STATUS_OK;

cleanup:
    if (menu.draw != NULL) {
        XftDrawDestroy(menu.draw);
    }
    if (menu.buffer != 0) {
        XFreePixmap(menu.dpy, menu.buffer);
    }
    if (menu.win != 0) {
        XUngrabKeyboard(menu.dpy, CurrentTime);
        XUngrabPointer(menu.dpy, CurrentTime);
        XDestroyWindow(menu.dpy, menu.win);
    }
    free_palette(&menu);
    if (menu.font != NULL) {
        XftFontClose(menu.dpy, menu.font);
    }
    XCloseDisplay(menu.dpy);
    return status;
}

/* Private function definitions */

/* Try each candidate name in order and keep the first that X accepts, so a
 * typo in one source falls through to the next instead of failing. */
static int load_color(menu_t *menu, const char *const candidates[],
                      int candidate_count, XftColor *out) {
    for (int i = 0; i < candidate_count; i++) {
        if (candidates[i] == NULL || candidates[i][0] == '\0') {
            continue;
        }
        if (XftColorAllocName(menu->dpy, DefaultVisual(menu->dpy, menu->screen),
                              DefaultColormap(menu->dpy, menu->screen),
                              candidates[i], out)) {
            return 1;
        }
        LOG_WARN("Built-in menu: unknown colour '%s'", candidates[i]);
    }
    return 0;
}

static int load_palette(menu_t *menu, const PowerConfig *config) {
    const char *cfg_bg = config != NULL ? config->menu_bg : NULL;
    const char *cfg_fg = config != NULL ? config->menu_fg : NULL;
    const char *cfg_sel_bg = config != NULL ? config->menu_sel_bg : NULL;
    const char *cfg_sel_fg = config != NULL ? config->menu_sel_fg : NULL;
    const char *cfg_border = config != NULL ? config->menu_border : NULL;

    const char *bg[] = {cfg_bg, XGetDefault(menu->dpy, "power", "background"),
                        FALLBACK_BG};
    const char *fg[] = {cfg_fg, XGetDefault(menu->dpy, "power", "foreground"),
                        FALLBACK_FG};
    const char *sel_bg[] = {cfg_sel_bg, XGetDefault(menu->dpy, "power", "color4"),
                            FALLBACK_SEL_BG};
    const char *sel_fg[] = {cfg_sel_fg, XGetDefault(menu->dpy, "power", "color0"),
                            FALLBACK_SEL_FG};

    if (!load_color(menu, bg, 3, &menu->bg) ||
        !load_color(menu, fg, 3, &menu->fg) ||
        !load_color(menu, sel_bg, 3, &menu->sel_bg) ||
        !load_color(menu, sel_fg, 3, &menu->sel_fg)) {
        return 0;
    }
    menu->has_colors = 1;

    /* The border follows the highlight colour unless set explicitly. */
    const char *border[] = {cfg_border, XGetDefault(menu->dpy, "power", "color4"),
                            FALLBACK_SEL_BG};
    return load_color(menu, border, 3, &menu->border);
}

static void free_palette(menu_t *menu) {
    if (!menu->has_colors) {
        return;
    }
    Visual *visual = DefaultVisual(menu->dpy, menu->screen);
    Colormap colormap = DefaultColormap(menu->dpy, menu->screen);
    XftColorFree(menu->dpy, visual, colormap, &menu->bg);
    XftColorFree(menu->dpy, visual, colormap, &menu->fg);
    XftColorFree(menu->dpy, visual, colormap, &menu->sel_bg);
    XftColorFree(menu->dpy, visual, colormap, &menu->sel_fg);
    XftColorFree(menu->dpy, visual, colormap, &menu->border);
}

/* Sizes are multiples of the font height so the menu scales with the font
 * size and the user's DPI instead of using fixed pixel values. */
static void compute_geometry(menu_t *menu, const char *const options[],
                             int count) {
    int font_height = menu->font->ascent + menu->font->descent;
    int widest = 0;
    for (int i = 0; i < count; i++) {
        XGlyphInfo extents;
        XftTextExtentsUtf8(menu->dpy, menu->font,
                           (const FcChar8 *)options[i],
                           (int)strlen(options[i]), &extents);
        if (extents.xOff > widest) {
            widest = extents.xOff;
        }
    }

    menu->pad = font_height / 2;
    menu->row_height = font_height * 2;
    menu->width = widest + font_height * 2;
    if (menu->width < font_height * 12) {
        menu->width = font_height * 12;
    }
    menu->height = menu->row_height * count + menu->pad * 2;
}

/* Centre on the monitor under the pointer, as dmenu does, so the menu shows
 * up where the user is looking on multi-monitor setups. */
static void place_window(menu_t *menu, int *x, int *y) {
    int mon_x = 0;
    int mon_y = 0;
    int mon_w = DisplayWidth(menu->dpy, menu->screen);
    int mon_h = DisplayHeight(menu->dpy, menu->screen);

    int screen_count = 0;
    XineramaScreenInfo *screens = NULL;
    if (XineramaIsActive(menu->dpy)) {
        screens = XineramaQueryScreens(menu->dpy, &screen_count);
    }
    if (screens != NULL) {
        Window unused_win;
        int pointer_x = 0;
        int pointer_y = 0;
        int unused_int;
        unsigned int unused_mask;
        XQueryPointer(menu->dpy, menu->root, &unused_win, &unused_win,
                      &pointer_x, &pointer_y, &unused_int, &unused_int,
                      &unused_mask);
        for (int i = 0; i < screen_count; i++) {
            if (pointer_x >= screens[i].x_org &&
                pointer_x < screens[i].x_org + screens[i].width &&
                pointer_y >= screens[i].y_org &&
                pointer_y < screens[i].y_org + screens[i].height) {
                mon_x = screens[i].x_org;
                mon_y = screens[i].y_org;
                mon_w = screens[i].width;
                mon_h = screens[i].height;
                break;
            }
        }
        XFree(screens);
    }

    *x = mon_x + (mon_w - menu->width - 2 * BORDER_WIDTH) / 2;
    *y = mon_y + (mon_h - menu->height - 2 * BORDER_WIDTH) / 2;
}

/* The menu is normally launched from a keybinding, so the keys that started
 * it may still be held when the window appears; retry until the WM releases
 * its own grab rather than failing on the first attempt. */
static int grab_input(menu_t *menu) {
    struct timespec pause = {0, 1000000L};
    int keyboard_grabbed = 0;
    for (int i = 0; i < GRAB_ATTEMPTS; i++) {
        if (XGrabKeyboard(menu->dpy, menu->root, True, GrabModeAsync,
                          GrabModeAsync, CurrentTime) == GrabSuccess) {
            keyboard_grabbed = 1;
            break;
        }
        nanosleep(&pause, NULL);
    }
    if (!keyboard_grabbed) {
        return 0;
    }

    /* The pointer grab is a convenience (click to pick, click away to
     * cancel); the menu still works from the keyboard without it. */
    for (int i = 0; i < GRAB_ATTEMPTS; i++) {
        if (XGrabPointer(menu->dpy, menu->win, False,
                         ButtonPressMask | PointerMotionMask, GrabModeAsync,
                         GrabModeAsync, None, None,
                         CurrentTime) == GrabSuccess) {
            break;
        }
        nanosleep(&pause, NULL);
    }
    return 1;
}

static void redraw(menu_t *menu, const char *const options[], int count,
                   int selected) {
    int font_height = menu->font->ascent + menu->font->descent;

    XftDrawRect(menu->draw, &menu->bg, 0, 0, (unsigned int)menu->width,
                (unsigned int)menu->height);
    for (int i = 0; i < count; i++) {
        int top = menu->pad + i * menu->row_height;
        const XftColor *text_color = &menu->fg;
        if (i == selected) {
            XftDrawRect(menu->draw, &menu->sel_bg, 0, top,
                        (unsigned int)menu->width,
                        (unsigned int)menu->row_height);
            text_color = &menu->sel_fg;
        }
        int baseline = top + (menu->row_height - font_height) / 2 +
                       menu->font->ascent;
        XftDrawStringUtf8(menu->draw, text_color, menu->font, font_height,
                          baseline, (const FcChar8 *)options[i],
                          (int)strlen(options[i]));
    }

    GC gc = XCreateGC(menu->dpy, menu->win, 0, NULL);
    XCopyArea(menu->dpy, menu->buffer, menu->win, gc, 0, 0,
              (unsigned int)menu->width, (unsigned int)menu->height, 0, 0);
    XFreeGC(menu->dpy, gc);
    XFlush(menu->dpy);
}

/* Digits only move the highlight and never confirm: a stray keypress must
 * not be able to power the machine off. Enter is the sole confirmation. */
static menu_result_t handle_key(XKeyEvent *event, int count, int *selected) {
    char text[8];
    KeySym keysym = NoSymbol;
    int length = XLookupString(event, text, (int)sizeof(text), &keysym, NULL);
    int ctrl = (event->state & ControlMask) != 0;

    switch (keysym) {
    case XK_Escape:
        return MENU_RESULT_CANCELLED;
    case XK_Return:
    case XK_KP_Enter:
        return MENU_RESULT_CHOSEN;
    case XK_Up:
    case XK_ISO_Left_Tab:
        *selected = menu_nav_step(*selected, count, -1);
        return MENU_RESULT_PENDING;
    case XK_Down:
    case XK_Tab:
        *selected = menu_nav_step(*selected, count, 1);
        return MENU_RESULT_PENDING;
    case XK_Home:
        *selected = 0;
        return MENU_RESULT_PENDING;
    case XK_End:
        *selected = count - 1;
        return MENU_RESULT_PENDING;
    default:
        break;
    }

    if (length == 1) {
        if (ctrl && text[0] == '\x03') { /* Ctrl+C */
            return MENU_RESULT_CANCELLED;
        }
        if ((ctrl && text[0] == '\x10') || text[0] == 'k') { /* Ctrl+P */
            *selected = menu_nav_step(*selected, count, -1);
        } else if ((ctrl && text[0] == '\x0e') || text[0] == 'j') { /* Ctrl+N */
            *selected = menu_nav_step(*selected, count, 1);
        } else if (text[0] == 'q') {
            return MENU_RESULT_CANCELLED;
        } else {
            int row = menu_nav_row_from_digit(text[0], count);
            if (row >= 0) {
                *selected = row;
            }
        }
    }
    return MENU_RESULT_PENDING;
}

static menu_result_t handle_button(menu_t *menu, XButtonEvent *event,
                                   int count, int *selected) {
    if (event->button == Button4) {
        *selected = menu_nav_step(*selected, count, -1);
        return MENU_RESULT_PENDING;
    }
    if (event->button == Button5) {
        *selected = menu_nav_step(*selected, count, 1);
        return MENU_RESULT_PENDING;
    }
    if (event->button != Button1) {
        return MENU_RESULT_PENDING;
    }

    int inside = event->x >= 0 && event->x < menu->width && event->y >= 0 &&
                 event->y < menu->height;
    if (!inside) {
        return MENU_RESULT_CANCELLED;
    }
    int row = menu_nav_row_at(event->y, menu->pad, menu->row_height, count);
    if (row < 0) {
        return MENU_RESULT_PENDING;
    }
    *selected = row;
    return MENU_RESULT_CHOSEN;
}

static menu_result_t event_loop(menu_t *menu, const char *const options[],
                                int count, int *selected) {
    menu_result_t result = MENU_RESULT_PENDING;
    while (result == MENU_RESULT_PENDING) {
        XEvent event;
        XNextEvent(menu->dpy, &event);
        switch (event.type) {
        case Expose:
            if (event.xexpose.count == 0) {
                redraw(menu, options, count, *selected);
            }
            break;
        case VisibilityNotify:
            if (event.xvisibility.state != VisibilityUnobscured) {
                XRaiseWindow(menu->dpy, menu->win);
            }
            break;
        case KeyPress:
            result = handle_key(&event.xkey, count, selected);
            redraw(menu, options, count, *selected);
            break;
        case ButtonPress:
            result = handle_button(menu, &event.xbutton, count, selected);
            redraw(menu, options, count, *selected);
            break;
        case MotionNotify: {
            int row = menu_nav_row_at(event.xmotion.y, menu->pad,
                                      menu->row_height, count);
            if (row >= 0 && row != *selected) {
                *selected = row;
                redraw(menu, options, count, *selected);
            }
            break;
        }
        default:
            break;
        }
    }
    return result;
}
