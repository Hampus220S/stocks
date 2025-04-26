/*
 * stocks.c - stock monitor
 *
 * Written by Hampus Fridholm
 */

#define TUI_IMPLEMENT
#include "tui.h"

#define DEBUG_IMPLEMENT
#include "debug.h"

#define STOCK_IMPLEMENT
#include "stock.h"

/*
 * Handle forward tab and backward tab event
 */
bool tab_event(tui_t* tui, int key)
{
  switch (key)
  {
    case KEY_TAB:
      return tui_tab_forward(tui);

    case KEY_RTAB:
      return tui_tab_backward(tui);

    default:
      break;
  }

  return false;
}

/*
 * Generic enter event for parent, enter first child window
 */
void parent_window_enter(tui_window_t* head)
{
  tui_window_parent_t* window = (tui_window_parent_t*) head;

  if (window->child_count > 0)
  {
    tui_window_set(head->tui, window->children[0]);
  }
}

/*
 * Enter event for item window, make border yellow
 */
void item_window_enter(tui_window_t* head)
{
  tui_window_parent_t* window = (tui_window_parent_t*) head;

  window->border.color.fg = TUI_COLOR_YELLOW;
}

/*
 * Exit event for item window, hide border
 */
void item_window_exit(tui_window_t* head)
{
  tui_window_parent_t* window = (tui_window_parent_t*) head;

  window->border.color.fg = TUI_COLOR_NONE;
}

/*
 * Data for stocks window
 */
typedef struct stocks_data_t
{
  tui_input_t* input;
  tui_list_t*  list;
  stock_t*     stock;
} stocks_data_t;

/*
 * Free function for stocks data
 */
void stocks_window_free(tui_window_t* window)
{
  stocks_data_t* data = window->data;

  tui_list_delete(&data->list);

  tui_input_delete(&data->input);

  stock_free(&data->stock);

  free(data);
}

/*
 * Enter event for stocks window, enter current item window
 */
void stocks_window_enter(tui_window_t* window)
{
  stocks_data_t* data = window->data;

  if (!data) return;

  tui_list_t* list = data->list;

  if (list && list->item_count > 0)
  {
    tui_window_set(window->tui, list->items[list->item_index]);
  }
}

/*
 * Keypress handler for stocks window, handle item list and search input
 */
bool stocks_window_key(tui_window_t* head, int key)
{
  tui_window_parent_t* stocks_window = (tui_window_parent_t*) head;

  stocks_data_t* data = head->data;

  tui_input_t* input = data->input;

  if (tui_input_event(input, key))
  {
    tui_window_parent_t* search_window = tui_window_window_parent_search((tui_window_t*) stocks_window, "search");

    if (search_window)
    {
      if (head->tui->window != (tui_window_t*) search_window)
      {
        tui_window_set(head->tui, (tui_window_t*) search_window);
      }
    }

    return true;
  }

  return false;
}

/*
 *
 */
static inline int grid_stock_y_get(stock_t* stock, int h, double value)
{
  return (h - 1) - ((double) (h - 1) * (value - stock->_low) / (stock->_high - stock->_low));
}

/*
 * Data of stock window
 */
typedef struct stock_data_t
{
  stock_t*           stock;
  int                value_index;
  tui_window_grid_t* chart;
  tui_window_text_t* window;
} stock_data_t;

/*
 * Free function for stock data
 */
void stock_window_free(tui_window_t* head)
{
  stock_data_t* data = head->data;

  free(data);
}

/*
 * Render cursor in chart window with vertical and horizontal lines
 */
static void chart_window_cursor_render(tui_window_t* head)
{
  tui_window_grid_t* window = (tui_window_grid_t*) head;
  
  stock_data_t* data = head->data;

  if (!data) return;

  stock_t* stock = data->stock;

  if (!stock) return;

  // Update cursor (value_index) based on resized stock
  if (data->value_index >= stock->_value_count)
  {
    data->value_index = stock->_value_count - 1;
  }

  int cursor_x = MAX(0, head->_rect.w - 1 - (int) data->value_index * 2);

  double value = stock->_values[stock->_value_count - 1 - data->value_index].close;

  int cursor_y = grid_stock_y_get(stock, head->_rect.h, value);

  short color = TUI_COLOR_YELLOW;

  for (int y = 0; y < head->_rect.h; y++)
  {
    tui_window_grid_square_t* square = tui_window_grid_square_get(window, cursor_x, y);

    square->symbol = '|';
    square->color.fg = color;
  }

  for (int x = 0; x < head->_rect.w; x++)
  {
    tui_window_grid_square_t* square = tui_window_grid_square_get(window, x, cursor_y);

    square->symbol = '-';
    square->color.fg = color;
  }

  tui_window_grid_square_t* square = tui_window_grid_square_get(window, cursor_x, cursor_y);

  square->symbol = ' ';
  square->color.bg = color;
}

