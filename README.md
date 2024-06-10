# cmdline
Command line parser for C using the getopt() library routine.

This is intended to be a robust command line parser with all of the features that one may need to implement a compiler. The core of this implementation is the POSIX getopt_long() function. It is also influenced by the Python optarg library. 

The idea is to augment the getopt function to implement a help screen and separate the actual command line options from the library routines. It also does a little extra error checking around retrieving things like a list of files from the command line.

The peripheral routines are taken from other project of mine and I consider them to be general-purpose library routines. 

## Build
Simply type ``make`` and if you have any ANSI C compiler installed the test programs should be made. There are 3 programs to test the buffers, the pointer lists, and the command line parser. The string routines are closely tied to the buffer routines, and so testing them separately is less important.

## Use
To find out how to use the GNU/POSIX command line parser, take a look at the man page for getopt(3). Also, just about every GNU app uses this. Run the test program and play with the command line that was implemented there. That should give sufficient direction on how to implement your own.

## TODO
There are some enhansements that could be done. Mostly around retrieving a list from the command line options. I would like to implement a syntax such as ``--opt=first:second:third`` where this creates a list of 3 items for this option that can be iterated. Also, if a option appears more than once in the command line, and lists are enabled for that option, then the arg is added to the list. As things are, the value is simply replaced.

