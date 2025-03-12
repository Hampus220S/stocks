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
 * TUI position
 *
 * By default, window will be centered
 */
typedef enum tui_pos_t
{
  TUI_POS_CENTER,
  TUI_POS_LEFT,
  TUI_POS_RIGHT,
  TUI_POS_TOP,
  TUI_POS_BOTTOM
} tui_pos_t;

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

  tui_pos_t  xpos;   // Horizontal alignment
  tui_pos_t  ypos;   // Vertical   alignment
} tui_rect_t;

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
 *
 */

typedef void* (*tui_window_list_item_event_t)(tui_window_list_item_t* item, int key);

typedef void* (*tui_window_event_t)(tui_window_t* window, int key);

typedef void* (*tui_menu_event_t)(tui_menu_t* menu, int key);

typedef void* (*tui_event_t)(tui_t* tui, int key);

/*
 * General window struct, same for all windows
 */
typedef struct tui_window_t
{
  tui_window_type_t  type;
  char*              name;   // ID name
  char*              title;  // Displaying title
  tui_rect_t         rect;
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
  tui_pos_t    xpos; // Horizontal position
  tui_pos_t    ypos; // Vertical   position
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
 * The active menu has it's own field
 *
 * The first window in array is the active one
 */
typedef struct tui_t
{
  tui_menu_t**   menus;
  size_t         menu_count;
  tui_menu_t*    menu;         // Current menu
  tui_window_t** windows;
  size_t         window_count;
  bool           is_running;   // TUI is running
  tui_event_t    event;
} tui_t;

/*
 * Declarations of TUI functions
 */

extern void   tui_init(void);

extern void   tui_quit(void);

extern tui_t* tui_create(void);

extern void   tui_free(tui_t** tui);


extern int                   tui_window_confirm_delete(tui_t* tui, char* name);

extern tui_window_confirm_t* tui_window_confirm_create(tui_t* tui, char* name, char* title, tui_rect_t rect, tui_window_event_t event, char* prompt, char* text_yes, char* text_no, bool answer);

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
 * Quit event - triggered from quit confirm window
 */
static inline void* tui_quit_window_event(tui_window_t* window, int key)
{
  switch (key)
  {
    case KEY_ENTER:
      window->tui->is_running = false;
      break;

    default:
      break;
  }

  return NULL;
}

/*
 *
 */
static inline void tui_window_active_set(tui_t* tui, char* name)
{

}

/*
 * Default behavior of TUI
 */
static inline void* tui_event(tui_t* tui, int key)
{
  switch (key)
  {
    case KEY_CTRLC:
      tui_window_active_set(tui, "quit");
      tui->is_running = false;
      break;

    default:
      break;
  }

  return NULL;
}

/*
 * Setup TUI by creating default windows
 */
static inline void tui_setup(tui_t* tui)
{
  tui_window_confirm_create(tui, "quit", NULL,
    (tui_rect_t) { 0 }, // Default values
    &tui_quit_window_event,
    "Do you want to quit?", "Yes", "No", TUI_NO
  );
}

/*
 * Create tui struct
 */
tui_t* tui_create(void)
{
  tui_t* tui = malloc(sizeof(tui_t));

  if (!tui)
  {
    return NULL;
  }

  tui_setup(tui);

  return tui;
}

/*
 * Free tui struct
 */
void tui_free(tui_t** tui)
{
  if (!tui || !(*tui)) return;

  free(*tui);

  *tui = NULL;
}

/*
 *
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
 *
 */


/*
 * Create ncurses WINDOW* for tui_window_t
 */
static inline WINDOW* tui_ncurses_window_create(tui_window_t* window)
{
  // return newwin(h, w, y, x);
  
  return NULL;
}

/*
 * Delete ncurses WINDOW*
 */
static inline void tui_ncurses_window_delete(WINDOW** window)
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
 *
 */
typedef struct tui_rect_abs_t
{
  int w;
  int h;
  int x;
  int y;
} tui_rect_abs_t;

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

  for (size_t index = 0; index < length; index++)
  {
    char letter = text[index];

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

      index = space_index;
    }
    else
    {
      line_w++;
    }

    // printf("index: %ld space_index: %d letter: %c line_w: %d\n", index, space_index, letter, line_w);
  }

  return h;
}

/*
 * Get the width of wrapped text given the height
 */
static inline int tui_text_w_get(char* text, int max_h)
{
  int length = strlen(text);

  int break_index = 0;

  int max_w = 0;

  // Store width of every line in text
  int line_ws[max_h];

  int line_amount = 1;

  for (size_t index = 0; index < length; index++)
  {
    char letter = text[index];

    int line_w = index - break_index;

    if (letter == '\n')
    {
      line_ws[line_amount - 1] = line_w;

      max_w = MAX(max_w, line_w);

      break_index = index;

      line_amount++;
    }

    // If the text doesn't fit, return the current max width
    if (line_amount > max_h)
    {
      return max_w;
    }

    // Store the width of last line
    if (index + 1 == length)
    {
      line_ws[line_amount - 1] = line_w;
    }
  }

  for (int count = 0; count <= (max_h - line_amount); count++)
  {
    // The following code halfs the largest width

    max_w = 0;
    int max_index = 0;

    for (int index = 0; index < (line_amount + count); index++)
    {
      int line_w = line_ws[index];

      printf("[%d] line_w: %d\n", index, line_w);

      if (line_w > max_w)
      {
        max_index = index;

        max_w = line_w;
      }
    }

    // The last iteration, the halfing is not interesting
    int half_w = line_ws[max_index] / 2;

    // Store an extra character if width was odd
    line_ws[max_index] = (line_ws[max_index] % 2 == 0) ? half_w : half_w + 1;

    // Store the other half at new slot
    line_ws[line_amount + count] = half_w;
  }

  // After the halfing, max width will be smaller
  return max_w;
}

