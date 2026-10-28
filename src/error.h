/**
 * @file   error.h
 * @author Martin Huleja (xhulejm00)
 * @date   2026-10-28
 *
 * @brief  Header file for error codes and error message
 */

#ifndef ERROR_H
#define ERROR_H

typedef enum error_code {
  ERROR_OK = 0,
  ERROR_LEXICAL = 1,
  ERROR_SYNTACTIC = 2,
  ERROR_UNDEFINED = 3,
  ERROR_REDEFINITION = 4,
  ERROR_FUNCTION_ARGS = 5,
  ERROR_TYPE_COMPATIBILITY = 6,
  ERROR_IMMUTABLE_ASSIGNMENT = 7,
  ERROR_SEMANTIC_OTHER = 10,
  ERROR_INTERNAL = 99
} ErrorCode;

/**
 * @brief Prints formatred error message to stderr
 *
 * Behaves similarly to fprintf().
 * Prints "Error <ErrorCode>: " followed by entered text to stderr.
 *
 * @note Written as part of 1st IJC project in March 2026
 *
 * @param err_code error code defined in ErrorCode enum
 * @param fmt error message
 * @param ... any number of arguments specifying wildcards in fmt
 */
void print_err(ErrorCode err_code, const char *fmt, ...);

#endif // ERROR_H

/*** End of file error.h ***/
