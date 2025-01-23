#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <curses.h>
#include <stdlib.h>
#include <json-c/json.h>

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
    PAL_BOX_0,
    PAL_BOX_1,
    PAL_BOX_2,
    PAL_BOX_3,
    PAL_BOX_4,
    PAL_BOX_5,
    PAL_BOX_6,
    PAL_BOX_7
} Position;

typedef enum
{
    MODE_NORMAL,
    MODE_INPUT
} EditorMode;

typedef struct
{
    WINDOW *win;
    int R, G, B;
} Pal_Box;

void init_rgb_color(short color_number, int r, int g, int b)
{
    // Convert RGB values from 0-255 range to 0-1000 range
    short rr = (short)(r * 1000 / 255);
    short gg = (short)(g * 1000 / 255);
    short bb = (short)(b * 1000 / 255);
    init_color(color_number, rr, gg, bb);
}

void draw_title(WINDOW *win, int xMax)
{
    mvwhline(win, 4, 1, 0, xMax - 2);
    mvwprintw(win, 2, xMax / 2 - 4, "Palettor");
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

    if ((int)col == pos)
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
    case PAL_BOX_0:
    case PAL_BOX_1:
    case PAL_BOX_2:
    case PAL_BOX_3:
    case PAL_BOX_4:
    case PAL_BOX_5:
    case PAL_BOX_6:
    case PAL_BOX_7:
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
    case PAL_BOX_0:
    case PAL_BOX_1:
    case PAL_BOX_2:
    case PAL_BOX_3:
    case PAL_BOX_4:
    case PAL_BOX_5:
    case PAL_BOX_6:
    case PAL_BOX_7:
    default:
        break;
    }
}

// Save palette to JSON
// TODO make a status message that displays for some time, success writing, failure to write etc;
void save_palette(Pal_Box *pal_boxes, int num_boxes, const char *filename)
{
    struct json_object *root = json_object_new_array();

    for (int i = 0; i < num_boxes; i++)
    {
        struct json_object *box = json_object_new_object();
        json_object_object_add(box, "R", json_object_new_int(pal_boxes[i].R));
        json_object_object_add(box, "G", json_object_new_int(pal_boxes[i].G));
        json_object_object_add(box, "B", json_object_new_int(pal_boxes[i].B));
        json_object_array_add(root, box);
    }

    const char *json_string = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
    FILE *f = fopen(filename, "w");
    if (!f)
    {
        // these message disappear way too fast
        mvprintw(0, 0, "Error opening file: %s", strerror(errno));
        refresh();
        return;
    }

    if (fputs(json_string, f) == EOF)
    {
        mvprintw(0, 0, "Error writing to file");
        refresh();
    }
    else
    {
        mvprintw(0, 0, "Saved to %s", filename);
        refresh();
    }

    fclose(f);
    json_object_put(root);
}

void handle_normal_mode(int ch, Position *pos, int *R, int *G, int *B, EditorMode *editor_mode, int *running, int num_pal_boxes, Pal_Box *pal_boxes, char *file_name)
{
    // the last up to 8 positions should be for the palette, first three will be the sliders
    int max_pos_count = 3 + num_pal_boxes - 1;
    switch (ch)
    {
    case 'q':
        *running = 0;
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
        if ((int)*pos < max_pos_count)
            (*pos)++;
        break;
    case 'p':
        if (*pos >= PAL_BOX_0 && *pos <= PAL_BOX_7)
        {
            int box_index = *pos - PAL_BOX_0; // Convert position to array index
            pal_boxes[box_index].R = *R;
            pal_boxes[box_index].G = *G;
            pal_boxes[box_index].B = *B;
        }
        break;
    case 'y':
        if (*pos >= PAL_BOX_0 && *pos <= PAL_BOX_7)
        {
            int box_index = *pos - PAL_BOX_0; // Convert position to array index
            *R = pal_boxes[box_index].R;
            *G = pal_boxes[box_index].G;
            *B = pal_boxes[box_index].B;
        }
        break;
    case 'm':
        save_palette(pal_boxes, num_pal_boxes, file_name);
        break;
    }
}

