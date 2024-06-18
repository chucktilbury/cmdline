/**
 * @file parse.h
 * 
 * @brief Public interface for parser.
 * 
 * @author Chuck Tilbury (chucktilbury@gmail.com)
 * @version 0.0
 * @date 2024-06-17
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _PARSE_H_
#define _PARSE_H_

#include "buffer.h"
#include "str.h"

typedef void (*cmdline_callback)();

typedef struct {
    int short_opt;
    const char* long_opt;
    const char* name;
    const char* help;
    StrLst* values;
    int flag; 
    cmdline_callback callback;
} _cmd_opt_t_;

typedef struct {
    const char* prog;
    const char* name;
    const char* version;
    const char* intro;
    const char* outtro;
    PtrLst* cmd_opts;
    String* sopts; 
    int flag;
    int min_reqd;
} _cmdline_t_;

void internal_parse_cmdline(int argc, char** argv);

#endif  /* _PARSE_H_ */
