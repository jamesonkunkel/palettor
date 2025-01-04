#include <curses.h>

void init_rgb_color(short color_number, int r, int g, int b)
{
    // Convert RGB values from 0-255 range to 0-1000 range
    short rr = (short)(r * 1000 / 255);
    short gg = (short)(g * 1000 / 255);
    short bb = (short)(b * 1000 / 255);
    init_color(color_number, rr, gg, bb);
}

int main()
{

    int R = 153;
    int G = 102;
    int B = 255;

    initscr();
    noecho();
    curs_set(0);

    if (has_colors() == FALSE)
    {
        endwin();
        printf("Your terminal does not support color\n");
        return 1;
    }

    start_color();

    init_rgb_color(COLOR_RED, R, G, B);
    init_pair(1, COLOR_BLACK, COLOR_RED);

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    WINDOW *win = newwin(yMax, xMax, 0, 0);
    box(win, 0, 0);

    // Create a smaller window inside the main window
    WINDOW *preview_win = newwin(yMax / 4, xMax / 4, 1, 3);

    // Fill the inner window with the custom color
    wbkgd(preview_win, COLOR_PAIR(1));
    wclear(preview_win);

    mvwprintw(win, yMax / 3 + 1, 3, "Colour value: R=%d, G=%d, B=%d", R, G, B);

    // Draw a horizontal line across the middle of the window
    mvwhline(win, yMax / 2, 1, 0, xMax - 2);

    // Refresh both windows to display the boxes
    wrefresh(win);
    wrefresh(preview_win);

    wgetch(win);
    endwin();
    return 0;
}