/*
 * Render line chart
 */
void chart_window_line_render(tui_window_t* head)
{
  tui_window_grid_t* window = (tui_window_grid_t*) head;
  
  stock_data_t* data = head->data;

  if (!data) return;

  stock_t* stock = data->stock;

  if (!stock) return;

  tui_size_t size = { .w = head->_rect.w, .h = head->_rect.h };

  if (tui_window_grid_resize(window, size) != 0)
  {
    error_print("tui_window_grid_resize");
  }

  // Limit stock to window size
  stock_resize(data->stock, head->_rect.w / 2);

  short color = (stock->_close > stock->_open) ? TUI_COLOR_GREEN : TUI_COLOR_RED;

  for (int index = 0; index < stock->_value_count; index++)
  {
    if (index >= stock->_value_count) break;

    int x = (head->_rect.w - 1 - (index * 2));

    stock_value_t value = stock->_values[stock->_value_count - 1 - index];

    int y = grid_stock_y_get(stock, head->_rect.h, value.close);

    tui_window_grid_square_set(window, x, y, (tui_window_grid_square_t)
    {
      .color.bg = color,
    });

    if (index + 1 >= stock->_value_count) break;

    x--;

    stock_value_t next_value = stock->_values[stock->_value_count - 2 - index];

    int next_y = grid_stock_y_get(stock, head->_rect.h, next_value.close);

    // If the next y is equal to y or just 1 from it
    if (abs(next_y - y) <= 1)
    {
      tui_window_grid_square_set(window, x, y, (tui_window_grid_square_t)
      {
        .color.bg = color,
      });
    }
    else
    {
      int upper = MIN(y, next_y);
      int lower = MAX(y, next_y);

      // From one below upper square to one over lower square
      for (y = upper + 1; y <= lower - 1; y++)
      {
        tui_window_grid_square_set(window, x, y, (tui_window_grid_square_t)
        {
          .color.bg = color,
        });
      }
    }
  }

  if (head->tui->window == head)
  {
    chart_window_cursor_render(head);
  }
}

/*
 * Render candlestick chart
 */
void chart_window_candle_render(tui_window_t* head)
{
  tui_window_grid_t* window = (tui_window_grid_t*) head;
  
  stock_data_t* data = head->data;

  if (!data) return;

  stock_t* stock = data->stock;

  if (!stock) return;

  tui_size_t size = { .w = head->_rect.w, .h = head->_rect.h };

  if (tui_window_grid_resize(window, size) != 0)
  {
    error_print("tui_window_grid_resize");
  }

  // Limit stock to window size
  stock_resize(data->stock, head->_rect.w / 2);

  for (int index = 0; index < stock->_value_count; index++)
  {
    if (index >= stock->_value_count) break;

    int x = (head->_rect.w - 1 - (index * 2));

    stock_value_t value = stock->_values[stock->_value_count - 1 - index];

    int close = grid_stock_y_get(stock, head->_rect.h, value.close);

    int open = grid_stock_y_get(stock, head->_rect.h, value.open);

    int low = grid_stock_y_get(stock, head->_rect.h, value.low);

    int high = grid_stock_y_get(stock, head->_rect.h, value.high);

    int first = MIN(close, open);

    int second = MAX(close, open);

    short color = (value.close > value.open) ? TUI_COLOR_GREEN : TUI_COLOR_RED;

    // Wick
    for (int y = high; y < first; y++)
    {
      tui_window_grid_square_set(window, x, y, (tui_window_grid_square_t)
      {
        .color.fg = color,
        .symbol = '|',
      });
    }

    // Wick
    for (int y = second; y < low; y++)
    {
      tui_window_grid_square_set(window, x, y, (tui_window_grid_square_t)
      {
        .color.fg = color,
        .symbol = '|',
      });
    }

    // Body
    for (int y = first; y <= second; y++)
    {
      tui_window_grid_square_set(window, x, y, (tui_window_grid_square_t)
      {
        .color.bg = color,
      });
    }
  }

  if (head->tui->window == head)
  {
    chart_window_cursor_render(head);
  }
}

