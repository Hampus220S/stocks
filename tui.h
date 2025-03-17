/*
 * tui.h - terminal user interface library
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2025-03-12
 *
 * This library depends on debug.h
 */

#ifndef TUI_H
#define TUI_H

#include <ncurses.h>
#include <stdbool.h>
#include <stdint.h>

#define TUI_YES true
#define TUI_NO  false

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

#define BETWEEN(x, min, max) (((x) >= (min)) && ((x) <= (max)))

#define KEY_CTRLC 3
#define KEY_CTRLZ 26
#define KEY_ESC   27
#define KEY_CTRLS 19
#define KEY_CTRLH 8
#define KEY_CTRLD 4
#define KEY_ENTR  10
#define KEY_TAB   9

/*
 * Normal rect struct with absolute values
 */
typedef struct tui_rect_abs_t
{
  int w;
  int h;
  int x;
  int y;
} tui_rect_abs_t;

/*
 * TUI position
 *
 * By default, window will be centered
 */
typedef enum tui_pos_t
{
  TUI_POS_CENTER = 0,
  TUI_POS_LEFT   = 1,
  TUI_POS_TOP    = 1,
  TUI_POS_RIGHT  = 2,
  TUI_POS_BOTTOM = 2
} tui_pos_t;

/*
 * Position factor converts tui_pos_t to
 *
 * LEFT   : 0
 * TOP    : 0
 * CENTER : 1
 * RIGHT  : 2
 * BOTTOM : 2
 */
const int TUI_POS_FACTOR[] = { 1, 0, 2 };

/*
 * TUI size type
 *
 * By default, window will fit it's content
 */
typedef enum tui_size_type_t
{
  TUI_SIZE_NONE, // Size not specified
  TUI_SIZE_REL,  // Relative size to parent
  TUI_SIZE_ABS,  // Absolute size in pixels
  TUI_SIZE_MAX   // Max available size
} tui_size_type_t;

/*
 * TUI size
 */
typedef struct tui_size_t
{
  tui_size_type_t type;
  union
  {
    float rel;
    int   abs;
  } value;
} tui_size_t;

/*
 * TUI rect
 */
typedef struct tui_rect_t
{
  tui_size_t w;
  tui_size_t h;

  tui_size_t l; // Margin to left
  tui_size_t r; // Margin to right
  tui_size_t t; // Margin on top
  tui_size_t b; // Margin on bottom

  tui_pos_t xpos; // Horizontal alignment
  tui_pos_t ypos; // Vertical   alignment
} tui_rect_t;

/*
 * TUI window title
 */
typedef struct tui_title_t
{
  char*     title;
  tui_pos_t pos;
  size_t    title_len;
} tui_title_t;

/*
 * Declarations of TUI structs
 */

typedef struct tui_t tui_t;

typedef struct tui_menu_t   tui_menu_t;

typedef struct tui_window_t tui_window_t;


typedef struct tui_window_list_t      tui_window_list_t;

typedef struct tui_window_list_item_t tui_window_list_item_t;

/*
 * Type of windows
 */
typedef enum tui_window_type_t
{
  TUI_WINDOW_PARENT,  // Window containing children
  TUI_WINDOW_CONFIRM, // Confirm button
  TUI_WINDOW_TEXT,    // Displaying text
  TUI_WINDOW_INPUT,   // Inputting  text
  TUI_WINDOW_LIST     // List of items
} tui_window_type_t;

/*
 * 
 */
typedef enum tui_parent_type_t
{
  TUI_PARENT_TUI,
  TUI_PARENT_MENU,
  TUI_PARENT_WINDOW
} tui_parent_type_t;

/*
 * Definitions of event function signatures
 */

typedef void (*tui_window_list_item_event_t)(tui_window_list_item_t* item, int key);

typedef void (*tui_window_event_t)(tui_window_t* window, int key);

typedef void (*tui_menu_event_t)(tui_menu_t* menu, int key);

typedef void (*tui_event_t)(tui_t* tui, int key);

/*
 * General window struct, same for all windows
 *
 * is_interact tells if the window can be interacted with
 */
typedef struct tui_window_t
{
  tui_window_type_t  type;
  char*              name;
  bool               is_visable;
  bool               is_interact;
  tui_rect_t         rect;
  tui_rect_abs_t     abs_rect;
  WINDOW*            window;
  tui_window_event_t event;
  tui_t*             tui;
  tui_parent_type_t  parent_type;
  union
  {
    tui_t*        tui;    // TUI_PARENT_TUI
    tui_menu_t*   menu;   // TUI_PARENT_MENU
    tui_window_t* window; // TUI_PARENT_WINDOW
  } parent;
} tui_window_t;

/*
 * Struct for item in list window
 */
typedef struct tui_window_list_item_t
{
  char*                        string; // Displaying string
  char*                        data;   // Carrying   data
  tui_window_list_item_event_t event;
  tui_window_list_t*           window; // Parent list window
  tui_t*                       tui;
} tui_window_list_item_t;

/*
 * Window struct for listing items
 */
typedef struct tui_window_list_t
{
  tui_window_t             head;
  tui_title_t              title;
  tui_window_list_item_t** items;
  size_t                   item_count;
  ssize_t                  item_count_max; // Max allowed items
  tui_window_list_item_t** show_items;
  size_t                   show_item_count;
  size_t                   show_item_index;
  char*                    show_filter;
  bool                     is_reverse; // List items in reverse
} tui_window_list_t;

