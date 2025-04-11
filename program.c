/*
 * program.c
 */

#define TUI_IMPLEMENT
#include "tui.h"

#define DEBUG_IMPLEMENT
#include "debug.h"

#define STOCK_IMPLEMENT
#include "stock.h"

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
 *
 */
void root_window_enter(tui_window_t* head)
{
  tui_window_parent_t* root_window = (tui_window_parent_t*) head;

  if (root_window->child_count > 0)
  {
    tui_window_set(head->tui, root_window->children[0]);
  }
}

/*
 *
 */
void list_search_enter(tui_window_t* window)
{
  tui_input_t* input = window->data;

  input->cursor = input->buffer_len;

  tui_input_string_update(input);
}

/*
 *
 */
void list_search_exit(tui_window_t* window)
{
  tui_input_t* input = window->data;

  input->cursor = input->buffer_len;

  tui_input_string_update(input);
}

/*
 *
 */
void item_window_enter(tui_window_t* window)
{
  window->color.bg = TUI_COLOR_RED;
}

/*
 *
 */
void item_window_exit(tui_window_t* window)
{
  window->color.bg = TUI_COLOR_CYAN;
}

/*
 *
 */
typedef struct list_data_t
{
  tui_input_t* input;
  tui_list_t*  list;
} list_data_t;

/*
 *
 */
void list_window_free(tui_window_t* window)
{
  list_data_t* data = window->data;

  tui_list_delete(&data->list);

  tui_input_delete(&data->input);

  free(data);
}

/*
 *
 */
void list_window_enter(tui_window_t* window)
{
  list_data_t* data = window->data;

  if (!data) return;

  tui_list_t* list = data->list;

  if (list && list->item_count > 0)
  {
    tui_window_set(window->tui, list->items[list->item_index]);
  }
}

/*
 *
 */
bool list_window_key(tui_window_t* head, int key)
{
  list_data_t* data = head->data;

  tui_input_t* input = data->input;

  if (tui_input_event(input, key))
  {
    return true;
  }

  tui_list_t* list = data->list;

  if (tui_list_event(list, key))
  {
    tui_window_t* child = list->items[list->item_index];

    tui_window_set(head->tui, child);

    return true;
  }

  return false;
}

/*
 *
 */
static inline int grid_stock_y_get(stock_t* stock, int h, double value)
{
  return h - ((double) h * (value - stock->_low) / (stock->_high - stock->_low)) - 1;
}

/*
 * Data of stock window
 */
typedef struct stock_data_t
{
  stock_t*           stock;
  int                value_index;
  char*              string;
  tui_window_grid_t* chart;
  tui_window_text_t* window;
} stock_data_t;

/*
 * Update stock value string associated with chart
 */
void chart_window_string_update(tui_window_t* head)
{
  info_print("chart_window_string_update");

  stock_data_t* data = head->data;

  if (!data->string) return;

  // Default value
  sprintf(data->string, "(none, none)");

  stock_t* stock = data->stock;

  if (!stock) return;

  if (data->value_index < stock->_value_count)
  {
    stock_value_t value = stock->_values[stock->_value_count - data->value_index - 1];

    time_t raw_time = (time_t) value.time;

    struct tm *timeinfo = localtime(&raw_time);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    if (sprintf(data->string, "[(%s, %.2f)]", buffer, value.close) < 0)
    {
      info_print("Failed to sprintf");
    }

    info_print("string: (%s)", data->string);
  }
}

/*
 *
 */
void stock_window_free(tui_window_t* head)
{
  stock_data_t* data = head->data;

  if (data)
  {
    free(data->string);

    free(data);
  }
}

/*
 *
 */
