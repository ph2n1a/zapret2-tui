#ifndef INCLUDE_UTILS_UTILS_H_
#define INCLUDE_UTILS_UTILS_H_

#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>

void collapse_spaces(char* str);
void remove_newlines(char* str);
int copy_file(const char *src, const char *dst);
int file_exists_in_dir(const char *dir, const char *filename);
int compare_files(const char *path1, const char *path2);
bool is_root();

#endif  // INCLUDE_UTILS_UTILS_H_
