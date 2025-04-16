/*
 * tui.h - terminal user interface library
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2025-04-08
 *
 * This library depends on debug.h
 */

#ifndef TUI_H
#define TUI_H

#include <ncurses.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

/*
 * Definitions of keys
 */

#define KEY_CTRLC  3
#define KEY_CTRLZ 26
#define KEY_ESC   27
#define KEY_CTRLS 19
#define KEY_CTRLH  8
#define KEY_CTRLD  4
#define KEY_ENTR  10
#define KEY_TAB    9
#define KEY_RTAB 353
#define KEY_SPACE 32

#define TUI_PARENT_SIZE 0

/*
 * Declarations of tui structs
 */

typedef struct tui_t tui_t;

typedef struct tui_menu_t tui_menu_t;

typedef struct tui_window_t tui_window_t;

/*
 * Window event struct
 *
 * init   - after initialization
 * free   - before freeing
 * enter  - on enter
 * exit   - on exit
 * update - before resize
 * render - before render
 * key    - on keypress
 */
typedef struct tui_window_event_t
{
  bool (*key)    (tui_window_t* window, int key);
  void (*enter)  (tui_window_t* window);
  void (*exit)   (tui_window_t* window);
  void (*update) (tui_window_t* window);
  void (*render) (tui_window_t* window);
  void (*free)   (tui_window_t* window);
  void (*init)   (tui_window_t* window);
} tui_window_event_t;

/*
 * Menu event struct
 */
typedef struct tui_menu_event_t
{
  bool (*key)   (tui_menu_t* menu, int key);
  void (*enter) (tui_menu_t* menu);
  void (*exit)  (tui_menu_t* menu);
} tui_menu_event_t;

/*
 * Tui event struct
 */
typedef struct tui_event_t
{
  bool (*key) (tui_t* tui, int key);
} tui_event_t;

/*
 * Size struct: width and height
 */
typedef struct tui_size_t
{
  int  w;
  int  h;
} tui_size_t;

const tui_size_t TUI_SIZE_NONE = { 0 };

/*
 * Rect struct
 */
typedef struct tui_rect_t
{
  int  w;
  int  h;
  int  x;
  int  y;
  bool is_none; // Hidden flag to represent NONE rect
} tui_rect_t;

const tui_rect_t TUI_RECT_NONE = { .is_none = true };

/*
 * Foreground and background color struct
 */
typedef struct tui_color_t
{
  short fg;
  short bg;
} tui_color_t;

/*
 * Definitions of colors
 *
 * These colors differ from ncurses colors by 1
 */
enum {
  TUI_COLOR_NONE,
  TUI_COLOR_BLACK,
  TUI_COLOR_RED,
  TUI_COLOR_GREEN,
  TUI_COLOR_YELLOW,
  TUI_COLOR_BLUE,
  TUI_COLOR_MAGENTA,
  TUI_COLOR_CYAN,
  TUI_COLOR_WHITE
};

/*
 * Border struct
 */
typedef struct tui_border_t
{
  bool        is_active;
  tui_color_t color;
} tui_border_t;

/*
 * Declarations of window types
 */

typedef struct tui_window_parent_t tui_window_parent_t;

typedef struct tui_window_text_t   tui_window_text_t;

typedef struct tui_window_grid_t   tui_window_grid_t;

/*
 * Type of window
 */
typedef enum tui_window_type_t
{
  TUI_WINDOW_PARENT,
  TUI_WINDOW_TEXT,
  TUI_WINDOW_GRID,
} tui_window_type_t;

/*
 * Window struct
 */
typedef struct tui_window_t
{
  tui_window_type_t    type;
  char*                name;
  bool                 is_hidden;
  bool                 w_grow;
  bool                 h_grow;
  tui_rect_t           rect;
  tui_rect_t           _rect;  // Temp calculated rect
  WINDOW*              window;
  tui_color_t          color;
  tui_color_t          _color; // Temp inherited color
  tui_window_event_t   event;
  tui_window_parent_t* parent;
  tui_menu_t*          menu;
  tui_t*               tui;
  void*                data;   // User attached data
} tui_window_t;

/*
 * Input data struct, that can be attached to window
 */
typedef struct tui_input_t
{
  char*              buffer;
  size_t             buffer_size;
  size_t             buffer_len;
  size_t             cursor;
  size_t             scroll;
  tui_window_text_t* window;
  char*              string; // Visable string
  tui_t*             tui;
} tui_input_t;

/*
 * List data struct, that can be attached to window
 */
typedef struct tui_list_t
{
  tui_window_t** items;
  size_t         item_count;
  size_t         item_index;
  bool           is_vertical;
  tui_t*         tui;
} tui_list_t;

/*
 * Position
 */
typedef enum tui_pos_t
{
  TUI_POS_START,
  TUI_POS_CENTER,
  TUI_POS_END
} tui_pos_t;

/*
 * Alignment
 */
typedef enum tui_align_t
{
  TUI_ALIGN_START,
  TUI_ALIGN_CENTER,
  TUI_ALIGN_END,
  TUI_ALIGN_BETWEEN,
  TUI_ALIGN_AROUND,
  TUI_ALIGN_EVENLY
} tui_align_t;

/*
 * Grid window square struct
 */
typedef struct tui_window_grid_square_t
{
  tui_color_t color;
  char        symbol;
} tui_window_grid_square_t;

/*
 * Grid window struct
 */
typedef struct tui_window_grid_t
{
  tui_window_t              head;
  tui_size_t                size;
  tui_size_t                _size;
  tui_window_grid_square_t* grid;
} tui_window_grid_t;

/*
 * Text window struct
 */
typedef struct tui_window_text_t
{
  tui_window_t head;
  char*        string;
  size_t       string_size;
  char*        text;
  bool         is_secret;
  tui_pos_t    pos;
  tui_align_t  align;
} tui_window_text_t;

/*
 * Parent window struct
 */
typedef struct tui_window_parent_t
{
  tui_window_t   head;
  tui_window_t** children;
  size_t         child_count;
  bool           is_vertical;
  tui_border_t   border;
  bool           has_padding;
  tui_pos_t      pos;
  tui_align_t    align;
} tui_window_parent_t;

/*
 * Menu struct
 */
typedef struct tui_menu_t
{
  char*            name;
  tui_color_t      color;
  tui_color_t      _color;
  tui_window_t**   windows;
  size_t           window_count;
  tui_menu_event_t event;
  tui_t*           tui;
} tui_menu_t;

/*
 * Cursor struct
 */
typedef struct tui_cursor_t
{
  bool is_active;
  int  x;
  int  y;
} tui_cursor_t;

/*
 * Tui struct
 */
typedef struct tui_t
{
  tui_size_t     size;
  tui_menu_t**   menus;
  size_t         menu_count;
  tui_window_t** windows;
  size_t         window_count;
  tui_menu_t*    menu;
  tui_window_t*  window;
  tui_color_t    color;
  tui_cursor_t   cursor;
  tui_event_t    event;
  bool           is_running;
} tui_t;

#endif // TUI_H

#ifdef TUI_IMPLEMENT

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"

/*
 * Get ncurses color index from tui color
 */
static inline short tui_color_index_get(tui_color_t color)
{
  return color.fg * 9 + color.bg;
}

/*
 * Inherit color in case of transparency
 */
static inline tui_color_t tui_color_inherit(tui_t* tui, tui_window_t* window, tui_color_t color)
{
  // If color has no transparency, it don't need to inherit
  if (color.fg != TUI_COLOR_NONE && color.bg != TUI_COLOR_NONE)
  {
    return color;
  }

  // Get inherit color
  tui_color_t inherit_color = { 0 };

  if (window)
  {
    inherit_color = window->_color;
  }
  else if (tui->menu)
  {
    inherit_color = tui->menu->_color;
  }
  else
  {
    inherit_color = tui->color;
  }

  // Apply inherit color in case of transparency
  if (color.fg == TUI_COLOR_NONE)
  {
    color.fg = inherit_color.fg;
  }

  if (color.bg == TUI_COLOR_NONE)
  {
    color.bg = inherit_color.bg;
  }

  return color;
}

/*
 * Turn on color of window
 */
static inline void tui_window_color_on(tui_window_t* window, tui_color_t color)
{
  wattron(window->window, COLOR_PAIR(tui_color_index_get(color)));
}

/*
 * Turn off color of window
 */
static inline void tui_window_color_off(tui_window_t* window, tui_color_t color)
{
  wattroff(window->window, COLOR_PAIR(tui_color_index_get(color)));
}

/*
 * Fill window with color
 */
static inline void tui_window_fill(tui_window_t* window)
{
  window->_color = tui_color_inherit(window->tui, (tui_window_t*) window->parent, window->color);

  wbkgd(window->window, COLOR_PAIR(tui_color_index_get(window->_color)));
}

/*
 * Fill menu with color
 */
static inline void tui_menu_fill(tui_menu_t* menu)
{
  menu->_color = tui_color_inherit(menu->tui, NULL, menu->color);

  bkgd(COLOR_PAIR(tui_color_index_get(menu->color)));
}

/*
 * Fill tui with color
 */
