append_format
=============

append_format - Append a separator and formatted data to a string in C

```c
int append_flags_sep_format(char **str, int flags, const char *sep,
                            const char *format, ...);
```

Example:
```c
  char *str = (char *)strdup("foo");
  if(str) {
    if(append_flags_sep_format(&str, 0, "; ", "%d %s", 123, "abc") >= 0) {
      printf("%s\n", str); /* prints "foo; 123 abc" */
    }
  }
```

In addition to the function above, there are several function-like macros that
wrap it. For illustrative purposes here are those macros as prototypes:

```c
/* same as append_flags_sep_format but no flags or separator */
int append_format(char **str, const char *format, ...);

/* same as append_flags_sep_format but no flags */
int append_sep_format(char **str, const char *sep, const char *format, ...);

/* same as append_sep_format but remove all trailing CR and LF from *str before
   and after appending to it */
int append_rmCRLFs_sep_format(char **str, const char *sep,
                              const char *format, ...);
                              
/* same as append_format but remove all trailing CR and LF from *str before and
   after appending to it */
int append_rmCRLFs_format(char **str, const char *format, ...);
```

append_format.{c,h} can be included in any C or C++ project.


Flags
-----

#### APPEND_REMOVE_CR_LF_BEFORE
Remove all trailing CR and LF from *str BEFORE appending to it.

#### APPEND_REMOVE_CR_LF_AFTER
Remove all trailing CR and LF from *str AFTER appending to it.

#### APPEND_REMOVE_CR_LF_BEFORE_AND_AFTER
Both of the above: Remove all trailing CR and LF from *str BEFORE and AFTER
appending to it.

#### APPEND_SEP_IF_STR_EMPTY
Append the separator even if *str before append is NULL or empty "".

#### APPEND_SEP_IF_FORMAT_EMPTY
Append the separator even if the format outcome to append is empty "".

#### APPEND_SEP_IF_STR_OR_FORMAT_EMPTY
Both of the above: Append the separator even if *str is NULL or empty "" OR the
format outcome to append is empty "".

#### APPEND_FLAGS_ALL
All of the flags. This value will of course change as flags are added or
removed.


Documentation
-------------

Full documentation is in the comment block above `append_flags_sep_format` in
[append_format.c](https://github.com/jay/append_format/blob/master/append_format.c).


Other
-----

### Testing

test_append_format is a Visual Studio solution that will test various
permutations of append_format. If a test fails two cpu beeps (\a\a) are sent
and then an attempt is made to break (or abort if that's not possible).

### License

append_format is licensed under the
[FreeBSD license](http://en.wikipedia.org/wiki/BSD_licenses#2-clause)
which is also referred to as the 2-clause BSD license or simplified BSD
license. It is a non-restrictive GPL-compatible free software license. It is
similar to the MIT license and is closed source permissive.

test_append_format is under the same FreeBSD license unless otherwise stated.
Currently there is only one exception, the 'dump' function is from libcurl
which uses an MIT/X derivate license.

You may not remove my copyright or the copyright of any contributors under the
terms of either license. For full license terms please review
[LICENSE.txt](https://github.com/jay/append_format/blob/master/LICENSE.txt).

### Source

The source can be found on [GitHub](https://github.com/jay/append_format).
Since you're reading this maybe you're already there?

### Send me any questions you have

Jay Satiro `<raysatiro$at$yahoo{}com>` and put append_format in the subject.
