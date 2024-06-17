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
//#include "str.h"
#include "memory.h"
#include "myassert.h"
//#include "buffer.h"
#include "cmdline.h"
#include "parse.h"
#include "errors.h"

// typedef Buffer _cmd_opts_t_;

// typedef struct {
//     int short_opt;
//     const char* long_opt;
//     const char* name;
//     const char* help;
//     //const char* value;
//     StrLst* values;
//     int flag; 
// } _cmd_opt_t_;

// typedef struct {
//     const char* prog;
//     const char* name;
//     const char* version;
//     const char* intro;
//     const char* outtro;
//     PtrLst* cmd_opts;
//     _cmd_opts_t_* options;
//     String* sopts; 
//     //StrLst* non_opts;
//     int flag;
//     int min_reqd;
// } _cmdline_t_;

static _cmdline_t_* cmdline = NULL;

// private interface for parser.
_cmdline_t_* _get_cmdline_() { return cmdline; }

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
 * @brief Return a string for the option type for error reporting.
 * 
 * @param type 
 * @return const char* 
 */
static inline const char* type_to_str(CmdType type) {

    if(type & CMD_BOOL) 
        return "bool";
    else if(type & CMD_NUM)
        return "number";
    else if(type & CMD_STR)
        return "string";

    return "unknown";
}

/**
 * @brief Copy the bytes in the str to the buffer as lowered case.
 * 
 * @param str 
 * @param buffer 
 * @param len 
 */
static void strnlower(const char* str, char* buffer, int len) {

    int i = 0;
    for(; i < len-1 && str[i] != '\0'; i++) {
        buffer[i] = tolower(str[i]);
    }
    buffer[i] = '\0';
}

/**
 * @brief If the str could be interpreted as the type that is specified in 
 * the op, the reutn true, otherwise, return false.
 * 
 * @param op 
 * @param str 
 * @return bool
 */
static inline bool validate(_cmd_opt_t_* op, const char* str) {

    if(op->flag & CMD_NUM) {
        // if all of the characters in the str are digits then return true.
        for(int i = 0; str[i] != '\0'; i++) {
            if(!isdigit(str[i]))
                return false;
        }
        return true;
    }
    else if(op->flag & CMD_BOOL) {
        // Most people will just 1 or 0. actual single character options 
        // return whether they were seen or not.
        char buf[10];
        strnlower(str, buf, sizeof(buf));
        if(!strncmp(buf, "true", sizeof(buf)) || 
                !strncmp(buf, "false", sizeof(buf)) ||
                !strncmp(buf, "yes", sizeof(buf)) ||
                !strncmp(buf, "no", sizeof(buf))) {
            return true;
        }
        else 
            return false;
    }
    else if(str[0] != '-') {
        // if the first character of the str is a '-', then it's probably not 
        // intended to be an arg. It's probably an option.
        //error("arg \"%s\" is malformed. See docs.", str);
        return true;
    }
    // don't validate for str to keep it generic.

    return false;
}

/**
 * @brief If it's a list, then there are ',' characters in it. Use strtok() to
 * iterate the string for tokens and save each one in the given list.
 * 
 * @param lst 
 * @param str 
 */
