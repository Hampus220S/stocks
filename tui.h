/*
 * tui.h - terminal user interface library
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2025-03-19
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

/*
 * Declarations of tui structs
 */

typedef struct tui_t tui_t;

typedef struct tui_menu_t tui_menu_t;

typedef struct tui_window_t tui_window_t;

/*
 * Definitions of event function signatures
 */

typedef bool (*tui_window_event_t)(tui_window_t* window, int key);

typedef bool (*tui_menu_event_t)(tui_menu_t* menu, int key);

typedef bool (*tui_event_t)(tui_t* tui, int key);

/*
 * Size = width and height
 */
typedef struct tui_size_t
{
  int  w;
  int  h;
} tui_size_t;

const tui_size_t TUI_SIZE_NONE = { 0 };

/*
 * Normal rect
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
 * Border
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

/*
 * Window struct
 */
typedef struct tui_window_t
{
  bool                 is_text;
  char*                name;
  bool                 is_visable;
  tui_rect_t           rect;
  tui_rect_t           _rect;  // Temp calculated rect
  WINDOW*              window;
  tui_color_t          color;
  tui_color_t          _color; // Temp inherited color
  tui_window_event_t   event;
  tui_window_parent_t* parent;
  tui_t*               tui;
  void*                data; // User attached data
} tui_window_t;

/*
 * Input data struct, that can be attached to window
 *
 * If text window is NULL:
 * - the input is not visable
 * - arrow keys don't work
 */
typedef struct tui_input_t
{
  char*              buffer;
  size_t             buffer_size;
  size_t             buffer_len;

  tui_window_text_t* window;
  char*              string; // Visable string
} tui_input_t;

/*
 * List data struct, that can be attached to window
 */
