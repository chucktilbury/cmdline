/**
 * @file parse.c
 * 
 * @brief Internal command line parser. See the readme for more information 
 * about the format of command line options parsed by this module.
 * 
 * @author Chuck Tilbury (chucktilbury@gmail.com)
 * @version 0.0
 * @date 2024-06-17
 * @copyright Copyright (c) 2024
 * 
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "memory.h"
#include "myassert.h"
#include "parse.h"
#include "errors.h"

typedef enum {
    // special characters
    EOS = 0,    // end of string
    EOI,        // end of input

    // token types
    TOK_DDASH = 100,  // a "--" token
    TOK_SDASH,  // a "-" token
    TOK_UNUM,   // parsed as strtol(str, NUMM, 16)
    TOK_INUM,   // parsed as strtol(str, NULL, 10)
    TOK_FNUM,   // parsed as strtod(str, NULL)
    TOK_BOOL,   // strings as true, false, off on
    TOK_WORD,   // anything that is not a number or a bool
    TOK_COLON,  // a ':'
    TOK_EQU,    // a '='
    TOK_COMMA,  // a ','
    TOK_END,    // no more tokens are available
    TOK_ERROR,  // received an error while scanning something
} _token_type_;

typedef struct {
    _token_type_ type;
    String* str;
} _token_t_;

static _cmdline_t_* cmdline;
_cmdline_t_* _get_cmdline_();

typedef struct {
    int argc;
    char** argv;
    int aidx;
    int sidx;
} _parser_t_;

static _parser_t_* parser;
static _token_t_* token = NULL;

// return the current character
static int get_char() {

    if(parser->aidx == parser->argc) 
        return EOI;  // no more characters are available
    else
        return parser->argv[parser->aidx][parser->sidx];
}

// return the new character.
static int consume_char() {

    if(parser->aidx == parser->argc) 
        return EOI;  // no more characters are available
    else if(parser->argv[parser->aidx][parser->sidx] == 0) {
        parser->aidx++;
        parser->sidx = 0;
        return get_char();
    }
    else {
        parser->sidx++;
        return parser->argv[parser->aidx][parser->sidx];
    }
}

static const char* tok_to_str(_token_type_ type) {

    return (type == TOK_DDASH)? "DDASH" :
        (type == TOK_SDASH)? "SDASH" :
        (type == TOK_UNUM)? "UNUM" :
        (type == TOK_INUM)? "INUM" :
        (type == TOK_FNUM)? "FNUM" :
        (type == TOK_BOOL)? "BOOL" :
        (type == TOK_WORD)? "WORD" :
        (type == TOK_COLON)? "COLON" :
        (type == TOK_EQU)? "EQU" :
        (type == TOK_COMMA)? "COMMA" :
        (type == TOK_ERROR)? "ERROR" :
        (type == TOK_END)? "END" : "UNKNOWN";
}

static _token_t_* get_token() {

    return token;
}

// returns true if the character is a single-character token.
// Includes the EOI and EOS characters.
static inline bool is_a_token(int ch) {

    switch(ch) {
        case '=':
        case ':':
        case ',':
        case '-':
        case EOI:
        case EOS:
            return true;
        default:
            return false;
    }
}

static bool capture_digits() {

    int ch = get_char();
    if(isdigit(ch)) {
        while(isdigit(ch)) {
            append_string_char(token->str, ch);
            ch = consume_char();
        }
    }
    else {
        error("expected a digit but got a '%c'", ch);
        return true;
    }

    return false;
}

static int capture_exponent() {

    int ch = get_char();
    if(ch == '-') {
        // exponent can be negative
        append_string_char(token->str, ch);
        ch = consume_char();

        if(capture_digits()) {
            return 101;
        }
    }
    else if(capture_digits())
        return 101;

    ch = get_char();
    if(is_a_token(ch)) {
        token->type = TOK_FNUM;
        return 100;
    }
    else {
        while(!is_a_token(ch)) {
            append_string_char(token->str, ch);
            ch = consume_char();
        }
        error("malformed exponent: %s", raw_string(token->str));
        token->type = TOK_ERROR;
        return 101;
    }

    return 100;
}

static int capture_mantissa() {

    int ch = get_char();
    while(isdigit(ch)) {
        append_string_char(token->str, ch);
        ch = consume_char();
    }

    if(is_a_token(ch)) {
        token->type = TOK_FNUM;
        return 100;
    }
    else if(ch == 'e' || ch == 'E') {
        append_string_char(token->str, ch);
        consume_char();
        return capture_exponent();
    }
    else {
        while(!is_a_token(ch)) {
            append_string_char(token->str, ch);
            ch = consume_char();
        }
        error("malformed floating point number: %s", raw_string(token->str));
        token->type = TOK_ERROR;
        return 101;
    }

    return 100;
}

static int capture_integer() {

    int ch = get_char();
    while(isdigit(ch)) {
        append_string_char(token->str, ch);
        ch = consume_char();
    }

    if(is_a_token(ch)) {
        token->type = TOK_INUM;
        return 100;
    }
    else if(ch == 'e' || ch == 'E') {
        append_string_char(token->str, ch);
        consume_char();
        return capture_exponent();
    }
    else if(ch == '.') {
        append_string_char(token->str, ch);
        consume_char();
        return capture_mantissa();
    }
    else {
        while(!is_a_token(ch)) {
            append_string_char(token->str, ch);
            ch = consume_char();
        }
        error("malformed integer: %s", raw_string(token->str));
        token->type = TOK_ERROR;
        return 101;
    }

    return 100;
}

static int capture_hex() {

    int ch = get_char();
    while(isxdigit(ch)) {
        append_string_char(token->str, ch);
        ch = consume_char();
    }

    if(is_a_token(ch)) {
        token->type = TOK_UNUM;
        return 100;
    }
    else {
        while(!is_a_token(ch)) {
            append_string_char(token->str, ch);
            ch = consume_char();
        }
        error("malformed hex number: %s", raw_string(token->str));
        token->type = TOK_ERROR;
        return 101;
    }

    return 100;
}

static void capture_number() {

    int state = 0;
    bool finished = false;

    while(!finished) {
        int ch = get_char();
        switch(state) {
            case 0:
                // check for zero. valid hex and floats can start with a 0.
                if(ch == '0') {
                    append_string_char(token->str, ch);
                    consume_char();
                    state = 1;
                }
                else {
                    append_string_char(token->str, ch);
                    consume_char();
                    state = capture_integer();
                } 
                break;

            case 1:
                // have a 0, check for an 'e', 'x', '.', or a token
                switch(ch) {
                    case 'e':
                    case 'E':
                        // nonesense to have an exponent with a leading zero, 
                        // but strtod() will parse it.
                        append_string_char(token->str, ch);
                        consume_char();
                        state = capture_exponent();
                        break;
                    case 'x':
                    case 'X':
                        // format of 0x???
                        append_string_char(token->str, ch);
                        consume_char();
                        state = capture_hex();
                        break;
                    case '.':
                        // format of 0.???
                        append_string_char(token->str, ch);
                        consume_char();
                        state = capture_mantissa();
                        break;
                    default:
                        if(is_a_token(ch)) {
                            // zero by itself
                            token->type = TOK_INUM;
                            state = 100;
                        }
                        else {
                            // malformed number
                            while(!is_a_token(ch)) {
                                append_string_char(token->str, ch);
                                ch = consume_char();
                            }
                            error("malformed number: %s", raw_string(token->str));
                            state = 101;
                        }
                        break;
                }
                break;

            case 100:
                // completed number
                finished = true;
                break;

            case 101:
                // discovered an error
                finished = true;
                token->type = TOK_ERROR;
                break;

            default:
                fprintf(stderr, "Fatal ERROR: %s: invlid state in cmd parser: %d\n", __func__, state);
                abort();
        }
    }
}

static void capture_word() {

    int ch = get_char();
    while(isprint(ch) && !is_a_token(ch)) {
        append_string_char(token->str, ch);
        ch = consume_char();
    }

    if(!comp_string_str(token->str, "true") ||
            !comp_string_str(token->str, "false") || 
            !comp_string_str(token->str, "on") ||
            !comp_string_str(token->str, "off"))
        token->type = TOK_BOOL;
    else
        token->type = TOK_WORD;

}

/**
 * @brief Return the new token and discard the old one.
 * 
 * @return _token_t_* 
 */
