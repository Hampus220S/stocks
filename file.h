/*
 * file.h - file read and write functions
 *
 * Written by Hampus Fridholm
 *
 * Last updated: 2025-05-01
 *
 *
 * char*  path_clean(char* path)
 *
 *
 * size_t file_size_get(const char* filepath)
 * 
 * size_t file_read(void* pointer, size_t size, const char* filepath)
 * 
 * size_t file_write(const void* pointer, size_t size, const char* filepath)
 *
 * int    file_remove(const char* filepath)
 *
 * int    file_rename(const char* old_filepath, const char* new_filepath)
 *
 *
 * size_t file_lines_read(char*** lines, size_t size, const char* filepath)
 *
 * void   file_lines_free(char*** lines, size_t count)
 *
 * 
 * int    files_get(char*** files, size_t* count, const char* path, int depth)
 *
 * size_t files_size_get(char** files, size_t count)
 *
 * size_t files_read(void* pointer, size_t size, char** files, size_t count)
 *
 * void   files_free(char** files, size_t count)
 * 
 * 
 * size_t dir_file_size_get(const char* dirpath, const char* name)
 * 
 * size_t dir_file_write(const void* pointer, size_t size, const char* dirpath, const char* name)
 * 
 * size_t dir_file_read(void* pointer, size_t size, const char* dirpath, const char* name)
 *
 * int    dir_file_remove(const char* dirpath, const char* name)
 *
 * int    dir_file_rename(const char* dirpath, const char* old_name, const char* new_name)
 */

/*
 * From here on, until FILE_IMPLEMENT,
 * it is like a normal header file with declarations
 */

#ifndef FILE_H
#define FILE_H

#include <stddef.h>

#define TYPE_NONE 0
#define TYPE_FILE 1
#define TYPE_DIR  2
#define TYPE_ELSE 3

extern char*  path_clean(char* path);


extern size_t file_size_get(const char* filepath);

extern size_t file_read(void* pointer, size_t size, const char* filepath);

extern size_t file_write(const void* pointer, size_t size, const char* filepath);

extern int    file_remove(const char* filepath);

extern int    file_rename(const char* old_filepath, const char* new_filepath);


extern size_t file_lines_read(char*** lines, size_t size, const char* filepath);

extern void   file_lines_free(char*** lines, size_t count);


extern int    files_get(char*** files, size_t* count, const char* path, int depth);

extern size_t files_size_get(char** files, size_t count);

extern size_t files_read(void* pointer, size_t size, char** files, size_t count);

extern void   files_free(char** files, size_t count);


extern size_t dir_file_size_get(const char* dirpath, const char* name);

extern size_t dir_file_write(const void* pointer, size_t size, const char* dirpath, const char* name);

extern size_t dir_file_read(void* pointer, size_t size, const char* dirpath, const char* name);

extern int    dir_file_remove(const char* dirpath, const char* name);

extern int    dir_file_rename(const char* dirpath, const char* old_name, const char* new_name);

#endif // FILE_H

/*
 * This header library file uses _IMPLEMENT guards
 *
 * If FILE_IMPLEMENT is defined, the definitions will be included
 */

#ifdef FILE_IMPLEMENT

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * Get number of bytes in file
 *
 * PARAMS
 * - const char* filepath | Path to file
 *
 * RETURN (size_t size)
 * - 0  | Failed to open file, or file is empty
 * - >0 | Number of bytes in file
 */
size_t file_size_get(const char* filepath)
{
  FILE* stream = fopen(filepath, "rb");

  if (!stream) return 0;

  fseek(stream, 0, SEEK_END); 

  size_t size = ftell(stream);

  fseek(stream, 0, SEEK_SET); 

  fclose(stream);

  return size;
}

/*
 * Remove file
 *
 * PARAMS
 * - const char* filepath | Path to file
 *
 * RETURN (same as remove)
 */
int file_remove(const char* filepath)
{
  return remove(filepath);
}

/*
 * Rename, or move, file
 *
 * PARAMS
 * - const char* old_filepath | Old path to file
 * - const char* new_filepath | New path to file
 *
 * RETURN (same as rename)
 */
int file_rename(const char* old_filepath, const char* new_filepath)
{
  return rename(old_filepath, new_filepath);
}