static inline void tui_fill(tui_t* tui)
{
  bkgd(COLOR_PAIR(tui_color_index_get(tui->color)));
}

/*
 * Draw window border with it's foreground color
 */
void tui_border_draw(tui_window_parent_t* window)
{
  tui_border_t border = window->border;

  if (!border.is_active) return;


  tui_window_t head = window->head;

  tui_color_t color = tui_color_inherit(head.tui, (tui_window_t*) window, border.color);

  tui_window_color_on((tui_window_t*) window, color);

  box(head.window, 0, 0);

  tui_window_color_off((tui_window_t*) window, color);
}

/*
 * Initialize tui colors
 *
 * ncurses color and index differ by 1
 */
void tui_colors_init(void)
{
  for (short fg_index = 0; fg_index < 9; fg_index++)
  {
    for (short bg_index = 0; bg_index < 9; bg_index++)
    {
      size_t index = fg_index * 9 + bg_index;

      short fg = (fg_index - 1);
      short bg = (bg_index - 1);

      init_pair(index, fg, bg);
    }
  }
}

/*
 * Initialize tui (ncurses)
 */
int tui_init(void)
{
  initscr();
  noecho();
  raw();
  keypad(stdscr, TRUE);

  if (start_color() == ERR || !has_colors())
  {
    endwin();

    return 1;
  }

  use_default_colors();

  tui_colors_init();

  clear();
  refresh();

  return 0;
}

/*
 * Quit tui (ncurses)
 */
void tui_quit(void)
{
  clear();

  refresh();

  endwin();
}

/*
 * Create ncurses WINDOW* for tui_window_t
 */
static inline WINDOW* tui_ncurses_window_create(tui_rect_t rect)
{
  // Can't create window with no size
  if (rect.w == 0 || rect.h == 0)
  {
    return NULL;
  }

  WINDOW* window = newwin(rect.h, rect.w, rect.y, rect.x);

  if (!window)
  {
    return NULL;
  }

  keypad(window, TRUE);

  return window;
}

/*
 * Resize ncurses WINDOW*
 */
static inline WINDOW* tui_ncurses_window_resize(WINDOW* window, tui_rect_t rect)
{
  // Don't resize window to no size
  if (rect.w == 0 || rect.h == 0)
  {
    return window;
  }

  wresize(window, rect.h, rect.w);

  mvwin(window, rect.y, rect.x);

  return window;
}

/*
 * Update ncurses WINDOW*, either creating it or resizing it
 */
static inline WINDOW* tui_ncurses_window_update(WINDOW* window, tui_rect_t rect)
{
  if (window)
  {
    return tui_ncurses_window_resize(window, rect);
  }
  else
  {
    return tui_ncurses_window_create(rect);
  }
}

/*
 * Delete ncurses WINDOW*
 */
static inline void tui_ncurses_window_free(WINDOW** window)
{
  if (!window || !(*window)) return;

  wclear(*window);

  wrefresh(*window);

  delwin(*window);

  *window = NULL;
}

/*
 * Configuration struct for creating tui
 */
typedef struct tui_config_t
{
  tui_color_t color;
  tui_event_t event;
} tui_config_t;

/*
 * Create tui struct
 */
tui_t* tui_create(tui_config_t config)
{
  tui_t* tui = malloc(sizeof(tui_t));

  if (!tui)
  {
    return NULL;
  }

  memset(tui, 0, sizeof(tui_t));

  *tui = (tui_t)
  {
    .size.w = getmaxx(stdscr),
    .size.h = getmaxy(stdscr),
    .event  = config.event,
    .color  = config.color
  };

  return tui;
}

static inline void tui_windows_free(tui_window_t*** windows, size_t* count);

/*
 * Free parent window struct
 */
static inline void tui_window_parent_free(tui_window_parent_t** window)
{
  tui_windows_free(&(*window)->children, &(*window)->child_count);

  tui_ncurses_window_free(&(*window)->head.window);

  free(*window);

  *window = NULL;
}

/*
 * Free text window struct
 */
static inline void tui_window_text_free(tui_window_text_t** window)
{
  tui_ncurses_window_free(&(*window)->head.window);

  free((*window)->string);

  free((*window)->text);

  free(*window);

  *window = NULL;
}

/*
 * Free grid window struct
 */
static inline void tui_window_grid_free(tui_window_grid_t** window)
{
  tui_ncurses_window_free(&(*window)->head.window);

  free((*window)->grid);

  free(*window);

  *window = NULL;
}

/*
 * Free window struct
 */
static inline void tui_window_free(tui_window_t** window)
{
  if (!window || !(*window)) return;

  if ((*window)->event.free)
  {
    (*window)->event.free(*window);
  }

  switch ((*window)->type)
  {
    case TUI_WINDOW_PARENT:
      tui_window_parent_free((tui_window_parent_t**) window);
      break;

    case TUI_WINDOW_TEXT:
      tui_window_text_free((tui_window_text_t**) window);
      break;

    case TUI_WINDOW_GRID:
      tui_window_grid_free((tui_window_grid_t**) window);
      break;

    default:
      break;
  }
}

/*
 * Free windows
 */
static inline void tui_windows_free(tui_window_t*** windows, size_t* count)
{
  if (!windows || !(*windows)) return;

  for (size_t index = 0; index < *count; index++)
  {
    tui_window_free(&(*windows)[index]);
  }

  free(*windows);

  *windows = NULL;
  *count = 0;
}

/*
 * Free menu struct
 */
static inline void tui_menu_free(tui_menu_t** menu)
{
  if (!menu || !(*menu)) return;

  tui_windows_free(&(*menu)->windows, &(*menu)->window_count);

  free(*menu);

  *menu = NULL;
}

/*
 * Delete (free) tui struct
 */
void tui_delete(tui_t** tui)
{
  if (!tui || !(*tui)) return;

  for (size_t index = 0; index < (*tui)->menu_count; index++)
  {
    tui_menu_free(&(*tui)->menus[index]);
  }

  free((*tui)->menus);

  tui_windows_free(&(*tui)->windows, &(*tui)->window_count);

  free(*tui);

  *tui = NULL;
}

/*
 * Trigger tui event
 *
 * Start by calling active window event handler
 *
 * If the event was handled, stop trigger events
 *
 * Otherwise, call parent event handler, and so on...
 */
bool tui_event(tui_t* tui, int key)
{
  tui_window_t* window = tui->window;

  while (window)
  {
    if (window->event.key && window->event.key(window, key))
    {
      return true;
    }

    window = (tui_window_t*) window->parent;
  }

  tui_menu_t* menu = tui->menu;

  if (menu && menu->event.key && menu->event.key(menu, key))
  {
    return true;
  }

  if (tui->event.key && tui->event.key(tui, key))
  {
    return true;
  }

  return false;
}

/*
 * Get the height of wrapped text given the width
 */
static inline int tui_text_h_get(char* text, int w)
{
  size_t length = strlen(text);

  if (length == 0 || w == 0)
  {
    return 0;
  }

  int h = 1;

  int x = 0;
  int space_index = 0;

  int last_space_index = space_index;

  for (size_t index = 0; index < length; index++)
  {
    char letter = text[index];

    if (letter == ' ')
    {
      space_index = index;
    }

    if (letter == '\n')
    {
      x = 0;

      h++;
    }
    else if (x >= w)
    {
      x = 0;

      h++;

      // Current word cannot be wrapped
      if (space_index == last_space_index)
      {
        return -1;
      }

      index = space_index;

      last_space_index = space_index;
    }
    else
    {
      x++;
    }
  }

  return h;
}

/*
 * Get the width of wrapped text given the height
 */
static inline int tui_text_w_get(char* text, int h)
{
  int left  = 1;
  int right = strlen(text);

  int min_w = right;

  // Try every value between left and right, inclusive left == right
  while (left <= right)
  {
    int mid = (left + right) / 2;

    int curr_h = tui_text_h_get(text, mid);

    // If width was too small to wrap, increase width
    if (curr_h == -1)
    {
      left = mid + 1;
    }
    // If height got to large, increase width
    else if (curr_h > h)
    {
      left = mid + 1;
    }
    else // If the height is smaller than max height, store current best width
    {
      min_w = mid;
      right = mid - 1;
    }
  }

  return min_w;
}

/*
 * Get widths of lines in text, regarding height
 */
static inline void tui_text_ws_get(int* ws, char* text, int h)
{
  int w = tui_text_w_get(text, h);

  size_t length = strlen(text);

  int y = 0;
  int x = 0;

  int space_index = 0;

  for (size_t index = 0; (index < length) && (y < h); index++)
  {
    char letter = text[index];

    if (letter == ' ')
    {
      space_index = index;
    }

    if (letter == ' ' && x == 0)
    {
      x = 0;
    }
    else if (letter == '\n')
    {
      ws[y++] = x;

      x = 0;
    }
    else if (x >= w)
    {
      // full line width - last partial word
      ws[y++] = x - (index - space_index);

      x = 0;

      index = space_index;
    }
    else
    {
      x++;
    }

    // Store the width of last line
    if (index + 1 == length)
    {
      ws[y] = x;
    }
  }
}

