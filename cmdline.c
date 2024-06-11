/**
 * @file cmdline.c
 * 
 * @brief Use the GNU command line parser getopt_long() to parse a command 
 * line with a fre enhansements.
 * 
 * @author Chuck Tilbury (chucktilbury@gmail.com)
 * @version 0.0
 * @date 2024-06-10
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>

#include "ptr_lst.h"
#include "str.h"
#include "memory.h"
#include "myassert.h"
#include "buffer.h"
#include "cmdline.h"

typedef Buffer _cmd_opts_t_;

typedef struct {
    int short_opt;
    const char* long_opt;
    const char* name;
    const char* help;
    //const char* value;
    StrLst* values;
    int flag; 
} _cmd_opt_t_;

typedef struct {
    const char* prog;
    const char* name;
    const char* version;
    const char* intro;
    const char* outtro;
    PtrLst* cmd_opts;
    _cmd_opts_t_* options;
    String* sopts; 
    //StrLst* non_opts;
    int flag;
    int min_reqd;
} _cmdline_t_;

static _cmdline_t_* cmdline = NULL;

/**
 * @brief Show an error message and then show the help message and then
 * exit the program.
 * 
 * @param fmt 
 * @param ... 
 */
static void error(const char* fmt, ...) {

    va_list args;

    fprintf(stderr, "\nCMD ERROR: ");

    const char* format = (fmt[0] == '+')? &fmt[1]: fmt;
    va_start(args, fmt);
    vfprintf(stderr, format, args);
    va_end(args);

    if(fmt[0] != '+') {
        fputs("\n", stderr);
        show_cmdline_help();
    }
}

/**
 * @brief Search for a short option in the command list.
 * 
 * @param c 
 * @return _cmd_opt_t_* 
 */
static _cmd_opt_t_* search_short(int c) {

    int post = 0;
    _cmd_opt_t_* ptr;
    while(NULL != (ptr = iterate_ptr_lst(cmdline->cmd_opts, &post))) {
        if(ptr->short_opt == c)
            return ptr;
    }

    return NULL;
}

/**
 * @brief Search for a long option in the command list.
 * 
 * @param opt 
 * @return _cmd_opt_t_* 
 */
static _cmd_opt_t_* search_long(const char* opt) {

    int post = 0;
    _cmd_opt_t_* ptr;
    while(NULL != (ptr = iterate_ptr_lst(cmdline->cmd_opts, &post))) {
        if(!strcmp(ptr->long_opt, opt))
            return ptr;
    }

    return NULL;
}

/**
 * @brief Search for a name in the command list.
 * 
 * @param name 
 * @return _cmd_opt_t_* 
 */
static _cmd_opt_t_* search_name(const char* name) {

    int post = 0;
    _cmd_opt_t_* ptr;
    while(NULL != (ptr = iterate_ptr_lst(cmdline->cmd_opts, &post))) {
        if(!strcmp(ptr->name, name))
            return ptr;
    }

    return NULL;
}

/**
 * @brief Search for an option that has no name in the command list.
 * 
 * @return _cmd_opt_t_* 
 */
static _cmd_opt_t_* search_no_name() {

    int post = 0;
    _cmd_opt_t_* ptr;
    while(NULL != (ptr = iterate_ptr_lst(cmdline->cmd_opts, &post))) {
        if(ptr->short_opt == 0 && strlen(ptr->long_opt) == 0)
            return ptr;
    }

    return NULL;
}

/**
 * @brief If it's a list, then there are ',' characters in it. Use strtok() to
 * iterate the string for tokens and save each one in the given list.
 * 
 * @param lst 
 * @param str 
 */
static void parse_str_lst(StrLst* lst, const char* str) {

    char* ptr = (char*)str;
    char *save, *token;

    while(true) {
        token = strtok_r(ptr, ",", &save);
        if(NULL == token)
            break;
        else {
            //printf("HERE!\n");
            append_str_lst(lst, create_string(token));
        }
        ptr = NULL;
    }
}

// static void show_str_list(StrLst* lst) {

//     int post = 0;
//     String* str;
//     while(NULL != (str = iterate_str_lst(lst, &post)))
//         printf("\"%s\"\n", raw_string(str));
// }

/******************************************************************************
 * 
 * Public Interface
 * 
 */

/**
 * @brief Initialize the command line. Allocates data structures and fills in 
 * what we have.
 * 
 * @param intro 
 * @param outtro 
 * @param name 
 * @param version 
 */