/*
 * Read a number of bytes from file to memory at pointer
 *
 * PARAMS
 * - void*       pointer  | Pointer to memory buffer
 * - size_t      size     | Number of bytes to read
 * - const char* filepath | Path to file
 *
 * RETURN (same as fread, size_t read_size)
 * - 0  | Failed to read file, or bad input
 * - >0 | Number of read bytes
 */
size_t file_read(void* pointer, size_t size, const char* filepath)
{
  if (!pointer) return 0;

  FILE* stream = fopen(filepath, "rb");

  if (!stream) return 0;

  size_t read_size = fread(pointer, 1, size, stream);

  fclose(stream);

  return read_size;
}

/*
 * Write a number of bytes from memory at pointer to file
 *
 * PARAMS
 * - const void* pointer  | Pointer to memory to read from
 * - size_t      size     | Number of bytes to write
 * - const char* filepath | Path to file
 *
 * RETURN (same as fwrite, size_t write_size)
 * - 0  | Failed to open file or bad pointer
 * - >0 | The number of written bytes
 */
size_t file_write(const void* pointer, size_t size, const char* filepath)
{
  if (!pointer) return 0;

  FILE* stream = fopen(filepath, "wb");

  if (!stream) return 0;

  size_t write_size = fwrite(pointer, 1, size, stream);

  fclose(stream);

  return write_size;
}

/*
 * Free lines read from file
 *
 * Set the pointer to NULL afterwards
 */
void file_lines_free(char*** lines, size_t count)
{
  if (!lines || !(*lines)) return;

  for (size_t index = 0; index < count; index++)
  {
    free((*lines)[index]);
  }

  free(*lines);

  *lines = NULL;
}

/*
 * Read lines from file seperated by a new-line character (\n)
 *
 * The allocated memory must be freed by the user
 *
 * RETURN (size_t count)
 * -  0 | Failed to read lines
 * - >0 | Number of lines
 */
size_t file_lines_read(char*** lines, size_t size, const char* filepath)
{
  char* buffer = malloc(sizeof(char) * (size + 1));

  if (!buffer) return 0;

  if (file_read(buffer, size, filepath) == 0)
  {
    free(buffer);

    return 0;
  }

  size_t count = 0;

  char* token = strtok(buffer, "\n");

  while (token)
  {
    char** temp_lines = realloc(*lines, sizeof(char*) * (count + 1));

    if (!temp_lines)
    {
      free(buffer);

      file_lines_free(lines, count);

      return 0;
    }

    *lines = temp_lines;

    (*lines)[count++] = strdup(token);

    token = strtok(NULL, "\n");
  }

  free(buffer);

  return count;
}

/*
 * Get the names of the files in directory
 *
 * PARAMS
 * - char**      names   | Names of files
 * - size_t*     count   | Number of names
 * - const char* dirpath | Path to directory
 * 
 * RETURN (int status)
 * - 0 | Success
 * - 1 | Failed to open directory
 */
int dir_file_names_get(char*** names, size_t* count, const char* dirpath)
{
  struct dirent* dire;

  DIR* dirp = opendir(dirpath);

  if (!dirp) return 1;

  while ((dire = readdir(dirp)) != NULL)
  {
    if (strcmp(dire->d_name, ".")  == 0 ||
        strcmp(dire->d_name, "..") == 0 ||
        dire->d_type != DT_REG)
    {
      continue;
    }

    char** new_names = realloc(*names, sizeof(char*) * (*count + 1));

    if (!new_names)
    {
      closedir(dirp);

      return 1;
    }

    *names = new_names;

    (*names)[(*count)++] = strdup(dire->d_name);
  }

  closedir(dirp);

  return 0;
}

/*
 * Read from file inside directory
 *
 * PARAMS
 * - void*       pointer | Pointer to memory to write to
 * - size_t      size    | Number of bytes to read
 * - const char* dirpath | Path to directory
 * - const char* name    | Name of file
 *
 * RETURN (same as fread, size_t read_size)
 * - 0  | Failed to read file, or bad input
 * - >0 | Number of read bytes
 */
