#include <curses.h>

void init_rgb_color(short color_number, int r, int g, int b)
{
    // Convert RGB values from 0-255 range to 0-1000 range
    short rr = (short)(r * 1000 / 255);
    short gg = (short)(g * 1000 / 255);
    short bb = (short)(b * 1000 / 255);
    init_color(color_number, rr, gg, bb);
}

void draw_slider(WINDOW *win, int y, int x_start, int x_end, int position)
{
    mvwhline(win, y, x_start, 0, x_end - x_start + 1);
    mvwaddch(win, y, x_start + position, '|');
}

void draw_colour_slider(WINDOW *win, int y, int x_start, int x_end, int value)
{
    int dx = x_end - x_start;
    int computed_value = (int)((value / 255.0) * dx); // Perform floating-point division
    mvwhline(win, y, x_start, 0, x_end - x_start + 1);
    mvwaddch(win, y, x_start + computed_value, '|');
}

int main()
{

    int R = 153;
    int G = 102;
    int B = 255;

    int slider_position = 125;

    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    if (has_colors() == FALSE)
    {
        endwin();
        printf("Your terminal does not support color\n");
        return 1;
    }

    start_color();

    while (1)
    {
        init_rgb_color(COLOR_RED, R, G, B);
        init_pair(1, COLOR_BLACK, COLOR_RED);

        int yMax, xMax;
        getmaxyx(stdscr, yMax, xMax);

        int slider_max = xMax - 4;

        WINDOW *win = newwin(yMax, xMax, 0, 0);
        box(win, 0, 0);

        // Draw preview window of colour
        WINDOW *preview_win = newwin(yMax / 4, xMax / 4, 1, 3);

        // Fill the preview window with the custom color
        wbkgd(preview_win, COLOR_PAIR(1));
        wclear(preview_win);

        mvwprintw(win, yMax / 3 + 1, 3, "Colour value: R=%d, G=%d, B=%d", R, G, B);

        draw_colour_slider(win, 1, xMax / 2, slider_max, slider_position);

        // Horizontal divider
        mvwhline(win, yMax / 2, 1, 0, xMax - 2);

        wrefresh(win);
        wrefresh(preview_win);

        int ch = wgetch(win);
        if (ch == 'q')
            break; // Exit on 'q'
        if (ch == 'a' && slider_position > 0)
            slider_position--;
        if (ch == 'd' && slider_position < 255)
            slider_position++;
    }

    endwin();
    return 0;
}