/*
 * Window struct for inputting text
 */
typedef struct tui_window_input_t
{
  tui_window_t head;
  tui_title_t  title;
  char*        buffer;
  size_t       buffer_size;
  size_t       buffer_len;
  int          cursor;      // Index of cursor
  int          scroll;      // Side scroll
  bool         is_hidden;   // Input is hidden
  bool         is_secret;   // Ability to hide input
  tui_pos_t    xpos;        // Horizontal position
  tui_pos_t    ypos;        // Vertical   position
} tui_window_input_t;

/*
 * Window struct for confirming something
 */
typedef struct tui_window_confirm_t
{
  tui_window_t head;
  char*        prompt;       // Question
  size_t       prompt_len;
  char*        text_yes;     // Yes text
  size_t       text_yes_len;
  char*        text_no;      // No  text
  size_t       text_no_len;
  bool         answer;
} tui_window_confirm_t;

/*
 * Window struct for displaying text
 */
typedef struct tui_window_text_t
{
  tui_window_t head;
  char*        text;
  size_t       text_len;
  tui_pos_t    xpos;        // Horizontal position
  tui_pos_t    ypos;        // Vertical   position
  bool         is_interact; // Requires interaction
  char*        text_close;
  size_t       text_close_len;
} tui_window_text_t;

/*
 * Window struct containing child windows
 */
typedef struct tui_window_parent_t
{
  tui_window_t   head;
  tui_window_t** children;
  size_t         child_count;
} tui_window_parent_t;

/*
 * Menu struct containing windows
 *
 * The first window in array is the active one
 */
typedef struct tui_menu_t
{
  char*            name;
  tui_window_t**   windows;
  size_t           window_count;
  tui_t*           tui;
  tui_menu_event_t event;
} tui_menu_t;

/*
 * TUI struct containing menus
 *
 * Active menu and active window has shortcuts
 * The active window should be either TUI window or inside menu
 *
 * The first window in array is the active one
 */
typedef struct tui_t
{
  int            w;
  int            h;
  tui_menu_t**   menus;
  size_t         menu_count;
  tui_window_t** windows;
  size_t         window_count;
  tui_menu_t*    menu;         // Active menu
  tui_window_t*  window;       // Active window
  bool           is_running;   // TUI is running
  tui_event_t    event;
} tui_t;

/*
 * Declarations of TUI functions
 */

extern void   tui_init(void);

extern void   tui_quit(void);

extern tui_t* tui_create(int w, int h, tui_event_t event);

extern void   tui_delete(tui_t** tui);

#endif // TUI_H

/*
 * This header library file uses _IMPLEMENT guards
 *
 * If TUI_IMPLEMENT is defined, the definitions will be included
 */

#ifdef TUI_IMPLEMENT

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"

/*
 * Init tui (ncurses)
 */
void tui_init(void)
{
  initscr();
  noecho();
  raw();
  keypad(stdscr, TRUE);
}

/*
 * Quit tui (ncurses)
 */
void tui_quit(void)
{
  endwin();
}

/*
 * Append menu to TUI
 */
static inline int tui_menu_append(tui_t* tui, tui_menu_t* menu)
{
  tui_menu_t** temp_menus = realloc(tui->menus, sizeof(tui_menu_t*) * (tui->menu_count + 1));

  if (!temp_menus)
  {
    errno = ENOMEM; // Out of memory

    return 1;
  }

  tui->menus = temp_menus;

  tui->menus[tui->menu_count++] = menu;

  return 0;
}

/*
 * Append window to TUI
 */
static inline int tui_window_append(tui_t* tui, tui_window_t* window)
{
  tui_window_t** temp_windows = realloc(tui->windows, sizeof(tui_window_t*) * (tui->window_count + 1));

  if (!temp_windows)
  {
    errno = ENOMEM; // Out of memory

    return 1;
  }

  tui->windows = temp_windows;

  tui->windows[tui->window_count++] = window;

  return 0;
}

/*
 * Append window to menu
 */
static inline int tui_menu_window_append(tui_menu_t* menu, tui_window_t* window)
{
  tui_window_t** temp_windows = realloc(menu->windows, sizeof(tui_window_t*) * (menu->window_count + 1));

  if (!temp_windows)
  {
    errno = ENOMEM; // Out of memory

    return 1;
  }

  menu->windows = temp_windows;

  menu->windows[menu->window_count++] = window;

  return 0;
}

/*
 * Create ncurses WINDOW* for tui_window_t
 */
