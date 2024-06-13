# CMDLINE
Command line parser for C using the getopt(3) library routine.

This is intended to be a robust command line parser with all of the features that one may need to implement a compiler. The core of this implementation is the POSIX getopt_long() function. It is also influenced by the Python optarg library. 

The idea is to augment the getopt function to implement a help screen and separate the actual command line options from the library routines. It also does a little extra error checking around retrieving things like a list of files from the command line.

The peripheral routines are taken from other project of mine and I consider them to be general-purpose library routines. 

## Build
Simply type ``make`` and if you have any ANSI C compiler installed the test programs should be made. There are no dependencies other than the getopt(3) library. This library should be completely portable to any operating system with no changes. However, if handling file names under Windows is a requirement, then some specific routines could be added to handle manipulating paths to the str.c module.

There are 3 programs to test the buffers, the pointer lists, and the command line parser. The string routines are closely tied to the buffer routines, and so testing them separately is less important.

## Use
This is basically a POSIX/GNU command line parser fit features added to make it more like what I want. Both long and short options are supported and the format of those options are the same. However, we have added a few features such as separating the definition of the command line from the code that parses it. This allows us to also do things like add type checking to options. To learn how to use the library routines, there is a test program at the end of the cmdline.c file that demonstrates most of the features. You can also check the getopts(3) man page for more information.
