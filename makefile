COMPILER := gcc

COMPILE_FLAGS := -Wall -g -O0 -std=gnu99 -oFast -Wno-missing-braces
LINKER_FLAGS := -lm -lncursesw -lcurl -ljson-c

stocks: stocks.c tui.h stock.h debug.h
	$(COMPILER) stocks.c $(COMPILE_FLAGS) $(LINKER_FLAGS) -o $@

clean:
	-rm 2>/dev/null stocks
