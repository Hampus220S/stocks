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

void root_window_enter(tui_window_t* head)
{
  tui_window_parent_t* root = (tui_window_parent_t*) head;

  tui_window_set(head->tui, root->children[0]);
}

void panel_window_enter(tui_window_t* head)
{
  tui_window_parent_t* panel = (tui_window_parent_t*) head;

  tui_window_set(head->tui, panel->children[0]);
}

bool panel_window_key(tui_window_t* panel, int key)
{
  tui_list_t* list = panel->data;

  if (list && tui_list_event(list, key))
  {
    tui_window_set(panel->tui, list->items[list->item_index]);

    return true;
  }

  return false;
}

void side_item_enter(tui_window_t* window)
{
  window->color.bg = TUI_COLOR_CYAN;
}

void side_item_exit(tui_window_t* window)
{
  window->color.bg = TUI_COLOR_NONE;
}

void side_text_enter(tui_window_t* window)
{
  tui_input_t* input = window->data;

  input->cursor = input->buffer_len;

  tui_input_string_update(input);
}

void side_text_exit(tui_window_t* window)
{
  tui_input_t* input = window->data;

  input->cursor = input->buffer_len;

  tui_input_string_update(input);
}

typedef struct side_data_t
{
  tui_input_t* input;
  tui_list_t*  list;
} side_data_t;

void side_window_free(tui_window_t* window)
{
  side_data_t* data = window->data;

  tui_list_delete(&data->list);

  tui_input_delete(&data->input);

  free(data);
}

void side_window_enter(tui_window_t* window)
{
  side_data_t* data = window->data;

  tui_list_t* list = data->list;

  tui_window_set(window->tui, list->items[list->item_index]);
}

void side_window_exit(tui_window_t* window)
{
  window->color.bg = TUI_COLOR_BLUE;
}