void init_cmdline(const char* intro, const char* outtro, 
                    const char* name, const char* version) {

    _cmdline_t_* ptr = _ALLOC_DS(_cmdline_t_);
    ptr->prog = NULL;
    ptr->intro = _COPY_STR(intro);
    ptr->outtro = _COPY_STR(outtro);
    ptr->name = _COPY_STR(name);
    ptr->version = _COPY_STR(version);
    ptr->cmd_opts = create_ptr_lst();
    ptr->sopts = create_string(NULL);
    append_string_str(ptr->sopts, "-:");
    //ptr->non_opts = create_str_lst();
    ptr->flag = 0;
    ptr->min_reqd = 0;

    struct option* opt = _ALLOC_DS(struct option); // allocate and init to 0
    ptr->options = (_cmd_opts_t_*)create_buffer(opt, sizeof(struct option));

    cmdline = ptr;
}

/**
 * @brief Free all memeory associated with the data structure.
 * 
 */
void uninit_cmdline() {

    if(cmdline != NULL) {
        if(cmdline->prog != NULL)
            _FREE(cmdline->prog);
        if(cmdline->intro != NULL)
            _FREE(cmdline->intro);
        if(cmdline->outtro != NULL)
            _FREE(cmdline->outtro);
        if(cmdline->name != NULL)
            _FREE(cmdline->name);
        if(cmdline->version != NULL)
            _FREE(cmdline->version);

        if(cmdline->cmd_opts != NULL) {
            int post = 0;
            _cmd_opt_t_* ptr;
            while(NULL != (ptr = iterate_ptr_lst(cmdline->cmd_opts, &post))) {
                if(ptr->name != NULL)
                    _FREE(ptr->name);
                if(ptr->help != NULL)
                    _FREE(ptr->help);
                if(ptr->long_opt != NULL)
                    _FREE(ptr->long_opt);
                if(ptr->values != NULL)
                    destroy_str_lst(ptr->values);
                _FREE(ptr);
            }
            destroy_ptr_lst(cmdline->cmd_opts);
        }

        // note to self: order of these operations is important
        if(cmdline->sopts != NULL)
            destroy_string(cmdline->sopts);
        if(cmdline->options != NULL)
            destroy_buffer((Buffer*)cmdline->options);
        // if(cmdline->non_opts != NULL)
        //     destroy_str_lst(cmdline->non_opts);
        _FREE(cmdline);
    }
}

/**
 * @brief Add an option to the command line parser. Do this before parsing 
 * the command line. The short opt must not be zero or -1. If it's a printable
 * character as given by isgraph(), then it's a cmd arg and listed as such. 
 * Otherwise it's used in the switch statement to discover the arg and it 
 * should be a value over 255. Practice is to use a value over 1000.
 * 
 * @param short_opt 
 * @param long_opt 
 * @param name 
 * @param help 
 * @param flag
 */
void add_cmdline(int short_opt, const char* long_opt,
                    const char* name, const char* help, 
                    const char* value, CmdType flag) {
    
    ASSERT_MSG(cmdline != NULL, "init the cmdline data structure before calling this.");

    // handle the short options.
    if(isgraph(short_opt)) 
        append_string_char(cmdline->sopts, short_opt);

    // set up the long options and the values
    const char* val;
    int has_arg;
    if(flag & CMD_RARG) {
        // argument is required
        append_string_char(cmdline->sopts, ':');
        has_arg = required_argument;
        val = _COPY_STR(value);
    }
    else if(flag & CMD_OARG) {
        // optional argument
        append_string_char(cmdline->sopts, ':');
        append_string_char(cmdline->sopts, ':');
        has_arg = optional_argument;
        val = _COPY_STR(value);
    }
    else {
        // no argument
        has_arg = no_argument;
        val = NULL;
    }
    
    if(flag & CMD_REQD)
        cmdline->min_reqd++;

    // handle the long options
    struct option* opt = _ALLOC_DS(struct option);
    opt->name = _COPY_STR(long_opt);
    opt->has_arg = has_arg;
    opt->flag = NULL; // always return the arg
    opt->val = short_opt;

    prepend_buffer(cmdline->options, opt, sizeof(struct option));

    // capture the help and all.
    _cmd_opt_t_* ptr = _ALLOC_DS(_cmd_opt_t_);
    ptr->short_opt = short_opt;
    ptr->long_opt = opt->name;
    ptr->help = _COPY_STR(help);
    ptr->name = _COPY_STR(name);
    ptr->values = create_str_lst();
    ptr->flag = flag;
    if(value != NULL)
        append_str_lst(ptr->values, create_string(val));

    append_ptr_lst(cmdline->cmd_opts, ptr);
}

/**
 * @brief Read the command line and fill out the data structure with the 
 * options. If the flag is non-zero then non-options are errors. 
 * 
 * Globals defined by getopt:
 * optind - The current index into the command line
 * opterr - If this is set to 0 then getopt does not print error messages. 
 *          Otherwise, getopt does print them by default.
 * 
 * @param argc 
 * @param argv 
 * @param flag
 */
