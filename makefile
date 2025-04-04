COMPILER := gcc

COMPILE_FLAGS := -Wall -g -O0 -std=gnu99 -oFast
LINKER_FLAGS := -lm -lgmp -lncursesw

program: program.c tui.h
	$(COMPILER) program.c $(COMPILE_FLAGS) $(LINKER_FLAGS) -o $@

clean:
	-rm 2>/dev/null program