/*
 * Set cursor to x y
 */
static inline void tui_cursor_set(tui_t* tui, int x, int y)
{
  tui->cursor = (tui_cursor_t)
  {
    .x = x,
    .y = y,
    .is_active = true
  };
}

/*
 * Extract ANSI code from string and increment index
 */
static inline char* tui_string_ansi_extract(char* string, size_t length, size_t* index)
{
  char* ansi = malloc(sizeof(char) * (length - *index + 1));

  if (!ansi)
  {
    return NULL;
  }

  size_t ansi_len = 0;

  (*index) += 2;

  while (*index < length && string[*index] != 'm')
  {
    ansi[ansi_len++] = string[(*index)++];
  }

  ansi[ansi_len] = '\0';

  return ansi;
}

/*
 * Handle ANSI escape code
 *
 * static tui_color_t color retains it's value between calls
 */
static inline void tui_string_ansi_handle(tui_window_t* window, char* ansi, int x, int y, int x_shift, int y_shift, tui_color_t* color)
{
  int code = atoi(ansi);

  // Reset everything
  if (code == 0)
  {
    *color = window->_color;

    wattroff(window->window, A_ATTRIBUTES);

    tui_window_color_on((tui_window_t*) window, window->_color);
  }
  // Cursor on
  else if (code == 5)
  {
    // Only set the cursor if the text window is the active window
    if (window->tui->window == (tui_window_t*) window)
    {
      tui_cursor_set(window->tui, window->_rect.x + x + x_shift, window->_rect.y + y + y_shift);
    }
  }
  // Foreground color
  else if (code >= 30 && code <= 37)
  {
    color->fg = code - 30;

    tui_window_color_on((tui_window_t*) window, *color);
  }
  // Background color
  else if (code >= 40 && code <= 47)
  {
    color->bg = code - 40;

    tui_window_color_on((tui_window_t*) window, *color);
  }
}

/*
 * Render text in rect in window
 */
static inline void tui_text_render(tui_window_text_t* window)
{
  tui_window_t head = window->head;

  tui_rect_t rect = head._rect;

  int h = tui_text_h_get(window->text, rect.w);

  // If text can't be displayed, don't render
  if (h <= 0)
  {
    return;
  }

  int ws[h];

  tui_text_ws_get(ws, window->text, h);

  // Store temporary color of letters
  tui_color_t color = head._color;

  int x = 0;
  int y = 0;

  size_t length = strlen(window->string);

  int y_shift = MAX(0, (float) window->pos / 2.f * (rect.h - h));

  for (size_t index = 0; index < length; index++)
  {
    char letter = window->string[index];

    int w = ws[y];

    int x_shift = MAX(0, (float) window->align / 2.f * (rect.w - w));

    if (letter == '\033')
    {
      char* ansi = tui_string_ansi_extract(window->string, length, &index);

      if (ansi)
      {
        tui_string_ansi_handle((tui_window_t*) window, ansi, x, y, x_shift, y_shift, &color);

        free(ansi);
      }
    }
    else if (letter == ' ' && x == 0)
    {
      x = 0;
    }
    else if (x >= w)
    {
      x = 0;

      y++;
    }
    else
    {
      if (y + y_shift < rect.h && x + x_shift < rect.w)
      {
        if (window->is_secret)
        {
          letter = '*';
        }

        mvwaddch(head.window, y_shift + y, x_shift + x, letter);
      }

      x++;
    }
  }
}

/*
 * Extract just the text from string
 *
 * ANSI escape characters will be left out
 */
static inline char* tui_text_extract(char* string)
{
  if (!string)
  {
    return NULL;
  }

  size_t length = strlen(string);

  char* text = malloc(sizeof(char) * (length + 1));

  if (!text)
  {
    return NULL;
  }

  size_t text_len = 0;

  for (size_t index = 0; index < length; index++)
  {
    char letter = string[index];

    if (letter == '\033')
    {
      while (index < length && string[index] != 'm') index++;
    }
    else
    {
      text[text_len++] = letter;
    }
  }

  text[text_len] = '\0';

  return text;
}

/*
 * Render text window
 */
static inline void tui_window_text_render(tui_window_text_t* window)
{
  tui_window_t head = window->head;

  werase(head.window);

  tui_window_fill((tui_window_t*) window);

  // Draw text
  if (window->text)
  {
    tui_text_render(window);
  }

  wrefresh(head.window);
}

/*
 * Render grid window
 */
static inline void tui_window_grid_render(tui_window_grid_t* window)
{
  tui_window_t head = window->head;

  werase(head.window);

  tui_window_fill((tui_window_t*) window);

  // Draw grid
  if (window->grid)
  {
    int x_shift = MAX(0, (head._rect.w - window->_size.w) / 2.f);
    int y_shift = MAX(0, (head._rect.h - window->_size.h) / 2.f);

    for (int y = 0; y < window->_size.h; y++)
    {
      for (int x = 0; x < window->_size.w; x++)
      {
        int index = y * window->_size.w + x;

        tui_window_grid_square_t square = window->grid[index];

        char symbol = square.symbol ? square.symbol : ' ';

        tui_color_t color = tui_color_inherit(head.tui, (tui_window_t*) window, square.color);

        tui_window_color_on((tui_window_t*) window, color);

        mvwaddch(head.window, y_shift + y, x_shift + x, symbol);

        tui_window_color_off((tui_window_t*) window, color);
      }
    }
  }

  wrefresh(head.window);
}

static inline void tui_window_render(tui_window_t* window);

/*
 * Render parent window with all it's children
 */
static inline void tui_window_parent_render(tui_window_parent_t* window)
{
  tui_window_t head = window->head;

  werase(head.window);

  tui_window_fill((tui_window_t*) window);

  // Draw border
  tui_border_draw(window);

  wrefresh(head.window);

  // Render children
  for (size_t index = 0; index < window->child_count; index++)
  {
    tui_window_render(window->children[index]);
  }
}

/*
 * Render window
 */
static inline void tui_window_render(tui_window_t* window)
{
  if (window->event.render)
  {
    window->event.render(window);
  }

  switch (window->type)
  {
    case TUI_WINDOW_PARENT:
      tui_window_parent_render((tui_window_parent_t*) window);
      break;

    case TUI_WINDOW_TEXT:
      tui_window_text_render((tui_window_text_t*) window);
      break;

    case TUI_WINDOW_GRID:
      tui_window_grid_render((tui_window_grid_t*) window);
      break;

    default:
      break;
  }
}

/*
 * Render windows
 */
static inline void tui_windows_render(tui_window_t** windows, size_t count)
{
  for (size_t index = count; index-- > 0;)
  {
    tui_window_render(windows[index]);
  }
}

/*
 * Calculate preliminary size of text window, based on text
 *
 * Size is temporarily stored in _rect
 *
 * Also, extract text from string
 */
static inline void tui_window_text_size_calc(tui_window_text_t* window)
{
  // Text window contains at least the cursor
  window->head._rect = (tui_rect_t) { .w = 1, .h = 1 };

  free(window->text);

  window->text = tui_text_extract(window->string);

  if (!window->text)
  {
    window->head._rect = TUI_RECT_NONE;

    return;
  }

  if (!window->head.rect.is_none)
  {
    window->head._rect = (tui_rect_t)
    {
      .w = MAX(0, window->head.rect.w),
      .h = MAX(0, window->head.rect.h)
    };
  }
  else if (window->text && strlen(window->text) > 0)
  {
    int h = tui_text_h_get(window->text, window->head.tui->size.w);

    int w = tui_text_w_get(window->text, h);

    window->head._rect = (tui_rect_t) { .w = w, .h = h};
  }
}

/*
 * Calculate preliminary size of grid window, based on grid
 *
 * Size is temporarily stored in _rect
 */
static inline void tui_window_grid_size_calc(tui_window_grid_t* window)
{
  // Text window contains at least the cursor
  window->head._rect = (tui_rect_t) { .w = 1, .h = 1 };

  if (!window->grid)
  {
    window->head._rect = TUI_RECT_NONE;

    return;
  }

  if (!window->head.rect.is_none)
  {
    window->head._rect = (tui_rect_t)
    {
      .w = MAX(0, window->head.rect.w),
      .h = MAX(0, window->head.rect.h)
    };
  }
  else
  {
    window->head._rect = (tui_rect_t)
    {
      .w = window->size.w,
      .h = window->size.h,
    };
  }
}

static inline void tui_window_size_calc(tui_window_t* window);

/*
 * Calculate preliminary size of parent window, based on children
 *
 * Size is temporarily stored in _rect
 */
