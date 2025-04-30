# Notes
- take new screenshots
- fix stocks_window_enter or remove it (fix switching from chart back to either search or list item)
- fix: search text disappears when it is too long to print (tui_text_render h -> 0)
- tui_list_event should not increase index if item window is not visable
- make value_window a child of chart_parent, like range_window

## Duplicate
- tui_..._window_..._search (only tui_window_t** windows and size_t count is needed)

## Maybe
- rename parent_window_enter to root_window_render (not genric -> specific)
- maybe, call init event from tui_..._window_append
- an interactive window must have interactive parents all the way up
  (therefor, create function that calculates

## Future
- 'f' for fullscreen in chart window
- up and down arrows for increasing and decreasing time
- create new menu with stock in table with HLOC values
