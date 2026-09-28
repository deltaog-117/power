#include "menu_wayland.h"
#include "../model/menu_nav.h"
#include "../model/menu_style.h"
#include "../../shared/logging/logger.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include <cairo.h>
#include <pango/pangocairo.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#define DEFAULT_FONT "monospace 12"
#define BORDER_WIDTH 2
#define BYTES_PER_PIXEL 4
#define BUTTON_LEFT 0x110 /* BTN_LEFT from linux/input-event-codes.h */
#define KEYCODE_OFFSET 8  /* evdev codes and xkb keycodes differ by 8 */
#define SCROLL_NOTCH 10.0 /* wl_fixed value one wheel click reports */
#define FONT_TEXT_SIZE 128

/* Fallbacks used only when power.conf gives no usable colour. They match the
 * X11 backend so both menus look the same out of the box. */
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
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shm *shm;
    struct wl_seat *seat;
    struct zwlr_layer_shell_v1 *layer_shell;
    struct wl_surface *surface;
    struct zwlr_layer_surface_v1 *layer_surface;
    struct wl_keyboard *keyboard;
    struct wl_pointer *pointer;

    struct xkb_context *xkb_context;
    struct xkb_keymap *xkb_keymap;
    struct xkb_state *xkb_state;

    struct wl_buffer *buffer;
    void *pixels;
    size_t pixels_size;
    cairo_surface_t *canvas;

    PangoFontDescription *font;
    menu_rgb_t bg;
    menu_rgb_t fg;
    menu_rgb_t sel_bg;
    menu_rgb_t sel_fg;
    menu_rgb_t border;

    const char *const *options;
    int count;
    int selected;
    menu_result_t result;

    int configured;
    int closed;
    int surface_width;
    int surface_height;

    int font_height;
    int row_height;
    int pad;
    int menu_width;
    int menu_height;

    int pointer_x;
    int pointer_y;
    double scroll;
} menu_t;

/* Private function prototypes */
static void resolve_color(const char *configured, const char *fallback,
                          const char *name, menu_rgb_t *out);
static int load_style(menu_t *menu, const PowerConfig *config);
static int measure_geometry(menu_t *menu);
static int create_buffer(menu_t *menu, int width, int height);
static void destroy_buffer(menu_t *menu);
static void set_source(cairo_t *cr, const menu_rgb_t *color);
static void redraw(menu_t *menu);
static int inner_x(const menu_t *menu);
static int inner_y(const menu_t *menu);
static int pointer_row(const menu_t *menu);
static void step_selection(menu_t *menu, int delta);
static void handle_key(menu_t *menu, xkb_keysym_t sym);
static void cleanup(menu_t *menu);

static void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *interface,
                            uint32_t version);
static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name);
static void seat_capabilities(void *data, struct wl_seat *seat,
                              uint32_t capabilities);
static void keyboard_keymap(void *data, struct wl_keyboard *keyboard,
                            uint32_t format, int32_t fd, uint32_t size);
static void keyboard_enter(void *data, struct wl_keyboard *keyboard,
                           uint32_t serial, struct wl_surface *surface,
                           struct wl_array *keys);
static void keyboard_leave(void *data, struct wl_keyboard *keyboard,
                           uint32_t serial, struct wl_surface *surface);
static void keyboard_key(void *data, struct wl_keyboard *keyboard,
                         uint32_t serial, uint32_t time, uint32_t key,
                         uint32_t state);
static void keyboard_modifiers(void *data, struct wl_keyboard *keyboard,
                               uint32_t serial, uint32_t depressed,
                               uint32_t latched, uint32_t locked,
                               uint32_t group);
static void pointer_enter(void *data, struct wl_pointer *pointer,
                          uint32_t serial, struct wl_surface *surface,
                          wl_fixed_t x, wl_fixed_t y);
static void pointer_leave(void *data, struct wl_pointer *pointer,
                          uint32_t serial, struct wl_surface *surface);
static void pointer_motion(void *data, struct wl_pointer *pointer,
                           uint32_t time, wl_fixed_t x, wl_fixed_t y);
