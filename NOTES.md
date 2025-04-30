# Notes
- take new screenshots
- fix stocks_window_enter or remove it (fix switching from chart back to either search or list item)
- fix: search text disappears when it is too long to print (tui_text_render h -> 0)
- tui_list_event should not increase index if item window is not visable
- in tui_children_rect_calc: don't make is_atomic windows invisable first
  (go through children again if align_size is larger than max_size and make !is_atomic windows invisable in first hand, then is_atomic windows if necessary)

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
