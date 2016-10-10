/* Tests for append_format.

append_format - Append a separator and formatted data to a string.

https://github.com/jay/append_format

LICENSE: FreeBSD license unless otherwise noted below.
Copyright (C) 2016 Jay Satiro <raysatiro@yahoo.com>
See LICENSE.txt for full license text.

LICENSE: libcurl license (MIT/X derivate)
This license covers the dump function taken from curl's docs/examples/debug.c
Copyright (C) 1998 - 2016, Daniel Stenberg, <daniel@haxx.se>, et al.
See LICENSE.txt for full license text.
*/

/*
To run the tests open append_format.sln and run the 'Debug' configuration, or:
g++ -Wall -Wextra -ggdb3 -I.. -o test_append_format test_append_format.cpp ../append_format.c
cl /W4 /MDd /Zi /I.. /D_CRTDBG_MAP_ALLOC test_append_format.cpp ../append_format.c /link /INCREMENTAL:NO
*/

#undef NDEBUG   /* always assert */

#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <assert.h>
#if defined(__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "append_format.h"

using namespace std;

#if defined(_WIN32) && !defined(_MSC_VER)
#warning "To enable the most extensive debugging in Windows use MSVC."
#endif

#ifndef MS_INLINE_PRAGMA
#if (_MSC_VER >= 1300)
#define MS_INLINE_PRAGMA(x)   __pragma(x)
#else
#define MS_INLINE_PRAGMA(x)
#endif
#endif

#ifdef _MSC_VER
#define BREAK_OR_ABORT() __debugbreak()
#elif defined(__GNUC__)
#define BREAK_OR_ABORT() __asm__ volatile("int $0x03")
#else
#define BREAK_OR_ABORT() abort()
#endif

#define ASSERT_BREAK(expr, msg) \
MS_INLINE_PRAGMA(warning(push)) \
MS_INLINE_PRAGMA(warning(disable:4127)) \
  if(!(expr)) { \
    ok = false; \
    std::string filename_d_(__FILE__); \
    size_t pos_d_ = filename_d_.find_last_of( "\\/" ); \
    if(pos_d_ != std::string::npos) { \
        filename_d_.erase(0, pos_d_ + 1); \
    } \
    std::stringstream ss_d_; \
    ss_d_ << "ERROR: Assertion failed: " << #expr << std::endl << filename_d_ \
          << ":" << __LINE__ << " , " << __FUNCTION__ << "(): " << msg; \
    std::cerr << std::endl << "\a\a" << ss_d_.str() << std::endl; \
    BREAK_OR_ABORT(); \
  } \
MS_INLINE_PRAGMA(warning(pop))

#ifdef _WIN32
#define MALLOC_SIZE(x) _msize((x))
#elif defined(__GNUC__)
#define MALLOC_SIZE(x) malloc_usable_size((x))
#elif defined(__APPLE__)
#define MALLOC_SIZE(x) malloc_size((x))
#else
#error "not implemented"
#endif