static inline void tui_window_parent_size_calc(tui_window_parent_t* parent)
{
  // Calculate size of every child window downwards
  for (size_t index = 0; index < parent->child_count; index++)
  {
    tui_window_t* child = parent->children[index];

    tui_window_size_calc(child);
  }

  parent->head._rect = (tui_rect_t) { 0 };

  if (!parent->head.rect.is_none)
  {
    parent->head._rect = (tui_rect_t)
    {
      .w = MAX(0, parent->head.rect.w),
      .h = MAX(0, parent->head.rect.h)
    };
  }
  else if (parent->children && parent->child_count)
  {
    tui_size_t align_size = (tui_size_t) { 0 };
    tui_size_t max_size   = (tui_size_t) { 0 };

    size_t align_count = 0;

    for (size_t index = 0; index < parent->child_count; index++)
    {
      tui_window_t* child = parent->children[index];

      max_size.w = MAX(max_size.w, child->_rect.w);
      max_size.h = MAX(max_size.h, child->_rect.h);

      if (!child->rect.is_none)
      {
        max_size.w = MAX(max_size.w, child->rect.x + child->rect.w);
        max_size.h = MAX(max_size.h, child->rect.y + child->rect.h);
      }
      else if (parent->is_vertical)
      {
        align_count++;

        align_size.h += child->_rect.h;

        align_size.w = MAX(align_size.w, child->_rect.w);
      }
      else
      {
        align_count++;

        align_size.w += child->_rect.w;

        align_size.h = MAX(align_size.h, child->_rect.h);
      }
    }

    if (parent->has_padding)
    {
      if (parent->is_vertical)
      {
        align_size.h += (align_count + 1);

        align_size.w += 4;
      }
      else
      {
        align_size.w += (align_count + 1) * 2;

        align_size.h += 2;
      }
    }

    if (parent->border.is_active)
    {
      align_size.w += 2;
      align_size.h += 2;
    }

    parent->head._rect = (tui_rect_t)
    {
      .h = MAX(max_size.h, align_size.h),
      .w = MAX(max_size.w, align_size.w)
    };
  }
}

/*
 * Calculate preliminary size of window, based on content
 *
 * Size is temporarily stored in _rect
 */
static inline void tui_window_size_calc(tui_window_t* window)
{
  switch (window->type)
  {
    case TUI_WINDOW_PARENT:
      tui_window_parent_size_calc((tui_window_parent_t*) window);
      break;

    case TUI_WINDOW_TEXT:
      tui_window_text_size_calc((tui_window_text_t*) window);
      break;

    case TUI_WINDOW_GRID:
      tui_window_grid_size_calc((tui_window_grid_t*) window);
      break;

    default:
      break;
  }
}

/*
 * Calculate the preliminary sizes of windows
 */
static inline void tui_windows_size_calc(tui_window_t** windows, size_t count)
{
  for (size_t index = 0; index < count; index++)
  {
    tui_window_size_calc(windows[index]);
  }
}

/*
 * Get starting y of aligned parent children
 */
static inline int tui_child_y_get(tui_window_parent_t* parent)
{
  int y = 0;

  if (parent->border.is_active)
  {
    y += 1;
  }

  if (parent->has_padding)
  {
    y += 1;
  }

  return y;
}

/*
 * Get starting x of aligned parent children
 */
static inline int tui_child_x_get(tui_window_parent_t* parent)
{
  int x = 0;

  if (parent->border.is_active)
  {
    x += 1;
  }

  if (parent->has_padding)
  {
    x += 2;
  }

  return x;
}

/*
 * Calculate rect of vertically aligned child
 *
 * align_count and align_index must be int, because they are used in integer arithmetic
 */
static inline void tui_child_vert_rect_calc(tui_rect_t* rect, tui_window_parent_t* parent, tui_window_t* child, tui_window_t* last_child, tui_size_t max_size, tui_size_t align_size, int align_count, int* align_index, int grow_count, int* grow_index)
{
  if (*align_index == 0)
  {
    rect->y = tui_child_y_get(parent);
  }

  rect->x = tui_child_x_get(parent);

  int h_space = (max_size.h - align_size.h);

  int h = child->_rect.h;

  int h_gap = 0;

  if (child->h_grow)
  {
    if (parent->has_padding)
    {
      // Add this gap after current child
      h_gap += 1;
      
      // recalculate h_space as the extra grown height between windows
      h_space = MAX(0, h_space - (align_count - 1) * 1); 
    }

    int gap = h_space / grow_count;

    h += gap;

    // Grow first windows a little bit more to fill out extra padding
    if (h_space - gap * grow_count > *grow_index)
    {
      h += 1;
    }

    (*grow_index)++;
  }
  // If other children is grown, but not this one
  else if (grow_count > 0)
  {
    if (parent->has_padding)
    {
      // Add this gap after current child
      h_gap += 1;
    }
  }
  else if (parent->align == TUI_ALIGN_BETWEEN)
  {
    // Add this gap after current child
    int gap = (float) h_space / (float) (align_count - 1);

    h_gap += gap;

    if (h_space - gap * (align_count - 1) >= *align_index)
    {
      h_gap += 1;
    }
  }
  else if (parent->align == TUI_ALIGN_AROUND)
  {
    // Add this gap before current child
    int gap = (float) h_space / (float) (align_count + 1);

    int rest = h_space - gap * (align_count + 1);

    if (*align_index == 0 && rest > 0)
    {
      rect->y += (float) rest / 2.f;
    }

    rect->y += gap;
  }
  else if (parent->align < 3) // START, CENTER, END
  {
    if (*align_index == 0) // First child to align
    {
      // Add this gap before current child
      if (parent->has_padding)
      {
        h_space = MAX(0, h_space - (align_count - 1) * 1); 
      }

      rect->y += (float) parent->align / 2.f * h_space;
    }
    else if (parent->has_padding)
    {
      // Add this gap after current child
      h_gap += 1;
    }
  }

  if (*align_index > 0)
  {
    rect->y += last_child->_rect.h + h_gap;
  }

  int w = child->w_grow ? max_size.w : child->_rect.w;

  rect->w = w;
  rect->h = h;

  rect->x += (float) parent->pos / 2.f * (max_size.w - w);

  (*align_index)++;
}

/*
 * Calculate rect of horizontally aligned child
 */
static inline void tui_child_horiz_rect_calc(tui_rect_t* rect, tui_window_parent_t* parent, tui_window_t* child, tui_window_t* last_child, tui_size_t max_size, tui_size_t align_size, int align_count, int* align_index, int grow_count, int* grow_index)
{
  if (*align_index == 0)
  {
    rect->x = tui_child_x_get(parent);
  }

  rect->y = tui_child_y_get(parent);

  int w_space = (max_size.w - align_size.w);

  int w = child->_rect.w;

  int w_gap = 0;

  if (child->w_grow)
  {
    if (parent->has_padding)
    {
      // Add this gap after current child
      w_gap += 2;

      w_space = MAX(0, w_space - (align_count - 1) * 2); 
    }

    int gap = w_space / grow_count;

    w += gap;

    // Grow first windows a little bit more to fill out extra padding
    if (w_space - gap * grow_count > *grow_index)
    {
      w += 1;
    }

    (*grow_index)++;
  }
  // If other children is grown, but not this one
  else if (grow_count > 0)
  {
    if (parent->has_padding)
    {
      // Add this gap after current child
      w_gap += 1;
    }
  }
  else if (parent->align == TUI_ALIGN_BETWEEN)
  {
    // Add this gap after current child
    int gap = (float) w_space / (float) (align_count - 1);

    w_gap += gap;

    // Add extra gap between first windows
    if (w_space - gap * (align_count - 1) >= *align_index)
    {
      w_gap += 1;
    }
  }
  else if (parent->align == TUI_ALIGN_AROUND)
  {
    int gap = (float) w_space / (float) (align_count + 1);

    int rest = w_space - gap * (align_count + 1);

    if (*align_index == 0 && rest > 0)
    {
      rect->x += (float) rest / 2.f;
    }

    rect->x += gap;
  }
  else if (parent->align < 3) // START, CENTER, END
  {
    if (*align_index == 0) // First child to align
    {
      // Add this gap before current child
      if (parent->has_padding)
      {
        // Remove all space between windows 
        // not including padding around the border
        w_space = MAX(0, w_space - (align_count - 1) * 2); 
      }

      rect->x += (float) parent->align / 2.f * w_space;
    }
    else if (parent->has_padding)
    {
      // Add this gap after current child
      w_gap += 2;
    }
  }

  if (*align_index > 0)
  {
    rect->x += last_child->_rect.w + w_gap;
  }

  int h = child->h_grow ? max_size.h : child->_rect.h;

  rect->w = w;
  rect->h = h;

  rect->y += (float) parent->pos / 2.f * (max_size.h - h);

  (*align_index)++;
}

/*
 * Calculate rect of aligned child
 */
static inline void tui_child_rect_calc(tui_rect_t* rect, tui_window_parent_t* parent, tui_window_t* child, tui_window_t* last_child, tui_size_t max_size, tui_size_t align_size, int align_count, int* align_index, int grow_count, int* grow_index)
{
  if (parent->is_vertical)
  {
    tui_child_vert_rect_calc(rect, parent, child, last_child, max_size, align_size, align_count, align_index, grow_count, grow_index);
  }
  else
  {
    tui_child_horiz_rect_calc(rect, parent, child, last_child, max_size, align_size, align_count, align_index, grow_count, grow_index);
  }
}

/*
 * Get rect for window from rect with potential negative values
 */