bool side_window_key(tui_window_t* head, int key)
{
  side_data_t* data = head->data;

  tui_input_t* input = data->input;

  if (tui_input_event(input, key))
  {
    if (input->buffer_len > 0)
    {
      info_print("Buffer: (%s)", input->buffer);
    }

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
static inline int grid_stock_y_get(double max, double min, int h, double value)
{
  return h - ((double) h * (value - min) / (max - min)) - 1;
}

/*
 *
 */
static inline double grid_stock_value_get(double max, double min, int h, int y)
{
  return ((double) (h - y) / (double) h) * (max - min) + min;
}

const char* STOCK_RANGES[]    = { "1d", "5d", "1mo", "3mo", "6mo", "1y" };

const char* STOCK_INTERVALS[] = { "1m", "5m", "30m", "1h", "1h", "1h" };


#define STOCK_RANGE_COUNT (sizeof(STOCK_RANGES) / sizeof(char*))

#define STOCK_INTERVAL_COUNT (sizeof(STOCK_INTERVALS) / sizeof(char*))

/*
 *
 */
static const char* stock_range_get(int index)
{
  if (index >= 0 && index < STOCK_RANGE_COUNT)
  {
    return STOCK_RANGES[index];
  }

  return NULL;
}

/*
 *
 */
static ssize_t stock_range_index_get(const char* range)
{
  for (ssize_t index = 0; index < STOCK_RANGE_COUNT; index++)
  {
    if (strcmp(STOCK_RANGES[index], range) == 0)
    {
      return index;
    }
  }

  return -1;
}

/*
 *
 */
static const char* stock_interval_get(int index)
{
  if (index >= 0 && index < STOCK_INTERVAL_COUNT)
  {
    return STOCK_INTERVALS[index];
  }

  return NULL;
}

/*
 *
 */
static ssize_t stock_interval_index_get(const char* interval)
{
  for (ssize_t index = 0; index < STOCK_INTERVAL_COUNT; index++)
  {
    if (strcmp(STOCK_INTERVALS[index], interval) == 0)
    {
      return index;
    }
  }

  return -1;
}

/*
 *
 */
typedef struct grid_data_t
{
  tui_grid_t*        grid;
  stock_t*           stock;
  double             _min;
  double             _max;
  char*              string;
  tui_window_text_t* window;
} grid_data_t;

/*
 *
 */
static void grid_stock_update(tui_window_t* window)
{
  grid_data_t* data = window->data;

  stock_t* stock = data->stock;

  ssize_t index = stock_range_index_get(stock->range);

  if (index == -1)
  {
    index = 0;
  }

  const char* interval = stock_interval_get(index);

  if (!interval)
  {
    interval = "1m";
  }

  stock_t* new_stock = stock_get(stock->symbol, stock->range, (char*) interval);

  if (new_stock)
  {
    stock_free(&stock);

    data->stock = new_stock;
  }
}

/*
 *
 */
void grid_window_string_update(tui_window_t* head)
{
  grid_data_t* data = head->data;

  if (!data->string) return;

  // Default value
  sprintf(data->string, "(none, none)");

  stock_t* stock = data->stock;

  tui_grid_t* grid = data->grid;

  if (!stock || !grid)
  {
    return;
  }


  int index = (head->_rect.w - grid->x) / 2;

  if (index < stock->_value_count)
  {
    int time = stock->_values[stock->_value_count - index - 1].time;

    double value = grid_stock_value_get(data->_max, data->_min, head->_rect.h, grid->y);

    time_t raw_time = (time_t) time;

    struct tm *timeinfo = localtime(&raw_time);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    if (sprintf(data->string, "[(%s, %.2f)]", buffer, value) < 0)
    {
      info_print("Failed to sprintf");
    }

    info_print("string: (%s)", data->string);
  }
}

/*
 *
 */
void grid_window_init(tui_window_t* head)
{
  tui_window_grid_t* window = (tui_window_grid_t*) head;

  grid_data_t* data = malloc(sizeof(grid_data_t));

  if (!data) return;

  memset(data, 0, sizeof(grid_data_t));

  head->data = data;

  data->stock = stock_get("TSLA", "6mo", "4h");

  data->string = malloc(sizeof(char) * 100);

  data->grid = tui_grid_create(head->tui, window);

  grid_window_string_update(head);
}

/*
 *
 */
void grid_window_free(tui_window_t* head)
{
  grid_data_t* data = head->data;

  if (data)
  {
    stock_free(&data->stock);

    tui_grid_delete(&data->grid);

    free(data->string);

    free(data);
  }
}

void grid_window_min_max_calc(tui_window_t* head)
{
  grid_data_t* data = head->data;

  stock_t* stock = data->stock;

  stock_value_t value = stock->_values[stock->_value_count - 1];

  data->_max = value.high;
  data->_min = value.low;

  int count = (float) head->_rect.w / 2.f;

  for (int index = 1; index < count; index++)
  {
    if (index >= stock->_value_count) break;

    stock_value_t value = stock->_values[stock->_value_count - 1 - index];

    data->_max = MAX(data->_max, value.high);
    data->_min = MIN(data->_min, value.low);
  }
}

/*
 *
 */
void grid_window_cursor_contain(tui_window_t* head)
{
  grid_data_t* data = head->data;

  tui_grid_t* grid = data->grid;

  if (grid->x < 0)
  {
    grid->x = 0;
  }
  else if (grid->x >= head->_rect.w)
  {
    grid->x = head->_rect.w - 1;
  }

  if (grid->y < 0)
  {
    grid->y = 0;
  }
  else if (grid->y >= head->_rect.h)
  {
    grid->y = head->_rect.h - 1;
  }
}

/*
 *
 */
void grid_window_cursor_render(tui_window_t* head)
{
  tui_window_grid_t* window = (tui_window_grid_t*) head;
  
  grid_data_t* data = head->data;

  tui_grid_t* grid = data->grid;

  grid_window_cursor_contain(head);

  tui_cursor_set(head->tui, head->_rect.x + grid->x, head->_rect.y + grid->y);

  for (int y = 0; y < head->_rect.h; y++)
  {
    tui_window_grid_square_t* square = tui_window_grid_square_get(window, grid->x, y);

    square->symbol = '|';
    square->color.fg = TUI_COLOR_WHITE;
  }

  for (int x = 0; x < head->_rect.w; x++)
  {
    tui_window_grid_square_t* square = tui_window_grid_square_get(window, x, grid->y);

    square->symbol = '-';
    square->color.fg = TUI_COLOR_WHITE;
  }

  tui_window_grid_square_t* square = tui_window_grid_square_get(window, grid->x, grid->y);

  square->symbol = ' ';
}

/*
 *
 */
void grid_window_render2(tui_window_t* head)
{
  // info_print("Grid render: w:%d h:%d", head->_rect.w, head->_rect.h);
  
  tui_window_grid_t* window = (tui_window_grid_t*) head;
  
  grid_data_t* data = head->data;

  if (!data) return;

  stock_t* stock = data->stock;

  tui_size_t size = { .w = head->_rect.w, .h = head->_rect.h };

  if (tui_window_grid_resize(window, size) != 0)
  {
    error_print("tui_window_grid_resize");
  }

  // Limit stock to window size
  stock_resize(data->stock, head->_rect.w / 2);

  grid_window_min_max_calc(head);

  int count = (float) head->_rect.w / 2.f;

  int first_index = MAX(0, (int) stock->_value_count - count - 1);

  double first_value = stock->_values[first_index].close;

  double last_value = stock->_values[stock->_value_count - 1].close;

  short color = (first_value > last_value) ? TUI_COLOR_RED : TUI_COLOR_GREEN;

  for (int index = 0; index < count; index++)
  {
    if (index >= stock->_value_count) break;

    int x = (head->_rect.w - (index * 2));

    stock_value_t value = stock->_values[stock->_value_count - 1 - index];

    int y = grid_stock_y_get(data->_max, data->_min, head->_rect.h, value.close);

    tui_window_grid_square_set(window, x, y, (tui_window_grid_square_t)
    {
      .color.bg = color,
    });

    if (index + 1 >= stock->_value_count) break;

    x--;

    stock_value_t next_value = stock->_values[stock->_value_count - 2 - index];

    int next_y = grid_stock_y_get(data->_max, data->_min, head->_rect.h, next_value.close);

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
    grid_window_cursor_render(head);
  }
}

/*
 *
 */
void grid_window_render(tui_window_t* head)
{
  // info_print("Grid render: w:%d h:%d", head->_rect.w, head->_rect.h);
  
  tui_window_grid_t* window = (tui_window_grid_t*) head;
  
  grid_data_t* data = head->data;

  if (!data) return;

  stock_t* stock = data->stock;

  tui_size_t size = { .w = head->_rect.w, .h = head->_rect.h };

  if (tui_window_grid_resize(window, size) != 0)
  {
    error_print("tui_window_grid_resize");
  }

  // Limit stock to window size
  stock_resize(data->stock, head->_rect.w / 2);

  grid_window_min_max_calc(head);

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

    int x = (head->_rect.w - (index * 2));

    stock_value_t value = stock->_values[stock->_value_count - 1 - index];

    int close = grid_stock_y_get(data->_max, data->_min, head->_rect.h, value.close);

    int open = grid_stock_y_get(data->_max, data->_min, head->_rect.h, value.open);

    int low = grid_stock_y_get(data->_max, data->_min, head->_rect.h, value.low);

    int high = grid_stock_y_get(data->_max, data->_min, head->_rect.h, value.high);

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
    grid_window_cursor_render(head);
  }
}

/*
 *
 */
bool grid_window_key(tui_window_t* head, int key)
{
  grid_data_t* data = head->data;

  if (!data)
  {
    return false;
  }

  stock_t* stock = data->stock;
  tui_grid_t* grid = data->grid;

  if (grid && tui_grid_event(grid, key))
  {
    grid_window_string_update(head);

    return true;
  }

  switch (key)
  {
    case KEY_SPACE:
      if (head->event.render == &grid_window_render)
      {
        head->event.render = &grid_window_render2;
      }
      else
      {
        head->event.render = &grid_window_render;
      }
      return true;

    case 'd':
      free(stock->range);

      stock->range = strdup("1d");

      grid_stock_update(head);
      return true;

    case 'y':
      free(stock->range);

      stock->range = strdup("1y");

      grid_stock_update(head);
      return true;

    case 'm':
      free(stock->range);

      stock->range = strdup("1mo");

      grid_stock_update(head);
      return true;

    default:
      break;
  }

  return false;
}

/*
 *
 */
void panel_window_free(tui_window_t* head)
{
  tui_list_t* list = head->data;

  tui_list_delete(&list);
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

  tui_window_parent_t* root = tui_menu_window_parent_create(menu, (tui_window_parent_config_t)
  {
    .rect = { 0 },
    .align = TUI_ALIGN_CENTER,
    .pos = TUI_POS_CENTER,
    .event.enter = &root_window_enter,
    .is_inflated = true,
  });

  tui_window_parent_t* panel = tui_parent_child_parent_create(root, (tui_window_parent_config_t)
  {
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_GREEN,
    },
    .rect = TUI_RECT_NONE,
    .border = (tui_border_t)
    {
      .is_active = true,
    },
    .has_padding = true,
    .event.key = &panel_window_key,
    .event.enter = &panel_window_enter,
    .align = TUI_ALIGN_BETWEEN,
    .pos = TUI_POS_END,
    .is_inflated = true,
    .is_vertical = false,
    .event.free = &panel_window_free,
  });

  tui_window_parent_t* left = tui_parent_child_parent_create(panel, (tui_window_parent_config_t)
  {
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_YELLOW,
    },
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
    .event.enter = &side_window_enter,
    .event.exit = &side_window_exit,
    .event.key = &side_window_key,
    .event.free = &side_window_free,
    .is_inflated = false,
    .has_padding = true,
    .pos = TUI_POS_CENTER,
    .align = TUI_ALIGN_CENTER,
  });

  tui_window_text_t* left_text = tui_parent_child_text_create(left, (tui_window_text_config_t)
  {
    .rect = TUI_RECT_NONE,
    .color.bg = TUI_COLOR_RED,
    .event.enter = &side_text_enter,
    .event.exit = &side_text_exit,
  });

  /*
  tui_window_parent_t* middle = tui_parent_child_parent_create(panel, (tui_window_parent_config_t)
  {
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_MAGENTA,
    },
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
    .is_inflated = true,
    .has_padding = true,
    .pos = TUI_POS_CENTER,
    .align = TUI_ALIGN_CENTER,
  });
  */

  tui_window_parent_t* root2 = tui_parent_child_parent_create(panel, (tui_window_parent_config_t)
  {
    .rect = TUI_RECT_NONE,
    .align = TUI_ALIGN_CENTER,
    .pos = TUI_POS_CENTER,
    .is_inflated = true,
    .has_padding = true,
    .is_vertical = true,
    .color.bg = TUI_COLOR_CYAN,
  });

  tui_window_grid_t* grid = tui_parent_child_grid_create(root2, (tui_window_grid_config_t)
  {
    .rect = TUI_RECT_NONE,
    .color.bg = TUI_COLOR_BLACK,
    .size = (tui_size_t)
    {
      .w = 20,
      .h = 10,
    },
    .event.render = &grid_window_render2,
    .event.key = &grid_window_key,
    .event.init = &grid_window_init,
    .event.free = &grid_window_free,
  });

  tui_window_text_t* value_window = tui_parent_child_text_create(root2, (tui_window_text_config_t)
  {
    .rect = (tui_rect_t)
    {
      .w = 0,
      .h = 1,
    }
  });

  grid_data_t* grid_data = grid->head.data;

  if (grid_data)
  {
    grid_data->window = value_window;

    value_window->string = grid_data->string;
  }

  char* left_strings[] =
  {
    "banana",
    "ba\033[42ml\033[33mlon\033[0mg",
    "|\n\033[42m\033[32m|\033[0m\n|",
    "segel"
  };

  for (size_t index = 0; index < 4; index++)
  {
    tui_parent_child_text_create(left, (tui_window_text_config_t)
    {
      .string = left_strings[index],
      .rect = TUI_RECT_NONE,
      .event.enter = &side_item_enter,
      .event.exit = &side_item_exit,
    });

    /*
    tui_parent_child_text_create(middle, (tui_window_text_config_t)
    {
      .string = left_strings[index],
      .rect = TUI_RECT_NONE,
    });
    */
  }

  side_data_t* left_data = malloc(sizeof(side_data_t));

  left_data->input = tui_input_create(tui, 100, left_text);

  left_text->head.data = left_data->input;

  left_data->list = tui_list_create(tui, left->is_vertical);

  for (size_t index = 0; index < left->child_count; index++)
  {
    tui_list_item_add(left_data->list, left->children[index]);
  }

  left->head.data = left_data;


  /*
  tui_window_parent_t* right = tui_parent_child_parent_create(panel, (tui_window_parent_config_t)
  {
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_RED,
    },
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
    .event.enter = &side_window_enter,
    .event.exit = &side_window_exit,
    .event.key = &side_window_key,
    .event.free = &side_window_free,
    .is_inflated = false,
    .pos = TUI_POS_CENTER,
    .align = TUI_ALIGN_CENTER,
  });

  tui_window_text_t* right_text = tui_parent_child_text_create(right, (tui_window_text_config_t)
  {
    .rect = TUI_RECT_NONE,
    .color.bg = TUI_COLOR_MAGENTA,
    .event.enter = &side_text_enter,
    .event.exit = &side_text_exit,
  });

  tui_window_parent_t* middle2 = tui_parent_child_parent_create(panel, (tui_window_parent_config_t)
  {
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_BLUE,
    },
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
    .is_inflated = false,
    .pos = TUI_POS_CENTER,
    .align = TUI_ALIGN_CENTER,
  });

  char* right_strings[] =
  {
    "cash",
    "credit",
    "cream",
    "stock",
    "market",
    "making"
  };

  for (size_t index = 0; index < 6; index++)
  {
    tui_parent_child_text_create(right, (tui_window_text_config_t)
    {
      .string = right_strings[index],
      .rect = TUI_RECT_NONE,
      .event.enter = &side_item_enter,
      .event.exit = &side_item_exit,
    });

    tui_parent_child_text_create(middle2, (tui_window_text_config_t)
    {
      .string = right_strings[index],
      .rect = TUI_RECT_NONE,
    });
  }

  side_data_t* right_data = malloc(sizeof(side_data_t));

  right_data->input = tui_input_create(tui, 100, right_text);

  right_text->head.data = right_data->input;

  right_data->list  = tui_list_create(tui, right->is_vertical);

  for (size_t index = 0; index < right->child_count; index++)
  {
    tui_list_item_add(right_data->list, right->children[index]);
  }

  right->head.data = right_data;
  */


  tui_list_t* list = tui_list_create(tui, panel->is_vertical);

  tui_list_item_add(list, (tui_window_t*) left);

  tui_list_item_add(list, (tui_window_t*) grid);

  panel->head.data = list;


  // tui_menu_set(tui, menu);
  
  tui_window_set(tui, (tui_window_t*) grid);


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
