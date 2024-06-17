# CMDLINE
Command line parser for C using the a hand-coded parser.

This is intended to be a robust command line parser with all of the features that one may need to implement a compiler. The core of this implementation a hand-coded parser. The layout and implementation of the parser was havily influenced by the Python optarg library. 

The peripheral routines are taken from other project of mine and I consider them to be general-purpose library routines. 

## Use
This command line parser is intended to be complete and robust for handling almost any situation for getting information from the command line. 

### Formats of options
* Single dash (aka short)
  * These can be combined into a single instance. For example, if there are options defined as 'a', 'g', and '1', then adding ``-1ag`` on the command line is identical to adding ``-a -1 -g``. These options are "switches" that have their state set by simply existing. However, single dash options can also have optional or required arguments. Arguments are always given preceeded by a ``=`` or a ``:`` character. Unlike the GNU and POSIX command line parsers, putting a space between the option and its argument is not allowed. If the ``-g`` option accepts a numeric argument then is may look like ``-g=123``. The other options could be combined to look like ``-1ag=123``. This is identical to ``-1 -a -g:123``. Note that the '=' and the ':' are interchangable. 
* Double dash (aka long)
  * Long options have the form of ``--option_name``. These cannot be combined like the short options, but all other rules defined for short options apply to their syntax. 
* No dash
  * The parser is able to store options that have no dash in front of them, such as file names or other random strings. These are stored internally as a list that can be iterated. 

### Format of option arguments
* Option argument types. If an option is declared as a particular type then it will be screened by the parser to make sure that the argument could be converted from a string to that type. If it can't then the parser publishes a syntax error and prints out the help text to stdout.
  * All option arguments can be retrieved as a string. 
  * Other types besides string
    * Integer. As converted by strtol in base 10.
    * Unsigned. As converted by strtol in base 16.
    * Float. As converted by strtod.
    * Bool. These are not the same as switches. They accept values of 'true', 'false', 'on', and 'off.
    * Quoted strings are copied intact, but with the quotes stripped.
* Lists. If an option is declared as a list, then it can accept values that are separated by a comma ','. There is no real limit on the number of items or their size, except by the maximum command line size defined by the operating system. 

## Build
Simply type ``make`` and if you have any ANSI C compiler installed the test programs should be made. There are no dependencies other than the normal C runtime. This library should be completely portable to any operating system with no changes. However, if handling file names under Windows is a requirement, then some specific routines could be added to handle manipulating paths in the str.c module.


