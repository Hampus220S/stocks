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

  if (tui_list_event(list, key))
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

static inline int grid_stock_y_get(int max, int min, int h, int value)
{
  return (float) h * (float) (value - min) / (float) (max - min);
}

const char* STOCK_RANGES[]    = { "1d", "5d", "1mo", "3mo", "6mo", "1y", "2y" };

const char* STOCK_INTERVALS[] = { "5m", "30m", "90m", "4h", "1d", "5d", "1wk" };


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
static void grid_stock_update(tui_window_t* window)
{
  stock_t* stock = window->data;

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

    window->data = new_stock;
  }
}

/*
 *
 */
void grid_window_free(tui_window_t* head)
{
  tui_window_grid_t* window = (tui_window_grid_t*) head;

  stock_free((stock_t**) &head->data);
}

/*
 *
 */
void grid_window_render(tui_window_t* head)
{
  info_print("Grid render: w:%d h:%d", head->_rect.w, head->_rect.h);
  
  tui_window_grid_t* window = (tui_window_grid_t*) head;

  if (!head->data) return;

  // grid_stock_update(head);

  stock_t* stock = head->data;

  tui_size_t size = { .w = head->_rect.w, .h = head->_rect.h };

  if (tui_window_grid_resize(window, size) != 0)
  {
    error_print("tui_window_grid_resize");
  }

  stock_value_t value = stock->values[stock->value_count - 1];

  int max = value.high;

  int min = value.low;

  int count = (float) head->_rect.w / 2.f;

  for (int index = 1; index < count; index++)
  {
    if (index >= stock->value_count) break;

    stock_value_t value = stock->values[stock->value_count - 1 - index];

    max = MAX(max, value.high + 0.5f);

    min = MIN(min, value.low);
  }

  /*
  info_print("max: %d", max);
  info_print("min: %d", min);
  info_print("high: %d", stock->high);
  info_print("low:  %d", stock->low);
  */

  for (int index = 0; index < count; index++)
  {
    if (index >= stock->value_count) break;

    int x = (head->_rect.w - (index * 2));

    stock_value_t value = stock->values[stock->value_count - 1 - index];

    int close = grid_stock_y_get(max, min, head->_rect.h, value.close);

    int open = grid_stock_y_get(max, min, head->_rect.h, value.open);

    int low = grid_stock_y_get(max, min, head->_rect.h, value.low);

    int high = grid_stock_y_get(max, min, head->_rect.h, value.high);

    int first = MAX(close, open);

    int second = MIN(close, open);

    /*
    info_print("index: %d", index);
    info_print("  high   : %d", high);
    info_print("  first  : %d", first);
    info_print("  second : %d", second);
    info_print("  low    : %d", low);
    */

    short color = (close > open) ? TUI_COLOR_GREEN : TUI_COLOR_RED;

    // Wick
    for (int y = high; y-- > first;)
    {
      tui_window_grid_square_set(window, x, 
          window->_size.h - y - 1, 
          (tui_window_grid_square_t)
      {
        .color.fg = color,
        .symbol = '|',
      });
    }

    // Wick
    for (int y = second; y-- > low;)
    {
      tui_window_grid_square_set(window, x,
          window->_size.h - y - 1, 
          (tui_window_grid_square_t)
      {
        .color.fg = color,
        .symbol = '|',
      });
    }

    // Body
    for (int y = first; y-- >= second;)
    {
      tui_window_grid_square_set(window, x,
          window->_size.h - y - 1, 
          (tui_window_grid_square_t)
      {
        .color.bg = color,
      });
    }
  }
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

  tui_window_parent_t* root2 = tui_window_parent_create(tui, (tui_window_parent_config_t)
  {
    .rect = { 0 },
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
    .event.render = &grid_window_render,
    .data = stock_get("AAPL", "3mo", "4h"),
    .event.free = &grid_window_free,
  });

  char* left_strings[] =
  {
    "banana",
    "ba\033[42ml\33[33mlon\033[0mg",
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

    tui_parent_child_text_create(middle, (tui_window_text_config_t)
    {
      .string = left_strings[index],
      .rect = TUI_RECT_NONE,
    });
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


  tui_list_t* list = tui_list_create(tui, panel->is_vertical);

  tui_list_item_add(list, (tui_window_t*) left);

  tui_list_item_add(list, (tui_window_t*) right);

  panel->head.data = list;


  // tui_menu_set(tui, menu);


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