static void pointer_button(void *data, struct wl_pointer *pointer,
                           uint32_t serial, uint32_t time, uint32_t button,
                           uint32_t state);
static void pointer_axis(void *data, struct wl_pointer *pointer, uint32_t time,
                         uint32_t axis, wl_fixed_t value);
static void layer_configure(void *data, struct zwlr_layer_surface_v1 *surface,
                            uint32_t serial, uint32_t width, uint32_t height);
static void layer_closed(void *data, struct zwlr_layer_surface_v1 *surface);

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};
static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_capabilities,
};
static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_keymap,
    .enter = keyboard_enter,
    .leave = keyboard_leave,
    .key = keyboard_key,
    .modifiers = keyboard_modifiers,
};
static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter,
    .leave = pointer_leave,
    .motion = pointer_motion,
    .button = pointer_button,
    .axis = pointer_axis,
};
static const struct zwlr_layer_surface_v1_listener layer_listener = {
    .configure = layer_configure,
    .closed = layer_closed,
};

/* Public function definitions */
PowerStatus menu_wayland_run(const char *const options[], int count,
                             const PowerConfig *config, char *selection,
                             size_t selection_size) {
    if (options == NULL || selection == NULL || selection_size == 0) {
        return POWER_STATUS_ERR_NULL_POINTER;
    }
    selection[0] = '\0';
    if (count <= 0) {
        return POWER_STATUS_ERR_NO_LAUNCHER;
    }
    if (getenv("WAYLAND_DISPLAY") == NULL) {
        LOG_DEBUG("Built-in Wayland menu: no WAYLAND_DISPLAY");
        return POWER_STATUS_ERR_NO_LAUNCHER;
    }

    menu_t menu;
    memset(&menu, 0, sizeof(menu));
    menu.options = options;
    menu.count = count;
    menu.result = MENU_RESULT_PENDING;
    PowerStatus status = POWER_STATUS_ERR_NO_LAUNCHER;

    if (load_style(&menu, config) != 0 || measure_geometry(&menu) != 0) {
        LOG_WARN("Built-in Wayland menu: cannot load font or colours");
        goto done;
    }

    menu.display = wl_display_connect(NULL);
    if (menu.display == NULL) {
        LOG_DEBUG("Built-in Wayland menu: cannot connect to the compositor");
        goto done;
    }
    menu.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (menu.xkb_context == NULL) {
        LOG_WARN("Built-in Wayland menu: cannot create the keyboard context");
        goto done;
    }

    menu.registry = wl_display_get_registry(menu.display);
    wl_registry_add_listener(menu.registry, &registry_listener, &menu);
    /* Three round trips, each answering the previous one: globals, then the
     * seat's capabilities, then the keyboard's keymap. */
    for (int i = 0; i < 3; i++) {
        if (wl_display_roundtrip(menu.display) < 0) {
            LOG_WARN("Built-in Wayland menu: compositor connection failed");
            goto done;
        }
    }
    if (menu.compositor == NULL || menu.shm == NULL ||
        menu.layer_shell == NULL) {
        LOG_DEBUG("Built-in Wayland menu: compositor lacks wlr-layer-shell");
        goto done;
    }
    if (menu.keyboard == NULL || menu.xkb_state == NULL) {
        LOG_WARN("Built-in Wayland menu: no keyboard available");
        goto done;
    }

    menu.surface = wl_compositor_create_surface(menu.compositor);
    /* A NULL output lets the compositor pick, normally the focused one. */
    menu.layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        menu.layer_shell, menu.surface, NULL,
        ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "power");
    zwlr_layer_surface_v1_add_listener(menu.layer_surface, &layer_listener,
                                       &menu);
    /* Anchoring all four edges with size 0x0 stretches the surface over the
     * output, so a click outside the menu can cancel it like on X11. */
    zwlr_layer_surface_v1_set_anchor(
        menu.layer_surface,
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
            ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    zwlr_layer_surface_v1_set_exclusive_zone(menu.layer_surface, -1);
    zwlr_layer_surface_v1_set_keyboard_interactivity(
        menu.layer_surface,
        ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE);
    zwlr_layer_surface_v1_set_size(menu.layer_surface, 0, 0);
    wl_surface_commit(menu.surface);

    while (!menu.configured && !menu.closed) {
        if (wl_display_dispatch(menu.display) < 0) {
            break;
        }
    }
    if (!menu.configured) {
        LOG_WARN("Built-in Wayland menu: compositor never configured it");
        goto done;
    }

    /* From here the menu was on screen, so any failure counts as a cancel:
     * falling back to a second menu would confuse the user. */
    status = POWER_STATUS_OK;
    while (menu.result == MENU_RESULT_PENDING) {
        if (wl_display_dispatch(menu.display) < 0) {
            menu.result = MENU_RESULT_CANCELLED;
        }
    }
    if (menu.result == MENU_RESULT_CHOSEN) {
        strncpy(selection, options[menu.selected], selection_size - 1);
        selection[selection_size - 1] = '\0';
    }

done:
    cleanup(&menu);
    return status;
}

