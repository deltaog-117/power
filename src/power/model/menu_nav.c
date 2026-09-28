#include "menu_nav.h"

int menu_nav_step(int selected, int count, int delta) {
    if (count <= 0) {
        return -1;
    }
    int next = (selected + delta) % count;
    if (next < 0) {
        next += count;
    }
    return next;
}

int menu_nav_row_from_digit(char key, int count) {
    if (key < '1' || key > '9') {
        return -1;
    }
    int row = key - '1';
    return row < count ? row : -1;
}

int menu_nav_row_at(int y, int top_pad, int row_height, int count) {
    if (row_height <= 0 || count <= 0) {
        return -1;
    }
    int offset = y - top_pad;
    if (offset < 0) {
        return -1;
    }
    int row = offset / row_height;
    return row < count ? row : -1;
}
