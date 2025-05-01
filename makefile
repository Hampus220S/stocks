#
# makefile for stocks
#
# Written by Hampus Fridholm
#

ifneq ($(shell uname), Linux)
	$(error Stocks is only available for Linux)
endif

.PHONY: apt-packages stocks-dir app

default: apt-packages stocks-dir stocks app

APT_PACKAGES := libjson-c-dev libcurl4-openssl-dev libncurses-dev

STOCKS_DIR := $(HOME)/.stocks

# Target for initializing stocks directory
stocks-dir:
	@echo "Checking if directory $(STOCKS_DIR) exists..."
	@if [ ! -d $(STOCKS_DIR) ]; then \
		echo "Directory $(STOCKS_DIR) does not exist. Creating it..."; \
		mkdir -p $(STOCKS_DIR); \
		cp stocks.txt $(STOCKS_DIR)/stocks.txt; \
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

APP_FILE := $(HOME)/.local/share/applications/stocks.desktop

# Target for creating desktop application
app:
	@echo "Checking if application exists...";
	@if [ ! -e $(APP_FILE) ]; then \
		echo "Application does not exist. Creating it..."; \
		sed -i 's@\/full\/path\/to\/repo@$(shell pwd)@g' stocks.desktop; \
		chmod +x stocks.desktop; \
		cp stocks.desktop $(APP_FILE); \
	else \
		echo "Application already exist."; \
	fi

COMPILE_FLAGS := -Wall -g -O0 -std=gnu99 -oFast -Wno-missing-braces
LINKER_FLAGS  := -lm -lncursesw -lcurl -ljson-c

stocks: stocks.c tui.h stock.h debug.h
	@echo "Compiling stocks program"
	gcc stocks.c $(COMPILE_FLAGS) $(LINKER_FLAGS) -o $@

# Target for removing stocks from computer
remove:
	@if [ -d $(STOCKS_DIR) ]; then \
		echo "Removing directory $(STOCKS_DIR)..."; \
		rm -r $(STOCKS_DIR); \
	fi
	@if [ -e stocks ]; then \
		echo "Removing stocks program..."; \
		rm stocks; \
	fi
	@if [ -e $(APP_FILE) ]; then \
		echo "Removing desktop application..."; \
		rm $(APP_FILE); \
	fi
