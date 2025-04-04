/*
 * program.c
 */

#define TUI_IMPLEMENT
#include "tui.h"

#define DEBUG_IMPLEMENT
#include "debug.h"

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
    tui_window_set(panel->tui, list->windows[list->index]);

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

typedef struct side_data_t
{
  tui_input_t* input;
  tui_list_t*  list;
} side_data_t;

void side_window_enter(tui_window_t* window)
{
  window->color.bg = TUI_COLOR_RED;

  side_data_t* data = window->data;

  tui_list_t* list = data->list;

  tui_window_set(window->tui, list->windows[list->index]);
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

  info_print("trying list");

  tui_list_t* list = data->list;

  if (tui_list_event(list, key))
  {
    info_print("list event done");
    tui_window_t* child = list->windows[list->index];

    tui_window_set(head->tui, child);
  }

  return false;
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
    .has_padding = false,
    .event.key = &panel_window_key,
    .event.enter = &panel_window_enter,
  });

  tui_window_parent_t* left = tui_parent_child_parent_create(panel, (tui_window_parent_config_t)
  {
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_BLUE,
    },
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
    .event.enter = &side_window_enter,
    .event.exit = &side_window_exit,
    .event.key = &side_window_key,
  });

  char* left_strings[] =
  {
    "banana",
    "ballong",
    "seven",
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
  }

  side_data_t* left_data = malloc(sizeof(side_data_t));

  left_data->input = tui_input_create(100, NULL);

  left_data->list = tui_list_create(left->is_vertical, left->children, left->child_count);

  left->head.data = left_data;


  tui_window_parent_t* right = tui_parent_child_parent_create(panel, (tui_window_parent_config_t)
  {
    .color = (tui_color_t)
    {
      .bg = TUI_COLOR_BLUE,
    },
    .rect = TUI_RECT_NONE,
    .is_vertical = true,
    .event.enter = &side_window_enter,
    .event.exit = &side_window_exit,
    .event.key = &side_window_key,
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
  }

  side_data_t* right_data = malloc(sizeof(side_data_t));

  right_data->input = tui_input_create(100, NULL);
  right_data->list  = tui_list_create(right->is_vertical, right->children, right->child_count);

  right->head.data = right_data;


  tui_list_t* list = tui_list_create(panel->is_vertical, panel->children, panel->child_count);

  panel->head.data = list;


  tui_menu_set(tui, menu);


  info_print("Starting tui");

  tui_start(tui);

  info_print("Stopping tui");

  tui_stop(tui);


  tui_list_delete(&left_data->list);

  tui_input_delete(&left_data->input);

  free(left_data);


  tui_list_delete(&right_data->list);

  tui_input_delete(&right_data->input);

  free(right_data);


  tui_delete(&tui);

  info_print("Deleted TUI");


  tui_quit();

  info_print("Quitted TUI");

  debug_file_close();

  return 0;
}