static inline tui_rect_t tui_window_rect_get(tui_rect_t rect, int parent_w, int parent_h)
{
  if (rect.h <= 0)
  {
    rect.h = MAX(0, parent_h + rect.h);
  }

  if (rect.w <= 0)
  {
    rect.w = MAX(0, parent_w + rect.w);
  }

  if (rect.x < 0)
  {
    rect.x = MAX(0, parent_w + rect.x);
  }

  if (rect.y < 0)
  {
    rect.y = MAX(0, parent_h + rect.y);
  }

  return rect;
}

/*
 * Calculate rect of parent's children
 *
 * Make use of the temporarily stored sizes in _rect
 */
static inline void tui_children_rect_calc(tui_window_parent_t* parent)
{
  tui_size_t align_size = (tui_size_t) { 0 };

  size_t align_count = 0;

  size_t grow_count = 0;

  for (size_t index = 0; index < parent->child_count; index++)
  {
    tui_window_t* child = parent->children[index];

    if (child->rect.is_none)
    {
      align_count++;

      if (parent->is_vertical)
      {
        align_size.h += child->_rect.h;

        align_size.w = MAX(align_size.w, child->_rect.w);

        if (child->h_grow)
        {
          grow_count++;
        }
      }
      else
      {
        align_size.w += child->_rect.w;

        align_size.h = MAX(align_size.h, child->_rect.h);

        if (child->w_grow)
        {
          grow_count++;
        }
      }
    }
  }

  tui_size_t max_size = { parent->head._rect.w, parent->head._rect.h };

  if (parent->has_padding)
  {
    max_size.w -= 4;
    max_size.h -= 2;
  }

  if (parent->border.is_active)
  {
    max_size.w -= 2;
    max_size.h -= 2;
  }

  // Limit size to parent's size
  align_size.w = MIN(align_size.w, max_size.w);

  align_size.h = MIN(align_size.h, max_size.h);

  // x and y is stored temporarily for children
  tui_rect_t rect = { 0 };

  tui_window_t* last_child = NULL;

  int align_index = 0;
  int grow_index  = 0;

  for (size_t index = 0; index < parent->child_count; index++)
  {
    tui_window_t* child = parent->children[index];

    if (child->rect.is_none)
    {
      tui_child_rect_calc(&rect, parent, child, last_child, max_size, align_size, align_count, &align_index, grow_count, &grow_index);

      child->_rect = rect;

      last_child = child;
    }
    else
    {
      tui_rect_t parent_rect = parent->head._rect;

      child->_rect = tui_window_rect_get(child->rect, parent_rect.w, parent_rect.h);
    }

    // Move child window into parent window
    child->_rect.x += parent->head._rect.x;

    child->_rect.y += parent->head._rect.y;

    child->window = tui_ncurses_window_update(child->window, child->_rect);

    if (child->type == TUI_WINDOW_PARENT)
    {
      tui_children_rect_calc((tui_window_parent_t*) child);
    }
  }
}

/*
 * Calculate rect of window
 */
static inline void tui_window_rect_calc(tui_window_t* window, int w, int h)
{
  if (!window->rect.is_none)
  {
    window->_rect = tui_window_rect_get(window->rect, w, h);
  }

  window->window = tui_ncurses_window_update(window->window, window->_rect);

  if(window->type == TUI_WINDOW_PARENT)
  {
    tui_children_rect_calc((tui_window_parent_t*) window);
  }
}

/*
 * Calculate rects of windows
 */
static inline void tui_windows_rect_calc(tui_window_t** windows, size_t count, int w, int h)
{
  for (size_t index = 0; index < count; index++)
  {
    tui_window_rect_calc(windows[index], w, h);
  }
}

/*
 * Calculate rect of every tui window
 *
 * tui base windows must have fixed rect (for now)
 */
static inline void tui_rect_calc(tui_t* tui)
{
  int w = tui->size.w;
  int h = tui->size.h;

  tui_windows_rect_calc(tui->windows, tui->window_count, w, h);

  tui_menu_t* menu = tui->menu;

  if (menu)
  {
    tui_windows_rect_calc(menu->windows, menu->window_count, w, h);
  }
}

/*
 * Calculate sizes of tui windows and menu windows
 */
static inline void tui_size_calc(tui_t* tui)
{
  tui_windows_size_calc(tui->windows, tui->window_count);

  tui_menu_t* menu = tui->menu;

  if (menu)
  {
    tui_windows_size_calc(menu->windows, menu->window_count);
  }
}

/*
 * Resize tui by recalculating every size and rect of windows
 */
static inline void tui_resize(tui_t* tui)
{
  tui->size.w = getmaxx(stdscr);
  tui->size.h = getmaxy(stdscr);

  tui_size_calc(tui);

  tui_rect_calc(tui);
}

/*
 * Update (change content) all windows and children in array
 */
static inline void tui_windows_update(tui_window_t** windows, size_t count)
{
  for (size_t index = 0; index < count; index++)
  {
    tui_window_t* window = windows[index];

    if (window && window->event.update)
    {
      window->event.update(window);
    }

    if (window && window->type == TUI_WINDOW_PARENT)
    {
      tui_window_parent_t* parent = (tui_window_parent_t*) window;

      tui_windows_update(parent->children, parent->child_count);
    }
  }
}

/*
 * Update (change content) all tui windows and children
 */
static inline void tui_update(tui_t* tui)
{
  tui_windows_update(tui->windows, tui->window_count);

  tui_menu_t* menu = tui->menu;

  if (menu)
  {
    tui_windows_update(menu->windows, menu->window_count);
  }
}

/*
 * Render tui - active menu and all windows
 */
void tui_render(tui_t* tui)
{
  tui->cursor.is_active = false;

  curs_set(0);

  tui_update(tui);

  tui_resize(tui);

  erase();

  tui_menu_t* menu = tui->menu;

  if (menu)
  {
    tui_menu_fill(menu);
  }
  else
  {
    tui_fill(tui);
  }

  refresh();
  
  // 3. Render tui windows
  tui_windows_render(tui->windows, tui->window_count);

  // 4. Render menu windows
  if (menu)
  {
    tui_windows_render(menu->windows, menu->window_count);
  }

  // Render the active window over all other windows
  if (tui->window)
  {
    tui_window_render(tui->window);
  }

  tui_cursor_t cursor = tui->cursor;

  if (cursor.is_active)
  {
    if (cursor.y >= 0 && cursor.y < tui->size.h &&
        cursor.x >= 0 && cursor.x < tui->size.w)
    {
      move(cursor.y, cursor.x);

      curs_set(1);
    }
  }
}

/*
 * Configuration struct for parent window
 */
typedef struct tui_window_parent_config_t
{
  char*              name;
  tui_window_event_t event;
  tui_rect_t         rect;
  bool               w_grow;
  bool               h_grow;
  tui_color_t        color;
  bool               is_hidden;
  tui_border_t       border;
  bool               has_padding;
  tui_pos_t          pos;
  tui_align_t        align;
  bool               is_vertical;
  void*              data;
} tui_window_parent_config_t;

/*
 * Just create tui_window_parent_t* object
 */
static inline tui_window_parent_t* _tui_window_parent_create(tui_t* tui, tui_window_parent_config_t config)
{
  tui_window_parent_t* window = malloc(sizeof(tui_window_parent_t));

  if (!window)
  {
    return NULL;
  }

  memset(window, 0, sizeof(tui_window_parent_t));

  tui_window_t head = (tui_window_t)
  {
    .type      = TUI_WINDOW_PARENT,
    .name      = config.name,
    .rect      = config.rect,
    .w_grow    = config.w_grow,
    .h_grow    = config.h_grow,
    .is_hidden = config.is_hidden,
    .color     = config.color,
    .event     = config.event,
    .data      = config.data,
    .tui       = tui
  };

  *window = (tui_window_parent_t)
  {
    .head        = head,
    .has_padding = config.has_padding,
    .border      = config.border,
    .pos         = config.pos,
    .align       = config.align,
    .is_vertical = config.is_vertical,
  };

  return window;
}

/*
 *
 */
void tui_window_text_string_set(tui_window_text_t* window, char* string)
{
  if (window && string)
  {
    size_t length = strlen(string);

    if (length >= window->string_size)
    {
      free(window->string);

      window->string = malloc(sizeof(char) * (length + 1));

      window->string_size = length + 1;
    }
    
    if (window->string)
    {
      strcpy(window->string, string);

      window->string[length] = '\0';
    }
  }
}

/*
 * Configuration struct for text window
 */
typedef struct tui_window_text_config_t
{
  char*              name;
  tui_window_event_t event;
  tui_rect_t         rect;
  bool               w_grow;
  bool               h_grow;
  tui_color_t        color;
  bool               is_hidden;
  char*              string;
  bool               is_secret;
  tui_pos_t          pos;
  tui_align_t        align;
  void*              data;
} tui_window_text_config_t;

/*
 * Just create tui_window_text_t* object
 */