size_t dir_file_read(void* pointer, size_t size, const char* dirpath, const char* name)
{
  size_t path_size = strlen(dirpath) + 1 + strlen(name);

  char filepath[path_size + 1];

  sprintf(filepath, "%s/%s", dirpath, name);

  return file_read(pointer, size, filepath);
}

/*
 * Write to file inside directory
 *
 * PARAMS
 * - const void* pointer | Pointer to memory to read from
 * - size_t      size    | Number of bytes to write
 * - const char* dirpath | Path to directory
 * - const char* name    | Name of file
 *
 * RETURN (same as fwrite, size_t write_size)
 * - 0  | Failed to open file or bad pointer
 * - >0 | The number of written bytes
 */
size_t dir_file_write(const void* pointer, size_t size, const char* dirpath, const char* name)
{
  size_t path_size = strlen(dirpath) + 1 + strlen(name);

  char filepath[path_size + 1];

  sprintf(filepath, "%s/%s", dirpath, name);

  return file_write(pointer, size, filepath);
}

/*
 * Get size of file inside directory
 *
 * PARAMS
 * - const char* dirpath | Path to directory
 * - const char* name    | Name of file
 *
 * RETURN (size_t size)
 * - 0  | Failed to open file, or file is empty
 * - >0 | Number of bytes in file
 */
size_t dir_file_size_get(const char* dirpath, const char* name)
{
  size_t path_size = strlen(dirpath) + 1 + strlen(name);

  char filepath[path_size + 1];

  sprintf(filepath, "%s/%s", dirpath, name);

  return file_size_get(filepath);
}

/*
 * Remove file in directory
 *
 * PARAMS
 * - const char* dirpath | Path to directory
 * - const char* name    | Name of file
 *
 * RETURN (same as remove)
 */
int dir_file_remove(const char* dirpath, const char* name)
{
  size_t path_size = strlen(dirpath) + 1 + strlen(name);

  char filepath[path_size + 1];

  sprintf(filepath, "%s/%s", dirpath, name);

  return file_remove(filepath);
}

/*
 * Rename, or move, file in directory
 *
 * PARAMS
 * - const char* dirpath  | Path to directory
 * - const char* old_name | Old name of file
 * - const char* new_name | New name of file
 *
 * RETURN (same as rename)
 */
int dir_file_rename(const char* dirpath, const char* old_name, const char* new_name)
{
  size_t old_path_size = strlen(dirpath) + 1 + strlen(old_name);

  char old_filepath[old_path_size + 1];

  sprintf(old_filepath, "%s/%s", dirpath, old_name);


  size_t new_path_size = strlen(dirpath) + 1 + strlen(new_name);

  char new_filepath[new_path_size + 1];

  sprintf(new_filepath, "%s/%s", dirpath, new_name);


  return file_rename(old_filepath, new_filepath);
}

/*
 * Split a string at deliminator into multiple smaller string parts
 *
 * PARAMS
 * - char** string_array | array to store splitted strings to
 *   - the array should be allocated with size >= split_count
 *
 * RETURN (size_t count)
 * - number of strings successfully splitted
 */
static inline size_t string_split_at_delim(char** string_array, const char* string, const char* delim, size_t split_count)
{
  if (!string_array || !string) return 0;

  if (split_count <= 0) return 0;

  char string_copy[strlen(string) + 1]; 
  strcpy(string_copy, string);

  char* string_token = NULL;
  
  string_token = strtok(string_copy, delim);

  size_t index;

  for (index = 0; (index < split_count) && string_token; index++)
  {
    string_array[index] = strdup(string_token);

    string_token = strtok(NULL, delim);
  }

  return index;
}

/*
 * Get the number of the specified symbol that are in string
 *
 * RETURN (size_t count)
 */
static inline size_t string_symbol_count_get(const char* string, char symbol)
{
  size_t count = 0;

  for (size_t index = 0; string[index] != '\0'; index++)
  {
    if (string[index] == symbol) count++;
  }
  
  return count;
}

/*
 * Free the string array created by string_split_at_delim
 */
static inline void string_array_free(char** string_array, size_t count)
{
  if (!string_array) return;

  for (size_t index = 0; index < count; index++)
  {
    char* string = string_array[index];

    if (string) free(string);
  }
}

