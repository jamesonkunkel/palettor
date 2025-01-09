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

typedef enum
{
    MODE_NORMAL,
    MODE_INPUT
} EditorMode;

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

void handle_decrease(Position pos, int *R, int *G, int *B)
{
    switch (pos)
    {
    case SLIDER_R:
        if (*R > 0)
            (*R)--;
        break;
    case SLIDER_G:
        if (*G > 0)
            (*G)--;
        break;
    case SLIDER_B:
        if (*B > 0)
            (*B)--;
        break;
    case VAL_R:
    case VAL_G:
    case VAL_B:
    default:
        break;
    }
}

void handle_increase(Position pos, int *R, int *G, int *B)
{
    switch (pos)
    {
    case SLIDER_R:
        if (*R < 255)
            (*R)++;
        break;
    case SLIDER_G:
        if (*G < 255)
            (*G)++;
        break;
    case SLIDER_B:
        if (*B < 255)
            (*B)++;
        break;
    case VAL_R:
    case VAL_G:
    case VAL_B:
    default:
        break;
    }
}

void handle_normal_mode(int ch, Position *pos, int *R, int *G, int *B, EditorMode *editor_mode)
{
    switch (ch)
    {
    case 'q':
        exit(0);
    case 'i':
        *editor_mode = MODE_INPUT;
        break;
    case 'a':
    case 'h':
        handle_decrease(*pos, R, G, B);
        break;
    case 'd':
    case 'l':
        handle_increase(*pos, R, G, B);
        break;
    case 'w':
    case 'k':
        if (*pos > 0)
            (*pos)--;
        break;
    case 's':
    case 'j':
        if (*pos < 5)
            (*pos)++;
        break;
    }
}

void handle_input_mode(char ch, char input_buffer[], int *input_pos, EditorMode *editor_mode, Position *pos, int *R, int *G, int *B)
{
    if (ch == 27)
    { // ESC key
        *editor_mode = false;
    }
    else if (ch == 127)
    { // Backspace key
        if (*input_pos > 0)
        {
            input_buffer[--(*input_pos)] = '\0';
        }
    }
    else if (*input_pos < MAX_INPUT - 1 && ch >= '0' && ch <= '9')
    {
        input_buffer[(*input_pos)++] = ch;
        input_buffer[*(input_pos)] = '\0';
    }
    else if ((*pos == VAL_R || *pos == VAL_G || *pos == VAL_B) && ch == 10)
    { // Enter key
        int val = is_valid_number(input_buffer);
        if (val != -1)
        {
            switch (*pos)
            {
            case VAL_R:
                *R = val;
                break;
            case VAL_G:
                *G = val;
                break;
            case VAL_B:
                *B = val;
                break;
            case SLIDER_R:
            case SLIDER_G:
            case SLIDER_B:
            default:
                break;
            }
        }
        *editor_mode = MODE_NORMAL;
        *input_pos = 0;
        input_buffer[0] = '\0';
    }
}

// Update init_pal_boxes function
WINDOW **init_pal_boxes(int num_pal_boxes, int yMax, int xMax)
{
    WINDOW **pal_boxes = malloc(num_pal_boxes * sizeof(WINDOW *));
    if (pal_boxes == NULL)
        return NULL;

    int box_width = 5;
    int box_height = 3;
    int start_x = 3;
    int start_y = yMax * 3 / 4; // Position at bottom

    for (int i = 0; i < num_pal_boxes; i++)
    {
        pal_boxes[i] = newwin(box_height, box_width,
                              start_y,
                              start_x + (i * (box_width + 10)));
        if (pal_boxes[i] == NULL)
            return NULL;
    }

    return pal_boxes;
}

void free_pal_boxes(int num_pal_boxes, WINDOW **pal_boxes)
{
    if (pal_boxes == NULL)
    {
        return;
    }

    for (int i = 0; i < num_pal_boxes; i++)
    {
        delwin(pal_boxes[i]);
    }

    free(pal_boxes);
}

int main()
{
    Position pos = SLIDER_G;
    EditorMode editor_mode = MODE_NORMAL;
    int R = 10, G = 100, B = 50;
    char input_buffer[MAX_INPUT] = {0};
    int input_pos = 0;
    int num_pal_boxes = 8;

    // Initialize ncurses
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

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    int slider_max = xMax - 4;

    WINDOW *win = newwin(yMax, xMax, 0, 0);
    WINDOW *preview_win = newwin(yMax / 4, xMax / 4, 1, 3);
    WINDOW **pal_boxes = init_pal_boxes(num_pal_boxes, yMax, xMax);

    if (!win || !preview_win || !pal_boxes)
    {
        endwin();
        return 1;
    }

    while (1)
    {
        // Clear windows for redraw
        wclear(win);
        wclear(preview_win);
        for (int i = 0; i < num_pal_boxes; i++)
        {
            wclear(pal_boxes[i]);
        }

        // Update colors
        init_rgb_color(COLOR_RED, R, G, B);
        init_pair(1, COLOR_BLACK, COLOR_RED);

        // Draw main window content
        box(win, 0, 0);
        mvwprintw(win, 1, xMax / 2 - 2, "R");
        mvwprintw(win, 3, xMax / 2 - 2, "G");
        mvwprintw(win, 5, xMax / 2 - 2, "B");

        // Draw sliders
        draw_colour_slider(win, 1, xMax / 2, slider_max, R, pos, RED);
        draw_colour_slider(win, 3, xMax / 2, slider_max, G, pos, GREEN);
        draw_colour_slider(win, 5, xMax / 2, slider_max, B, pos, BLUE);

        // Draw preview window
        wbkgd(preview_win, COLOR_PAIR(1));
        box(preview_win, 0, 0);

        // Draw mode indicator
        if (editor_mode == MODE_INPUT)
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
            curs_set(0);
            noecho();
        }

        // Draw values and divider
        draw_colour_vals(win, yMax, pos, R, G, B);
        mvwhline(win, yMax / 2, 1, 0, xMax - 2);

        // Refresh main windows
        wnoutrefresh(win);
        wnoutrefresh(preview_win);

        // Update palette boxes
        for (int i = 0; i < num_pal_boxes; i++)
        {
            box(pal_boxes[i], 0, 0);
            wbkgd(pal_boxes[i], COLOR_PAIR(1));
            wnoutrefresh(pal_boxes[i]);
        }

        doupdate();

        // Handle input
        int ch = wgetch(win);
        if (editor_mode == MODE_INPUT)
        {
            handle_input_mode(ch, input_buffer, &input_pos, &editor_mode, &pos, &R, &G, &B);
        }
        else
        {
            handle_normal_mode(ch, &pos, &R, &G, &B, &editor_mode);
        }
    }

    // Cleanup
    free_pal_boxes(num_pal_boxes, pal_boxes);
    delwin(preview_win);
    delwin(win);
    endwin();
    return 0;
}
