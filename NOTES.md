# Notes
- prevent curl from outputting text in ncurses screen
- divide up has_padding to padding between children and around children
  (add padding between search and list windows)
- don't change item border color on exit or enter, set color in item_window_render
- take new screenshots
- fix stocks_window_enter or remove it (fix switching from chart back to either search or list item)

## Duplicate
- tui_..._window_..._search (only tui_window_t** windows and size_t count is needed)

## Maybe
- rename parent_window_enter to root_window_render (not genric -> specific)
- maybe, call init event from tui_..._window_append
- an interactive window must have interactive parents all the way up
  (therefor, create function that calculates

## Future
- 'f' for fullscreen in chart window
- create new menu with stock in table with HLOC values