/*
 * Filter the strings in a path, by excluding unnecessary directories
 *
 * RETURN (size_t new_count)
 * - new count of strings, after filtering
 */
static inline size_t path_strings_filter(char** string_array, size_t count)
{
  size_t filter_count = 0;

  for (size_t index = 0; index < count; index++)
  {
    char* string = string_array[index];

    if (strcmp(string, ".") == 0 ||
        strcmp(string, "")  == 0)
    {
      continue;
    }

    if (string != string_array[filter_count])
    {
      strcpy(string_array[filter_count], string);
    }

    filter_count++;
  }

  return filter_count;
}

/*
 * Concatonate path strings to create the cleaned path
 *
 * RETURN (char* path)
 */
static inline char* path_strings_concat(char** string_array, size_t count, char* path)
{
  memset(path, '\0', sizeof(char) * strlen(path));

  for (size_t index = 0; index < count; index++)
  {
    char* string = string_array[index];

    if (index > 0) strcat(path, "/");

    strcat(path, string);
  }

  return path;
}

/*
 * Clean path from unnecessary /-symbols and other junk
 *
 * This is done by splitting up the path and
 * putting the valuable pieces back
 */
char* path_clean(char* path)
{
  size_t dir_count = string_symbol_count_get(path, '/');

  char* string_array[dir_count + 1];

  size_t split_count = string_split_at_delim(string_array, path, "/", dir_count + 1);

  // 1. Filter out the directory names to keep
  size_t filter_count = path_strings_filter(string_array, split_count);

  // 2. Concatonate directory names and file name to path
  path = path_strings_concat(string_array, filter_count, path);

  string_array_free(string_array, split_count);

  return path;
}

/*
 * Concatonate directory path and child name to get full path
 * 
 * If the dirpath is the current directory, just keep the child name
 *
 * PARAMS
 * - const char* dirpath    | Path to directory
 * - const char* child_name | Name of child (either file or dir)
 *
 * EXPECT
 * - both dirpath and child_name are allocated
 *
 * RETURN (char* fullpath)
 *
 * Note: Remember to free the allocated memory
 */
static inline char* full_path_create(const char* dirpath, const char* child_name)
{
  size_t size = strlen(dirpath) + strlen(child_name) + 2;

  char* fullpath = malloc(sizeof(char) * size);

  sprintf(fullpath, "%s/%s", dirpath, child_name);

  return fullpath;
}

static inline int dir_files_get(char*** files, size_t* count, const char* dirpath, int depth);

/*
 * Allocate files in directory child - either a file or a new directory
 *
 * PARAMS
 * - char***     files      | Pointer to filepaths
 * - size_t*     count      | Number of files
 * - const char* dirpath    | Path to directory
 * - int         child_type | Type of child - file or dir
 * - const char* child_name | Name of child - filename or dirname
 * - int         depth      | Search depth limit
 *
 * RETURN (int file_amount)
 * -  0 | Path not to file or dir
 * - >0 | Number of files
 */
static inline int dir_child_files_get(char*** files, size_t* count, const char* dirpath, int child_type, const char* child_name, int depth)
{
  switch (child_type)
  {
    case DT_REG:
      char** new_files = realloc(*files, sizeof(char*) * (*count + 1));

      if (!new_files) return 0;

      *files = new_files;

      (*files)[(*count)++] = full_path_create(dirpath, child_name);

      return 1;

    case DT_DIR:
      char* fullpath = full_path_create(dirpath, child_name);

      int new_depth = (depth == -1) ? -1 : (depth - 1);

      int file_amount = dir_files_get(files, count, fullpath, new_depth);

      free(fullpath);

      return file_amount;

    default: return 0;
  }
}

/*
 * Get and allocate array of files in directory, recursivly
 *
 * PARAMS
 * - char***     files   | Pointer to filepaths
 * - size_t*     count   | Number of files
 * - const char* dirpath | Path to directory
 * - int         depth   | Search depth limit
 * 
 * Note:
 * If the depth is -1, search indefinitely
 * If the depth is  0, stop searching
 *
 * RETURN (int file_amount)
 * -  0 | Path not to dir, bad input or no files
 * - >0 | Number of files
 */