void chart_window_cursor_render(tui_window_t* head)
{
  tui_window_grid_t* window = (tui_window_grid_t*) head;
  
  stock_data_t* data = head->data;

  stock_t* stock = data->stock;

  if (!stock) return;

  int cursor_x = MAX(0, head->_rect.w - 1 - (int) data->value_index * 2);

  double value = stock->_values[stock->_value_count - 1 - data->value_index].close;

  int cursor_y = grid_stock_y_get(stock, head->_rect.h, value);

  tui_cursor_set(head->tui, head->_rect.x + cursor_x, head->_rect.y + cursor_y);

  for (int y = 0; y < head->_rect.h; y++)
  {
    tui_window_grid_square_t* square = tui_window_grid_square_get(window, cursor_x, y);

    square->symbol = '|';
    square->color.fg = TUI_COLOR_WHITE;
  }

  for (int x = 0; x < head->_rect.w; x++)
  {
    tui_window_grid_square_t* square = tui_window_grid_square_get(window, x, cursor_y);

    square->symbol = '-';
    square->color.fg = TUI_COLOR_WHITE;
  }

  tui_window_grid_square_t* square = tui_window_grid_square_get(window, cursor_x, cursor_y);

  square->symbol = ' ';
}

/*
 *
 */
void chart_window_line_render(tui_window_t* head)
{
  info_print("Grid render: w:%d h:%d", head->_rect.w, head->_rect.h);
  
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

  int count = (float) head->_rect.w / 2.f;

  int first_index = MAX(0, (int) stock->_value_count - count - 1);

  double first_value = stock->_values[first_index].close;

  double last_value = stock->_values[stock->_value_count - 1].close;

  short color = (first_value > last_value) ? TUI_COLOR_RED : TUI_COLOR_GREEN;

  for (int index = 0; index < count; index++)
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

    int upper = MIN(y, next_y);
    int lower = MAX(y, next_y);

    for (y = upper; y < lower; y++)
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

  chart_window_string_update(head);
}

/*
 *
 */