/* Private function definitions */

/* A bad colour in power.conf falls back instead of failing, so a typo never
 * costs the user their power menu. */
static void resolve_color(const char *configured, const char *fallback,
                          const char *name, menu_rgb_t *out) {
    if (configured != NULL && configured[0] != '\0') {
        if (menu_style_parse_color(configured, out) == 0) {
            return;
        }
        LOG_WARN("Built-in Wayland menu: invalid %s '%s' (use #rgb or #rrggbb)",
                 name, configured);
    }
    (void)menu_style_parse_color(fallback, out);
}

static int load_style(menu_t *menu, const PowerConfig *config) {
    const char *font_text = DEFAULT_FONT;
    if (config != NULL && config->menu_font != NULL) {
        font_text = config->menu_font;
    }
    char pango_text[FONT_TEXT_SIZE];
    if (menu_style_font_to_pango(font_text, pango_text, sizeof(pango_text)) !=
        0) {
        LOG_WARN("Built-in Wayland menu: invalid font '%s'; using default",
                 font_text);
        snprintf(pango_text, sizeof(pango_text), "%s", DEFAULT_FONT);
    }
    menu->font = pango_font_description_from_string(pango_text);
    if (menu->font == NULL) {
        return -1;
    }

    resolve_color(config != NULL ? config->menu_bg : NULL, FALLBACK_BG,
                  "menu_bg", &menu->bg);
    resolve_color(config != NULL ? config->menu_fg : NULL, FALLBACK_FG,
                  "menu_fg", &menu->fg);
    resolve_color(config != NULL ? config->menu_sel_bg : NULL, FALLBACK_SEL_BG,
                  "menu_sel_bg", &menu->sel_bg);
    resolve_color(config != NULL ? config->menu_sel_fg : NULL, FALLBACK_SEL_FG,
                  "menu_sel_fg", &menu->sel_fg);
    /* The border follows the highlight colour unless set explicitly. */
    resolve_color(config != NULL ? config->menu_border : NULL, FALLBACK_SEL_BG,
                  "menu_border", &menu->border);
    if (config == NULL || config->menu_border == NULL ||
        config->menu_border[0] == '\0') {
        menu->border = menu->sel_bg;
    }
    return 0;
}

/* Sizes are multiples of the font height so the menu scales with the font
 * size instead of using fixed pixel values, as the X11 menu does. */
static int measure_geometry(menu_t *menu) {
    cairo_surface_t *scratch =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t *cr = cairo_create(scratch);
    PangoLayout *layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description(layout, menu->font);

    int width = 0;
    int height = 0;
    pango_layout_set_text(layout, "Ag", -1);
    pango_layout_get_pixel_size(layout, &width, &height);
    menu->font_height = height;

    int widest = 0;
    for (int i = 0; i < menu->count; i++) {
        pango_layout_set_text(layout, menu->options[i], -1);
        pango_layout_get_pixel_size(layout, &width, &height);
        if (width > widest) {
            widest = width;
        }
    }
    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(scratch);
    if (menu->font_height <= 0) {
        return -1;
    }

    menu->pad = menu->font_height / 2;
    menu->row_height = menu->font_height * 2;
    menu->menu_width = widest + menu->font_height * 2;
    if (menu->menu_width < menu->font_height * 12) {
        menu->menu_width = menu->font_height * 12;
    }
    menu->menu_height = menu->row_height * menu->count + menu->pad * 2;
    return 0;
}