static inline int dir_files_get(char*** files, size_t* count, const char* dirpath, int depth)
{
  if (depth == 0 || depth < -1) return 0;

  struct dirent* dire;

  DIR* dirp = opendir(dirpath);

  if (!dirp) return 0;

  int file_amount = 0;

  while ((dire = readdir(dirp)) != NULL)
  {
    // If the child name starts with a dot, it should not be read
    if (dire->d_name[0] == '.') continue;

    file_amount += dir_child_files_get(files, count, dirpath, dire->d_type, dire->d_name, depth);
  }

  closedir(dirp);

  return file_amount;
}

/*
 * Determine the type of path, either dir or file
 *
 * PARAMS
 * - const char* path | Path to some kind of file or dir
 *
 * RETURN (int type)
 * - TYPE_NONE | Path doesn't exist
 * - TYPE_FILE | File
 * - TYPE_DIR  | Directory
 * - TYPE_ELSE | Something else
 */
static inline int path_type_get(const char* path)
{
  struct stat pstat;

  if (stat(path, &pstat) == -1) return TYPE_NONE;

  switch (pstat.st_mode & S_IFMT)
  {
    case S_IFREG: return TYPE_FILE;

    case S_IFDIR: return TYPE_DIR;

    case S_IFBLK: case S_IFCHR: case S_IFIFO: case S_IFLNK: case S_IFSOCK:
      return TYPE_ELSE;

    default: return TYPE_NONE;
  }
}

/*
 * Get and allocate array of paths to either file or dir
 *
 * PARAMS
 * - char***     files | Pointer to filepaths
 * - size_t*     count | Number of files
 * - const char* path  | Path to either file or dir
 * - int         depth | Search depth limit
 *
 * RETURN (int status)
 * -  0 | Bad input, or path is neither file or dir
 * - >0 | Number of files
 */
int files_get(char*** files, size_t* count, const char* path, int depth)
{
  if (!files || !count || !path) return 0;

  switch (path_type_get(path))
  {
    case TYPE_FILE:
      char** new_files = realloc(*files, sizeof(char*) * (*count + 1));

      if (!new_files) return 0;

      *files = new_files;

      (*files)[(*count)++] = strdup(path);

      return 1;

    case TYPE_DIR:
      return dir_files_get(files, count, path, depth);

    default: return 0;
  }
}

/*
 * Get the combined size of the files
 *
 * PARAMS
 * - char** files | Filepaths
 * - size_t count | Number of files
 *
 * RETURN (size_t size)
 */
size_t files_size_get(char** files, size_t count)
{
  if (!files) return 0;

  size_t size = 0;

  for (size_t index = 0; index < count; index++)
  {
    size += file_size_get(files[index]);
  }

  return size;
}

/*
 * Read and concatonate multiple files into single memory at pointer
 *
 * If the total read data would overwrite the buffer,
 * only read the amount of data that the buffer can hold
 *
 * PARAMS
 * - void*  pointer | Pointer to memory buffer
 * - size_t size    | Number of bytes to read
 * - char** files   | Filepaths
 * - size_t count   | Number of files
 *
 * RETURN (size_t read_size)
 * - 0  | Bad input, or files are empty
 * - >0 | Number of total read bytes
 */
size_t files_read(void* pointer, size_t size, char** files, size_t count)
{
  if (!pointer || !files) return 0;

  size_t read_size = 0;

  for (size_t index = 0; index < count; index++)
  {
    size_t file_size = file_size_get(files[index]);

    // If the current file would fill up the buffer,
    // read the last available bytes and return
    if (read_size + file_size > size)
    {
      read_size += file_read(pointer + read_size, size - read_size, files[index]);
      
      break;
    }

    read_size += file_read(pointer + read_size, file_size, files[index]);
  }

  return read_size;
}

/*
 * Free the pointers in the files string array
 *
 * PARAMS
 * - char** files | Filepaths
 * - size_t count | Number of files
 */
void files_free(char** files, size_t count)
{
  if (!files) return;

  for (size_t index = 0; index < count; index++)
  {
    char* file = files[index];

    if (file) free(file);
  }

  free(files);
}

#endif // FILE_IMPLEMENT