void chart_window_candle_render(tui_window_t* head)
{
  // info_print("Grid render: w:%d h:%d", head->_rect.w, head->_rect.h);
  
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

  /*
  info_print("max: %d", max);
  info_print("min: %d", min);
  info_print("high: %d", stock->high);
  info_print("low:  %d", stock->low);
  */

  int count = (float) head->_rect.w / 2.f;

  for (int index = 0; index < count; index++)
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

    /*
    info_print("index: %d", index);
    info_print("  high   : %d", high);
    info_print("  first  : %d", first);
    info_print("  second : %d", second);
    info_print("  low    : %d", low);
    */

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

  chart_window_string_update(head);
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
 * Initialize data1 window by creating child windows
 */
void data1_window_init(tui_window_t* head)
{
  tui_window_parent_t* data1_window = (tui_window_parent_t*) head;

  tui_window_parent_t* labels_window = tui_parent_child_parent_create(data1_window, (tui_window_parent_config_t)
  {
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
  });

  tui_window_parent_t* values_window = tui_parent_child_parent_create(data1_window, (tui_window_parent_config_t)
  {
    .name = "values",
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
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
      .string = labels[index],
    });

    tui_parent_child_text_create(values_window, (tui_window_text_config_t)
    {
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
  });

  tui_window_parent_t* values_window = tui_parent_child_parent_create(data2_window, (tui_window_parent_config_t)
  {
    .name = "values",
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
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
      .string = labels[index],
    });

    tui_parent_child_text_create(values_window, (tui_window_text_config_t)
    {
      .name   = names[index],
      .string = "none",
    });
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

  tui_window_text_t* symbol = tui_parent_child_text_search(data_window, "data1 values symbol");

  if (symbol)
  {
    sprintf(buffer, "%s", stock->symbol);

    symbol->string = strdup(buffer);
  }


  tui_window_text_t* name = tui_parent_child_text_search(data_window, "data1 values name");

  if (name)
  {
    sprintf(buffer, "%s", stock->name);

    name->string = strdup(buffer);
  }


  tui_window_text_t* exchange = tui_parent_child_text_search(data_window, "data1 values exchange");

  if (exchange)
  {
    sprintf(buffer, "%s", stock->exchange);

    exchange->string = strdup(buffer);
  }


  tui_window_text_t* currency = tui_parent_child_text_search(data_window, "data1 values currency");

  if (currency)
  {
    sprintf(buffer, "%s", stock->currency);

    currency->string = strdup(buffer);
  }


  tui_window_text_t* volume = tui_parent_child_text_search(data_window, "data2 values volume");

  if (volume)
  {
    sprintf(buffer, "%d", stock->volume);

    volume->string = strdup(buffer);
  }


  tui_window_text_t* open = tui_parent_child_text_search(data_window, "data2 values open");

  if (open)
  {
    sprintf(buffer, "%.2f", stock->_open);

    open->string = strdup(buffer);
  }


  tui_window_text_t* high = tui_parent_child_text_search(data_window, "data2 values high");

  if (high)
  {
    sprintf(buffer, "%.2f", stock->_high);

    high->string = strdup(buffer);
  }


  tui_window_text_t* low = tui_parent_child_text_search(data_window, "data2 values low");

  if (low)
  {
    sprintf(buffer, "%.2f", stock->_low);

    low->string = strdup(buffer);
  }
}

/*
 * Initialize data window by creating child windows
 */
void data_window_init(tui_window_t* head)
{
  tui_window_parent_t* data_window = (tui_window_parent_t*) head;

  tui_window_parent_t* data1_window = tui_parent_child_parent_create(data_window, (tui_window_parent_config_t)
  {
    .rect = TUI_RECT_NONE,
    .event.init = &data1_window_init,
  });

  tui_window_parent_t* data2_window = tui_parent_child_parent_create(data_window, (tui_window_parent_config_t)
  {
    .rect = TUI_RECT_NONE,
    .event.init = &data2_window_init,
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

  data->string = malloc(sizeof(char) * 100);

  chart_window_string_update(head);


  tui_window_grid_t* chart_window = tui_parent_child_grid_create(stock_window, (tui_window_grid_config_t)
  {
    .rect = TUI_RECT_NONE,
    .color.bg = TUI_COLOR_BLACK,
    .size = (tui_size_t)
    {
      .w = 20,
      .h = 10,
    },
    .event.render = &chart_window_line_render,
    .event.key = &chart_window_key,
    .data = data,
  });

  data->chart = chart_window;

  tui_window_text_t* value_window = tui_parent_child_text_create(stock_window, (tui_window_text_config_t)
  {
    .rect = TUI_RECT_NONE,
    .data = data,
  });

  if (value_window)
  {
    data->window = value_window;

    value_window->string = data->string;
  }

  tui_parent_child_parent_create(stock_window, (tui_window_parent_config_t)
  {
    .rect = TUI_RECT_NONE,
    .event.init = &data_window_init,
    .data = data,
  });
}

/*
 *
 */
tui_window_parent_t* tui_grand_parent_get(tui_window_t* window, size_t level)
{
  tui_window_parent_t* parent = window->parent;

  for (size_t index = 1; parent && index < level; index++)
  {
    parent = parent->head.parent;
  }

  return parent;
}

/*
 *
 */
bool item_window_key(tui_window_t* head, int key)
{
  tui_window_parent_t* root_window = tui_grand_parent_get(head, 2);

  if (!root_window)
  {
    return false;
  }

  tui_window_t* stock_window = tui_parent_child_search(root_window, "stock");

  if (!stock_window)
  {
    return false;
  }

  stock_data_t* data = stock_window->data;

  if (!data)
  {
    return false;
  }

  switch (key)
  {
    case KEY_ENTR:
      if (data->chart)
      {
        data->stock = head->data;

        tui_window_set(head->tui, (tui_window_t*) data->chart);

        return true;
      }
      
      return false;

    default:
      break;
  }

  return false;
}

/*
 *
 */
void item_window_free(tui_window_t* head)
{
  if (head->data)
  {
    stock_free((stock_t**) &head->data);
  }
}

/*
 *
 */
void item_window_render(tui_window_t* head)
{
  tui_window_parent_t* item_window = (tui_window_parent_t*) head;

  stock_t* stock = head->data;

  if (!stock) return;

  char buffer[64];

  tui_window_text_t* symbol_window = tui_parent_child_text_search(item_window, "symbol");

  if (symbol_window)
  {
    sprintf(buffer, "%s", stock->symbol);

    symbol_window->string = strdup(buffer);
  }

  tui_window_text_t* value_window = tui_parent_child_text_search(item_window, "value");

  if (value_window)
  {
    sprintf(buffer, "%.2f", stock->_close);

    value_window->string = strdup(buffer);
  }
}

/*
 *
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
    .string = "Tjoho",
  });

  tui_parent_child_text_create(item_window, (tui_window_text_config_t)
  {
    .name = "value",
    .rect = TUI_RECT_NONE,
    .string = "Hejsan",
  });
}

/*
 *
 */
void list_window_init(tui_window_t* head)
{
  tui_window_parent_t* list_window = (tui_window_parent_t*) head;

  list_data_t* data = malloc(sizeof(list_data_t));

  if (!data) return;

  head->data = data;

  data->input = tui_input_create(head->tui, 100, NULL);

  data->list  = tui_list_create(head->tui, list_window->is_vertical);

  char* symbols[] =
  {
    "AAPL",
    "TSLA",
    "SPGI",
    "SBUX",
  };

  for (size_t index = 0; index < 4; index++)
  {
    char* symbol = symbols[index];

    stock_t* stock = stock_create(symbol, "1d");

    if (!stock) continue;

    tui_window_parent_t* item_window = tui_parent_child_parent_create(list_window, (tui_window_parent_config_t)
    {
      .name = symbol,
      .rect = TUI_RECT_NONE,
      .event.init = &item_window_init,
      .event.free = &item_window_free,
      .event.enter = &item_window_enter,
      .event.exit = &item_window_exit,
      .event.key = &item_window_key,
      .event.render = &item_window_render,
      .data = stock,
      .color.bg = TUI_COLOR_CYAN,
    });

    if (data->list)
    {
      tui_list_item_add(data->list, (tui_window_t*) item_window);
    }
  }
}

/*
 * Initialize root window by creating child windows
 */
void root_window_init(tui_window_t* head)
{
  info_print("root_window_init");

  tui_window_parent_t* root_window = (tui_window_parent_t*) head;

  tui_window_parent_t* list_window = tui_parent_child_parent_create(root_window, (tui_window_parent_config_t)
  {
    .name = "list",
    .rect = TUI_RECT_NONE,
    .event.init = &list_window_init,
    .event.enter = &list_window_enter,
    .event.free = &list_window_free,
    .event.key = &list_window_key,
    .color.bg = TUI_COLOR_BLUE,
    .is_vertical = true,
    .has_padding = true,
  });

  tui_window_parent_t* stock_window = tui_parent_child_parent_create(root_window, (tui_window_parent_config_t)
  {
    .name = "stock",
    .rect = TUI_RECT_NONE,
    .event.init = &stock_window_init,
    .color.bg = TUI_COLOR_GREEN,
    .is_vertical = true,
    .is_inflated = true,
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
    .color = (tui_color_t)
    {
      .fg = TUI_COLOR_BLACK,
    },
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
    .color = (tui_color_t)
    {
      .fg = TUI_COLOR_BLACK
    }
  });

  tui_window_parent_t* root_window = tui_menu_window_parent_create(menu, (tui_window_parent_config_t)
  {
    .rect = { 0 },
    .align = TUI_ALIGN_CENTER,
    .pos = TUI_POS_CENTER,
    .event.enter = &root_window_enter,
    .event.init  = &root_window_init,
    .is_inflated = true,
    .color.bg = TUI_COLOR_RED,
  });

  tui_menu_set(tui, menu);

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
