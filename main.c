#include <curses.h>
#include <stdlib.h>
#define MAX_INPUT 100

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
    VAL_R,
    VAL_G,
    VAL_B,
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

void draw_colour_vals(WINDOW *win, int yMax, int pos, int R, int G, int B)
{
    mvwprintw(win, yMax / 3, 17, "R");
    mvwprintw(win, yMax / 3, 21, "G");
    mvwprintw(win, yMax / 3, 25, "B");

    mvwprintw(win, yMax / 3 + 1, 3, "Colour value: ");

    mvwprintw(win, yMax / 3 + 1, 17, "%d", R);

    if (pos == VAL_R)
    {
        wattron(win, A_REVERSE);
        mvwprintw(win, yMax / 3 + 1, 17, "%d", R);
        wattroff(win, A_REVERSE);
    }
    else
    {
        mvwprintw(win, yMax / 3 + 1, 17, "%d", R);
    }

    if (pos == VAL_G)
    {
        wattron(win, A_REVERSE);
        mvwprintw(win, yMax / 3 + 1, 21, "%d", G);
        wattroff(win, A_REVERSE);
    }
    else
    {
        mvwprintw(win, yMax / 3 + 1, 21, "%d", G);
    }

    if (pos == VAL_B)
    {
        wattron(win, A_REVERSE);
        mvwprintw(win, yMax / 3 + 1, 25, "%d", B);
        wattroff(win, A_REVERSE);
    }
    else
    {
        mvwprintw(win, yMax / 3 + 1, 25, "%d", B);
    }
}

int is_valid_number(char input_buffer[])
{
    // empty string
    if (input_buffer[0] == '\0')
    {
        return -1;
    }

    int pos = 0;
    while (input_buffer[pos] != '\0')
    {
        if (input_buffer[pos] < '0' || input_buffer[pos] > '9')
        {
            return -1; // invalid character
        }
        pos++;
    }

    int value = atoi(input_buffer);
    if (value < 0 || value > 255)
    {
        return -1;
    }

    return value;
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

    char input_buffer[MAX_INPUT] = {0};
    int input_pos = 0;
    bool input_mode = false;

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

        mvwprintw(win, 1, xMax / 2 - 2, "R");
        mvwprintw(win, 3, xMax / 2 - 2, "G");
        mvwprintw(win, 5, xMax / 2 - 2, "B");
        draw_colour_slider(win, 1, xMax / 2, slider_max, R, pos, RED);
        draw_colour_slider(win, 3, xMax / 2, slider_max, G, pos, GREEN);
        draw_colour_slider(win, 5, xMax / 2, slider_max, B, pos, BLUE);

        // Horizontal divider
        mvwhline(win, yMax / 2, 1, 0, xMax - 2);

        // Draw input field
        if (input_mode)
        {

            mvwprintw(win, yMax - 3, 3, "-- INPUT --");
            curs_set(1);
            echo();
            mvwprintw(win, yMax - 3, 15, "%-*s", MAX_INPUT, input_buffer);
            wmove(win, yMax - 3, 10 + input_pos);
        }
        else
        {

            mvwprintw(win, yMax - 3, 3, "-- NORMAL --");
            curs_set(0); // Hide cursor
            noecho();    // Don't show typed characters
        }

        draw_colour_vals(win, yMax, pos, R, G, B);

        wrefresh(win);
        wrefresh(preview_win);

        int ch = wgetch(win);

        if (input_mode)
        {
            if (ch == 27)
            { // ESC key
                input_mode = false;
            }
            else if (ch == KEY_BACKSPACE || ch == 127)
            {
                if (input_pos > 0)
                {
                    input_buffer[--input_pos] = '\0';
                }
            }
            else if (input_pos < MAX_INPUT - 1 && ch >= '0' && ch <= '9')
            {
                input_buffer[input_pos++] = ch;
                input_buffer[input_pos] = '\0';
            }
            else if ((pos == VAL_R || pos == VAL_G || pos == VAL_B) && ch == 10)
            { // Enter key
                int val = is_valid_number(input_buffer);
                if (val != -1)
                {
                    switch (pos)
                    {
                    case VAL_R:
                        R = val;
                        break;
                    case VAL_G:
                        G = val;
                        break;
                    case VAL_B:
                        B = val;
                        break;
                    case SLIDER_R:
                    case SLIDER_G:
                    case SLIDER_B:
                    default:
                        break;
                    }
                }
                input_mode = false;
                input_pos = 0;
                input_buffer[0] = '\0';
            }
        }
        else
        {
            if (ch == 'q')
                break;
            else if (ch == 'i')
                input_mode = true;
            else if (ch == 'a' || ch == 'h')
            {
                switch (pos)
                {
                case SLIDER_R:
                    if (R > 0)
                        R--;
                    break;
                case SLIDER_G:
                    if (G > 0)
                        G--;
                    break;
                case SLIDER_B:
                    if (B > 0)
                        B--;
                    break;
                case VAL_R:
                case VAL_G:
                case VAL_B:
                default:
                    break;
                }
            }
            else if (ch == 'd' || ch == 'l')
            {
                switch (pos)
                {
                case SLIDER_R:
                    if (R < 255)
                        R++;
                    break;
                case SLIDER_G:
                    if (G < 255)
                        G++;
                    break;
                case SLIDER_B:
                    if (B < 255)
                        B++;
                    break;
                case VAL_R:
                case VAL_G:
                case VAL_B:
                default:
                    break;
                }
            }
            else if ((ch == 'w' || ch == 'k') && pos > 0)
                pos--;
            else if ((ch == 's' || ch == 'j') && pos < 5)
                pos++;
        }
        wclear(win);
        wclear(preview_win);
        delwin(win);
        delwin(preview_win);
    }
    endwin();
    return 0;
}
