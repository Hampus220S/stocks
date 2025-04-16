# Notes
- fix flickering screen
- fix tui_tab_ functions: only is_interact windows can be tabbed to
- 'f' for fullscreen in chart window
- list item is marked when viewing stock in chart
- search should only enter chart if stock was found
- print prices on every second line vertically on left side of chart
  (0, 0, w:10, h:0) with .has_padding = true
- print titles of every window border
- TAB AND RTAB must be handled in tui_list_event if item_index should be updated

## Maybe
- remove tui_grand_parent_get and allow '.' (parent) in search

## Maybe (not) Panel
Note: Why not panel? parent, menu and tui has differences that should be respected, not generalized
- create tui_panel_t (window container) for tui, menu and parent
- don't distinguish between parent, menu and tui in context of child parent.
- remove specific window_ _create functions and make it more generic.

## Duplicate
- tui_..._window_..._search (only tui_window_t** windows and size_t count is needed)
- maybe, call init event from tui_..._window_append

## Future
- fix tui_tab_forward and tui_tab_backward
- tidy up stock.h header library
- create new menu with stock in table with HLOC values
