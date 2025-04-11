# Notes
- fix flickering screen
- remove specific window_ _create functions and make it more generic.
- fix tui_tab_ functions: only is_interact windows can be tabbed to
- it looks like: chart is not resizing when window is resizing, only one refresh later

## Maybe
- Remove rect argument and use child->_rect instead
- move is_inflated from parent to specific children
  and split up to x and y inflation
- utilize that tui, menu and parent has same fields (create generic functions)

## Future
- fix tui_tab_forward and tui_tab_backward
- tidy up stock.h header library