/* data dump function is from curl's docs/examples/debug.c */
static
void dump(const char *text,
          FILE *stream, unsigned char *ptr, size_t size,
          char nohex)
{
  size_t i;
  size_t c;

  unsigned int width=0x10;

  if(nohex)
    /* without the hex output, we can fit more on screen */
    width = 0x40;

  fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
          text, (long)size, (long)size);

  for(i=0; i<size; i+= width) {

    fprintf(stream, "%4.4lx: ", (long)i);

    if(!nohex) {
      /* hex not disabled, show it */
      for(c = 0; c < width; c++)
        if(i+c < size)
          fprintf(stream, "%02x ", ptr[i+c]);
        else
          fputs("   ", stream);
    }

    for(c = 0; (c < width) && (i+c < size); c++) {
      /* check for 0D0A; if found, skip past and start a new line of output */
      if(nohex && (i+c+1 < size) && ptr[i+c]==0x0D && ptr[i+c+1]==0x0A) {
        i+=(c+2-width);
        break;
      }
      fprintf(stream, "%c",
              (ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.');
      /* check again for 0D0A, to avoid an extra \n if it's at width */
      if(nohex && (i+c+2 < size) && ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A) {
        i+=(c+3-width);
        break;
      }
    }
    fputc('\n', stream); /* newline */
  }
  fflush(stream);
}

/* valid testing states for param 'char **str' */
enum state_str {
  STATE_STR_IS_NULL,                  /* str is NULL */
  STATE_STR_DEREF_IS_NULL,            /* str is != NULL but *str is NULL */
  STATE_STR_DEREF_DEREF_IS_EMPTY,     /* *str points to an empty string "" */
  STATE_STR_DEREF_DEREF_NORMAL,       /* *str points to a non-empty string that
                                         DOES NOT end in a trailing cr or lf */
  STATE_STR_DEREF_DEREF_NORMAL_CRLF,  /* *str points to a non-empty string that
                                         DOES end in a trailing cr or lf */
  STATE_STR_LAST
};

/* valid testing states for param 'const char *sep' */
enum state_sep {
  STATE_SEP_IS_NULL,                /* sep is NULL */
  STATE_SEP_DEREF_IS_EMPTY,         /* sep points to an empty string "" */
  STATE_SEP_DEREF_IS_NORMAL,        /* sep points to a non-empty string that
                                       DOES NOT end in a trailing cr or lf */
  STATE_SEP_DEREF_IS_NORMAL_CRLF,   /* sep points to a non-empty string that
                                       DOES end in a trailing cr or lf */
  STATE_SEP_LAST
};

/* valid testing states for param 'const char *format, ...' */
enum state_format {
  STATE_FORMAT_DEREF_IS_EMPTY,      /* format points to an empty string "" */

  /* format points to a non-empty string that DOES NOT end in a trailing cr or
     lf, and without any arg specifier such as %s */
  STATE_FORMAT_DEREF_IS_NORMAL,

  /* format points to a non-empty string that DOES end in a trailing cr or lf,
     and without any arg specifier such as %s */
  STATE_FORMAT_DEREF_IS_NORMAL_CRLF,

  /* format points to a non-empty string that DOES NOT end in a trailing cr or
     lf, and with only one arg specifier %s */
  STATE_FORMAT_DEREF_IS_NORMAL_WITH_ARG,

  /* format points to a non-empty string that DOES end in a trailing cr or lf,
     and with only one arg specifier %s */
  STATE_FORMAT_DEREF_IS_NORMAL_WITH_ARG_CRLF,

  STATE_FORMAT_LAST
};

void trim_all_trailing_cr_or_lf(char *x)
{
  size_t xlen = x ? strlen(x) : 0;

  if(!xlen)
    return;

  char *clip = NULL;
  char *p = &x[xlen];

  do {
    --p;
    if(*p != '\r' && *p != '\n')
      break;
    clip = p;
  } while(p != x);

  if(clip)
    *clip = '\0';
}

/* runtests uses these in various combinations to create the test parameters */
struct test_content {
  const char *string;
  const char *sep;
  const char *format_no_arg;
  const char *format_with_arg;
  const char *arg;
  const char *some_trailing_crlfs;
};

bool runtests(struct test_content *content, const char *specific_test_to_run)
{
  /* this function's return value. set false by ASSERT_BREAK. */
  bool ok = true;

  /* invalid flag test */
  assert(!(APPEND_FLAGS_ALL & 0x80000000));
  assert(-2 == append_flags_sep_format(NULL, 0x80000000, NULL, ""));

  int str_state = 0;
  char **str = NULL;
  char *str_placeholder = NULL; /* for many tests str = &str_placeholder */

  int sep_state = 0;
  char *sep = NULL;

  int format_state = 0;
  char *format = NULL;
  const char *format_arg = NULL;

  int flags = 0;

  /* set in the debugger */
  bool run_current_test_repeatedly = false;

  if(specific_test_to_run) {
    ASSERT_BREAK(4 == sscanf(specific_test_to_run,"%d.%d.%d.%d",
                             &str_state, &sep_state, &format_state, &flags),
                 "");
    ASSERT_BREAK(0 <= str_state && str_state < STATE_STR_LAST, "");
    ASSERT_BREAK(0 <= sep_state && sep_state < STATE_SEP_LAST, "");
    ASSERT_BREAK(0 <= format_state && format_state < STATE_FORMAT_LAST, "");
    ASSERT_BREAK(0 <= flags && flags <= APPEND_FLAGS_ALL, "");
  }

  for(bool firstrun = true; ; firstrun = false) {
#ifdef _CRTDBG_MAP_ALLOC
    ASSERT_BREAK(_CrtCheckMemory(), "heap corruption");
#endif

    free(format);
    format = NULL;

    free(sep);
    sep = NULL;

    free(str_placeholder);
    str_placeholder = NULL;

    if(firstrun)
      ;
    else if(!run_current_test_repeatedly) {
      if(specific_test_to_run)
        break;

      if(flags++ == APPEND_FLAGS_ALL) {
        flags = 0;

        if(++format_state == STATE_FORMAT_LAST) {
          format_state = 0;

          if(++sep_state == STATE_SEP_LAST) {
            sep_state = 0;

            if(++str_state == STATE_STR_LAST) {
              break;
            }
          }
        }
      }
    }

    /* skip this iteration if it contains a flag that doesn't exist */
    if(flags != (flags & APPEND_FLAGS_ALL))
      continue;

    switch(format_state) {
    default:
      assert(0); /* all states must be implemented */

    case STATE_FORMAT_DEREF_IS_EMPTY:
      format_arg = NULL;
      format = (char *)malloc(1);
      *format = '\0';
      break;

    case STATE_FORMAT_DEREF_IS_NORMAL:
      if(!strlen(content->format_no_arg))
        continue;
      format_arg = NULL;
      format = (char *)malloc(strlen(content->format_no_arg) + 1);
      strcpy(format, content->format_no_arg);
      break;

    case STATE_FORMAT_DEREF_IS_NORMAL_CRLF:
      if(!strlen(content->some_trailing_crlfs))
        continue;
      format_arg = NULL;
      format = (char *)malloc(strlen(content->format_no_arg) +
                              strlen(content->some_trailing_crlfs) + 1);
      strcpy(format, content->format_no_arg);
      strcat(format, content->some_trailing_crlfs);
      break;

    case STATE_FORMAT_DEREF_IS_NORMAL_WITH_ARG:
      if(!strlen(content->format_with_arg))
        continue;
      format_arg = content->arg;
      format = (char *)malloc(strlen(content->format_with_arg) + 1);
      strcpy(format, content->format_with_arg);
      break;

    case STATE_FORMAT_DEREF_IS_NORMAL_WITH_ARG_CRLF:
      if(!strlen(content->some_trailing_crlfs))
        continue;
      format_arg = content->arg;
      format = (char *)malloc(strlen(content->format_with_arg) +
                              strlen(content->some_trailing_crlfs) + 1);
      strcpy(format, content->format_with_arg);
      strcat(format, content->some_trailing_crlfs);
      break;
    }

    switch(sep_state) {
    default:
      assert(0); /* all states must be implemented */

    case STATE_SEP_IS_NULL:
      sep = NULL;
      break;

    case STATE_SEP_DEREF_IS_EMPTY:
      sep = (char *)malloc(1);
      *sep = '\0';
      break;

    case STATE_SEP_DEREF_IS_NORMAL:
      if(!strlen(content->sep))
        continue;
      sep = (char *)malloc(strlen(content->sep) + 1);
      strcpy(sep, content->sep);
      break;

    case STATE_SEP_DEREF_IS_NORMAL_CRLF:
      if(!strlen(content->some_trailing_crlfs))
        continue;
      sep = (char *)malloc(strlen(content->sep) +
                           strlen(content->some_trailing_crlfs) + 1);
      strcpy(sep, content->sep);
      strcat(sep, content->some_trailing_crlfs);
      break;
    }

    str = (char **)0xAAAAAAAA; /* failsafe */

    switch(str_state) {
    default:
      assert(0); /* all states must be implemented */

    case STATE_STR_IS_NULL:
      str = NULL;
      break;

    case STATE_STR_DEREF_IS_NULL:
      str = &str_placeholder;
      *str = NULL;
      break;

    case STATE_STR_DEREF_DEREF_IS_EMPTY:
      str = &str_placeholder;
      *str = (char *)malloc(1);
      **str = '\0';
      break;

    case STATE_STR_DEREF_DEREF_NORMAL:
      if(!strlen(content->string))
        continue;
      str = &str_placeholder;
      *str = (char *)malloc(strlen(content->string) + 1);
      strcpy(*str, content->string);
      break;

    case STATE_STR_DEREF_DEREF_NORMAL_CRLF:
      if(!strlen(content->some_trailing_crlfs))
        continue;
      str = &str_placeholder;
      *str = (char *)malloc(strlen(content->string) +
                            strlen(content->some_trailing_crlfs) + 1);
      strcpy(*str, content->string);
      strcat(*str, content->some_trailing_crlfs);
      break;
    }

    fprintf(stderr, "Running test %d.%d.%d.%d\n", str_state, sep_state,
            format_state, flags);

    char expected[1024];
    memset(expected, 0xAA, sizeof expected);
    expected[0] = '\0';

    if(str && *str && **str)
      strcat(expected, *str);

    if((flags & APPEND_REMOVE_CR_LF_BEFORE))
      trim_all_trailing_cr_or_lf(expected);

    char formatted_outcome_to_append[1024];
    memset(formatted_outcome_to_append, 0xAA,
           sizeof formatted_outcome_to_append);
    formatted_outcome_to_append[0] = '\0';

    ASSERT_BREAK(0 <= sprintf(formatted_outcome_to_append, format, format_arg),
                 "sprintf failed");

    /* the separator isn't used if either *str is NULL or empty OR the
       formatted outcome to append is empty. */
    if(sep && *sep &&
       (*expected || (flags & APPEND_SEP_IF_STR_EMPTY)) &&
       (*formatted_outcome_to_append ||
        (flags & APPEND_SEP_IF_FORMAT_EMPTY))) {
      strcat(expected, sep);
    }

    strcat(expected, formatted_outcome_to_append);

    if((flags & APPEND_REMOVE_CR_LF_AFTER))
      trim_all_trailing_cr_or_lf(expected);

    size_t expectedlen = strlen(expected);

    int ret = append_flags_sep_format(str, flags, sep, format, format_arg);

    ASSERT_BREAK(ret >= 0, "append_flags_sep_format error " << ret);

    ASSERT_BREAK(str_state == STATE_STR_IS_NULL || *str,
                 "*str is NULL, that is unexpected when str");

    if(str_state != STATE_STR_IS_NULL &&
       memcmp(*str, expected, expectedlen + 1)) {
      dump("*str", stderr, (unsigned char *)*str, MALLOC_SIZE(*str), 0);
      dump("expected", stderr, (unsigned char *)expected,
           sizeof expected, 0);

      ASSERT_BREAK(0, "*str compare first " << expectedlen + 1 <<
                      " bytes to expected");
    }

    ASSERT_BREAK(expectedlen == (size_t)ret, expectedlen << " != " << ret);
  }

  return ok;
}

int main(int argc, char *argv[])
{
  /* set crtdbg options before anything else */
#ifdef _CRTDBG_MAP_ALLOC
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
  // For now use the default _CRT_ASSERT to get the message box to select debug
  // https://msdn.microsoft.com/en-us/library/ezb1wyez.aspx
  //_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
  //_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
  _CrtSetDbgFlag(
    _CRTDBG_ALLOC_MEM_DF |       /* Enable debug heap allocations */
    _CRTDBG_DELAY_FREE_MEM_DF);  /* This flag keeps track of freed memory, but
                                    as a result the heap keeps growing. Since
                                    our tests are not memory intensive this is
                                    fine for now. */
#endif // _CRTDBG_MAP_ALLOC

  (void)argc;

  /* If this is NULL then all tests are run */
  const char *specific_test = argv[1];

  /* EXIT_SUCCESS when true. set false by ASSERT_BREAK. */
  bool ok = true;

  /* Tests set these NULL or empty when necessary. All content must be != NULL.
     Also: None except 'some_trailing_crlfs' should have trailing cr or lf
           because the tests will add that when necessary.
     Also: The expected outcome should be less than 1024 since a fixed-size
           buffer is used for testing.
   */
  struct test_content content;

  memset(&content, 0, sizeof content);
  content.string = "foo";
  content.sep = "; ";
  content.format_no_arg = "bar";
  content.format_with_arg = "_%s_";         /* must contain only one % of %s */
  content.arg = "qux";
  content.some_trailing_crlfs = "\r\n\r";   /* must contain 1+ of \r and/or \n
                                               in any order */
  ok = ok && runtests(&content, specific_test);

#ifdef _CRTDBG_MAP_ALLOC
  ASSERT_BREAK(_CrtCheckMemory(), "heap corruption");
  ASSERT_BREAK(!_CrtDumpMemoryLeaks(), "memory leak");
#endif
  fprintf(stderr, "%s\n", ok ? "EXIT_SUCCESS" : "EXIT_FAILURE");
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
