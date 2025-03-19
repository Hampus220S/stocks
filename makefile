COMPILER := gcc

OMPILE_FLAGS := -Werror -Wall -g -O0 -std=gnu99 -oFast
LINKER_FLAGS := -lm -lgmp -lncursesw

program: program.c
	$(COMPILER) program.c $(COMPILE_FLAGS) $(LINKER_FLAGS) -o $@

clean:
	-rm 2>/dev/null program