/* The pixel memory lives in an anonymous memfd shared with the compositor;
 * Cairo draws straight into the mapping, so there is no copy per redraw. */
static int create_buffer(menu_t *menu, int width, int height) {
    destroy_buffer(menu);
    if (width <= 0 || height <= 0) {
        return -1;
    }
    int stride = width * BYTES_PER_PIXEL;
    size_t size = (size_t)stride * (size_t)height;

    int fd = memfd_create("power-menu", MFD_CLOEXEC);
    if (fd < 0) {
        return -1;
    }
    if (ftruncate(fd, (off_t)size) != 0) {
        close(fd);
        return -1;
    }
    void *pixels = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pixels == MAP_FAILED) {
        close(fd);
        return -1;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(menu->shm, fd, (int32_t)size);
    menu->buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride,
                                             WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    menu->pixels = pixels;
    menu->pixels_size = size;
    menu->canvas = cairo_image_surface_create_for_data(
        pixels, CAIRO_FORMAT_ARGB32, width, height, stride);
    return 0;
}

static void destroy_buffer(menu_t *menu) {
    if (menu->canvas != NULL) {
        cairo_surface_destroy(menu->canvas);
        menu->canvas = NULL;
    }
    if (menu->buffer != NULL) {
        wl_buffer_destroy(menu->buffer);
        menu->buffer = NULL;
    }
    if (menu->pixels != NULL) {
        munmap(menu->pixels, menu->pixels_size);
        menu->pixels = NULL;
        menu->pixels_size = 0;
    }
}

static void set_source(cairo_t *cr, const menu_rgb_t *color) {
    cairo_set_source_rgb(cr, color->red, color->green, color->blue);
}

/* Top-left of the menu's inner area (inside the border), centred on the
 * surface. Hit-testing and drawing share these so they cannot disagree. */
static int inner_x(const menu_t *menu) {
    return (menu->surface_width - menu->menu_width) / 2;
}

static int inner_y(const menu_t *menu) {
    return (menu->surface_height - menu->menu_height) / 2;
}

static void redraw(menu_t *menu) {
    if (!menu->configured || menu->canvas == NULL) {
        return;
    }
    cairo_t *cr = cairo_create(menu->canvas);

    /* The surface covers the whole output but only the menu is opaque. */
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    int x = inner_x(menu);
    int y = inner_y(menu);
    set_source(cr, &menu->border);
    cairo_rectangle(cr, x - BORDER_WIDTH, y - BORDER_WIDTH,
                    menu->menu_width + 2 * BORDER_WIDTH,
                    menu->menu_height + 2 * BORDER_WIDTH);
    cairo_fill(cr);
    set_source(cr, &menu->bg);
    cairo_rectangle(cr, x, y, menu->menu_width, menu->menu_height);
    cairo_fill(cr);

    PangoLayout *layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description(layout, menu->font);
    for (int i = 0; i < menu->count; i++) {
        int top = y + menu->pad + i * menu->row_height;
        const menu_rgb_t *text_color = &menu->fg;
        if (i == menu->selected) {
            set_source(cr, &menu->sel_bg);
            cairo_rectangle(cr, x, top, menu->menu_width, menu->row_height);
            cairo_fill(cr);
            text_color = &menu->sel_fg;
        }
        int text_width = 0;
        int text_height = 0;
        pango_layout_set_text(layout, menu->options[i], -1);
        pango_layout_get_pixel_size(layout, &text_width, &text_height);
        set_source(cr, text_color);
        cairo_move_to(cr, x + menu->font_height,
                      top + (menu->row_height - text_height) / 2);
        pango_cairo_show_layout(cr, layout);
    }
    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_flush(menu->canvas);

    wl_surface_attach(menu->surface, menu->buffer, 0, 0);
    wl_surface_damage(menu->surface, 0, 0, menu->surface_width,
                      menu->surface_height);
    wl_surface_commit(menu->surface);
}

