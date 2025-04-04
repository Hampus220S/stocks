# Notes
- create tui_window_set and tui_menu_set
- tui_menu_set should immediatly call tui_window_set
- create struct of event instead of just one handler
  - key (existing one)
  - enter
  - exit
  - resize
- tui_window_set should call old_window.event.exit and new_window.event.enter
