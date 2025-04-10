# Notes
- fix flickering screen
- remove specific window_ _create functions and make it more generic.
- remove tui_grid_t
- in chart, follow close price and render vertical line
- grid_data_t _min and _max will be stock.high stock.low if the whole graph is visable
  then, remove _min and _max.

## Maybe
- Remove rect argument and use child->_rect instead
- move is_inflated from parent to specific children
- remove STOCK_INTERVALS and STOCK_RANGES

## Future
- fix tui_tab_forward and tui_tab_backward
- tidy up stock.h header library