static inline tui_window_text_t* _tui_window_text_create(tui_t* tui, tui_window_text_config_t config)
{
  tui_window_text_t* window = malloc(sizeof(tui_window_text_t));

  if (!window)
  {
    return NULL;
  }

  memset(window, 0, sizeof(tui_window_text_t));

  tui_window_t head = (tui_window_t)
  {
    .type      = TUI_WINDOW_TEXT,
    .name      = config.name,
    .rect      = config.rect,
    .w_grow    = config.w_grow,
    .h_grow    = config.h_grow,
    .is_hidden = config.is_hidden,
    .color     = config.color,
    .event     = config.event,
    .data      = config.data,
    .tui       = tui
  };

  *window = (tui_window_text_t)
  {
    .head      = head,
    .is_secret = config.is_secret,
    .pos       = config.pos,
    .align     = config.align
  };

  tui_window_text_string_set(window, config.string);

  return window;
}

/*
 * Configuration struct for grid window
 */
typedef struct tui_window_grid_config_t
{
  char*              name;
  tui_window_event_t event;
  tui_rect_t         rect;
  bool               w_grow;
  bool               h_grow;
  tui_color_t        color;
  bool               is_hidden;
  tui_size_t         size;
  void*              data;
} tui_window_grid_config_t;

/*
 * Resize grid and store size in _size
 */
int tui_window_grid_resize(tui_window_grid_t* window, tui_size_t size)
{
  if (size.w <= 0 || size.h <= 0)
  {
    return 1;
  }

  free(window->grid);

  int square_count = size.w * size.h;

  tui_window_grid_square_t* grid = malloc(sizeof(tui_window_grid_square_t) * square_count);

  if (!grid)
  {
    return 2;
  }

  memset(grid, 0, sizeof(tui_window_grid_square_t) * square_count);

  window->grid = grid;

  window->_size = size;

  return 0;
}

/*
 * Just create tui_window_grid_t* object
 */
static inline tui_window_grid_t* _tui_window_grid_create(tui_t* tui, tui_window_grid_config_t config)
{
  tui_window_grid_t* window = malloc(sizeof(tui_window_grid_t));

  if (!window)
  {
    return NULL;
  }

  memset(window, 0, sizeof(tui_window_grid_t));

  tui_window_t head = (tui_window_t)
  {
    .type      = TUI_WINDOW_GRID,
    .name      = config.name,
    .rect      = config.rect,
    .w_grow    = config.w_grow,
    .h_grow    = config.h_grow,
    .is_hidden = config.is_hidden,
    .color     = config.color,
    .event     = config.event,
    .data      = config.data,
    .tui       = tui
  };

  *window = (tui_window_grid_t)
  {
    .head = head,
    .size = config.size,
  };

  if (tui_window_grid_resize(window, config.size) != 0)
  {
    free(window);

    return NULL;
  }

  return window;
}

/*
 * Append window to array of windows
 */
static inline int tui_windows_window_append(tui_window_t*** windows, size_t* count, tui_window_t* window)
{
  tui_window_t** temp_windows = realloc(*windows, sizeof(tui_window_t*) * (*count + 1));

  if (!temp_windows)
  {
    info_print("tui_windows_window_append realloc: %s", strerror(errno));

    return 1;
  }

  *windows = temp_windows;

  (*windows)[*count] = window;

  (*count)++;

  return 0;
}

/*
 * Append window to tui windows
 */
static inline int tui_window_append(tui_t* tui, tui_window_t* window)
{
  return tui_windows_window_append(&tui->windows, &tui->window_count, window);
}

/*
 * Append window to menu windows, and store menu reference in window
 */
static inline int tui_menu_window_append(tui_menu_t* menu, tui_window_t* window)
{
  window->menu = menu;

  return tui_windows_window_append(&menu->windows, &menu->window_count, window);
}

/*
 * Append child to parent children, and store parent and menu reference in child
 */
static inline int tui_parent_child_append(tui_window_parent_t* parent, tui_window_t* child)
{
  child->parent = parent;
  child->menu   = parent->head.menu;

  return tui_windows_window_append(&parent->children, &parent->child_count, child);
}

/*
 * Create parent window and add it to tui
 */
tui_window_parent_t* tui_window_parent_create(tui_t* tui, tui_window_parent_config_t config)
{
  tui_window_parent_t* window = _tui_window_parent_create(tui, config);

  if (!window)
  {
    return NULL;
  }

  if (tui_window_append(tui, (tui_window_t*) window) != 0)
  {
    tui_window_parent_free(&window);

    return NULL;
  }

  if (window->head.event.init)
  {
    window->head.event.init((tui_window_t*) window);
  }

  return window;
}

/*
 * Create parent window and add it to menu
 */
tui_window_parent_t* tui_menu_window_parent_create(tui_menu_t* menu, tui_window_parent_config_t config)
{
  tui_window_parent_t* window = _tui_window_parent_create(menu->tui, config);

  if (!window)
  {
    return NULL;
  }

  if (tui_menu_window_append(menu, (tui_window_t*) window) != 0)
  {
    tui_window_parent_free(&window);

    return NULL;
  }

  if (window->head.event.init)
  {
    window->head.event.init((tui_window_t*) window);
  }

  return window;
}

/*
 * Create parent window and add it to window as child
 */
tui_window_parent_t* tui_parent_child_parent_create(tui_window_parent_t* parent, tui_window_parent_config_t config)
{
  tui_window_parent_t* child = _tui_window_parent_create(parent->head.tui, config);

  if (!child)
  {
    return NULL;
  }

  if (tui_parent_child_append(parent, (tui_window_t*) child) != 0)
  {
    tui_window_parent_free(&child);

    return NULL;
  }

  if (child->head.event.init)
  {
    child->head.event.init((tui_window_t*) child);
  }

  return child;
}

/*
 * Create text window and add it to tui
 */
tui_window_text_t* tui_window_text_create(tui_t* tui, tui_window_text_config_t config)
{
  tui_window_text_t* window = _tui_window_text_create(tui, config);

  if (!window)
  {
    return NULL;
  }

  if (tui_window_append(tui, (tui_window_t*) window) != 0)
  {
    tui_window_text_free(&window);

    return NULL;
  }

  if (window->head.event.init)
  {
    window->head.event.init((tui_window_t*) window);
  }

  return window;
}

/*
 * Create text window and add it to menu
 */
tui_window_text_t* tui_menu_window_text_create(tui_menu_t* menu, tui_window_text_config_t config)
{
  tui_window_text_t* window = _tui_window_text_create(menu->tui, config);

  if (!window)
  {
    return NULL;
  }

  if (tui_menu_window_append(menu, (tui_window_t*) window) != 0)
  {
    tui_window_text_free(&window);

    return NULL;
  }

  if (window->head.event.init)
  {
    window->head.event.init((tui_window_t*) window);
  }

  return window;
}

/*
 * Create text window and add it to window as child
 */
tui_window_text_t* tui_parent_child_text_create(tui_window_parent_t* parent, tui_window_text_config_t config)
{
  tui_window_text_t* child = _tui_window_text_create(parent->head.tui, config);

  if (!child)
  {
    return NULL;
  }

  if (tui_parent_child_append(parent, (tui_window_t*) child) != 0)
  {
    tui_window_text_free(&child);

    return NULL;
  }

  if (child->head.event.init)
  {
    child->head.event.init((tui_window_t*) child);
  }

  return child;
}

/*
 * Create grid window and add it to tui
 */
tui_window_grid_t* tui_window_grid_create(tui_t* tui, tui_window_grid_config_t config)
{
  tui_window_grid_t* window = _tui_window_grid_create(tui, config);

  if (!window)
  {
    return NULL;
  }

  if (tui_window_append(tui, (tui_window_t*) window) != 0)
  {
    tui_window_grid_free(&window);

    return NULL;
  }

  if (window->head.event.init)
  {
    window->head.event.init((tui_window_t*) window);
  }

  return window;
}

/*
 * Create grid window and add it to menu
 */
tui_window_grid_t* tui_menu_window_grid_create(tui_menu_t* menu, tui_window_grid_config_t config)
{
  tui_window_grid_t* window = _tui_window_grid_create(menu->tui, config);

  if (!window)
  {
    return NULL;
  }

  if (tui_menu_window_append(menu, (tui_window_t*) window) != 0)
  {
    tui_window_grid_free(&window);

    return NULL;
  }

  if (window->head.event.init)
  {
    window->head.event.init((tui_window_t*) window);
  }

  return window;
}

/*
 * Create grid window and add it to window as child
 */
tui_window_grid_t* tui_parent_child_grid_create(tui_window_parent_t* parent, tui_window_grid_config_t config)
{
  tui_window_grid_t* child = _tui_window_grid_create(parent->head.tui, config);

  if (!child)
  {
    return NULL;
  }

  if (tui_parent_child_append(parent, (tui_window_t*) child) != 0)
  {
    tui_window_grid_free(&child);

    return NULL;
  }

  if (child->head.event.init)
  {
    child->head.event.init((tui_window_t*) child);
  }

  return child;
}

/*
 * Set color and symbol of square in grid window
 */
void tui_window_grid_square_set(tui_window_grid_t* window, int x, int y, tui_window_grid_square_t square)
{
  if (x >= 0 && x < window->_size.w &&
      y >= 0 && y < window->_size.h)
  {
    int index = y * window->_size.w + x;

    window->grid[index] = square;
  }
}