/* Row under the pointer, or -1 when it is outside the menu's inner area. */
static int pointer_row(const menu_t *menu) {
    int local_x = menu->pointer_x - inner_x(menu);
    int local_y = menu->pointer_y - inner_y(menu);
    if (local_x < 0 || local_x >= menu->menu_width || local_y < 0 ||
        local_y >= menu->menu_height) {
        return -1;
    }
    return menu_nav_row_at(local_y, menu->pad, menu->row_height, menu->count);
}

static void step_selection(menu_t *menu, int delta) {
    menu->selected = menu_nav_step(menu->selected, menu->count, delta);
}

/* Digits only move the highlight and never confirm: a stray keypress must
 * not be able to power the machine off. Enter is the sole confirmation. */
static void handle_key(menu_t *menu, xkb_keysym_t sym) {
    int ctrl = xkb_state_mod_name_is_active(menu->xkb_state, XKB_MOD_NAME_CTRL,
                                            XKB_STATE_MODS_EFFECTIVE) > 0;
    switch (sym) {
    case XKB_KEY_Escape:
        menu->result = MENU_RESULT_CANCELLED;
        return;
    case XKB_KEY_Return:
    case XKB_KEY_KP_Enter:
        menu->result = MENU_RESULT_CHOSEN;
        return;
    case XKB_KEY_Up:
    case XKB_KEY_ISO_Left_Tab:
        step_selection(menu, -1);
        return;
    case XKB_KEY_Down:
    case XKB_KEY_Tab:
        step_selection(menu, 1);
        return;
    case XKB_KEY_Home:
        menu->selected = 0;
        return;
    case XKB_KEY_End:
        menu->selected = menu->count - 1;
        return;
    default:
        break;
    }

    if (ctrl) {
        if (sym == XKB_KEY_c) {
            menu->result = MENU_RESULT_CANCELLED;
        } else if (sym == XKB_KEY_p) {
            step_selection(menu, -1);
        } else if (sym == XKB_KEY_n) {
            step_selection(menu, 1);
        }
        return;
    }
    if (sym == XKB_KEY_k) {
        step_selection(menu, -1);
    } else if (sym == XKB_KEY_j) {
        step_selection(menu, 1);
    } else if (sym == XKB_KEY_q) {
        menu->result = MENU_RESULT_CANCELLED;
    } else if (sym >= XKB_KEY_1 && sym <= XKB_KEY_9) {
        int row = menu_nav_row_from_digit((char)sym, menu->count);
        if (row >= 0) {
            menu->selected = row;
        }
    }
}

/* Every resource may be missing, so cleanup can run from any failure point.
 * The layer-shell proxy is left to wl_display_disconnect: its destroy request
 * only exists from protocol version 3 and the menu binds version 1. */
static void cleanup(menu_t *menu) {
    destroy_buffer(menu);
    if (menu->layer_surface != NULL) {
        zwlr_layer_surface_v1_destroy(menu->layer_surface);
    }
    if (menu->surface != NULL) {
        wl_surface_destroy(menu->surface);
    }
    if (menu->pointer != NULL) {
        wl_pointer_destroy(menu->pointer);
    }
    if (menu->keyboard != NULL) {
        wl_keyboard_destroy(menu->keyboard);
    }
    if (menu->seat != NULL) {
        wl_seat_destroy(menu->seat);
    }
    if (menu->shm != NULL) {
        wl_shm_destroy(menu->shm);
    }
    if (menu->compositor != NULL) {
        wl_compositor_destroy(menu->compositor);
    }
    if (menu->registry != NULL) {
        wl_registry_destroy(menu->registry);
    }
    if (menu->display != NULL) {
        wl_display_flush(menu->display);
        wl_display_disconnect(menu->display);
    }
    if (menu->xkb_state != NULL) {
        xkb_state_unref(menu->xkb_state);
    }
    if (menu->xkb_keymap != NULL) {
        xkb_keymap_unref(menu->xkb_keymap);
    }
    if (menu->xkb_context != NULL) {
        xkb_context_unref(menu->xkb_context);
    }
    if (menu->font != NULL) {
        pango_font_description_free(menu->font);
    }
}