static _token_t_* consume_token() {

    clear_string(token->str);
    token->type = TOK_END;
    
    int state = 0;
    bool finished = false;

    while(!finished) {
        int ch = get_char();
        switch(state) {
            case 0:
                // dispatch to the first character of a new token
                switch(ch) {
                    case EOS:
                        // skip the end-of-string characters
                        consume_char();
                        break;
                    case EOI:
                        token->type = TOK_END;
                        state = 100;
                        break;
                    case '-':
                        append_string_char(token->str, ch);
                        consume_char();
                        state = 1;
                        break;
                    case ':':
                        append_string_char(token->str, ch);
                        token->type = TOK_COLON;
                        consume_char();
                        state = 100;
                        break;
                    case '=':
                        append_string_char(token->str, ch);
                        token->type = TOK_EQU;
                        consume_char();
                        state = 100;
                        break;
                    case ',':
                        append_string_char(token->str, ch);
                        token->type = TOK_COMMA;
                        consume_char();
                        state = 100;
                        break;
                    default:
                        if(isdigit(ch)) {
                            capture_number();
                            state = 100;
                        }
                        else {
                            append_string_char(token->str, ch);
                            consume_char();
                            capture_word();
                            state = 100;
                        }
                }
                break;

            case 1:
                if(ch == '-') {
                    append_string_char(token->str, ch);
                    token->type = TOK_DDASH;
                    consume_char();
                    state = 100;
                }
                else {
                    token->type = TOK_SDASH;
                    state = 100;
                }
                break;

            case 100:
                // completed token ready for return
                finished = true;
                break;
            default:
                fprintf(stderr, "Fatal ERROR: %s: invlid state in cmd parser: %d\n", __func__, state);
                abort();
        }
    }

    return token;
}

void init_parser(int argc, char** argv) {

    parser = _ALLOC_DS(_parser_t_);
    parser->argc = argc;
    parser->argv = argv;
    parser->aidx = 1;
    parser->sidx = 0;

    token = _ALLOC_DS(_token_t_);
    token->str = create_string(NULL);
    token->type = TOK_END;

    cmdline = _get_cmdline_();

    consume_token();
}

void internal_parse_cmdline(int argc, char** argv, int flag) {

}

// int main(int argc, char** argv) {

//     init_parser(argc, argv);

//     _token_t_* tok = get_token();
//     while(true) {
//         printf("token: %s '%s'\n", tok_to_str(tok->type), raw_string(tok->str));
//         if(tok->type == TOK_END || token->type == TOK_ERROR)
//             break;
//         tok = consume_token();
//     }
//     return 0;
// }