static inline WINDOW* tui_ncurses_window_create(int w, int h, int x, int y)
{
  WINDOW* window = newwin(h, w, y, x);

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
static inline void tui_ncurses_window_resize(WINDOW* window, int w, int h, int x, int y)
{
  wresize(window, h, w);

  mvwin(window, y, x);
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
 * Get the absolute pixel size from tui_size
 * in case of relative size, parent_size is used in relation
 */
static inline int tui_size_abs_get(int parent_size, tui_size_t tui_size)
{
  switch (tui_size.type)
  {
    case TUI_SIZE_REL:
      return parent_size * tui_size.value.rel;

    case TUI_SIZE_ABS:
      return tui_size.value.abs;

    case TUI_SIZE_MAX:
      return parent_size;

    default: case TUI_SIZE_NONE:
      return 0;
  }
}

/*
 * Get the absolute value of width, regarding left and right margins
 */
static inline int tui_rect_abs_w_get(int l, int r, int parent_w, tui_size_t w)
{
  int abs_w = tui_size_abs_get(parent_w, w);

  return MIN(parent_w - l - r, abs_w);
}

/*
 * Get the absolute value of height, regarding top and bottom margins
 */
static inline int tui_rect_abs_h_get(int t, int b, int parent_h, tui_size_t h)
{
  int abs_h = tui_size_abs_get(parent_h, h);

  return MIN(parent_h - t - b, abs_h);
}

/*
 * Get the height of wrapped text given the width
 */
static inline int tui_text_h_get(char* text, int max_w)
{
  size_t length = strlen(text);

  int h = 1;

  int line_w = 0;
  int space_index = 0;

  int last_space_index = space_index;

  for (size_t index = 0; index < length; index++)
  {
    char letter = text[index];

    // printf("before index: %ld space_index: %d letter: %c line_w: %d\n", index, space_index, letter, line_w);

    if (letter == ' ')
    {
      space_index = index;
    }

    if (letter == '\n')
    {
      line_w = 0;

      h++;
    }
    else if (line_w >= max_w)
    {
      line_w = 0;

      h++;

      // Current word cannot be wrapped
      if (space_index == last_space_index)
      {
        info_print("Cannot be wrapped: max_w: %d\n", max_w);
        return -1;
      }

      index = space_index;

      last_space_index = space_index;
    }
    else
    {
      line_w++;
    }

    // printf("after  index: %ld space_index: %d letter: %c line_w: %d\n", index, space_index, letter, line_w);
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

    // printf("mid: %d curr_h: %d h: %d\n", mid, curr_h, h);

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

    // printf("left: %d right: %d w: %d\n", left, right, w);
  }

  return min_w;
}

/*
 * Get widths of lines in text, regarding max height
 */
static inline void tui_text_ws_get(int* ws, char* text, int h)
{
  int max_w = tui_text_w_get(text, h);

  info_print("max_w: %d\n", max_w);

  size_t length = strlen(text);

  int line_index = 0;
  int line_w = 0;

  int space_index = 0;

  for (size_t index = 0; (index < length) && (line_index < h); index++)
  {
    char letter = text[index];

    if (letter == ' ')
    {
      space_index = index;
    }

    if (letter == ' ' && line_w == 0)
    {
      line_w = 0;
    }
    else if (letter == '\n')
    {
      ws[line_index++] = line_w;

      line_w = 0;
    }
    else if (line_w >= max_w)
    {
      // full line width - last partial word
      ws[line_index++] = line_w - (index - space_index);

      line_w = 0;

      index = space_index;

      // printf("index: %ld\n", index);

      // usleep(100000);
    }
    else
    {
      line_w++;
    }

    // Store the width of last line
    if (index + 1 == length)
    {
      ws[line_index] = line_w;
    }
  }
}

/*
 * Render text in rect in window
 *
 * xpos determines if the text is aligned to the left, centered or right
 */
static inline void tui_text_render(WINDOW* window, char* text, tui_rect_abs_t rect, tui_pos_t xpos, tui_pos_t ypos)
{
  int h = tui_text_h_get(text, rect.w);

  int ws[h];

  tui_text_ws_get(ws, text, h);


  size_t length = strlen(text);

  int line_index = 0;
  int line_w = 0;

  int y = 0;

  for (size_t index = 0; index < length; index++)
  {
    char letter = text[index];

    if (letter == ' ' && line_w == 0)
    {
      line_w = 0;
    }
    else if (line_w >= ws[line_index])
    {
      line_index++;

      line_w = 0;

      y++;
    }
    else
    {
      int x_shift = TUI_POS_FACTOR[xpos] * (rect.w - ws[line_index]) / 2;

      int y_shift = TUI_POS_FACTOR[ypos] * (rect.h - h) / 2;

      // info_print("rect.h: %d y_shift: %d", rect.h, y_shift);

      mvwprintw(window, rect.y + y_shift + y, rect.x + x_shift + line_w, "%c", letter);

      line_w++;
    }
  }
}

/*
 * Get x position of tui_rect_abs_t
 */
static inline int tui_rect_abs_x_get(tui_pos_t pos, int l, int r, int parent_w, int w)
{
  switch (pos)
  {
    case TUI_POS_LEFT:
      return l;

    case TUI_POS_RIGHT:
      return parent_w - w - r;

    default: case TUI_POS_CENTER:
      return (float) (parent_w - w) / 2.f;
  }
}

/*
 * Get y position of tui_rect_abs_t
 */
static inline int tui_rect_abs_y_get(tui_pos_t pos, int t, int b, int parent_h, int h)
{
  switch (pos)
  {
    case TUI_POS_TOP:
      return t;

    case TUI_POS_BOTTOM:
      return parent_h - h - b;

    default: case TUI_POS_CENTER:
      return (float) (parent_h - h) / 2.f;
  }
}

/*
 * Get absolute size of text window
 * 
 * If window is popup, extra height for button is required
 */
static inline tui_rect_abs_t tui_window_text_rect_abs_get(tui_window_text_t* window, int parent_w, int parent_h)
{
  tui_rect_t rect = window->head.rect;

  int l = tui_size_abs_get(parent_w, rect.l);
  int r = tui_size_abs_get(parent_w, rect.r);
  int t = tui_size_abs_get(parent_h, rect.t);
  int b = tui_size_abs_get(parent_h, rect.b);

  int w = tui_rect_abs_w_get(l, r, parent_w, rect.w);
  int h = tui_rect_abs_h_get(t, b, parent_h, rect.h);

  int text_w = MAX(1, w - 4);
  int text_h = MAX(1, window->is_interact ? (h - 4) : (h - 2));

  // If size is not specified, height is sat to minimum
  if (rect.w.type == TUI_SIZE_NONE && rect.h.type == TUI_SIZE_NONE)
  {
    int text_len = strlen(window->text);

    text_h = tui_text_h_get(window->text, text_len);
  }

  if (rect.w.type == TUI_SIZE_NONE)
  {
    text_w = tui_text_w_get(window->text, text_h);
  }
  else if (rect.h.type == TUI_SIZE_NONE)
  {
    text_h = tui_text_h_get(window->text, text_w);
  }

  w = text_w + 4;
  h = window->is_interact ? (text_h + 4) : (text_h + 2);

  int x = tui_rect_abs_x_get(rect.xpos, l, r, parent_w, w);
  int y = tui_rect_abs_y_get(rect.ypos, t, b, parent_h, h);

  return (tui_rect_abs_t) { w, h, x, y };
}

/*
 * Get absolute size of confirm window
 */
static inline tui_rect_abs_t tui_window_confirm_rect_abs_get(tui_window_confirm_t* window, int parent_w, int parent_h)
{
  tui_rect_t rect = window->head.rect;

  int l = tui_size_abs_get(parent_w, rect.l);
  int r = tui_size_abs_get(parent_w, rect.r);
  int t = tui_size_abs_get(parent_h, rect.t);
  int b = tui_size_abs_get(parent_h, rect.b);

  int w = tui_rect_abs_w_get(l, r, parent_w, rect.w);
  int h = tui_rect_abs_h_get(t, b, parent_h, rect.h);

  int text_w = MAX(1, w - 4);
  int text_h = MAX(1, h - 4);

  // If size is not specified, height is sat to minimum
  if (rect.w.type == TUI_SIZE_NONE && rect.h.type == TUI_SIZE_NONE)
  {
    int text_len = strlen(window->prompt);

    text_h = tui_text_h_get(window->prompt, text_len);
  }

  if (rect.w.type == TUI_SIZE_NONE)
  {
    text_w = tui_text_w_get(window->prompt, text_h);
  }
  else if (rect.h.type == TUI_SIZE_NONE)
  {
    text_h = tui_text_h_get(window->prompt, text_w);
  }

  w = text_w + 4;
  h = text_h + 4;

  int x = tui_rect_abs_x_get(rect.xpos, l, r, parent_w, w);
  int y = tui_rect_abs_y_get(rect.ypos, t, b, parent_h, h);

  return (tui_rect_abs_t) { w, h, x, y };
}

/*
 * Just create a tui_window_text_t object
 */
static inline tui_window_text_t* _tui_window_text_create(tui_t* tui, char* name, bool is_visable, tui_rect_t rect, tui_window_event_t event, char* text, bool is_interact, char* text_close, tui_pos_t xpos, tui_pos_t ypos)
{
  tui_window_text_t* window = malloc(sizeof(tui_window_text_t));

  if (!window)
  {
    errno = ENOMEM; // Out of memory

    return NULL;
  }
  
  memset(window, 0, sizeof(tui_window_t));

  tui_window_t head = (tui_window_t)
  {
    .type        = TUI_WINDOW_TEXT,
    .name        = name,
    .is_visable  = is_visable,
    .is_interact = is_interact,
    .rect        = rect,
    .event       = event,
    .tui         = tui
  };

  *window = (tui_window_text_t)
  {
    .head           = head,
    .text           = text,
    .text_len       = strlen(text),
    .is_interact    = is_interact,
    .xpos           = xpos,
    .ypos           = ypos
  };

  if (text_close)
  {
    window->text_close     = text_close;
    window->text_close_len = strlen(text_close);
  }

  return window;
}

/*
 * Just create a tui_window_confirm_t object
 */
static inline tui_window_confirm_t* _tui_window_confirm_create(tui_t* tui, char* name, bool is_visable, tui_rect_t rect, tui_window_event_t event, char* prompt, char* text_yes, char* text_no, bool answer)
{
  tui_window_confirm_t* window = malloc(sizeof(tui_window_confirm_t));

  if (!window)
  {
    errno = ENOMEM; // Out of memory

    return NULL;
  }
  
  memset(window, 0, sizeof(tui_window_t));

  tui_window_t head = (tui_window_t)
  {
    .type        = TUI_WINDOW_CONFIRM,
    .name        = name,
    .is_visable  = is_visable,
    .is_interact = true,
    .rect        = rect,
    .event       = event,
    .tui         = tui
  };

  *window = (tui_window_confirm_t)
  {
    .head         = head,
    .prompt       = prompt,
    .prompt_len   = strlen(prompt),
    .text_yes     = text_yes,
    .text_yes_len = strlen(text_yes),
    .text_no      = text_no,
    .text_no_len  = strlen(text_no),
    .answer       = answer
  };

  return window;
}

/*
 * Create a confirm window and append it to TUI windows
 */
tui_window_confirm_t* tui_window_confirm_create(tui_t* tui, char* name, bool is_visable, tui_rect_t rect, tui_window_event_t event, char* prompt, char* text_yes, char* text_no, bool answer)
{
  if (!tui || !name)
  {
    errno = EFAULT; // Bad address

    return NULL;
  }

  tui_window_confirm_t* window = _tui_window_confirm_create(tui, name, is_visable, rect, event, prompt, text_yes, text_no, answer);

  if (!window)
  {
    return NULL;
  }

  // Assign window parent
  window->head.parent_type = TUI_PARENT_TUI;
  window->head.parent.tui  = tui;

  tui_rect_abs_t abs_rect = tui_window_confirm_rect_abs_get(window, tui->w, tui->h);

  // Assign window ncurses window
  window->head.abs_rect = abs_rect;
  window->head.window   = tui_ncurses_window_create(abs_rect.w, abs_rect.h, abs_rect.x, abs_rect.y);

  if (tui_window_append(tui, (tui_window_t*) window) != 0)
  {
    free(window);

    return NULL;
  }

  return window;
}

/*
 * Create a text window and append it to TUI
 */
tui_window_text_t* tui_window_text_create(tui_t* tui, char* name, bool is_visable, tui_rect_t rect, tui_window_event_t event, char* text, bool is_interact, char* text_close, tui_pos_t xpos, tui_pos_t ypos)
{
  if (!tui || !name)
  {
    errno = EFAULT; // Bad address

    return NULL;
  }

  tui_window_text_t* window = _tui_window_text_create(tui, name, is_visable, rect, event, text, is_interact, text_close, xpos, ypos);

  if (!window)
  {
    return NULL;
  }

  // Assign window parent
  window->head.parent_type = TUI_PARENT_TUI;
  window->head.parent.tui  = tui;

  tui_rect_abs_t abs_rect = tui_window_text_rect_abs_get(window, tui->w, tui->h);

  // Assign window ncurses window
  window->head.abs_rect = abs_rect;
  window->head.window   = tui_ncurses_window_create(abs_rect.w, abs_rect.h, abs_rect.x, abs_rect.y);

  if (tui_window_append(tui, (tui_window_t*) window) != 0)
  {
    free(window);

    return NULL;
  }

  return window;
}

/*
 * Create a text window and append it to menu
 */
tui_window_text_t* tui_menu_window_text_create(tui_menu_t* menu, char* name, bool is_visable, tui_rect_t rect, tui_window_event_t event, char* text, bool is_interact, char* text_close, tui_pos_t xpos, tui_pos_t ypos)
{
  if (!menu || !name)
  {
    errno = EFAULT; // Bad address

    return NULL;
  }

  tui_window_text_t* window = _tui_window_text_create(menu->tui, name, is_visable, rect, event, text, is_interact, text_close, xpos, ypos);

  if (!window)
  {
    return NULL;
  }

  // Assign window parent
  window->head.parent_type = TUI_PARENT_MENU;
  window->head.parent.menu = menu;

  tui_rect_abs_t abs_rect = tui_window_text_rect_abs_get(window, menu->tui->w, menu->tui->h);

  // Assign window ncurses window
  window->head.abs_rect = abs_rect;
  window->head.window   = tui_ncurses_window_create(abs_rect.w, abs_rect.h, abs_rect.x, abs_rect.y);

  if (tui_menu_window_append(menu, (tui_window_t*) window) != 0)
  {
    free(window);

    return NULL;
  }

  return window;
}

/*
 * Create menu and append it to TUI
 */
tui_menu_t* tui_menu_create(tui_t* tui, char* name, tui_menu_event_t event)
{
  if (!tui || !name)
  {
    errno = EFAULT; // Bad address

    return NULL;
  }

  tui_menu_t* menu = malloc(sizeof(tui_menu_t));

  if (!menu)
  {
    errno = ENOMEM; // Out of memory
    
    return NULL;
  }

  *menu = (tui_menu_t)
  {
    .name  = name,
    .event = event,
    .tui   = tui
  };

  if (tui_menu_append(tui, menu) != 0)
  {
    free(menu);

    return NULL;
  }

  return menu;
}

/*
 * Just free tui_window_confirm_t struct
 */
static inline void tui_window_confirm_free(tui_window_confirm_t** window)
{
  tui_ncurses_window_free(&(*window)->head.window);

  free(*window);

  *window = NULL;
}

/*
 * Just free tui_window_text_t struct
 */
static inline void tui_window_text_free(tui_window_text_t** window)
{
  tui_ncurses_window_free(&(*window)->head.window);

  free(*window);

  *window = NULL;
}

/*
 * Get index of menu window by name
 */
static inline ssize_t tui_menu_window_index_get(tui_menu_t* menu, char* name)
{
  if (!menu || !name)
  {
    return -1;
  }

  for (ssize_t index = 0; index < menu->window_count; index++)
  {
    tui_window_t* window = menu->windows[index];

    if (window && strcmp(window->name, name) == 0)
    {
      return index;
    }
  }

  return -1;
}

/*
 * Get index of TUI window by name
 */
static inline ssize_t tui_window_index_get(tui_t* tui, char* name)
{
  if (!tui || !name)
  {
    return -1;
  }

  for (ssize_t index = 0; index < tui->window_count; index++)
  {
    tui_window_t* window = tui->windows[index];

    if (window && strcmp(window->name, name) == 0)
    {
      return index;
    }
  }

  return -1;
}

/*
 * Get index of TUI menu by name
 */
static inline ssize_t tui_menu_index_get(tui_t* tui, char* name)
{
  if (!tui || !name)
  {
    return -1;
  }

  for (ssize_t index = 0; index < tui->menu_count; index++)
  {
    tui_menu_t* menu = tui->menus[index];

    if (menu && strcmp(menu->name, name) == 0)
    {
      return index;
    }
  }

  return -1;
}

/*
 * Rotate array of windows - to cycle active window
 */
static inline void tui_windows_rotate(tui_window_t** windows, size_t count, size_t turns)
{
  if (turns == 0) return;

  size_t shift = turns % count;

  tui_window_t* temp_windows[shift];

  for (size_t index = 0; index < shift; index++)
  {
    temp_windows[index] = windows[index];
  }

  for (size_t index = 0; index < (count - shift); index++)
  {
    windows[index] = windows[shift + index];
  }

  for (size_t index = 0; index < shift; index++)
  {
    windows[count - shift + index] = temp_windows[index];
  }
}

/*
 * Choose new active window from array of windows
 */
static inline tui_window_t* tui_windows_active_window_get(tui_window_t** windows, size_t count)
{
  for (size_t index = 0; index < count; index++)
  {
    tui_window_t* window = windows[index];

    if (!window->is_visable)
    {
      continue;
    }

    if (window->type == TUI_WINDOW_PARENT)
    {
      tui_window_parent_t* parent = (tui_window_parent_t*) window;

      return tui_windows_active_window_get(parent->children, parent->child_count);
    }
  
    if (window->is_interact)
    {
      return window;
    }
  }

  return NULL;
}

/*
 * Set TUI active menu
 */
static inline void tui_active_menu_set(tui_t* tui, char* name)
{
  ssize_t index = tui_menu_index_get(tui, name);

  if (index != -1)
  {
    tui_menu_t* menu = tui->menus[index];

    tui->menu = menu;

    tui_window_t* window = tui_windows_active_window_get(menu->windows, menu->window_count);

    if (window)
    {
      tui->window = window;
    }
  }
}

/*
 * Set TUI active window
 */
static inline void tui_active_window_set(tui_t* tui, char* name)
{
  ssize_t index = tui_window_index_get(tui, name);

  if (index != -1)
  {
    tui_window_t* window = tui->windows[index];

    window->is_visable = true;

    tui->window = window;

    tui_windows_rotate(tui->windows, tui->window_count, index);
  }
}

/*
 * Unset TUI active window
 *
 * Automatically set new active window
 */
static inline void tui_active_window_unset(tui_t* tui)
{
  tui->window = NULL;

  tui_window_t* window = tui_windows_active_window_get(tui->windows, tui->window_count);

  if (window)
  {
    tui->window = window;
  }
  else
  {
    tui_menu_t* menu = tui->menu;

    window = tui_windows_active_window_get(menu->windows, menu->window_count);

    if (window)
    {
      tui->window = window;
    }
  }
}

/*
 * Delete TUI confirm window
 */
int tui_window_confirm_delete(tui_t* tui, char* name)
{
  if (!tui || !name)
  {
    return 1;
  }

  ssize_t window_index = tui_window_index_get(tui, name);

  if (window_index == -1)
  {
    return 2;
  }

  tui_window_t* window = tui->windows[window_index];

  tui_window_confirm_free((tui_window_confirm_t**) &window);


  for (size_t index = window_index; index < (tui->window_count - 1); index++)
  {
    tui->windows[index] = tui->windows[index + 1];
  }

  tui_window_t** temp_windows = realloc(tui->windows, sizeof(tui_window_t*) * (tui->window_count - 1));

  if (!temp_windows)
  {
    // Here, the window is destroyed, but the windows array isn't shrunk
    return 3;
  }

  tui->windows = temp_windows;

  return 0;
}

/*
 * Free window struct
 */
static inline void tui_window_free(tui_window_t** window)
{
  if (!window || !(*window)) return;

  switch ((*window)->type)
  {
    case TUI_WINDOW_CONFIRM:
      tui_window_confirm_free((tui_window_confirm_t**) window);
      break;

    case TUI_WINDOW_TEXT:
      tui_window_text_free((tui_window_text_t**) window);
      break;

    default:
      break;
  }
}

/*
 * Free menu struct
 */
static inline void tui_menu_free(tui_menu_t** menu)
{
  if (!menu || !(*menu)) return;

  for (size_t index = 0; index < (*menu)->window_count; index++)
  {
    tui_window_t* window = (*menu)->windows[index];

    tui_window_free(&window);
  }

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
    tui_menu_t* menu = (*tui)->menus[index];

    tui_menu_free(&menu);
  }

  for (size_t index = 0; index < (*tui)->window_count; index++)
  {
    tui_window_t* window = (*tui)->windows[index];

    tui_window_free(&window);
  }

  free(*tui);

  *tui = NULL;
}

/*
 * Render window title
 */
static inline void tui_title_render(tui_window_t* window, tui_title_t title)
{
  int max_w = (window->abs_rect.w - 2);

  int x_shift = TUI_POS_FACTOR[title.pos] * (max_w - title.title_len) / 2;

  wmove(window->window, 0, 1 + x_shift);

  wprintw(window->window, "%s", title.title);
}

/*
 * Show selected text if it is the active window
 */
static inline void tui_window_select_show(tui_t* tui, tui_window_t* window)
{
  if (window == tui->window)
  {
    wattron(window->window, A_REVERSE);
  }
}

/*
 * Hide selected text
 */
static inline void tui_window_select_hide(tui_t* tui, tui_window_t* window)
{
  wattroff(window->window, A_REVERSE);
}

/*
 * Render text window
 */
static inline void tui_window_text_render(tui_window_text_t* window)
{
  tui_window_t head = window->head;

  curs_set(0);

  werase(head.window);

  tui_rect_abs_t abs_rect = head.abs_rect;

  tui_rect_abs_t text_rect =
  {
    .x = 2,
    .y = 1,
    .w = abs_rect.w - 4,
    .h = window->is_interact ? (abs_rect.h - 4) : (abs_rect.h - 2)
  };


  box(head.window, 0, 0);


  tui_text_render(head.window, window->text, text_rect, window->xpos, window->ypos);

  if (window->is_interact)
  {
    int x_shift = (head.abs_rect.w - window->text_close_len) / 2;

    wmove(head.window, head.abs_rect.h - 2, x_shift);

    tui_window_select_show(head.tui, (tui_window_t*) window);

    wprintw(head.window, "%s", window->text_close);

    tui_window_select_hide(head.tui, (tui_window_t*) window);
  }

  wrefresh(head.window);
}

/*
 * Render confirm window
 */
static inline void tui_window_confirm_render(tui_window_confirm_t* window)
{
  tui_window_t head = window->head;

  curs_set(0);

  werase(head.window);


  tui_rect_abs_t abs_rect = head.abs_rect;


  tui_rect_abs_t text_rect =
  {
    .x = 2,
    .y = 1,
    .w = abs_rect.w - 4,
    .h = abs_rect.h - 4
  };

  box(head.window, 0, 0);

  tui_text_render(head.window, window->prompt, text_rect, TUI_POS_CENTER, TUI_POS_CENTER);


  int answer_len = (window->text_yes_len + 1 + window->text_no_len);

  int x_shift = (head.abs_rect.w - answer_len) / 2;

  wmove(head.window, head.abs_rect.h - 2, x_shift);


  if (window->answer == TUI_YES)
  {
    tui_window_select_show(head.tui, (tui_window_t*) window);
  }

  wprintw(head.window, "%s", window->text_yes);

  tui_window_select_hide(head.tui, (tui_window_t*) window);


  waddch(head.window, ' ');

  
  if (window->answer == TUI_NO)
  {
    tui_window_select_show(head.tui, (tui_window_t*) window);
  }

  wprintw(head.window, "%s", window->text_no);

  tui_window_select_hide(head.tui, (tui_window_t*) window);


  wrefresh(head.window);
}

/*
 * Render window
 */
static inline void tui_window_render(tui_window_t* window)
{
  switch (window->type)
  {
    case TUI_WINDOW_CONFIRM:
      tui_window_confirm_render((tui_window_confirm_t*) window);
      break;

    case TUI_WINDOW_TEXT:
      tui_window_text_render((tui_window_text_t*) window);
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
    tui_window_t* window = windows[index];

    if (window->is_visable)
    {
      tui_window_render(window);
    }
  }
}

/*
 * Render TUI
 */
static inline void tui_render(tui_t* tui)
{
  curs_set(0);

  refresh();

  tui_menu_t* menu = tui->menu;

  if (menu)
  {
    tui_windows_render(menu->windows, menu->window_count);
  }

  tui_windows_render(tui->windows, tui->window_count);
}

/*
 * Resize text window
 */
static inline void tui_window_text_resize(tui_window_text_t* window, int parent_w, int parent_h)
{
  tui_rect_abs_t abs_rect = tui_window_text_rect_abs_get(window, parent_w, parent_h);

  window->head.abs_rect = abs_rect;

  info_print("window_text w:%d h:%d x:%d y:%d",
      abs_rect.w, abs_rect.h, abs_rect.x, abs_rect.y);

  tui_ncurses_window_resize(window->head.window, abs_rect.w, abs_rect.h, abs_rect.x, abs_rect.y);
}

/*
 * Resize confirm window
 */
static inline void tui_window_confirm_resize(tui_window_confirm_t* window, int parent_w, int parent_h)
{
  tui_rect_abs_t abs_rect = tui_window_confirm_rect_abs_get(window, parent_w, parent_h);

  window->head.abs_rect = abs_rect;

  tui_ncurses_window_resize(window->head.window, abs_rect.w, abs_rect.h, abs_rect.x, abs_rect.y);
}

/*
 * Resize window
 */
static inline void tui_window_resize(tui_window_t* window, int parent_w, int parent_h)
{
  switch (window->type)
  {
    case TUI_WINDOW_CONFIRM:
      tui_window_confirm_resize((tui_window_confirm_t*) window, parent_w, parent_h);
      break;

    case TUI_WINDOW_TEXT:
      tui_window_text_resize((tui_window_text_t*) window, parent_w, parent_h);
      break;

    default:
      break;
  }
}

/*
 * Resize windows
 */
static inline void tui_windows_resize(tui_window_t** windows, size_t count, int parent_w, int parent_h)
{
  for (size_t index = 0; index < count; index++)
  {
    tui_window_t* window = windows[index];

    tui_window_resize(window, parent_w, parent_h);
  }
}

/*
 * Resize TUI based on terminal new size
 */
static inline void tui_resize(tui_t* tui)
{
  tui->w = getmaxx(stdscr);
  tui->h = getmaxy(stdscr);

  info_print("Resize: %dx%d", tui->w, tui->h);

  tui_windows_resize(tui->windows, tui->window_count, tui->w, tui->h);

  for (size_t index = 0; index < tui->menu_count; index++)
  {
    tui_menu_t* menu = tui->menus[index];

    tui_windows_resize(menu->windows, menu->window_count, tui->w, tui->h);
  }
}

/*
 * Event
 */

/*
 * Default event handling for confirm window
 */
static inline void tui_window_confirm_event(tui_window_confirm_t* window, int key)
{
  switch (key)
  {
    case 'l': case 'L': case KEY_RIGHT:
      window->answer = TUI_NO;
      break;

    case 'h': case 'H': case KEY_LEFT:
      window->answer = TUI_YES;
      break;

    default:
      break;
  }
}

/*
 * Default event handling for different window types
 */
static inline void tui_window_event(tui_window_t* window, int key)
{
  switch (window->type)
  {
    case TUI_WINDOW_CONFIRM:
      tui_window_confirm_event((tui_window_confirm_t*) window, key);
      break;

    default:
      break;
  }
}

/*
 * Handle key press from size window
 *
 * Close the size window on any keypress except resize
 */
static inline void tui_size_window_event(tui_window_t* head, int key)
{
  if (key != KEY_RESIZE)
  {
    head->is_visable = false;

    tui_active_window_unset(head->tui);
  }
}

/*
 * Quit event - triggered from quit confirm window
 */
static inline void tui_quit_window_event(tui_window_t* head, int key)
{
  tui_window_confirm_t* window = (tui_window_confirm_t*) head;

  switch (key)
  {
    case KEY_ENTR:
      head->is_visable = false;

      tui_active_window_unset(head->tui);

      if (window->answer == TUI_YES)
      {
        head->tui->is_running = false;
      }
      break;

    default:
      break;
  }
}

/*
 * Default behavior of TUI
 */
static inline void tui_event(tui_t* tui, int key)
{
  switch (key)
  {
    case KEY_CTRLC:
      tui_active_window_set(tui, "quit");
      break;

    case KEY_RESIZE:
      tui_resize(tui);
      break;

    default:
      break;
  }
}

/*
 * Trigger event of active window
 */
static inline void tui_event_trigger(tui_t* tui, int key)
{
  if (tui->window)
  {
    tui_window_t* window = tui->window;

    // Trigger default behavior of window
    tui_window_event(window, key);

    if (window->event)
    {
      window->event(window, key);
    }

    if (window->parent_type == TUI_PARENT_MENU)
    {
      tui_menu_t* menu = window->parent.menu;

      if (menu->event)
      {
        menu->event(menu, key);
      }
    }
  }

  tui_event(tui, key);

  if (tui->event)
  {
    tui->event(tui, key);
  }
}

/*
 * Setup TUI by creating default windows
 */
static inline void tui_setup(tui_t* tui, int w, int h)
{
  tui_window_confirm_create(tui, "quit", false,
    (tui_rect_t) { 0 }, // Default values
    &tui_quit_window_event,
    "Do you want to quit?", "Yes", "No", TUI_NO
  );

  tui_window_text_create(tui, "size", true,
    (tui_rect_t)
    {
      .w = (tui_size_t)
      {
        .type = TUI_SIZE_ABS,
        .value.abs = w
      },
      .h = (tui_size_t)
      {
        .type = TUI_SIZE_ABS,
        .value.abs = h
      }
    },
    &tui_size_window_event,
    "Resize terminal to fit this window",
    false,
    NULL,
    TUI_POS_CENTER,
    TUI_POS_CENTER
  );
}

/*
 * Create tui struct
 */
tui_t* tui_create(int w, int h, tui_event_t event)
{
  tui_t* tui = malloc(sizeof(tui_t));

  if (!tui)
  {
    return NULL;
  }

  memset(tui, 0, sizeof(tui_t));

  *tui = (tui_t)
  {
    .w     = getmaxx(stdscr),
    .h     = getmaxy(stdscr),
    .event = event
  };

  tui_setup(tui, w, h);

  return tui;
}

/*
 * Stop TUI
 */
void tui_stop(tui_t* tui)
{
  if (!tui) return;

  tui->is_running = false;
}

/*
 * Start TUI, render text and handle keyboard input
 */
void tui_start(tui_t* tui)
{
  if (!tui) return;

  tui->is_running = true;

  tui_active_window_set(tui, "size");

  tui_render(tui);

  int key;

  while (tui->is_running && (key = wgetch(stdscr)))
  {
    erase();

    mvprintw(1, 1, "KEY: %d", key);

    if (key == KEY_CTRLS)
    {
      tui->is_running = false;
      break;
    }

    tui_event_trigger(tui, key);

    tui_render(tui);
  }
}

#endif // TUI_IMPLEMENT