static void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *interface,
                            uint32_t version) {
    menu_t *menu = data;
    (void)version;
    /* Version 1 of each interface is all the menu needs, and binding the
     * lowest version keeps it working on the oldest compositors. */
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        menu->compositor =
            wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        menu->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0 &&
               menu->seat == NULL) {
        menu->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(menu->seat, &seat_listener, menu);
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        menu->layer_shell = wl_registry_bind(
            registry, name, &zwlr_layer_shell_v1_interface, 1);
    }
}

static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name) {
    (void)data;
    (void)registry;
    (void)name;
}

static void seat_capabilities(void *data, struct wl_seat *seat,
                              uint32_t capabilities) {
    menu_t *menu = data;
    if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && menu->keyboard == NULL) {
        menu->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(menu->keyboard, &keyboard_listener, menu);
    }
    /* The pointer is a convenience; the menu still works from the keyboard. */
    if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && menu->pointer == NULL) {
        menu->pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(menu->pointer, &pointer_listener, menu);
    }
}

static void keyboard_keymap(void *data, struct wl_keyboard *keyboard,
                            uint32_t format, int32_t fd, uint32_t size) {
    menu_t *menu = data;
    (void)keyboard;
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 || size == 0) {
        close(fd);
        return;
    }
    char *text = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (text == MAP_FAILED) {
        return;
    }
    struct xkb_keymap *keymap = xkb_keymap_new_from_string(
        menu->xkb_context, text, XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(text, size);
    if (keymap == NULL) {
        return;
    }
    struct xkb_state *state = xkb_state_new(keymap);
    if (state == NULL) {
        xkb_keymap_unref(keymap);
        return;
    }
    if (menu->xkb_state != NULL) {
        xkb_state_unref(menu->xkb_state);
    }
    if (menu->xkb_keymap != NULL) {
        xkb_keymap_unref(menu->xkb_keymap);
    }
    menu->xkb_keymap = keymap;
    menu->xkb_state = state;
}

static void keyboard_enter(void *data, struct wl_keyboard *keyboard,
                           uint32_t serial, struct wl_surface *surface,
                           struct wl_array *keys) {
    (void)data;
    (void)keyboard;
    (void)serial;
    (void)surface;
    (void)keys;
}

/* Exclusive focus is only taken away when the compositor has moved on (a
 * lock screen, another overlay); a menu that can no longer see keys must
 * not linger on screen. */
static void keyboard_leave(void *data, struct wl_keyboard *keyboard,
                           uint32_t serial, struct wl_surface *surface) {
    menu_t *menu = data;
    (void)keyboard;
    (void)serial;
    (void)surface;
    if (menu->result == MENU_RESULT_PENDING) {
        menu->result = MENU_RESULT_CANCELLED;
    }
}

static void keyboard_key(void *data, struct wl_keyboard *keyboard,
                         uint32_t serial, uint32_t time, uint32_t key,
                         uint32_t state) {
    menu_t *menu = data;
    (void)keyboard;
    (void)serial;
    (void)time;
    if (state != WL_KEYBOARD_KEY_STATE_PRESSED || menu->xkb_state == NULL ||
        menu->result != MENU_RESULT_PENDING) {
        return;
    }
    xkb_keysym_t sym =
        xkb_state_key_get_one_sym(menu->xkb_state, key + KEYCODE_OFFSET);
    handle_key(menu, sym);
    if (menu->result == MENU_RESULT_PENDING) {
        redraw(menu);
    }
}

static void keyboard_modifiers(void *data, struct wl_keyboard *keyboard,
                               uint32_t serial, uint32_t depressed,
                               uint32_t latched, uint32_t locked,
                               uint32_t group) {
    menu_t *menu = data;
    (void)keyboard;
    (void)serial;
    if (menu->xkb_state != NULL) {
        xkb_state_update_mask(menu->xkb_state, depressed, latched, locked, 0, 0,
                              group);
    }
}

