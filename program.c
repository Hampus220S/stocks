/*
 * program.c
 */

#define DEBUG_IMPLEMENT
#include "debug.h"

/*
 * Main function
 */
int main(int argc, char* argv[])
{
  debug_file_open("debug.log");

  debug_file_close();

  return 0;
}
