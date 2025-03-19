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

/*
 * Declarations of TUI structs
 */

typedef struct tui_t tui_t;

typedef struct tui_menu_t tui_menu_t;

typedef struct tui_window_t tui_window_t;

/*
 * Definitions of event function signatures
 */

typedef void (*tui_window_event_t)(tui_window_t* window, int key);

typedef void (*tui_menu_event_t)(tui_menu_t* menu, int key);

typedef void (*tui_event_t)(tui_t* tui, int key);

/*
 * Type of window
 *
 * 1. Parent window contains child windows
 * 2. Text window displays text
 * 3. Input window inputs text
 */
typedef enum tui_window_type_t
{
  TUI_WINDOW_PARENT,
  TUI_WINDOW_TEXT,
  TUI_WINDOW_INPUT
} tui_window_type_t;

/*
 * Normal rect
 */
typedef struct tui_rect_t
{
  int w;
  int h;
  int x;
  int y;
} tui_rect_t;

/*
 * Colors
 */
typedef enum tui_color_t
{
  TUI_COLOR_NONE,
  TUI_COLOR_BLACK,
  TUI_COLOR_RED,
  TUI_COLOR_GREEN,
  TUI_COLOR_YELLOW,
  TUI_COLOR_BLUE,
  TUI_COLOR_MAGENTA,
  TUI_COLOR_CYAN,
  TUI_COLOR_WHITE
} tui_color_t;

/*
 * Border
 */
typedef struct tui_border_t
{
  tui_color_t fg_color;
  tui_color_t bg_color;
} tui_border_t;

/*
 * Type of parent
 */
typedef enum tui_parent_type_t
{
  TUI_PARENT_TUI,
  TUI_PARENT_MENU,
  TUI_PARENT_WINDOW
} tui_parent_type_t;

/*
 * Window struct
 */
typedef struct tui_window_t
{
  tui_window_type_t  type;
  char*              name;
  bool               is_visable;
  bool               is_interact;
  bool               is_locked;
  tui_rect_t         rect;
  WINDOW*            window;
  tui_color_t        fg_color;
  tui_color_t        bg_color;
  tui_border_t*      border;
  tui_window_event_t event;
  tui_parent_type_t  parent_type;
  union
  {
    tui_t*           tui;
    tui_menu_t*      menu;
    tui_window_t*    window;
  }                  parent;
  tui_t*             tui;
} tui_window_t;

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
 * Input window struct
 */
typedef struct tui_window_input_t
{
  tui_window_t* head;
  char*         buffer;
  size_t        buffer_size;
  size_t        buffer_len;
  size_t        cursor;
  size_t        scroll;
  bool          is_secret;
  bool          is_hidden;
  tui_pos_t     pos;
} tui_window_input_t;

/*
 * Text window struct
 */
typedef struct tui_window_text_t
{
  tui_window_t* head;
  char*         string;
  char*         text;
  tui_pos_t     pos;
  tui_align_t   align;
} tui_window_text_t;

/*
 * Parent window struct
 */
typedef struct tui_window_parent_t
{
  tui_window_t* children;
  size_t        child_count;
  bool          is_vertical;
  tui_pos_t     pos;
  tui_align_t   align;
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
 * TUI struct
 */
typedef struct tui_t
{
  int            w;
  int            h;
  tui_menu_t**   menus;
  size_t         menu_count;
  tui_window_t** windows;
  size_t         window_count;
  tui_window_t** tab_windows;
  size_t         tab_window_count;
  tui_menu_t*    menu;
  tui_window_t*  window;
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
 * Init tui (ncurses)
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

#endif // TUI_IMPLEMENT