static void pointer_enter(void *data, struct wl_pointer *pointer,
                          uint32_t serial, struct wl_surface *surface,
                          wl_fixed_t x, wl_fixed_t y) {
    menu_t *menu = data;
    (void)pointer;
    (void)serial;
    (void)surface;
    menu->pointer_x = wl_fixed_to_int(x);
    menu->pointer_y = wl_fixed_to_int(y);
}

static void pointer_leave(void *data, struct wl_pointer *pointer,
                          uint32_t serial, struct wl_surface *surface) {
    (void)data;
    (void)pointer;
    (void)serial;
    (void)surface;
}

static void pointer_motion(void *data, struct wl_pointer *pointer,
                           uint32_t time, wl_fixed_t x, wl_fixed_t y) {
    menu_t *menu = data;
    (void)pointer;
    (void)time;
    menu->pointer_x = wl_fixed_to_int(x);
    menu->pointer_y = wl_fixed_to_int(y);
    int row = pointer_row(menu);
    if (row >= 0 && row != menu->selected &&
        menu->result == MENU_RESULT_PENDING) {
        menu->selected = row;
        redraw(menu);
    }
}

static void pointer_button(void *data, struct wl_pointer *pointer,
                           uint32_t serial, uint32_t time, uint32_t button,
                           uint32_t state) {
    menu_t *menu = data;
    (void)pointer;
    (void)serial;
    (void)time;
    if (button != BUTTON_LEFT || state != WL_POINTER_BUTTON_STATE_PRESSED ||
        menu->result != MENU_RESULT_PENDING) {
        return;
    }

    int local_x = menu->pointer_x - inner_x(menu);
    int local_y = menu->pointer_y - inner_y(menu);
    int inside = local_x >= 0 && local_x < menu->menu_width && local_y >= 0 &&
                 local_y < menu->menu_height;
    if (!inside) {
        menu->result = MENU_RESULT_CANCELLED;
        return;
    }
    int row = pointer_row(menu);
    if (row >= 0) {
        menu->selected = row;
        menu->result = MENU_RESULT_CHOSEN;
    }
}

/* A wheel click reports SCROLL_NOTCH; touchpads report many small values, so
 * accumulate and step once per notch instead of once per event. */
static void pointer_axis(void *data, struct wl_pointer *pointer, uint32_t time,
                         uint32_t axis, wl_fixed_t value) {
    menu_t *menu = data;
    (void)pointer;
    (void)time;
    if (axis != WL_POINTER_AXIS_VERTICAL_SCROLL ||
        menu->result != MENU_RESULT_PENDING) {
        return;
    }
    menu->scroll += wl_fixed_to_double(value);
    if (menu->scroll >= SCROLL_NOTCH) {
        step_selection(menu, 1);
        menu->scroll = 0.0;
        redraw(menu);
    } else if (menu->scroll <= -SCROLL_NOTCH) {
        step_selection(menu, -1);
        menu->scroll = 0.0;
        redraw(menu);
    }
}

static void layer_configure(void *data, struct zwlr_layer_surface_v1 *surface,
                            uint32_t serial, uint32_t width, uint32_t height) {
    menu_t *menu = data;
    zwlr_layer_surface_v1_ack_configure(surface, serial);
    if (width == 0 || height == 0) {
        return;
    }
    if (menu->configured && (int)width == menu->surface_width &&
        (int)height == menu->surface_height) {
        redraw(menu);
        return;
    }
    if (create_buffer(menu, (int)width, (int)height) != 0) {
        LOG_WARN("Built-in Wayland menu: cannot allocate a %ux%u buffer", width,
                 height);
        menu->closed = 1;
        menu->result = MENU_RESULT_CANCELLED;
        return;
    }
    menu->surface_width = (int)width;
    menu->surface_height = (int)height;
    menu->configured = 1;
    redraw(menu);
}

static void layer_closed(void *data, struct zwlr_layer_surface_v1 *surface) {
    menu_t *menu = data;
    (void)surface;
    menu->closed = 1;
    if (menu->result == MENU_RESULT_PENDING) {
        menu->result = MENU_RESULT_CANCELLED;
    }
}
