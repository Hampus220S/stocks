/*
 * tui.h - terminal user interface library
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2025-03-11
 */

#ifndef TUI_H
#define TUI_H

typedef struct tui_t tui_t;

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

#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct tui_t
{
  bool is_running;
} tui_t;

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