void parse_cmdline(int argc, char** argv, int flag) {

    ASSERT_MSG(cmdline != NULL, "init the cmdline data structure before calling this.");

    cmdline->prog = _COPY_STR(argv[0]);
    cmdline->flag = flag;

    if(argc <= cmdline->min_reqd) 
        error("at least %d command arguments are required.", cmdline->min_reqd);

    const char* optstr = raw_string(cmdline->sopts);
    struct option* opts = (struct option*)raw_buffer(cmdline->options);
    int opt, idx = 0;
    bool finished = false;
    while(!finished) {
        opt = getopt_long(argc, argv, optstr, opts, &idx);
        if(opt >= 0) {
            if(opt == '?') {
                // have an unknown arg.
                error("unknown argument: '%s'.", argv[optind]);
            }
            else if(opt == ':') {
                // missing command argument
                error("argument '%s' requires a value.", argv[optind-1]);
            }
            else if(opt == 'h') {
                // help arg (doesn't return)
                show_cmdline_help();
            }
            else if(opt == 'V') {
                // just show the version, performing no other processing
                printf("\n%s: v%s\n", cmdline->name, cmdline->version);
                exit(1);
            }
            else if(opt == 1) {
                // accept non-arg entries such as file names
                _cmd_opt_t_* op = search_no_name();
                ASSERT_MSG(op != NULL, "internal error no name opt not found");
                op->flag |= CMD_SEEN;

                if(cmdline->flag == ALLOW_NOPT) {
                    append_str_lst(op->values, create_string(optarg));
                }
                else {
                    error("unexpected name on command line: %s", optarg);
                }
            }
            else if(opt == 0) {
                // have a long arg with no corresponding short arg.
                _cmd_opt_t_* op = search_long(opts[idx].name);
                ASSERT_MSG(op != NULL, "internal error long opt not found: '%s'", opts[idx].name);
                op->flag |= CMD_SEEN;

                if(optarg) {
                    if(op->flag & CMD_LIST) 
                        parse_str_lst(op->values, optarg);
                    else {
                        clear_str_lst(op->values);
                        append_str_lst(op->values, create_string(optarg));
                    }
                }
            }
            else {
                // have a normal arg.
                _cmd_opt_t_* op = search_short(opt);
                ASSERT_MSG(op != NULL, "internal error short opt not found: '%c'", opt);
                op->flag |= CMD_SEEN;

                if(optarg) {
                    char* val = (optarg[0] == ':' || optarg[0] == '=')? &optarg[1] : optarg;
                    if(op->flag & CMD_LIST) 
                        parse_str_lst(op->values, val);
                    else {
                        clear_str_lst(op->values);
                        append_str_lst(op->values, create_string(val));
                    }
                }
            }
        }
        else 
            finished = true;
    }

    // verify that all of the required options have a value
    int post = 0;
    _cmd_opt_t_* op;
    while(NULL != (op = iterate_ptr_lst(cmdline->cmd_opts, &post))) {
        if((op->flag & CMD_REQD) && (!(op->flag & CMD_SEEN) || (op->values->len == 0))) {
            if(op->short_opt != 0)
                error("required command parameter '-%c' missing.", op->short_opt);
            else if(strlen(op->long_opt) > 0)
                error("required command parameter '--%s' missing.", op->long_opt);
            else 
                error("required command parameter '%s' missing.", op->name);
        }
    }

}

/**
 * @brief Iterate the option, if it's a list. Otherwise, just get it.
 * 
 * @param name 
 * @param post 
 * @return const char* 
 */
String* iterate_cmdline(const char* name, int* post) {

    _cmd_opt_t_* opt = search_name(name);
    ASSERT_MSG(opt != NULL, "cannot find the option searched for: %s", name);

    return iterate_str_lst(opt->values, post);
}

/**
 * @brief Retrieve a command line option from the data structure. If it's a 
 * list, just keep retrieving the first option.
 * 
 * @param name 
 * @return const char* 
 */
String* get_cmdline(const char* name) {

    _cmd_opt_t_* opt = search_name(name);
    ASSERT_MSG(opt != NULL, "cannot find the option searched for: %s", name);

    int post = 0;
    return iterate_cmdline(name, &post);
}

/**
 * @brief Show the help message and exit the program.
 * 
 */