/*
 * Grid window key event
 */
bool chart_window_key(tui_window_t* head, int key)
{
  stock_data_t* data = head->data;

  if (!data) return false;

  stock_t* stock = data->stock;

  if (!stock) return false;

  switch (key)
  {
    case KEY_SPACE:
      if (head->event.render == &chart_window_candle_render)
      {
        head->event.render = &chart_window_line_render;
      }
      else
      {
        head->event.render = &chart_window_candle_render;
      }
      return true;

    case KEY_RIGHT:
      if (data->value_index > 0)
      {
        data->value_index--;

        return true;
      }

      data->value_index = 0;

      return false;

    case KEY_LEFT:
      if (data->value_index < (stock->_value_count - 1))
      {
        data->value_index++;

        return true;
      }

      data->value_index = stock->_value_count - 1;

      return false;

    case KEY_ESC:
      tui_window_parent_t* stocks_window = tui_window_window_parent_search(head, ". . . stocks");

      if (stocks_window)
      {
        tui_window_set(head->tui, (tui_window_t*) stocks_window);

        return true;
      }

      return false;

    case 'u':
      stock_update(stock);

      return true;

    case 'd':
      stock_zoom(stock, "1d");

      return true;

    case 'w':
      stock_zoom(stock, "1wk");

      return true;

    case 'm':
      stock_zoom(stock, "1mo");

      return true;

    case 'y':
      stock_zoom(stock, "1y");

      return true;

    case 'x':
      stock_zoom(stock, "max");

      return true;

    default:
      break;
  }

  return false;
}

/*
 * Update event for range window, update range string
 */
void range_window_update(tui_window_t* head)
{
  tui_window_text_t* window = (tui_window_text_t*) head;

  stock_data_t* data = head->data;

  if (!data) return;

  stock_t* stock = data->stock;

  if (!stock) return;

  tui_window_text_string_set(window, stock->range);
}

/*
 * Update event for prices window, resize height and update prices
 */
void prices_window_update(tui_window_t* head)
{
  tui_window_parent_t* window = (tui_window_parent_t*) head;

  stock_data_t* data = head->data;

  if (!data) return;

  stock_t* stock = data->stock;

  if (!stock) return;

  // 1. Free all child windows
  tui_windows_free(&window->children, &window->child_count);

  int lines = (head->_rect.h - 1) / 2;

  char buffer[64];

  for (int index = 0; index < lines; index++)
  {
    double fraction = ((float) ((lines - 1) - index) / (float) (lines - 1));

    double price = fraction * (stock->_high - stock->_low) + stock->_low;

    sprintf(buffer, "%.2f", price);

    tui_parent_child_text_create(window, (tui_window_text_config_t)
    {
      .rect = TUI_RECT_NONE,
      .string = buffer,
    });
  }
}

/*
 * Initialize data1 window by creating child windows
 */
void data1_window_init(tui_window_t* head)
{
  tui_window_parent_t* data1_window = (tui_window_parent_t*) head;

  tui_window_parent_t* labels_window = tui_parent_child_parent_create(data1_window, (tui_window_parent_config_t)
  {
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
    .pos = TUI_POS_END,
  });

  tui_window_parent_t* values_window = tui_parent_child_parent_create(data1_window, (tui_window_parent_config_t)
  {
    .name = "values",
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
    .pos = TUI_POS_START,
  });

  char* labels[] =
  {
    "Symbol   : ",
    "Name     : ",
    "Exchange : ",
    "Currency : ",
  };

  char* names[] =
  {
    "symbol",
    "name",
    "exchange",
    "currency",
  };

  for (size_t index = 0; index < 4; index++)
  {
    tui_parent_child_text_create(labels_window, (tui_window_text_config_t)
    {
      .rect = TUI_RECT_NONE,
      .string = labels[index],
    });

    tui_parent_child_text_create(values_window, (tui_window_text_config_t)
    {
      .rect = TUI_RECT_NONE,
      .name   = names[index],
      .string = "none",
    });
  }
}

