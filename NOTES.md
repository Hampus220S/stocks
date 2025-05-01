# Notes
- fix stocks_window_enter or remove it (fix switching from chart back to either search or list item)
- fix: search text disappears when it is too long to print (tui_text_render h -> 0)
- in tui_children_rect_calc: don't make is_atomic windows invisable first
  (go through children again if align_size is larger than max_size and make !is_atomic windows invisable in first hand, then is_atomic windows if necessary)

## Duplicate
- tui_..._window_..._search (only tui_window_t** windows and size_t count is needed)

## Maybe
- maybe, call init event from tui_..._window_append
- an interactive window must have interactive parents all the way up
  (therefor, create function that calculates

## Future
- 'f' for fullscreen in chart window
- up and down arrows for increasing and decreasing time
- create new menu with stock in table with HLOC values
