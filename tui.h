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

/*
 * TUI position
 */
typedef enum tui_pos_t
{
  TUI_POS_NONE,
  TUI_POS_LEFT,
  TUI_POS_RIGHT,
  TUI_POS_TOP,
  TUI_POS_BOTTOM,
  TUI_POS_CENTER
} tui_pos_t;

/*
 * TUI size type
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
  tui_size_t width;
  tui_size_t height;

  tui_size_t left;   // Margin to left
  tui_size_t right;  // Margin to right
  tui_size_t top;    // Margin on top
  tui_size_t bottom; // Margin on bottom

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
 * General window struct, same for all windows
 */
typedef struct tui_window_t
{
  tui_window_type_t type;
  char*             name;   // ID name
  WINDOW*           window;
  char*             title;  // Displaying title
  bool              is_child;
  union
  {
    tui_window_t* window; // is_child : true
    tui_menu_t*   menu;   // is_child : false
  } parent;
  void* (*event)(tui_window_t* window, int key);
  tui_t*            tui;
} tui_window_t;

/*
 * Struct for item in list window
 */
typedef struct tui_window_list_item_t
{
  char*              string; // Displaying string
  char*              data;   // Carrying   data
  void* (*event)(tui_window_list_item_t* item, int key);
  tui_window_list_t* window; // Parent list window
  tui_t*             tui;
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
  tui_window_t*  child;       // Current child window
} tui_window_parent_t;

/*
 * Menu struct containing windows
 */
typedef struct tui_menu_t
{
  char*          name;
  tui_window_t** windows;
  size_t         window_count;
  tui_window_t*  window;       // Current window
  tui_t*         tui;
  void* (*event)(tui_menu_t* menu, int key);
} tui_menu_t;

/*
 * TUI struct containing menus
 */
typedef struct tui_t
{
  tui_menu_t** menus;
  size_t       menu_count;
  tui_menu_t*  menu;       // Current menu
  bool         is_running; // TUI is running
  void* (*event)(tui_t* tui, int key);
} tui_t;

/*
 * Declarations of TUI functions
 */

extern void   tui_init(void);

extern void   tui_quit(void);

extern tui_t* tui_create(void);

extern void   tui_free(tui_t** tui);

#endif // TUI_H

/*
 * This header library file uses _IMPLEMENT guards
 *
 * If TUI_IMPLEMENT is defined, the definitions will be included
 */

#ifdef TUI_IMPLEMENT

#include <stdlib.h>

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

#endif // TUI_IMPLEMENT