/*
 * Get square at x y in grid window
 */
tui_window_grid_square_t* tui_window_grid_square_get(tui_window_grid_t* window, int x, int y)
{
  if (x >= 0 && x < window->_size.w &&
      y >= 0 && y < window->_size.h)
  {
    int index = y * window->_size.w + x;

    return &window->grid[index];
  }

  return NULL;
}

/*
 * Update visable string from input buffer
 *
 * The buffer can for example be partially visable
 */
static inline void tui_input_string_update(tui_input_t* input)
{
  size_t string_len = 0;

  for (size_t index = input->scroll; index < input->cursor; index++)
  {
    input->string[string_len++] = input->buffer[index];
  }

  strcpy(input->string + string_len, "\033[5m");

  string_len += 4;

  for (size_t index = input->cursor; index < input->buffer_len; index++)
  {
    input->string[string_len++] = input->buffer[index];
  }

  tui_window_t* window = input->tui->window;

  // If the cursor is at the end of the string,
  // and input text window is the active window,
  // add space for cursor
  if (input->cursor == input->buffer_len &&
      window && (tui_window_t*) input->window == window)
  {
    input->string[string_len++] = ' ';
  }

  input->string[string_len] = '\0';
}

/*
 * Create input struct
 */
tui_input_t* tui_input_create(tui_t* tui, size_t size, tui_window_text_t* window)
{
  tui_input_t* input = malloc(sizeof(tui_input_t));

  if (!input)
  {
    return NULL;
  }

  memset(input, 0, sizeof(tui_input_t));

  *input = (tui_input_t)
  {
    .buffer_size = size,
    .tui         = tui,
    .window      = window,
  };


  input->buffer = malloc(sizeof(char) * (size + 1));

  if (!input->buffer)
  {
    free(input);

    return NULL;
  }

  memset(input->buffer, '\0', sizeof(char) * (size + 1));


  input->string = malloc(sizeof(char) * (size + 6));

  if (!input->string)
  {
    free(input->buffer);

    free(input);

    return NULL;
  }

  tui_input_string_update(input);

  return input;
}

/*
 * Delete input struct
 */
void tui_input_delete(tui_input_t** input)
{
  if (!input || !(*input)) return;

  free((*input)->buffer);

  free((*input)->string);

  free(*input);

  *input = NULL;
}

/*
 * Add symbol to input buffer
 */
static inline bool tui_input_symbol_add(tui_input_t* input, int key)
{
  if (input->buffer_len >= input->buffer_size)
  {
    return false;
  }

  if (key < 32 || key > 126)
  {
    return false;
  }

  char symbol = key;

  // Shift characters forward to make room for new character
  for (size_t index = input->buffer_len + 1; index-- > (input->cursor + 1);)
  {
    input->buffer[index] = input->buffer[index - 1];
  }

  input->buffer[input->cursor] = symbol;

  input->buffer_len++;


  if (input->cursor < input->buffer_len)
  {
    input->cursor++;
  }

  // The cursor is at the end of the input window
  /*
  if((win->cursor - win->scroll) >= (win->head.w - 2))
  {
    win->scroll++; // Scroll one more character
  }
  */

  tui_input_string_update(input);

  return true;
}

/*
 * Delete symbol from input buffer
 */
static inline bool tui_input_symbol_del(tui_input_t* input)
{
  if (input->cursor <= 0 || input->buffer_len <= 0)
  {
    return false;
  }

  // Fill in the room left by the deleted character
  for (size_t index = input->cursor - 1; index < input->buffer_len; index++)
  {
    input->buffer[index] = input->buffer[index + 1];
  }

  input->buffer_len--;

  input->buffer[input->buffer_len] = '\0';

  input->cursor = MIN(input->cursor - 1, input->buffer_len);

  tui_input_string_update(input);

  return true;
}

/*
 * Scroll right in input buffer
 */
static inline bool tui_input_scroll_right(tui_input_t* input)
{
  // If input text window is not the active window, scrolling is disabled
  if (!input->window || input->tui->window != (tui_window_t*) input->window)
  {
    return false;
  }

  // The cursor can not be further than the text itself
  if (input->cursor >= input->buffer_len)
  {
    return false;
  }

  input->cursor++;

  tui_input_string_update(input);

  return true;
}

/*
 * Scroll left in input buffer
 */
static inline bool tui_input_scroll_left(tui_input_t* input)
{
  // If input text window is not the active window, scrolling is disabled
  if (!input->window || input->tui->window != (tui_window_t*) input->window)
  {
    return false;
  }

  // Can't scroll past the beginning
  if (input->cursor == 0)
  {
    return false;
  }

  input->cursor--;

  // If the cursor is to the left of the window,
  // scroll to the start of the cursor
  if (input->cursor < input->scroll)
  {
    input->scroll = input->cursor;
  }

  tui_input_string_update(input);

  return true;
}

/*
 * Handle keypress in input window
 */
bool tui_input_event(tui_input_t* input, int key)
{
  switch (key)
  {
    case KEY_RIGHT:
      return tui_input_scroll_right(input);

    case KEY_LEFT:
      return tui_input_scroll_left(input);

    case KEY_BACKSPACE:
      return tui_input_symbol_del(input);
    
    default:
      return tui_input_symbol_add(input, key);
  }

  return false;
}

/*
 * Add item to list
 */
int tui_list_item_add(tui_list_t* list, tui_window_t* item)
{
  tui_window_t** temp_items = realloc(list->items, sizeof(tui_window_t*) * (list->item_count + 1));

  if (!temp_items)
  {
    return 1;
  }

  list->items = temp_items;

  list->items[list->item_count++] = item;

  return 0;
}

/*
 * Create list struct
 */
tui_list_t* tui_list_create(tui_t* tui, bool is_vertical)
{
  tui_list_t* list = malloc(sizeof(tui_list_t));

  if (!list)
  {
    return NULL;
  }

  memset(list, 0, sizeof(tui_list_t));

  *list = (tui_list_t)
  {
    .is_vertical = is_vertical,
    .tui         = tui,
  };

  return list;
}

/*
 * Delete list struct
 */
void tui_list_delete(tui_list_t** list)
{
  if (!list || !(*list)) return;

  free((*list)->items);

  free(*list);

  *list = NULL;
}

/*
 * Scroll forward to next selected item
 */
static inline bool tui_list_scroll_forward(tui_list_t* list)
{
  if (list->item_index >= (list->item_count - 1))
  {
    return false;
  }

  list->item_index++;

  return true;
}

/*
 * Scroll backwards to next selected item
 */
static inline bool tui_list_scroll_backward(tui_list_t* list)
{
  if (list->item_index <= 0)
  {
    return false;
  }

  list->item_index--;

  return true;
}

/*
 * Handle list event
 */
bool tui_list_event(tui_list_t* list, int key)
{
  if (list->is_vertical)
  {
    switch (key)
    {
      case KEY_DOWN:
        return tui_list_scroll_forward(list);

      case KEY_UP:
        return tui_list_scroll_backward(list);

      default:
        break;
    }
  }
  else
  {
    switch (key)
    {
      case KEY_RIGHT:
        return tui_list_scroll_forward(list);

      case KEY_LEFT:
        return tui_list_scroll_backward(list);

      default:
        break;
    }
  }

  return false;
}

/*
 * Set window to active window
 *
 * Call enter event for new window and exit event for old window
 *
 * Set window's menu to active menu
 */
void tui_window_set(tui_t* tui, tui_window_t* window)
{
  if (tui->window == window) return;

  tui_window_t* last_window = tui->window;

  tui->window = window;

  if (last_window && last_window->event.exit)
  {
    last_window->event.exit(last_window);
  }

  if (window && window->event.enter)
  {
    window->event.enter(window);
  }

  if (window->menu)
  {
    tui->menu = window->menu;
  }
}

/*
 * Set menu to active menu
 *
 * Call enter event for new menu and exit event for old menu
 *
 * Set window to active window if needed
 */
void tui_menu_set(tui_t* tui, tui_menu_t* menu)
{
  if (tui->menu == menu) return;

  if (tui->menu && tui->menu->event.exit)
  {
    tui->menu->event.exit(tui->menu);
  }

  tui->menu = menu;

  // If the active window is from another menu,
  // choose a window in menu to set active
  if (!tui->window ||
      (tui->window && tui->window->menu && tui->window->menu != menu))
  {
    for (size_t index = 0; index < menu->window_count; index++)
    {
      tui_window_t* window = menu->windows[index];

      // Change this to check if window is visable and interactive
      if (window)
      {
        tui_window_set(tui, window);

        break;
      }
    }
  }

  if (menu && menu->event.enter)
  {
    menu->event.enter(menu);
  }
}

/*
 * Configuration struct for menu
 */
typedef struct tui_menu_config_t
{
  char*            name;
  tui_color_t      color;
  tui_menu_event_t event;
} tui_menu_config_t;

/*
 * Create menu and append it to tui
 */