/*
 * Initialize data2 window by creating child windows
 */
void data2_window_init(tui_window_t* head)
{
  tui_window_parent_t* data2_window = (tui_window_parent_t*) head;

  tui_window_parent_t* labels_window = tui_parent_child_parent_create(data2_window, (tui_window_parent_config_t)
  {
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
    .pos = TUI_POS_END,
  });

  tui_window_parent_t* values_window = tui_parent_child_parent_create(data2_window, (tui_window_parent_config_t)
  {
    .name = "values",
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
    .pos = TUI_POS_START,
  });

  char* labels[] =
  {
    "Volume : ",
    "Open   : ",
    "High   : ",
    "Low    : ",
  };

  char* names[] =
  {
    "volume",
    "open",
    "high",
    "low",
  };

  for (size_t index = 0; index < 4; index++)
  {
    tui_parent_child_text_create(labels_window, (tui_window_text_config_t)
    {
      .rect = TUI_RECT_NONE,
      .string = labels[index],
    });

    tui_parent_child_text_create(values_window, (tui_window_text_config_t)
    {
      .rect = TUI_RECT_NONE,
      .name   = names[index],
      .string = "none",
    });
  }
}

/*
 * Get value string and store it in buffer
 */
static int value_string_get(char* buffer, size_t size, stock_value_t value)
{
  if (value.time == 0 || value.close == 0)
  {
    if (snprintf(buffer, size, "(none, none)") < 0)
    {
      return 1;
    }

    return 0;
  }

  char time_buffer[64];

  time_t raw_time = (time_t) value.time;

  struct tm* timeinfo = localtime(&raw_time);

  if (strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M", timeinfo) < 0)
  {
    return 2;
  }

  if (snprintf(buffer, size, "(%s, %.2f)", time_buffer, value.close) < 0)
  {
    return 3;
  }

  return 0;
}

/*
 * Update value window by updating the value string
 */
void value_window_update(tui_window_t* head)
{
  tui_window_text_t* window = (tui_window_text_t*) head;

  stock_data_t* data = head->data;

  if (!data) return;

  stock_t* stock = data->stock;

  if (!stock) return;

  stock_value_t value = { 0 };

  // If the cursor is within the chart, get cursor value
  if (data->value_index < stock->_value_count)
  {
    value = stock->_values[stock->_value_count - data->value_index - 1];
  }

  char buffer[64];

  if (head->tui->window == (tui_window_t*) data->chart &&
      value_string_get(buffer, sizeof(buffer), value) == 0)
  {
    tui_window_text_string_set(window, buffer);
  }
  else
  {
    tui_window_text_string_set(window, "");
  }
}

/*
 * Fill data window with stock values
 */
void data_window_fill(tui_window_t* head)
{
  tui_window_parent_t* data_window = (tui_window_parent_t*) head;

  stock_data_t* data = head->data;

  if (!data) return;
  
  stock_t* stock = data->stock;
  
  if (!stock) return;
  

  char buffer[64];

  tui_window_text_t* symbol = tui_window_window_text_search((tui_window_t*) data_window, "data1 values symbol");

  if (symbol)
  {
    sprintf(buffer, "%s", stock->symbol);

    tui_window_text_string_set(symbol, buffer);
  }


  tui_window_text_t* name = tui_window_window_text_search((tui_window_t*) data_window, "data1 values name");

  if (name)
  {
    sprintf(buffer, "%s", stock->name);

    tui_window_text_string_set(name, buffer);
  }


  tui_window_text_t* exchange = tui_window_window_text_search((tui_window_t*) data_window, "data1 values exchange");

  if (exchange)
  {
    sprintf(buffer, "%s", stock->exchange);

    tui_window_text_string_set(exchange, buffer);
  }


  tui_window_text_t* currency = tui_window_window_text_search((tui_window_t*) data_window, "data1 values currency");

  if (currency)
  {
    sprintf(buffer, "%s", stock->currency);

    tui_window_text_string_set(currency, buffer);
  }


  tui_window_text_t* volume = tui_window_window_text_search((tui_window_t*) data_window, "data2 values volume");

  if (volume)
  {
    if (stock->volume > 0)
    {
      sprintf(buffer, "%d", stock->volume);

      tui_window_text_string_set(volume, buffer);
    }
    else
    {
      tui_window_text_string_set(volume, "none");
    }
  }


  tui_window_text_t* open = tui_window_window_text_search((tui_window_t*) data_window, "data2 values open");

  if (open)
  {
    sprintf(buffer, "%.2f", stock->open);

    tui_window_text_string_set(open, buffer);
  }


  tui_window_text_t* high = tui_window_window_text_search((tui_window_t*) data_window, "data2 values high");

  if (high)
  {
    sprintf(buffer, "%.2f", stock->high);

    tui_window_text_string_set(high, buffer);
  }


  tui_window_text_t* low = tui_window_window_text_search((tui_window_t*) data_window, "data2 values low");

  if (low)
  {
    sprintf(buffer, "%.2f", stock->low);

    tui_window_text_string_set(low, buffer);
  }
}

/*
 * Initialize data window by creating child windows
 */
void data_window_init(tui_window_t* head)
{
  tui_window_parent_t* data_window = (tui_window_parent_t*) head;

  tui_parent_child_parent_create(data_window, (tui_window_parent_config_t)
  {
    .name = "data1",
    .rect = TUI_RECT_NONE,
    .event.init = &data1_window_init,
    .pos = TUI_POS_CENTER,
  });

  tui_parent_child_parent_create(data_window, (tui_window_parent_config_t)
  {
    .name = "data2",
    .rect = TUI_RECT_NONE,
    .event.init = &data2_window_init,
    .pos = TUI_POS_CENTER,
  });

  tui_parent_child_text_create(data_window, (tui_window_text_config_t)
  {
    .string = " Data ",
    .rect = (tui_rect_t)
    {
      .w = 0,
      .h = 1,
    },
    .align = TUI_ALIGN_CENTER,
  });
}

/*
 * Initialize stock window by creating child windows and data
 */
void stock_window_init(tui_window_t* head)
{
  tui_window_parent_t* stock_window = (tui_window_parent_t*) head;

  stock_data_t* data = malloc(sizeof(stock_data_t));

  if (!data) return;

  memset(data, 0, sizeof(stock_data_t));

  head->data = data;

  tui_window_parent_t* chart_parent = tui_parent_child_parent_create(stock_window, (tui_window_parent_config_t)
  {
    .rect = TUI_RECT_NONE,
    .h_grow = true,
    .w_grow = true,
    .border = (tui_border_t)
    {
      .is_active = true,
      .color.fg = TUI_COLOR_WHITE,
    },
  });

  tui_window_grid_t* chart_window = tui_parent_child_grid_create(chart_parent, (tui_window_grid_config_t)
  {
    .rect = TUI_RECT_NONE,
    .size = (tui_size_t)
    {
      .w = 20,
      .h = 10,
    },
    .event.render = &chart_window_line_render,
    .event.key = &chart_window_key,
    .data = data,
    .h_grow = true,
    .w_grow = true,
  });

  data->chart = chart_window;

  tui_parent_child_text_create(chart_parent, (tui_window_text_config_t)
  {
    .string = " Chart ",
    .rect = (tui_rect_t)
    {
      .w = 0,
      .h = 1,
    },
    .align = TUI_ALIGN_CENTER,
  });

  tui_parent_child_parent_create(chart_parent, (tui_window_parent_config_t)
  {
    .rect = (tui_rect_t)
    {
      .w = 10,
      .h = 0,
    },
    .is_vertical = true,
    .has_padding = true,
    .event.update = &prices_window_update,
    .data = data,
  });

  tui_parent_child_text_create(chart_parent, (tui_window_text_config_t)
  {
    .rect = (tui_rect_t)
    {
      .y = 1,
      .w = -2,
      .h = 1,
    },
    .align = TUI_ALIGN_END,
    .color.fg = TUI_COLOR_WHITE,
    .event.update = &range_window_update,
    .data = data,
  });

  tui_window_text_t* value_window = tui_parent_child_text_create(stock_window, (tui_window_text_config_t)
  {
    .string = "",
    .rect = TUI_RECT_NONE,
    .event.update = &value_window_update,
    .color.fg = TUI_COLOR_YELLOW,
    .align = TUI_ALIGN_CENTER,
    .w_grow = true,
    .data = data,
  });

  data->window = value_window;

  tui_parent_child_parent_create(stock_window, (tui_window_parent_config_t)
  {
    .border = (tui_border_t)
    {
      .is_active = true,
    },
    .name = "data",
    .rect = TUI_RECT_NONE,
    .color.fg = TUI_COLOR_WHITE,
    .event.init = &data_window_init,
    .has_padding = true,
    .data = data,
    .align = TUI_ALIGN_CENTER,
    .w_grow = true,
  });
}

/*
 * Keypress handler for item window, enter will show chart of stock
 */
bool item_window_key(tui_window_t* head, int key)
{
  stock_t* stock = head->data;

  tui_window_parent_t* stock_window = tui_window_window_parent_search(head, ". . . stock");

  if (!stock_window)
  {
    return false;
  }

  stock_data_t* data = stock_window->head.data;

  if (!data)
  {
    return false;
  }

  switch (key)
  {
    case KEY_ENTR:
      if (data->chart)
      {
        stock_zoom(stock, "1d");

        data->stock = stock;

        tui_window_set(head->tui, (tui_window_t*) data->chart);

        tui_window_parent_t* data_window = tui_window_window_parent_search((tui_window_t*) stock_window, "data");

        if (data_window)
        {
          data_window_fill((tui_window_t*) data_window);
        }

        return true;
      }
      
      return false;

    default:
      break;
  }

  return false;
}

/*
 * Free function for item window stock object
 */
void item_window_free(tui_window_t* head)
{
  if (head->data)
  {
    stock_free((stock_t**) &head->data);
  }
}

/*
 * Render item window, white border when viewing it's chart
 */
void item_window_render(tui_window_t* head)
{
  tui_window_parent_t* window = (tui_window_parent_t*) head;

  stock_t* stock = head->data;

  if (!stock) return;

  tui_window_parent_t* stock_window = tui_window_window_parent_search(head, ". . . stock");

  if (!stock_window) return;

  stock_data_t* stock_data = stock_window->head.data;

  if (!stock_data) return;

  if (head->tui->window == (tui_window_t*) stock_data->chart && 
      stock_data->stock == stock)
  {
    window->border.color.fg = TUI_COLOR_WHITE;
  }
}

/*
 * Update item window, rendering stock symbol and stock price
 */
void item_window_update(tui_window_t* head)
{
  tui_window_parent_t* item_window = (tui_window_parent_t*) head;

  stock_t* stock = head->data;

  if (!stock) return;

  short color = (stock->close > stock->open) ? TUI_COLOR_GREEN : TUI_COLOR_RED;

  char buffer[64];

  tui_window_text_t* symbol_window = tui_window_window_text_search((tui_window_t*) item_window, "symbol");

  if (symbol_window)
  {
    sprintf(buffer, "%s    ", stock->symbol);

    tui_window_text_string_set(symbol_window, buffer);
  }

  tui_window_text_t* value_window = tui_window_window_text_search((tui_window_t*) item_window, "value");

  if (value_window)
  {
    sprintf(buffer, "%.2f", stock->close);

    tui_window_text_string_set(value_window, buffer);

    value_window->head.color.fg = color;
  }
}

/*
 * Initialize item window, creating symbol and value child windows
 */
void item_window_init(tui_window_t* head)
{
  tui_window_parent_t* item_window = (tui_window_parent_t*) head;

  stock_t* stock = head->data;

  if (!stock) return;

  tui_parent_child_text_create(item_window, (tui_window_text_config_t)
  {
    .name = "symbol",
    .rect = TUI_RECT_NONE,
    .align = TUI_ALIGN_START,
  });

  tui_parent_child_text_create(item_window, (tui_window_text_config_t)
  {
    .name = "value",
    .rect = TUI_RECT_NONE,
    .align = TUI_ALIGN_END,
  });
}

/*
 * Enter event for list window, enter current item window
 */
void list_window_enter(tui_window_t* head)
{
  stocks_data_t* data = head->data;

  if (!data) return;

  tui_list_t* list = data->list;

  if (list && list->item_count > 0)
  {
    tui_window_set(head->tui, list->items[list->item_index]);
  }
}

/*
 * Keypress handler for list window, scroll between items
 */
bool list_window_key(tui_window_t* head, int key)
{
  stocks_data_t* data = head->data;

  if (!data)
  {
    return false;
  }

  tui_list_t* list = data->list;

  if (!list)
  {
    return false;
  }

  if (tui_list_event(list, key))
  {
    tui_window_t* child = list->items[list->item_index];

    tui_window_set(head->tui, child);

    return true;
  }

  return false;
}

/*
 * Initialize list window, creating item windows for default stocks
 */
void list_window_init(tui_window_t* head)
{
  tui_window_parent_t* list_window = (tui_window_parent_t*) head;

  stocks_data_t* data = head->data;

  char* symbols[] =
  {
    "SEK=X",
    "^OMX",
    "AAPL",
    "TSLA",
    "SPGI",
  };

  for (size_t index = 0; index < 5; index++)
  {
    char* symbol = symbols[index];

    stock_t* stock = stock_create(symbol);

    if (!stock) continue;

    tui_window_parent_t* item_window = tui_parent_child_parent_create(list_window, (tui_window_parent_config_t)
    {
      .name         = symbol,
      .rect         = TUI_RECT_NONE,
      .border       = (tui_border_t)
      {
        .is_active  = true,
      },
      .has_padding  = false,
      .event.init   = &item_window_init,
      .event.free   = &item_window_free,
      .event.enter  = &item_window_enter,
      .event.exit   = &item_window_exit,
      .event.key    = &item_window_key,
      .event.update = &item_window_update,
      .event.render = &item_window_render,
      .data         = stock,
      .align        = TUI_ALIGN_BETWEEN,
      .w_grow       = true,
    });

    tui_list_item_add(data->list, (tui_window_t*) item_window);
  }

  tui_parent_child_text_create(list_window, (tui_window_text_config_t)
  {
    .string = " Stocks ",
    .rect = (tui_rect_t)
    {
      .w = 0,
      .h = 1,
    },
    .align = TUI_ALIGN_CENTER,
  });
}

/*
 * Enter event for search window, make border yellow
 */
void search_window_enter(tui_window_t* head)
{
  tui_window_parent_t* window = (tui_window_parent_t*) head;

  window->border.color.fg = TUI_COLOR_YELLOW;
}

/*
 * Exit event for search window, make border white again
 */
void search_window_exit(tui_window_t* head)
{
  tui_window_parent_t* window = (tui_window_parent_t*) head;

  window->border.color.fg = TUI_COLOR_WHITE;
}

/*
 * Keypress handler for search window, on enter view inputted stock's chart
 */
bool search_window_key(tui_window_t* head, int key)
{
  tui_window_parent_t* stock_window = tui_window_window_parent_search(head, ". . stock");

  if (!stock_window)
  {
    return false;
  }

  stock_data_t* stock_data = stock_window->head.data;

  if (!stock_data)
  {
    return false;
  }

  stocks_data_t* data = head->data;

  if (!data)
  {
    return false;
  }

  if (key == KEY_ENTR)
  {
    char* symbol = data->input->buffer;

    stock_t* stock = stock_create(symbol);

    if (!stock)
    {
      return true;
    }

    stock_free(&data->stock);

    data->stock = stock;

    stock_data->stock = data->stock;

    tui_window_grid_t* chart_window = stock_data->chart;

    if (chart_window)
    {
      tui_window_set(head->tui, (tui_window_t*) chart_window);

      tui_window_parent_t* data_window = tui_window_window_parent_search((tui_window_t*) stock_window, "data");

      if (data_window)
      {
        data_window_fill((tui_window_t*) data_window);
      }
    }

    return true;
  }

  return false;
}

/*
 * Update search window by rendering inputted text
 */
void search_window_update(tui_window_t* head)
{
  stocks_data_t* data = head->data;

  tui_window_text_t* text_window = tui_window_window_text_search(head, "text");

  if (text_window)
  {
    tui_window_text_string_set(text_window, data->input->string);
  }
}

/*
 * Initialize search window, create inputted text window
 */
void search_window_init(tui_window_t* head)
{
  tui_window_parent_t* search_window = (tui_window_parent_t*) head;

  stocks_data_t* data = head->data;

  if (!data) return;

  tui_window_text_t* text_window = tui_parent_child_text_create(search_window, (tui_window_text_config_t)
  {
    .name = "text",
    .rect = TUI_RECT_NONE,
  });

  data->input = tui_input_create(head->tui, 100, text_window);

  tui_parent_child_text_create(search_window, (tui_window_text_config_t)
  {
    .string = " Search ",
    .rect = (tui_rect_t)
    {
      .w = 0,
      .h = 1,
    },
    .align = TUI_ALIGN_CENTER,
  });
}

/*
 * Initialize stocks window, create list window and search window
 */
void stocks_window_init(tui_window_t* head)
{
  tui_window_parent_t* stocks_window = (tui_window_parent_t*) head;

  stocks_data_t* data = malloc(sizeof(stocks_data_t));

  if (!data) return;

  memset(data, 0, sizeof(stocks_data_t));

  head->data = data;

  data->list  = tui_list_create(head->tui, stocks_window->is_vertical);

  tui_parent_child_parent_create(stocks_window, (tui_window_parent_config_t)
  {
    .name         = "search",
    .rect         = TUI_RECT_NONE,
    .data         = data,
    .event.init   = &search_window_init,
    .event.update = &search_window_update,
    .event.key    = &search_window_key,
    .event.enter  = &search_window_enter,
    .event.exit   = &search_window_exit,
    .has_padding  = false,
    .w_grow       = true,
    .border       = (tui_border_t)
    {
      .is_active  = true,
      .color.fg   = TUI_COLOR_WHITE,
    },
    .is_interact = true,
  });

  tui_parent_child_parent_create(stocks_window, (tui_window_parent_config_t)
  {
    .name        = "list",
    .rect        = TUI_RECT_NONE,
    .data        = data,
    .event.init  = &list_window_init,
    .event.key   = &list_window_key,
    .event.enter = &list_window_enter,
    .is_vertical = true,
    .has_padding = false,
    .h_grow      = true,
    .border      = (tui_border_t)
    {
      .is_active = true,
      .color.fg  = TUI_COLOR_WHITE,
    },
    .is_interact = true,
  });
}

/*
 * Initialize root window by creating child windows
 */
void root_window_init(tui_window_t* head)
{
  tui_window_parent_t* root_window = (tui_window_parent_t*) head;

  tui_parent_child_parent_create(root_window, (tui_window_parent_config_t)
  {
    .name        = "stocks",
    .rect        = TUI_RECT_NONE,
    .event.init  = &stocks_window_init,
    .event.enter = &stocks_window_enter,
    .event.free  = &stocks_window_free,
    .event.key   = &stocks_window_key,
    .is_vertical = true,
    .has_padding = false,
    .h_grow      = true,
    .is_interact = true,
  });

  tui_parent_child_parent_create(root_window, (tui_window_parent_config_t)
  {
    .name        = "stock",
    .rect        = TUI_RECT_NONE,
    .event.init  = &stock_window_init,
    .event.free  = &stock_window_free,
    .is_vertical = true,
    .w_grow      = true,
    .h_grow      = true,
  });
}

/*
 * Inittialize menu by creating root window
 */
void menu_init(tui_menu_t* menu)
{
  tui_menu_window_parent_create(menu, (tui_window_parent_config_t)
  {
    .name = "root",
    .rect = TUI_PARENT_RECT,
    .align = TUI_ALIGN_CENTER,
    .pos = TUI_POS_CENTER,
    .event.enter = &parent_window_enter,
    .event.init  = &root_window_init,
    .has_padding = true,
  });
}

/*
 * Main function
 */
int main(int argc, char* argv[])
{
  debug_file_open("debug.log");

  if(tui_init() != 0)
  {
    info_print("Failed to initialize TUI");

    debug_file_close();

    return 1;
  }

  info_print("Initialized TUI");

  tui_t* tui = tui_create((tui_config_t)
  {
    .event.key = &tab_event,
  });

  if (!tui)
  {
    info_print("Failed to create TUI");

    tui_quit();

    debug_file_close();

    return 2;
  }

  info_print("Created TUI");

  tui_menu_t* menu = tui_menu_create(tui, (tui_menu_config_t)
  {
    .event.init = &menu_init,
  });

  // Set list window as the active window
  tui_menu_window_search_set(menu, "root stocks list");

  info_print("Starting tui");

  tui_start(tui);

  info_print("Stopping tui");

  tui_stop(tui);

  tui_delete(&tui);

  info_print("Deleted TUI");

  tui_quit();

  info_print("Quitted TUI");

  debug_file_close();

  return 0;
}
