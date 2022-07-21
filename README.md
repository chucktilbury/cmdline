# cmdline
Simple command line parser for programs written in C.

This is intended to be a very simple and easy to use command line parser library routine. It is a little like Puthon's argparse library, but much simpler. 

## Parameter format
Parameters are a single characer preceeded by a '-'. No attempt is made to be compliant with any other library or specification. This is bare and simple. A parameter is followed by an optional space. For example ```-v10``` is equivalent to ```-v 10```. Command parameters may not be combined as with the GNU libraries. This library expects to create a list of files and any parameters that are not recignized are added to the file list. If you have some parameters that are not formalized by this library, you can add them and parse them manually. 

## Types of args
These should be fairly self-explanitory. 
- Number
- String
- Toggle (i.e. a switch)
- Callback with a parameter
- Callback without a parameter

See the example(s) for more information.