/*
 * Get widths of lines in text, regarding max height
 */
static inline void tui_text_ws_get(int* ws, char* text, int max_h)
{
  int max_w = tui_text_w_get(text, max_h);

  printf("max_w: %d\n", max_w);

  size_t length = strlen(text);

  int line_index = 0;
  int line_w = 0;

  int space_index = 0;

  for (size_t index = 0; (index < length) && (line_index < max_h); index++)
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

  // If size is not specified, height is sat to minimum
  if (rect.w.type == TUI_SIZE_NONE && rect.h.type == TUI_SIZE_NONE)
  {
    h = 5;
  }

  int text_w = MAX(0, w - 2);
  int text_h = MAX(0, h - 4);

  if (rect.w.type == TUI_SIZE_NONE)
  {
    text_w = tui_text_w_get(window->prompt, text_h);
  }
  else if (rect.h.type == TUI_SIZE_NONE)
  {
    text_h = tui_text_h_get(window->prompt, text_w);
  }

  w = text_w + 2;
  h = text_h + 4;

  int x = tui_rect_abs_x_get(rect.xpos, l, r, parent_w, w);
  int y = tui_rect_abs_y_get(rect.ypos, t, b, parent_h, h);

  return (tui_rect_abs_t) { w, h, x, y };
}

/*
 * Just create a tui_window_confirm_t object
 */
static inline tui_window_confirm_t* _tui_window_confirm_create(tui_t* tui, char* name, char* title, tui_rect_t rect, tui_window_event_t event, char* prompt, char* text_yes, char* text_no, bool answer)
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
    .type  = TUI_WINDOW_CONFIRM,
    .name  = name,
    .title = title,
    .rect  = rect,
    .event = event,
    .tui   = tui,
    .parent_type = TUI_PARENT_TUI,
    .parent.tui  = tui
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
tui_window_confirm_t* tui_window_confirm_create(tui_t* tui, char* name, char* title, tui_rect_t rect, tui_window_event_t event, char* prompt, char* text_yes, char* text_no, bool answer)
{
  if (!tui || !name)
  {
    errno = EFAULT; // Bad address

    return NULL;
  }

  tui_window_confirm_t* window = _tui_window_confirm_create(tui, name, title, rect, event, prompt, text_yes, text_no, answer);

  if (!window)
  {
    return NULL;
  }

  if (tui_window_append(tui, (tui_window_t*) window) != 0)
  {
    free(window);

    return NULL;
  }

  return window;
}

/*
 * Just free tui_window_confirm_t struct
 */
static inline void _tui_window_confirm_delete(tui_window_confirm_t** window)
{
  tui_ncurses_window_delete(&(*window)->head.window);

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

  _tui_window_confirm_delete((tui_window_confirm_t**) &window);


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
 * Trigger window event
 *
 * If the window is a parent window, trigger child events
 */
static inline void tui_window_event_trigger(tui_window_t* window, int key)
{
  if (window->event)
  {
    window->event(window, key);
  }

  if (window->type == TUI_WINDOW_PARENT)
  {
    tui_window_parent_t* parent = (tui_window_parent_t*) window;

    for (size_t index = 0; index < parent->child_count; index++)
    {
      tui_window_t* child = parent->children[index];

      tui_window_event_trigger(child, key);
    }
  }
}

/*
 * Trigger menu event and windows' events
 */
static inline void tui_menu_event_trigger(tui_menu_t* menu, int key)
{
  if (menu->event)
  {
    menu->event(menu, key);
  }

  for (size_t index = 0; index < menu->window_count; index++)
  {
    tui_window_t* window = menu->windows[index];

    tui_window_event_trigger(window, key);
  }
}

/*
 * Trigger TUI event, menus' events and windows' events
 */
static inline void tui_event_trigger(tui_t* tui, int key)
{
  if (tui->event)
  {
    tui->event(tui, key);
  }

  for (size_t index = 0; index < tui->menu_count; index++)
  {
    tui_menu_t* menu = tui->menus[index];

    tui_menu_event_trigger(menu, key);
  }

  for (size_t index = 0; index < tui->window_count; index++)
  {
    tui_window_t* window = tui->windows[index];

    tui_window_event_trigger(window, key);
  }
}

/*
 * Render TUI menu with it's windows
 */
static inline void tui_menu_render(tui_menu_t* menu)
{

}

/*
 * Render TUI window
 */
static inline void tui_window_render(tui_window_t* window)
{
  curs_set(0);

  werase(window->window);

  mvwprintw(window->window, 1, 2, "HEJ");

  wrefresh(window->window);
}

/*
 * Render TUI
 */
static inline void tui_render(tui_t* tui)
{
  curs_set(0);

  refresh();

  if (tui->menu)
  {
    tui_menu_render(tui->menu);
  }

  for (size_t index = 0; index < tui->window_count; index++)
  {
    tui_window_t* window = tui->windows[index];

    tui_window_render(window);
  }
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
