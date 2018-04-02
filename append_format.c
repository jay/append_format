/* append_format - Append a separator and formatted data to a string.

https://github.com/jay/append_format

LICENSE: FreeBSD license
Copyright (C) 2016 Jay Satiro <raysatiro@yahoo.com>
See LICENSE.txt for full license text.
*/

#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "append_format.h"

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* append a separator (sep) and formatted data to *str

append_format(&msg, "%s", "foo");
append_sep_format(&msg, "; ", "%s", "foo");
append_rmCRLFs_format(&msg, "%s", "asdf"));
append_rmCRLFs_sep_format(&msg, "; ", "%s", "asdf"));

str must be a pointer to a pointer or NULL.
*str must be a C-runtime heap-allocated string or NULL.
sep must be a pointer or NULL.
format and additional args are the same as snprintf.

sep is ignored if *str is NULL or empty "" OR the format outcome to append is
empty "". flags can alter this behavior.

*str is reallocated by this function and the address it points to may change.

Flags
-----
AF_REMOVE_CR_LF_BEFORE_APPEND:     Remove all trailing CR and LF from *str
                                   BEFORE appending to it.

AF_REMOVE_CR_LF_AFTER_APPEND:      Remove all trailing CR and LF from *str
                                   AFTER appending to it.

AF_REMOVE_CR_LF_BEFORE_AND_AFTER_APPEND:    Both of the above. append_rmCRLFs
                                            function-like macros use this flag.

AF_APPEND_SEP_IF_STR_EMPTY:        Append the separator even if *str before
                                   append is NULL or empty "".

AF_APPEND_SEP_IF_FORMAT_EMPTY:     Append the separator even if the format
                                   outcome to append is empty "".

AF_APPEND_SEP_ALWAYS:              Both of the above.

AF_ALL_FLAGS:                      All flags. This value will change as flags
                                   are added.

success: the new length of *str (or if !str then the length *str would've been)
failure: -1: vsnprintf/memory error; the content of *str is unchanged but if
             the realloc was successful then the location may have changed
failure: -2: unrecognized flag; the content and location of *str is unchanged
*/
int append_flags_sep_format(char **str, int flags, const char *sep,
                            const char *format, ...)
{
  int count;
  va_list args;
  char *buf;
  size_t bufsize;
  size_t oldlen, seplen, crlflen;
  int retcode = -1;
  char *placeholder = NULL;

  /* Unrecognized flags should be checked before anything else and return -2 */
  if((flags & ~AF_ALL_FLAGS)) {
    retcode = -2;
    goto cleanup;
  }

  if(!str)
    str = &placeholder;

  va_start(args, format);
#ifdef _WIN32
  count = _vscprintf(format, args);
#else
  count = vsnprintf(NULL, 0, format, args);
#endif
  va_end(args);

  if(count < 0 || (unsigned)count != (size_t)count)
    goto cleanup;

  if(sep &&
     ((*str && **str) || (flags & AF_APPEND_SEP_IF_STR_EMPTY)) &&
     (count || (flags & AF_APPEND_SEP_IF_FORMAT_EMPTY))) {
    seplen = strlen(sep);
  }
  else {
    sep = "";
    seplen = 0;
  }

  oldlen = *str ? strlen(*str) : 0;

  bufsize = 1;

  bufsize += oldlen;
  if(bufsize < oldlen)
    goto cleanup;

  bufsize += seplen;
  if(bufsize < seplen)
    goto cleanup;

  bufsize += (size_t)count;
  if(bufsize < (size_t)count)
    goto cleanup;

  if(bufsize > (unsigned)INT_MAX)
    goto cleanup;

  buf = (char *)realloc(*str, bufsize);
  if(!buf)
    goto cleanup;

  *str = buf;

  crlflen = 0;

  if((flags & AF_REMOVE_CR_LF_BEFORE_APPEND) && oldlen) {
    char *p = &buf[oldlen];
    do {
      --p;
      if(*p != '\r' && *p != '\n')
        break;
      ++crlflen;
    } while(p != buf);
    /* Move to slack space, if vsnprintf fails we will need to restore them */
    memmove(&buf[bufsize - crlflen], &buf[oldlen - crlflen], crlflen);
  }

  strcpy(&buf[oldlen - crlflen], sep);

  va_start(args, format);
#ifdef _WIN32
  count = _vsnprintf(
#else
  count = vsnprintf(
#endif
    &buf[oldlen - crlflen + seplen], (size_t)(count + 1), format, args);
  va_end(args);

  if(count != (int)(bufsize - oldlen - seplen - 1)) {
    memmove(&buf[oldlen - crlflen], &buf[bufsize - crlflen], crlflen);
    buf[oldlen] = '\0';
    goto cleanup;
  }

  if((flags & AF_REMOVE_CR_LF_AFTER_APPEND) && (bufsize - crlflen - 1)) {
    char *p = &buf[bufsize - crlflen - 1];
    do {
      --p;
      if(*p != '\r' && *p != '\n')
        break;
      ++crlflen;
    } while(p != buf);
    buf[bufsize - crlflen - 1] = '\0';
  }

  retcode = (int)(bufsize - crlflen - 1);
cleanup:
  free(placeholder);
  return retcode;
}
