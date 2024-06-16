/**
 * @file test.c
 * 
 * @brief Test the command line. Play with this to understand how to use it.
 * 
 * @author Chuck Tilbury (chucktilbury@gmail.com)
 * @version 0.0
 * @date 2024-06-16
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdio.h>

#include "cmdline.h"

// examples:
//
//  ./test_cmd  -b:asdasd asdsds --bump=123123123 -a=123,345,456 --add=098098
// outputs:
//  veto: false
//  plow: 8086
//  verbose: 0
//  bump: 123123123
//  one: false
//  two: false
//  three: false
//  add: '123', '345', '456', '098098'
//  files: 'asdsds'
//
// ./test_cmd -12 --add plastic,metal glass leather
// outputs:
//  veto: false
//  plow: 8086
//  verbose: 0
//  bump: blouts
//  one: true
//  two: true
//  three: false
//  add: 'plastic', 'metal'
//  files: 'glass', 'leather'


static const char* bool_to_str(bool val) {

    return val? "true": "false";
}

/**
 * @brief Test/Example program.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char** argv) {

    // the data structures are internal, so there is no need to declare them.
    init_cmdline("This is the testing thing.", "Report bugs to your friends.", "Test Cmdline", "0.0.0");

    // add some command options.
    // If an opt is required then it does not need a default value
    add_cmdline('a', "add", "otters", "Add the things to the otters.", NULL, CMD_STR|CMD_LIST|CMD_RARG|CMD_REQD);

    // If an opt is optional, then should have a default value
    add_cmdline('b', "bump", "bump", "Bump the things with the otters.", "blouts", CMD_STR|CMD_OARG);

    // If the opt is present, then it requires an arg, but the opt is optional.
    add_cmdline('v', NULL, "verbo", "int 0 - 10 default is 0. Set the verbosity.", "0", CMD_NUM|CMD_RARG);

    // An arg is required but the opt is optional. Correct boolean values are "yes", "no", "true", and "false".
    add_cmdline(0, "veto", "veto", "Vote the otters down.", "false", CMD_BOOL|CMD_RARG);

    // get_cmdline_as_num() or get_cmdline_as_str() return the same value with a different type.
    add_cmdline(0, "plow", "plow", "Plow the otter's fields.", "8086", CMD_NUM|CMD_RARG);

    // If these are present on the command line, they are set to true. When a short arg is 
    // specified, then they can be combined such as -12. See examples above.
    add_cmdline('1', NULL, "one", "One of them", NULL, CMD_BOOL);
    add_cmdline('2', NULL, "two", "Two of them", NULL, CMD_BOOL);
    add_cmdline('3', NULL, "three", "Three of them", NULL, CMD_BOOL);

    // add some default command line parameters that are handled in the 
    // parser. These options should not be defined in any other context 
    // without first modifying the parse_cmdline() option parser.
    add_cmdline('V', "version", NULL, "show the name and version", NULL, CMD_NARG|CMD_BOOL);
    add_cmdline('h', "help", NULL, "show this help information", NULL, CMD_NARG|CMD_BOOL);

    // Special list of files has no command option.
    add_cmdline(0, NULL, "list of files", "list of files to be processed", NULL, CMD_STR|CMD_REQD);

    // Actually read the command line and parse the variables.
    parse_cmdline(argc, argv, ALLOW_NOPT);

    // see what we got.
    printf("veto: %s\n", bool_to_str(get_cmdline_as_bool("veto")));
    printf("plow: %s\n", get_cmdline("plow"));
    printf("verbose: %d\n", get_cmdline_as_num("verbo"));
    printf("bump: %s\n", get_cmdline("bump"));

    // stand-alone bools.
    printf("one: %s\n", bool_to_str(get_cmdline_as_bool("one")));
    printf("two: %s\n", bool_to_str(get_cmdline_as_bool("two")));
    printf("three: %s\n", bool_to_str(get_cmdline_as_bool("three")));

    const char* str;
    int post = 0;

    // access list options
    printf("add: ");
    while(NULL != (str = iterate_cmdline("otters", &post)))
        printf("'%s', ", str);
    printf("\b\b \n");

    post = 0;
    printf("files: ");
    while(NULL != (str = iterate_cmdline("list of files", &post)))
        printf("'%s', ", str);
    printf("\b\b \n");

    //show_cmdline_help();
    return 0;
}

