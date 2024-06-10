/**
 * @file cmdline.h
 * 
 * @brief 
 * 
 * @author Chuck Tilbury (chucktilbury@gmail.com)
 * @version 0.0
 * @date 2024-06-10
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef _CMDLINE_H_
#define _CMDLINE_H_

#include "str.h"

typedef enum {
    // nneded by getarg()
    CMD_NONE = 0x00,
    CMD_RARG = 0x01,
    CMD_OPT = 0x02,

    // data type helpers
    CMD_STR = 0x04,
    CMD_LIST = 0x08,
    CMD_NUM = 0x10,
    CMD_BOOL = 0x20,

    // internal flags, do not use
    CMD_REQD = 0x40,
    CMD_SEEN = 0x80,
} CmdType;

void init_cmdline(const char* intro, const char* outtro);
void uninit_cmdline();
void add_cmdline(int short_opt, const char* long_opt, 
                    const char* name, const char* help, 
                    const char* def_val, CmdType flag);
void parse_cmdline(int argc, char** argv, int flag);
const char* get_cmdline(const char* name);
String* iterate_cmdline(int* idx);
void show_cmdline_help();

#endif  /* _CMDLINE_H_ */
