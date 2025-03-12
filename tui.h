/*
 * tui.h - terminal user interface library
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2025-03-11
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
    .tui   = tui
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

  erase();

  if (tui->menu)
  {
    tui_menu_render(tui->menu);
  }

  for (size_t index = 0; index < tui->window_count; index++)
  {
    tui_window_t* window = tui->windows[index];

    tui_window_render(window);
  }

  refresh();
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
