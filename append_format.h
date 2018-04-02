/* append_format - Append a separator and formatted data to a string.

https://github.com/jay/append_format

LICENSE: FreeBSD license
Copyright (C) 2016 Jay Satiro <raysatiro@yahoo.com>
See LICENSE.txt for full license text.
*/

#ifndef APPEND_FORMAT_H
#define APPEND_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* append a separator (sep) and formatted data to *str.
   Documented in the comment block above the function definition. */
int append_flags_sep_format(char **str, int flags, const char *sep,
                            const char *format, ...);

/* Remove all trailing CR and LF from *str BEFORE appending to it */
#define AF_REMOVE_CR_LF_BEFORE_APPEND   (1<<0)

/* Remove all trailing CR and LF from *str AFTER appending to it */
#define AF_REMOVE_CR_LF_AFTER_APPEND    (1<<1)

/* Remove all trailing CR and LF from *str BEFORE and AFTER appending to it */
#define AF_REMOVE_CR_LF_BEFORE_AND_AFTER_APPEND \
  (AF_REMOVE_CR_LF_BEFORE_APPEND | \
   AF_REMOVE_CR_LF_AFTER_APPEND)

/* Append the separator even if *str before append is NULL or empty "" */
#define AF_APPEND_SEP_IF_STR_EMPTY      (1<<2)

/* Append the separator even if the format outcome to append is empty "" */
#define AF_APPEND_SEP_IF_FORMAT_EMPTY   (1<<3)

/* Append the separator even if *str is NULL or empty "" OR the format outcome
   to append is empty "" */
#define AF_APPEND_SEP_ALWAYS \
  (AF_APPEND_SEP_IF_STR_EMPTY | \
   AF_APPEND_SEP_IF_FORMAT_EMPTY)

#define AF_ALL_FLAGS \
  (AF_REMOVE_CR_LF_BEFORE_APPEND | \
   AF_REMOVE_CR_LF_AFTER_APPEND | \
   AF_APPEND_SEP_IF_STR_EMPTY | \
   AF_APPEND_SEP_IF_FORMAT_EMPTY)

/* same as append_flags_sep_format but no flags */
#define append_sep_format(str, sep, format, ...) \
  append_flags_sep_format(str, 0, sep, format, __VA_ARGS__)

/* same as append_sep_format but remove all trailing CR and LF from *str before
   and after appending to it */
#define append_rmCRLFs_sep_format(str, sep, format, ...) \
  append_flags_sep_format(str, AF_REMOVE_CR_LF_BEFORE_AND_AFTER_APPEND, \
                          sep, format, __VA_ARGS__)

/* same as append_flags_sep_format but no flags or separator */
#define append_format(str, format, ...) \
  append_flags_sep_format(str, 0, NULL, format, __VA_ARGS__)

/* same as append_format but remove all trailing CR and LF from *str before and
   after appending to it */
#define append_rmCRLFs_format(str, format, ...) \
  append_flags_sep_format(str, AF_REMOVE_CR_LF_BEFORE_AND_AFTER_APPEND, \
                          NULL, format, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // APPEND_FORMAT_H