typedef struct tui_list_t
{
  tui_window_t* windows;
  size_t        window_count;
  size_t        window_index;
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
 * Text window struct
 */
typedef struct tui_window_text_t
{
  tui_window_t head;
  char*        string;
  char*        text;
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
  bool           is_inflated;
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
  tui_window_t**   windows;
  size_t           window_count;
  tui_menu_event_t event;
  tui_t*           tui;
} tui_menu_t;

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
 * Inherit color from parent in case of transparency
 */
static inline tui_color_t tui_window_color_inherit(tui_window_t* window, tui_color_t color)
{
  // If color has no transparency, it don't need to inherit
  if (color.fg != TUI_COLOR_NONE && color.bg != TUI_COLOR_NONE)
  {
    return color;
  }

  // Get color of parent
  tui_window_t* parent = (tui_window_t*) window->parent;

  tui_color_t parent_color;

  if (parent)
  {
    parent_color = parent->_color;
  }
  else
  {
    parent_color = window->tui->color;
  }

  // Inherit color from parent
  if (color.fg == TUI_COLOR_NONE)
  {
    color.fg = parent_color.fg;
  }

  if (color.bg == TUI_COLOR_NONE)
  {
    color.bg = parent_color.bg;
  }

  return color;
}

/*
 * Fill window with color
 */
static inline void tui_window_fill(tui_window_t* window)
{
  window->_color = tui_window_color_inherit(window, window->color);

  wbkgd(window->window, COLOR_PAIR(tui_color_index_get(window->_color)));
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

  tui_color_t color = tui_window_color_inherit((tui_window_t*) window, border.color);

  wattron(head.window, COLOR_PAIR(tui_color_index_get(color)));

  box(head.window, 0, 0);
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

  free((*window)->text);

  free(*window);

  *window = NULL;
}

/*
 * Free window struct
 */
static inline void tui_window_free(tui_window_t** window)
{
  if (!window || !(*window)) return;

  if ((*window)->is_text)
  {
    tui_window_text_free((tui_window_text_t**) window);
  }
  else
  {
    tui_window_parent_free((tui_window_parent_t**) window);
  }
}

/*
 * Free windows
 */
static inline void tui_windows_free(tui_window_t*** windows, size_t* count)
{
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
 * Delete tui struct
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
 * Trigger tui events
 */
void tui_event(tui_t* tui, int key)
{

}

/*
 * Get the height of wrapped text given the width
 */
static inline int tui_text_h_get(char* text, int w)
{
  size_t length = strlen(text);

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
        // info_print("Cannot be wrapped: w: %d\n", w);
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
 * Render text in rect in window
 *
 * xpos determines if the text is aligned to the left, centered or right
 */
static inline void tui_text_render(tui_window_text_t* window)
{
  tui_window_t head = window->head;

  tui_rect_t rect = head._rect;

  int h = tui_text_h_get(window->text, rect.w);

  int ws[h];

  tui_text_ws_get(ws, window->text, h);

  int x = 0;
  int y = 0;

  size_t length = strlen(window->string);

  int y_shift = MAX(0, (float) window->pos / 2.f * (rect.h - h));

  for (size_t index = 0; index < length; index++)
  {
    char letter = window->string[index];

    int w = ws[y];

    if (letter == '\033')
    {
      while (index < length && window->string[index] != 'm') index++;
    }
    if (letter == ' ' && x == 0)
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
      int x_shift = MAX(0, (float) window->align / 2.f * (rect.w - w));

      if (y + y_shift < rect.h && x + x_shift < rect.w)
      {
        // info_print("mvwaddch(%d, %d, %c)", y_shift + y, x_shift + x, letter);

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
  if (!string) return NULL;

  char* text = strdup(string);

  size_t length = strlen(string);

  memset(text, '\0', sizeof(char) * (length + 1));

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
  info_print("tui_window_render: %s x:%d y:%d w:%d h:%d", window->name,
      window->_rect.x,
      window->_rect.y,
      window->_rect.w,
      window->_rect.h
  );

  // Unable to render if _rect is not calculated
  if (window->_rect.is_none)
  {
    info_print("tui_window_render: _rect not calculated");
    return;
  }

  if (window->is_text)
  {
    tui_window_text_render((tui_window_text_t*) window);
  }
  else
  {
    tui_window_parent_render((tui_window_parent_t*) window);
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
  window->head._rect = (tui_rect_t) { 0 };

  free(window->text);

  window->text = tui_text_extract(window->string);

  if (!window->head.rect.is_none)
  {
    window->head._rect = (tui_rect_t)
    {
      .w = MAX(0, window->head.rect.w),
      .h = MAX(0, window->head.rect.h)
    };
  }
  else if (window->text)
  {
    int h = tui_text_h_get(window->text, window->head.tui->size.w);

    int w = tui_text_w_get(window->text, h);

    window->head._rect = (tui_rect_t) { .w = w, .h = h};
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
  if (window->is_text)
  {
    tui_window_text_size_calc((tui_window_text_t*) window);
  }
  else
  {
    tui_window_parent_size_calc((tui_window_parent_t*) window);
  }

  info_print("tui_window_size_calc %s w:%d h:%d", window->name, window->_rect.w, window->_rect.h);
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
 *
 */
static inline int tui_align_y_get(tui_window_parent_t* parent)
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
 *
 */
static inline int tui_align_x_get(tui_window_parent_t* parent)
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
 * Calculate rect of aligned child
 */
static inline void tui_align_rect_calc(tui_rect_t* rect, tui_window_parent_t* parent, tui_window_t* child, tui_window_t* last_child, tui_size_t max_size, tui_size_t align_size, size_t align_count, size_t align_index)
{
  if (parent->is_vertical)
  {
    if (align_index == 0)
    {
      rect->y = tui_align_y_get(parent);
    }

    rect->x = tui_align_x_get(parent);

    int h_space = (max_size.h - align_size.h);

    int h = child->_rect.h;

    int h_gap = 0;

    if (parent->is_inflated)
    {
      if (parent->has_padding)
      {
        // Add this gap after current child
        h_gap += 1;
        
        h_space = MAX(0, h_space - align_count + 1); 
      }

      int gap = h_space / align_count;

      h += gap;

      if (h_space - gap * align_count > align_index)
      {
        h += 1;
      }
    }
    else if (parent->align == TUI_ALIGN_BETWEEN)
    {
      // Add this gap after current child
      h_gap += h_space / (align_count - 1);
    }
    else if (parent->align == TUI_ALIGN_AROUND)
    {
      // Add this gap before current child
      int gap = (float) h_space / (float) (align_count + 1);

      int rest = h_space - gap * (align_count + 1);

      if (align_index == 0 && rest > 0)
      {
        rect->y += (float) rest / 2.f;
      }

      rect->y += gap;
    }
    else if (parent->align < 3) // START, CENTER, END
    {
      if (align_index == 0) // First child to align
      {
        // Add this gap before current child
        if (parent->has_padding)
        {
          h_space = MAX(0, h_space - align_count + 1); 
        }

        rect->y += (float) parent->align / 2.f * h_space;
      }
      else if (parent->has_padding)
      {
        // Add this gap after current child
        h_gap += 1;
      }
    }

    if (align_index > 0)
    {
      rect->y += last_child->_rect.h + h_gap;
    }

    int w = parent->is_inflated ? max_size.w : child->_rect.w;

    rect->w = w;
    rect->h = h;

    rect->x += (float) parent->pos / 2.f * (max_size.w - w);
  }
  else
  {
    if (align_index == 0)
    {
      rect->x = tui_align_x_get(parent);
    }

    rect->y = tui_align_y_get(parent);

    int w_space = (max_size.w - align_size.w);

    int w = child->_rect.w;

    int w_gap = 0;

    if (parent->is_inflated)
    {
      if (parent->has_padding)
      {
        // Add this gap after current child
        w_gap += 1;
        
        w_space = MAX(0, w_space - align_count + 1); 
      }

      int gap = w_space / align_count;

      w += gap;

      if (w_space - gap * align_count > align_index)
      {
        w += 1;
      }
    }
    else if (parent->align == TUI_ALIGN_BETWEEN)
    {
      // Add this gap after current child
      w_gap += w_space / (align_count - 1);
    }
    else if (parent->align == TUI_ALIGN_AROUND)
    {
      int gap = (float) w_space / (float) (align_count + 1);

      int rest = w_space - gap * (align_count + 1);

      if (align_index == 0 && rest > 0)
      {
        rect->x += (float) rest / 2.f;
      }

      rect->x += gap;
    }
    else if (parent->align < 3) // START, CENTER, END
    {
      if (align_index == 0) // First child to align
      {
        // Add this gap before current child
        if (parent->has_padding)
        {
          w_space = MAX(0, w_space - align_count + 1); 
        }

        rect->x += (float) parent->align / 2.f * w_space;
      }
      else if (parent->has_padding)
      {
        // Add this gap after current child
        w_gap += 1;
      }
    }

    if (align_index > 0)
    {
      rect->x += last_child->_rect.w + w_gap;
    }

    int h = parent->is_inflated ? max_size.h : child->_rect.h;

    rect->w = w;
    rect->h = h;

    rect->y += (float) parent->pos / 2.f * (max_size.h - h);
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
 * Calculate rect of parent children
 *
 * Make use of the temporarily stored sizes in _rect
 */
static inline void tui_children_rect_calc(tui_window_parent_t* parent)
{
  tui_size_t align_size = (tui_size_t) { 0 };

  size_t align_count = 0;

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
      }
      else
      {
        align_size.w += child->_rect.w;

        align_size.h = MAX(align_size.h, child->_rect.h);
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

  tui_rect_t rect = { 0 };

  tui_window_t* last_child = NULL;

  size_t align_index = 0;

  for (size_t index = 0; index < parent->child_count; index++)
  {
    tui_window_t* child = parent->children[index];

    if (child->rect.is_none)
    {
      tui_align_rect_calc(&rect, parent, child, last_child, max_size, align_size, align_count, align_index);

      child->_rect = rect;

      last_child = child;

      align_index++;
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

    if (!child->is_text)
    {
      tui_children_rect_calc((tui_window_parent_t*) child);
    }
  }
}

/*
 * Calculate rects of windows
 */
static inline void tui_windows_rect_calc(tui_window_t** windows, size_t count, int w, int h)
{
  for (size_t index = 0; index < count; index++)
  {
    tui_window_t* window = windows[index];

    if (window->rect.is_none)
    {
      window->_rect = window->_rect;
    }
    else
    {
      window->_rect = tui_window_rect_get(window->rect, w, h);
    }

    window->window = tui_ncurses_window_update(window->window, window->_rect);

    if(!window->is_text)
    {
      tui_children_rect_calc((tui_window_parent_t*) window);
    }
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
 * Render tui - active menu and all windows
 */
void tui_render(tui_t* tui)
{
  curs_set(0);

  // 1. Resize every window to account for content
  tui_resize(tui);

  // 2. Fill tui background
  erase();

  tui_fill(tui);

  refresh();
  
  // 3. Render tui windows
  tui_windows_render(tui->windows, tui->window_count);

  // 4. Render menu windows
  tui_menu_t* menu = tui->menu;

  if (menu)
  {
    tui_windows_render(menu->windows, menu->window_count);
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
  tui_color_t        color;
  bool               is_visable;
  tui_border_t       border;
  bool               has_padding;
  tui_pos_t          pos;
  tui_align_t        align;
  bool               is_vertical;
  bool               is_inflated;
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
    .is_text    = false,
    .name       = config.name,
    .rect       = config.rect,
    .is_visable = config.is_visable,
    .color      = config.color,
    .event      = config.event,
    .tui        = tui
  };

  *window = (tui_window_parent_t)
  {
    .head        = head,
    .has_padding = config.has_padding,
    .border      = config.border,
    .pos         = config.pos,
    .align       = config.align,
    .is_vertical = config.is_vertical,
    .is_inflated = config.is_inflated
  };

  return window;
}

/*
 * Configuration struct for text window
 */
typedef struct tui_window_text_config_t
{
  char*              name;
  tui_window_event_t event;
  tui_rect_t         rect;
  tui_color_t        color;
  bool               is_visable;
  char*              string;
  tui_pos_t          pos;
  tui_align_t        align;
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
    .is_text    = true,
    .name       = config.name,
    .rect       = config.rect,
    .is_visable = config.is_visable,
    .color      = config.color,
    .event      = config.event,
    .tui        = tui
  };

  *window = (tui_window_text_t)
  {
    .head   = head,
    .string = config.string,
    .pos    = config.pos,
    .align  = config.align
  };

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
 * Create parent window and add it to tui
 */
tui_window_parent_t* tui_window_parent_create(tui_t* tui, tui_window_parent_config_t config)
{
  tui_window_parent_t* window = _tui_window_parent_create(tui, config);

  if (!window)
  {
    return NULL;
  }

  if (tui_windows_window_append(&tui->windows, &tui->window_count, (tui_window_t*) window) != 0)
  {
    tui_window_parent_free(&window);

    return NULL;
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

  if (tui_windows_window_append(&menu->windows, &menu->window_count, (tui_window_t*) window) != 0)
  {
    tui_window_parent_free(&window);

    return NULL;
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

  child->head.parent = parent;

  if (tui_windows_window_append(&parent->children, &parent->child_count, (tui_window_t*) child) != 0)
  {
    tui_window_parent_free(&child);

    return NULL;
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

  if (tui_windows_window_append(&tui->windows, &tui->window_count, (tui_window_t*) window) != 0)
  {
    tui_window_text_free(&window);

    return NULL;
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

  if (tui_windows_window_append(&menu->windows, &menu->window_count, (tui_window_t*) window) != 0)
  {
    tui_window_text_free(&window);

    return NULL;
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

  child->head.parent = parent;

  if (tui_windows_window_append(&parent->children, &parent->child_count, (tui_window_t*) child) != 0)
  {
    tui_window_text_free(&child);

    return NULL;
  }

  return child;
}

#endif // TUI_IMPLEMENT
