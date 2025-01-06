#include <curses.h>

typedef enum
{
    RED,
    GREEN,
    BLUE,
} Colour;

typedef enum
{
    SLIDER_R,
    SLIDER_G,
    SLIDER_B,
} Position;

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

void draw_colour_slider(WINDOW *win, int y, int x_start, int x_end, int value, int pos, Colour col)
{
    int dx = x_end - x_start;
    int computed_value = (int)((value / 255.0) * dx); // Perform floating-point division
    mvwhline(win, y, x_start, 0, x_end - x_start + 1);

    if (col == pos)
    {
        wattron(win, A_REVERSE);
        mvwaddch(win, y, x_start + computed_value, '|');
        wattroff(win, A_REVERSE);
    }
    else
    {
        mvwaddch(win, y, x_start + computed_value, '|');
    }
}

int main()
{
    Position pos = SLIDER_G;

    int R = 10;
    int G = 100;
    int B = 50;

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

        mvwprintw(win, 1, xMax / 2 - 2, "R");
        mvwprintw(win, 3, xMax / 2 - 2, "G");
        mvwprintw(win, 5, xMax / 2 - 2, "B");
        draw_colour_slider(win, 1, xMax / 2, slider_max, R, pos, RED);
        draw_colour_slider(win, 3, xMax / 2, slider_max, G, pos, GREEN);
        draw_colour_slider(win, 5, xMax / 2, slider_max, B, pos, BLUE);

        // Horizontal divider
        mvwhline(win, yMax / 2, 1, 0, xMax - 2);

        wrefresh(win);
        wrefresh(preview_win);

        int ch = wgetch(win);
        if (ch == 'q')
            break; // Exit on 'q'
        if (ch == 'a' | ch == 'h')
        {
            switch (pos)
            {
            case SLIDER_R:
                if (R > 0)
                {
                    R--;
                }
                break;
            case SLIDER_G:
                if (G > 0)
                {
                    G--;
                }
                break;
            case SLIDER_B:
                if (B > 0)
                {
                    B--;
                }
                break;
            }
        }
        if (ch == 'd' | ch == 'l')
        {
            switch (pos)
            {
            case SLIDER_R:
                if (R < 255)
                {
                    R++;
                }
                break;
            case SLIDER_G:
                if (G < 255)
                {
                    G++;
                }
                break;
            case SLIDER_B:
                if (B < 255)
                {
                    B++;
                }
                break;
            }
        }
        if ((ch == 'w' | ch == 'k') && pos > 0)
            pos--;
        if ((ch == 's' | ch == 'j') && pos < 2)
            pos++;
    }

    endwin();
    return 0;
}