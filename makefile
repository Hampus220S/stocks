#
# makefile for stocks
#
# Written by Hampus Fridholm
#

.PHONY: stocks apt-packages stocks-dir

default: apt-packages stocks-dir stocks

APT_PACKAGES := libjson-c-dev libcurl4-openssl-dev libncurses-dev

STOCKS_DIR := $(HOME)/.stocks

# Target for initializing stocks directory
stocks-dir:
	@echo "Checking if directory $(STOCKS_DIR) exists..."
	@if [ ! -d "$(STOCKS_DIR)" ]; then \
		echo "Directory $(STOCKS_DIR) does not exist. Creating it..."; \
		mkdir -p "$(STOCKS_DIR)"; \
		cp "stocks.txt" "$(STOCKS_DIR)/stocks.txt"; \
	else \
		echo "Directory $(STOCKS_DIR) already exists."; \
	fi

# Target for installing apt packages
apt-packages:
	@echo "Checking and installing missing packages..."
	@for package in $(APT_PACKAGES); do \
		if ! dpkg -l | grep -q $$package; then \
			echo "Package $$package not found. Installing..."; \
			sudo apt-get update && sudo apt-get install -y $$package; \
		else \
			echo "Package $$package already installed."; \
		fi \
	done

COMPILE_FLAGS := -Wall -g -O0 -std=gnu99 -oFast -Wno-missing-braces
LINKER_FLAGS  := -lm -lncursesw -lcurl -ljson-c

stocks: stocks.c tui.h stock.h debug.h
	@echo "Compiling stocks program"
	gcc stocks.c $(COMPILE_FLAGS) $(LINKER_FLAGS) -o $@

# Target for removing stocks from computer
remove:
	@echo "Checking if directory $(STOCKS_DIR) exists..."
	@if [ ! -d "$(STOCKS_DIR)" ]; then \
		echo "Directory $(STOCKS_DIR) does not exist. Do nothing..."; \
	else \
		echo "Directory $(STOCKS_DIR) exists. Removing it..."; \
		rm -r $(STOCKS_DIR); \
	fi
	@echo "Removing stocks program"
	@-rm 2>/dev/null stocks
