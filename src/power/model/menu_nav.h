#ifndef POWER_MENU_NAV_H
#define POWER_MENU_NAV_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Move the highlighted row by @p delta, wrapping at both ends.
 *
 * @param selected Currently highlighted row (any int; -1 means none yet).
 * @param count Number of rows in the menu.
 * @param delta Rows to move (negative moves up).
 * @return New row in [0, count), or -1 if @p count is not positive.
 */
int menu_nav_step(int selected, int count, int delta);

/**
 * @brief Map a typed digit ('1'..'9') to a zero-based row.
 *
 * @param key Character typed by the user.
 * @param count Number of rows in the menu.
 * @return Row index, or -1 if @p key is not a digit naming an existing row.
 */
int menu_nav_row_from_digit(char key, int count);

/**
 * @brief Find which row a vertical pixel offset falls on.
 *
 * @param y Offset from the top of the window.
 * @param top_pad Padding above the first row, in pixels.
 * @param row_height Height of one row, in pixels.
 * @param count Number of rows in the menu.
 * @return Row index, or -1 if @p y is outside every row or the geometry
 *         is not positive.
 */
int menu_nav_row_at(int y, int top_pad, int row_height, int count);

#ifdef __cplusplus
}
#endif

#endif /* POWER_MENU_NAV_H */