static void parse_str_lst(_cmd_opt_t_* op, const char* str) {

    char* ptr = (char*)str;
    char *save, *token;

    while(true) {
        token = strtok_r(ptr, ",", &save);
        if(NULL == token)
            break;
        else {
            if(validate(op, token))
                append_str_lst(op->values, create_string(token));
            else 
                error("cannot validate argument \"%s\" to be a %s", 
                        optarg, type_to_str(op->flag));
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

    append_buffer(cmdline->options, opt, sizeof(struct option));

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
                        parse_str_lst(op, optarg);
                    else {
                        clear_str_lst(op->values);
                        if(validate(op, optarg))
                            append_str_lst(op->values, create_string(optarg));
                        else 
                            error("cannot validate argument \"%s\" to be a %s", 
                                    optarg, type_to_str(op->flag));
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
                        parse_str_lst(op, val);
                    else {
                        clear_str_lst(op->values);
                        if(validate(op, optarg))
                            append_str_lst(op->values, create_string(optarg));
                        else 
                            error("cannot validate argument \"%s\" to be a %s", 
                                    optarg, type_to_str(op->flag));
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
const char* iterate_cmdline(const char* name, int* post) {

    _cmd_opt_t_* opt = search_name(name);
    ASSERT_MSG(opt != NULL, "cannot find the option searched for: %s", name);

    return raw_string(iterate_str_lst(opt->values, post));
}

/**
 * @brief Retrieve a command line option from the data structure. If it's a 
 * list, just keep retrieving the first option.
 * 
 * @param name 
 * @return const char* 
 */
const char* get_cmdline(const char* name) {

    _cmd_opt_t_* opt = search_name(name);
    ASSERT_MSG(opt != NULL, "cannot find the option searched for: %s", name);

    int post = 0;
    return iterate_cmdline(name, &post);
}

/**
 * @brief Get the cmdline as num object
 * 
 * @param name 
 * @return int 
 */
int get_cmdline_as_num(const char* name) {

    return (int)strtol(get_cmdline(name), NULL, 10);
}

/**
 * @brief Get the cmdline as bool object
 * 
 * @param name 
 * @return true 
 * @return false 
 */
bool get_cmdline_as_bool(const char* name) {

    _cmd_opt_t_* opt = search_name(name);
    return (opt->flag & CMD_SEEN);
}

/**
 * @brief Get the cmdline as str object
 * 
 * @param name 
 * @return const char* 
 */
const char* get_cmdline_as_str(const char* name) {

    return get_cmdline(name);
}

/**
 * @brief Show the help message and exit the program.
 * 
 */
void show_cmdline_help() {

    char tmp[64];

    printf("\nUsage: %s [options]", cmdline->prog);
    if(!cmdline->flag)
        printf(" files\n");
    else 
        printf("\n");

    printf("%s v%s\n%s\n\n", cmdline->name, cmdline->version, cmdline->intro);
    printf("Options:\n");
    printf("  Parm             Args        Help\n");
    printf("-+----------------+-----------+---------------------------------------------\n");

    int post = 0;
    _cmd_opt_t_* ptr;

    while(NULL != (ptr = iterate_ptr_lst(cmdline->cmd_opts, &post))) {
        if(isgraph(ptr->short_opt) || strlen(ptr->long_opt) > 0) {
            strcpy(tmp, " ");
            if(isgraph(ptr->short_opt)) // could be zero
                snprintf(tmp, sizeof(tmp), "-%c", ptr->short_opt);
            printf("%4s", tmp);
            //printf("%s", tmp);

            strcpy(tmp, " ");
            if(strlen(ptr->long_opt) > 0) // should never be NULL
                snprintf(tmp, sizeof(tmp), "--%s", ptr->long_opt);
            printf(" %-14s", tmp);
            //printf(" %s", tmp);
            
            if((ptr->flag & CMD_RARG) || (ptr->flag & CMD_OARG)) {
                int c = (ptr->flag & CMD_NUM)? 'N' : 
                        (ptr->flag & CMD_STR)? 'S': 
                        (ptr->flag & CMD_BOOL)? 'B' : '?';

                if(ptr->flag & CMD_LIST) 
                    snprintf(tmp, sizeof(tmp), "[%c,%c, ...]", c, c);    
                else 
                    snprintf(tmp, sizeof(tmp), "[%c]", c);    
            }
            else
                strcpy(tmp, "  ");
            printf("%-12s", tmp);
            //printf("%s", tmp);

            if(ptr->flag & CMD_REQD) 
                snprintf(tmp, sizeof(tmp), "(reqd) %s", ptr->help);
            else
                snprintf(tmp, sizeof(tmp), "%s", ptr->help);
            printf("%s\n", tmp);            
        }
        else {
            snprintf(tmp, sizeof(tmp), "%s", ptr->name);
            printf("  %-17s", tmp);

            int c = (ptr->flag & CMD_NUM)? 'N' : 
                    (ptr->flag & CMD_STR)? 'S': 
                    (ptr->flag & CMD_BOOL)? 'B' : '?';
            snprintf(tmp, sizeof(tmp), "[%c,%c, ...]", c, c);            
            printf("%-12s", tmp);

            if(ptr->flag & CMD_REQD) 
                snprintf(tmp, sizeof(tmp), "(reqd) %s", ptr->help);
            else
                snprintf(tmp, sizeof(tmp), "%s", ptr->help);
            printf("%s\n", tmp);            
        }
    }
    printf("-+----------------+-----------+---------------------------------------------\n");
    printf("  S = string, N = number, B = bool ('on'|'off'|'true'|'false')\n");

    printf("\n%s\n\n", cmdline->outtro);
    exit(1);
}