void handle_input_buffer(Pal_Box *pal_boxes, int num_boxes, char input_buffer[])
{
    // case where we see save path/to/save/name.json
    char *save_prefix = "save ";
    int res = strncmp(save_prefix, input_buffer, 5);

    if (res == 0)
    {
        char *file_path = input_buffer + 5;
        save_palette(pal_boxes, num_boxes, file_path);
    }
}

void handle_input_mode(char ch, char input_buffer[], int *input_pos, EditorMode *editor_mode, Position *pos, int *R, int *G, int *B, Pal_Box *pal_boxes, int num_pal_boxes)
{
    if (ch == 27) // ESC key
    {
        *editor_mode = false;
        *input_pos = 0;
        input_buffer[0] = '\0';
    }
    else if (ch == 127) // Backspace key
    {
        if (*input_pos > 0)
        {
            input_buffer[--(*input_pos)] = '\0';
        }
    }
    else if (*input_pos < MAX_INPUT - 1 && ch != 10) // Ensure we don't add enters to the buffer
    {
        input_buffer[(*input_pos)++] = ch;
        input_buffer[*(input_pos)] = '\0';
    }
    else if (ch == 10) // Enter key
    {

        int val = is_valid_number(input_buffer);

        // Case where the user has inputed a number and we are on sliders
        if ((*pos == SLIDER_R || *pos == SLIDER_G || *pos == SLIDER_B) && val != -1)
        {
            switch (*pos)
            {
            case SLIDER_R:
                *R = val;
                break;
            case SLIDER_G:
                *G = val;
                break;
            case SLIDER_B:
                *B = val;
                break;
            default:
                break;
            }
        }
        else
        {
            handle_input_buffer(pal_boxes, num_pal_boxes, input_buffer);
        }

        // Alaways exit input mode at the end of a command
        *editor_mode = MODE_NORMAL;
        *input_pos = 0;
        input_buffer[0] = '\0';
    }
}

// Update init_pal_boxes function
Pal_Box *init_pal_boxes(int num_pal_boxes, int yMax, int xMax)
{
    Pal_Box *pal_boxes = malloc(num_pal_boxes * sizeof(Pal_Box));
    if (pal_boxes == NULL)
        return NULL;

    int box_height = 3;
    int box_width = 5;
    int start_y = yMax * 3 / 4;
    int margin_x = 5;
    int space_each = (xMax - 3) / num_pal_boxes;

    for (int i = 0; i < num_pal_boxes; i++)
    {
        WINDOW *new_win = newwin(box_height, box_width,
                                 start_y,
                                 i * space_each + margin_x);

        if (new_win == NULL)
        {
            return NULL;
        }

        int grey_value = (i * 255) / (num_pal_boxes - 1);
        Pal_Box new_box;
        new_box.win = new_win;
        new_box.R = grey_value;
        new_box.G = grey_value;
        new_box.B = grey_value;

        pal_boxes[i] = new_box;
    }

    return pal_boxes;
}

void free_pal_boxes(int num_pal_boxes, Pal_Box *pal_boxes)
{
    if (pal_boxes == NULL)
    {
        return;
    }

    for (int i = 0; i < num_pal_boxes; i++)
    {
        delwin(pal_boxes[i].win);
    }

    free(pal_boxes);
}

void draw_pal_boxes(WINDOW *win, int yMax, int xMax, int num_pal_boxes, Pal_Box *pal_boxes, Position pos)
{
    for (int i = 0; i < num_pal_boxes; i++)
    {
        // Allocate unique color index and pair for each palette box
        int color_index = 100 + i; // The import bit is here, you can't just reuse the same colour index (ex: COLOR_GREEN) because the paint will use the last assigned value duhhh
        int color_pair_index = i + 2;

        init_rgb_color(color_index, pal_boxes[i].R, pal_boxes[i].G, pal_boxes[i].B);
        init_pair(color_pair_index, COLOR_BLACK, color_index);

        // Highlight the selected palette box
        if (i + 3 == (int)pos)
        {
            wattron(win, A_REVERSE);
        }

        wbkgd(pal_boxes[i].win, COLOR_PAIR(color_pair_index));
        mvwprintw(win, yMax * 3 / 4 + 4, i * ((xMax - 3) / num_pal_boxes) + 5, "col %d", i);
        wattroff(win, A_REVERSE);
        wnoutrefresh(pal_boxes[i].win);
    }
}

