/**
 * @file   error.c
 * @author Martin Huleja (xhulejm00)
 * @date   2026-10-28
 *
 * @brief  Implementation of error reporting function
 */

#include "error.h"
#include <stdarg.h>
#include <stdio.h>

void print_err(ErrorCode err_code, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  // Print error message
  fprintf(stderr, "Error(%d): ", err_code);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");

  va_end(args);
}

/*** End of file error.c ***/