void show_cmdline_help() {

    printf("\nUsage: %s [options]", cmdline->prog);
    if(!cmdline->flag)
        printf(" files\n");
    else 
        printf("\n\n");

    printf("%s v%s\n%s\n\n", cmdline->name, cmdline->version, cmdline->intro);
    printf("Options:\n");
    int post = 0;
    _cmd_opt_t_* ptr;
    printf("  Parm             Args      Help\n");
    printf("--------------------------------------------------------------------------\n");
    while(NULL != (ptr = iterate_ptr_lst(cmdline->cmd_opts, &post))) {
        if(isgraph(ptr->short_opt) || strlen(ptr->long_opt) > 0) {
            if(isgraph(ptr->short_opt))
                printf("  -%c ", ptr->short_opt);
            else
                printf("     ");

            if(strlen(ptr->long_opt) > 0) 
                printf("--%-12s", ptr->long_opt);
            else
                printf("              ");
            
            if((ptr->flag & CMD_RARG) || (ptr->flag & CMD_OARG)) {
                int c = (ptr->flag & CMD_NUM)? 'N' : 
                        (ptr->flag & CMD_STR)? 'S': 
                        (ptr->flag & CMD_BOOL)? 'B' : '?';

                if(ptr->flag & CMD_LIST) 
                    printf("[%c,%c,...] ", c, c);    
                else 
                    printf("[%c]       ", c);    
            }
            else
                printf("[]        ");

            if(strlen(ptr->help) > 0)
                printf("%s ", ptr->help);

            printf("%s ", (ptr->flag & CMD_REQD)? "(reqd)": "");
            //printf("0x%02X", ptr->flag);
            fputc('\n', stdout);
        }
        else {
            int c = (ptr->flag & CMD_NUM)? 'N' : 
                    (ptr->flag & CMD_STR)? 'S': 
                    (ptr->flag & CMD_BOOL)? 'B' : '?';
            printf("  %-17s[%c,%c,...] %s\n", ptr->name, c, c, ptr->help);
        }
    }
    printf("--------------------------------------------------------------------------\n");
    printf("  S = string, N = number, B = bool\n");

    printf("\n%s\n\n", cmdline->outtro);
    exit(1);
}

// /**
//  * @brief Iterate the extra parameters if there are any. Returns NULL at 
//  * the end.
//  * 
//  * @param idx 
//  * @return const char* 
//  */
// String* iterate_nonopts(int *idx) {

//     return iterate_str_lst(cmdline->non_opts, idx);
// }


/**
 * @brief Testing
 * 
 */
#ifdef TEST_CMDLINE

// example:
//  ./test_cmd  -b:asdasd asdsds --bump=123123123 -a=123,345,456 --add=098098
// outputs:
//  veto: (null)
//  plow: (null)
//  verbo: (null)
//  bump: 123123123
//  add: 123, 345, 456, 098098
//  files: asdsds
int main(int argc, char** argv) {

    init_cmdline("This is the testing thing.", "Report bugs to your friends.", "Test Cmdline", "0.0.0");
    add_cmdline('a', "add", "otters", "Add the things to the otters.", NULL, CMD_STR|CMD_LIST|CMD_RARG|CMD_REQD);
    add_cmdline('b', "bump", "bump", "Bump the things with the otters.", "blouts", CMD_STR|CMD_OARG);
    add_cmdline('v', NULL, "verbo", "int 0 - 10 default is 0. Set the verbosity.", "0", CMD_NUM|CMD_RARG);
    add_cmdline(0, "veto", "veto", "Vote the otters down.", NULL, CMD_STR|CMD_RARG);
    add_cmdline(0, "plow", "plow", "Plow the otter's fields.", NULL, CMD_NUM|CMD_RARG);

    // add some default command line parameters that are handled in the 
    // parser. These options should not be defined in any other context 
    // without first modifying the option parser.
    add_cmdline('V', "version", NULL, "show the name and version", NULL, CMD_NARG|CMD_BOOL);
    add_cmdline('h', "help", NULL, "show this help information", NULL, CMD_NARG|CMD_BOOL);

    // Special list of files has no command option.
    add_cmdline(0, NULL, "list of files", "list of files to be processed", NULL, CMD_STR|CMD_REQD);

    // Actually read the command line and parse the variables.
    parse_cmdline(argc, argv, ALLOW_NOPT);

    // see what we got.
    printf("veto: %s\n", raw_string(get_cmdline("veto")));
    printf("plow: %s\n", raw_string(get_cmdline("plow")));
    printf("verbose: %s\n", raw_string(get_cmdline("verbo")));
    printf("bump: %s\n", raw_string(get_cmdline("bump")));

    String* str;
    int post = 0;
    printf("add: ");
    while(NULL != (str = iterate_cmdline("otters", &post)))
        printf("'%s', ", raw_string(str));
    printf("\b\b \n");

    post = 0;
    printf("files: ");
    while(NULL != (str = iterate_cmdline("list of files", &post)))
        printf("'%s', ", raw_string(str));
    printf("\b\b \n");

    //show_cmdline_help();
    return 0;
}

#endif