char *get_absolute_path(void)
{
    // Get home directory
    const char *home = getenv("HOME");
    if (!home)
    {
        struct passwd *pw = getpwuid(getuid());
        if (pw)
            home = pw->pw_dir;
    }

    // Construct full path
    char *full_path = malloc(strlen(home) + strlen("/Documents/p.json") + 1);
    if (!full_path)
        return NULL;

    sprintf(full_path, "%s/Documents/p.json", home);
    return full_path;
}

int main()
{
    int running = 1;
    Position pos = SLIDER_G;
    EditorMode editor_mode = MODE_NORMAL;
    int R = 100, G = 100, B = 100;
    char input_buffer[MAX_INPUT] = {0};
    int input_pos = 0;
    int num_pal_boxes = 8;

    char *file_path = get_absolute_path();
    if (!file_path)
    {
        endwin();
        fprintf(stderr, "Could not create file path\n");
        return 1;
    }

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
    int slider_max = xMax - 12;

    WINDOW *win = newwin(yMax, xMax, 0, 0);
    WINDOW *preview_win = newwin(yMax / 6, xMax / 2, yMax * 1 / 2, xMax / 4);
    Pal_Box *pal_boxes = init_pal_boxes(num_pal_boxes, yMax, xMax);

    if (!win || !preview_win || !pal_boxes)
    {
        endwin();
        return 1;
    }

    while (running == 1)
    {
        // Clear windows for redraw
        werase(win);
        werase(preview_win);
        for (int i = 0; i < num_pal_boxes; i++)
        {
            // werase(pal_boxes[i].win);
        }

        // Update colors
        init_rgb_color(COLOR_RED, R, G, B);
        init_pair(1, COLOR_BLACK, COLOR_RED);

        // Draw main window content
        box(win, 0, 0);
        draw_title(win, xMax);
        mvwprintw(win, yMax / 6, 5, "R:");
        mvwprintw(win, yMax / 6 + 2, 5, "G:");
        mvwprintw(win, yMax / 6 + 4, 5, "B:");

        // Draw sliders
        draw_colour_slider(win, yMax / 6, 8, slider_max, R, pos, RED);
        draw_colour_slider(win, yMax / 6 + 2, 8, slider_max, G, pos, GREEN);
        draw_colour_slider(win, yMax / 6 + 4, 8, slider_max, B, pos, BLUE);

        // Draw values next to sliders
        mvwprintw(win, yMax / 6, slider_max + 5, "%d", R);
        mvwprintw(win, yMax / 6 + 2, slider_max + 5, "%d", G);
        mvwprintw(win, yMax / 6 + 4, slider_max + 5, "%d", B);

        // mvwprintw(win, 2, 2, "%d", pos);

        // Draw preview window
        wbkgd(preview_win, COLOR_PAIR(1));

        // Draw mode indicator
        if (editor_mode == MODE_INPUT)
        {
            mvwprintw(win, yMax - 3, 3, "-- INPUT --");
            mvwprintw(win, yMax - 3, 15, "%-*s", MAX_INPUT, input_buffer);
            wmove(win, yMax - 3, 10 + input_pos);
            echo();
        }
        else
        {
            mvwprintw(win, yMax - 3, 3, "-- NORMAL --");
            noecho();
        }

        // Refresh main windows
        wnoutrefresh(win);
        wnoutrefresh(preview_win);

        draw_pal_boxes(win, yMax, xMax, num_pal_boxes, pal_boxes, pos);
        doupdate();

        // Handle input
        int ch = wgetch(win);
        if (editor_mode == MODE_INPUT)
        {
            handle_input_mode(ch, input_buffer, &input_pos, &editor_mode, &pos, &R, &G, &B, pal_boxes, num_pal_boxes);
        }
        else
        {
            handle_normal_mode(ch, &pos, &R, &G, &B, &editor_mode, &running, num_pal_boxes, pal_boxes, file_path);
        }
    }

    // Cleanup
    delwin(win);
    delwin(preview_win);
    free_pal_boxes(num_pal_boxes, pal_boxes);
    free(file_path);
    endwin();
    return 0;
}