tui_menu_t* tui_menu_create(tui_t* tui, tui_menu_config_t config)
{
  tui_menu_t* menu = malloc(sizeof(tui_menu_t));

  if (!menu)
  {
    return NULL;
  }

  *menu = (tui_menu_t)
  {
    .name  = config.name,
    .color = config.color,
    .event = config.event,
    .tui   = tui,
  };

  tui_menu_t** temp_menus = realloc(tui->menus, sizeof(tui_menu_t) * (tui->menu_count + 1));

  if (!temp_menus)
  {
    free(menu);

    return NULL;
  }

  tui->menus = temp_menus;

  tui->menus[tui->menu_count++] = menu;

  return menu;
}

/*
 * Stop tui
 */
void tui_stop(tui_t* tui)
{
  tui->is_running = false;
}

/*
 * Start tui - main loop
 */
void tui_start(tui_t* tui)
{
  tui->is_running = true;

  tui_render(tui);

  int key;

  while (tui->is_running && (key = wgetch(stdscr)))
  {
    // info_print("KEY: %d", key);

    if (key == KEY_CTRLC)
    {
      tui->is_running = false;

      break;
    }

    tui_event(tui, key);

    tui_render(tui);
  }
}

/*
 * Get index of window in array of windows
 */
static ssize_t tui_window_index_get(tui_window_t** windows, size_t count, tui_window_t* window)
{
  for (size_t index = 0; index < count; index++)
  {
    if (windows[index] == window)
    {
      return index;
    }
  }
  
  return -1;
}

/*
 * Search through array of windows and activate first interactable window
 */
static bool tui_windows_tab_forward(tui_t* tui, tui_window_t** windows, size_t count)
{
  for (size_t index = 0; index < count; index++)
  {
    tui_window_t* window = windows[index];

    if (!window->is_hidden) // && window->is_interactable
    {
      tui_window_set(tui, window);

      return true;
    }
  }
  
  return false;
}

/*
 * Tab window forwards
 */
bool tui_tab_forward(tui_t* tui)
{
  tui_window_t* window = tui->window;

  if (!window)
  {
    return false;
  }

  tui_window_parent_t* parent;

  if (window->type == TUI_WINDOW_PARENT)
  {
    parent = (tui_window_parent_t*) window;

    window = NULL;
  }
  else
  {
    parent = window->parent;
  }

  while (parent)
  {
    size_t child_index = tui_window_index_get(parent->children, parent->child_count, window);

    if (child_index == -1)
    {
      error_print("tui_window_index_get");

      return false;
    }

    child_index += 1;

    if (tui_windows_tab_forward(tui, parent->children + child_index, parent->child_count - child_index))
    {
      return true;
    }

    window = (tui_window_t*) parent;

    parent = parent->head.parent;
  }

  tui_menu_t* menu = tui->menu;

  if (menu)
  {
    size_t window_index = tui_window_index_get(menu->windows, menu->window_count, window);

    if (window_index == -1)
    {
      error_print("tui_window_index_get");

      return false;
    }

    window_index += 1;

    if (tui_windows_tab_forward(tui, menu->windows + window_index, menu->window_count - window_index))
    {
      return true;
    }
  }
  else 
  {
    size_t window_index = tui_window_index_get(tui->windows, tui->window_count, window);

    if (window_index == -1)
    {
      error_print("tui_window_index_get");

      return false;
    }

    window_index += 1;

    if (tui_windows_tab_forward(tui, tui->windows + window_index, tui->window_count - window_index))
    {
      return true;
    }
  }

  if (tui_windows_tab_forward(tui, tui->windows, tui->window_count))
  {
    return true;
  }

  if (tui_windows_tab_forward(tui, menu->windows, menu->window_count))
  {
    return true;
  }

  return false;
}

/*
 * Search through array of windows backwards and activate first interactable window
 */
static bool tui_windows_tab_backward(tui_t* tui, tui_window_t** windows, size_t count)
{
  for (size_t index = count; index-- > 0;)
  {
    tui_window_t* window = windows[index];

    if (!window->is_hidden) // && window->is_interactable
    {
      tui_window_set(tui, window);

      return true;
    }
  }
  
  return false;
}

/*
 * Tab window backwards
 */
bool tui_tab_backward(tui_t* tui)
{
  tui_window_t* window = tui->window;

  if (!window)
  {
    return false;
  }

  tui_window_parent_t* parent = window->parent;

  while (parent)
  {
    ssize_t child_index = tui_window_index_get(parent->children, parent->child_count, window);

    if (child_index == -1)
    {
      error_print("tui_window_index_get");

      return false;
    }

    if (tui_windows_tab_backward(tui, parent->children, child_index))
    {
      return true;
    }

    window = (tui_window_t*) parent;

    parent = parent->head.parent;
  }

  tui_menu_t* menu = tui->menu;

  if (menu)
  {
    ssize_t window_index = tui_window_index_get(menu->windows, menu->window_count, window);
    
    if (window_index == -1)
    {
      error_print("tui_window_index_get");

      return false;
    }

    if (tui_windows_tab_backward(tui, menu->windows, window_index))
    {
      return true;
    }
  }
  else 
  {
    ssize_t window_index = tui_window_index_get(tui->windows, tui->window_count, window);

    if (window_index == -1)
    {
      error_print("tui_window_index_get");

      return false;
    }

    if (tui_windows_tab_backward(tui, tui->windows, window_index))
    {
      return true;
    }
  }

  // Small problem: now it's not garanteing that last child window is activated
  // This means, when tabbing backwards, it is not looping correctly

  if (tui_windows_tab_backward(tui, tui->windows, tui->window_count))
  {
    return true;
  }

  if (tui_windows_tab_backward(tui, menu->windows, menu->window_count))
  {
    return true;
  }

  return false;
}

tui_window_t* tui_window_window_search(tui_window_t* window, char* search);

/*
 * Search for window in array of windows and children
 */
tui_window_t* tui_windows_window_search(tui_window_t** windows, size_t count, char* search)
{
  char* rest = strchr(search, ' ');

  size_t length = rest ? (rest - search) : strlen(search);

  for (size_t index = 0; index < count; index++)
  {
    tui_window_t* window = windows[index];

    if (window && window->name &&
        strncmp(window->name, search, length) == 0)
    {
      if (!rest) return window;

      return tui_window_window_search(window, rest + 1);
    }
  }

  return NULL;
}

/*
 * Search tui for window
 */
tui_window_t* tui_window_search(tui_t* tui, char* search)
{
  char* rest = strchr(search, ' ');

  size_t length = rest ? (rest - search) : strlen(search);

  if (strncmp(search, ".", length) == 0)
  {
    return NULL;
  }

  return tui_windows_window_search(tui->windows, tui->window_count, search);
}

/*
 * Search menu for window
 */
tui_window_t* tui_menu_window_search(tui_menu_t* menu, char* search)
{
  char* rest = strchr(search, ' ');

  size_t length = rest ? (rest - search) : strlen(search);

  if (strncmp(search, ".", length) == 0)
  {
    return NULL;
  }

  return tui_windows_window_search(menu->windows, menu->window_count, search);
}

/*
 * Search window from base window
 */
tui_window_t* tui_window_window_search(tui_window_t* window, char* search)
{
  // Base case, if not window or not search
  if (!window || !search || strlen(search) == 0)
  {
    return window;
  }

  char* rest = strchr(search, ' ');

  size_t length = rest ? (rest - search) : strlen(search);

  if (strncmp(search, ".", length) == 0)
  {
    if (!rest)
    {
      return (tui_window_t*) window->parent;
    }
    else if (window->parent)
    {
      return tui_window_window_search((tui_window_t*) window->parent, rest + 1);
    }
    else if (window->menu)
    {
      return tui_menu_window_search(window->menu, rest + 1);
    }

    return tui_window_search(window->tui, rest + 1);
  }
  else if (window->type == TUI_WINDOW_PARENT)
  {
    tui_window_parent_t* parent = (tui_window_parent_t*) window;

    return tui_windows_window_search(parent->children, parent->child_count, search);
  }
  
  return NULL;
}

/*
 * Search for text window from base window
 */
tui_window_text_t* tui_window_window_text_search(tui_window_t* base, char* search)
{
  tui_window_t* window = tui_window_window_search(base, search);

  if (window && window->type == TUI_WINDOW_TEXT)
  {
    return (tui_window_text_t*) window;
  }

  return NULL;
}

/*
 * Search for parent window from base window
 */
tui_window_parent_t* tui_window_window_parent_search(tui_window_t* base, char* search)
{
  tui_window_t* window = tui_window_window_search(base, search);

  if (window && window->type == TUI_WINDOW_PARENT)
  {
    return (tui_window_parent_t*) window;
  }

  return NULL;
}

/*
 * Search for grid window from base window
 */
tui_window_grid_t* tui_window_window_grid_search(tui_window_t* base, char* search)
{
  tui_window_t* window = tui_window_window_search(base, search);

  if (window && window->type == TUI_WINDOW_GRID)
  {
    return (tui_window_grid_t*) window;
  }

  return NULL;
}

#endif // TUI_IMPLEMENT
