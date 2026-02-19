/*

    ===~----------------------------------------------~===
----| 10DAL - The "10 Dollars A License" license. (v4.1) |----
    ===~----------------------------------------------~===
Copyright (c) 2026 Viktor Hugo Caetano M. Goulart
_______
Summary
-------
Do whatever you want, be a nice person.
___________
Definitions
-----------
1. "The Software" means any version of this software and associated files.
2. "Military Purposes" means direct use of the The Software in
   the planning, execution or support of military operations.
3. "Harmful Intent" means deliberate use of The Software with the aim of causing 
   significant physical, moral or mental harm* to people or property.
   * Examples of "harm" include: harassment, anything that may hurt a person
     physically - including torture, human trafficking, genocide,
     creation of weapons, etc.
__________
Permission
----------
The The Software is licensed under the 10DAL license, which is
subject to the conditions below, permission is granted, free of charge, to any
person obtaining a copy of the The Software, to - but not limited to, except if you are
Richard M. StallMan - use the The Software, to modify the The Software, to distribute the
The Software, to sell the The Software, and/or to relicense the The Software.
However, if you are Richard M. StallMan, your rights of using the The Software are immediately revoked.
_________________________________________________________________________
Optional Good-Practice Suggestions to Become a Better Human (non-binding)
-------------------------------------------------------------------------
1. Be kind to people. Donate money, give gifts, help them.
2. Sign for volunteer work and/or donate 10 dollars monthly to your
   local charity.
3. Do NOT waste anything that comes from natural non-renewable resources.
4. Be tolerant. Everything in humanity, including the device, be it physical or
   digital, that you are using to read this right now, came from cooperation.
5. Do NOT insult other people without a prior reason to it.
6. Study upon topics you consider yourself ignorant about.
7. If you use the The Software with Harmful Intent or with Military Purposes,
   you are specially encouraged to make donations and volunteer for people 
   in your area.
___________________
Warranty Disclaimer
-------------------
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef CLI_PARSER_H
#define CLI_PARSER_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CP_NUMBER_INVALID (0.0/0.0)
#define CpNumberIsValid(number) ((number) == (number))

// internal usage
#define cp__streq(str1, str2) (strcmp(str1, str2) == 0)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OPTK_BOOL,
    OPTK_NUMBER,
    OPTK_STRING
} Cp_Opt_Kind;
typedef struct {
    void *holder;
    Cp_Opt_Kind kind;
    const char *name;
    const char short_name;
    char *short_desc;
    void *user_data;
} Cp_Opt;

#define CP_PARSE_ERR_LEN 1024
typedef struct Cp_Ctx {
    char err[CP_PARSE_ERR_LEN];
    char **argv;
    // Stores arguments such as file names in a compiler.
    // E.g.
    // `./app -myopt=AAAAAAAAAA file1.c file2.c`
    // `------------------------^^^^^^^^^^^^^^^` those are stored inside `argumentv`.
    // `argumentv` only considers stuff *after* the `subcommand` as arguments.
    char **argumentv;
    Cp_Opt *optv;
    const char *app_name;
    uintmax_t optc;
    int argc;
    int argi; // internal index for where we are in argv
    int argumentc;
    int argumentcap;
    bool dashdash_halt;
} Cp_Ctx;
Cp_Ctx *cp_newCtx(int argc, char *argv[], uintmax_t optc, Cp_Opt optv[], int argumentcap, char *argumentv[]);
void cp_freeCtx(Cp_Ctx *ctx);

// internal usage
bool cp__parseLongOpt(Cp_Ctx *ctx, Cp_Opt opt);
bool cp__parseShortOpt(Cp_Ctx *ctx, Cp_Opt opt, int arg_amount);

int cp_parseUntil(Cp_Ctx *ctx, uintmax_t subcommandc, const char *subcommandv[]);
int cp_parse(Cp_Ctx *ctx);

// internal usage
bool cp__strHasPrefix(const char *str, const char *prefix);


#ifdef __cplusplus
}
#endif 

#ifdef CLI_PARSER_IMPLEMENTATION
bool cp__strHasPrefix(const char *str, const char *prefix) {
    if(str == NULL || prefix == NULL) {
        return false;
    }
    int str_len = strlen(str); // perfect naming
    int prefix_len = strlen(prefix);
    if(prefix_len > str_len) {
        return false;
    }

    for(int i = 0; i < prefix_len; ++i) {
        if(str[i] != prefix[i]) { 
            return false;
        }
    }
    return true;
}

Cp_Ctx *cp_newCtx(int argc, char *argv[], uintmax_t optc, Cp_Opt optv[], int argumentcap, char *argumentv[]) {
    if(
        optc == 0 ||
        optv == NULL ||
        argc < 1 ||
        argv == NULL
    ) {
        return NULL;
    }
    
    Cp_Ctx *ctx = calloc(1, sizeof(Cp_Ctx));
    if(ctx == NULL) {
        return NULL;
    }

    ctx->argc = argc;
    ctx->argv = argv;

    ctx->optc = optc;
    ctx->optv = optv;

    ctx->argumentcap = argumentcap;
    ctx->argumentv = argumentv;

    return ctx;
}
void cp_freeCtx(Cp_Ctx *ctx) {
    if(ctx == NULL) return;
    free(ctx);
}

bool cp__parseLongOpt(Cp_Ctx *ctx, Cp_Opt opt) {
    const char *arg = ctx->argv[ctx->argi];
    if(cp__strHasPrefix(arg, "--")) {
        arg+=2;
    }
    switch(opt.kind) {
        case OPTK_BOOL: {
            if(strlen(opt.name) != strlen(arg)) {
                snprintf(
                    ctx->err, CP_PARSE_ERR_LEN,
                    "At argument near %d: Argument of type `bool` takes no argument.", ctx->argi
                );
                return false;
            }
            *(bool*)(opt.holder) = true;
        } break;
        case OPTK_STRING: {
            int name_len = strlen(opt.name);
            int arg_len = strlen(arg);
            
            if(name_len == arg_len) {
                // the value is on the next argument
                if((ctx->argi)+1 >= ctx->argc) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected argument but got nothing.", ctx->argi
                    );
                    return false;
                }
                arg = ctx->argv[++ctx->argi];
                *(char**)(opt.holder) = (char*)arg;
                break;
            }
            char *assign;
            if((assign = strchr(arg, '=')) == NULL) {
                if((assign = strchr(arg, ':')) == NULL) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected either '=', ':' or ' ' to set value. E.g. `flag=this flag:that`.", ctx->argi
                    );
                    return false;
                }
            }
            ++assign;
            *(char**)(opt.holder) = assign;
        } break;
        case OPTK_NUMBER: {
            int name_len = strlen(opt.name);
            int arg_len = strlen(arg);
            if(name_len == arg_len) {
                // the value is on the next argument
                if((ctx->argi)+1 >= ctx->argc) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected argument but got nothing.", ctx->argi
                    );
                    return false;
                }
                arg = ctx->argv[++ctx->argi];
                double value;
                if(sscanf(arg, "%lf", &value) != 1) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected a number literal.", ctx->argi
                    );
                    return false;
                }
                *(double*)(opt.holder) = value;
                break;
            }
            char *assign;
            if((assign = strchr(arg, '=')) == NULL) {
                if((assign = strchr(arg, ':')) == NULL) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected either '=', ':' or ' ' to set value. E.g. `flag=this flag:that`.", ctx->argi
                    );
                    return false;
                }
            }
            ++assign;
            double value;
            if(sscanf(assign, "%lf", &value) != 1) {
                snprintf(
                    ctx->err, CP_PARSE_ERR_LEN,
                    "At argument near %d: Expected a number literal.", ctx->argi
                );
                return false;
            }
            *(double*)(opt.holder) = value;
        } break;
        default: {
            strncpy(ctx->err, "Unknown option kind.", CP_PARSE_ERR_LEN);
            return false; // in theory, unreachable
        } break;
    }

    return true;
}
bool cp__parseShortOpt(Cp_Ctx *ctx, Cp_Opt opt, int arg_amount) {
    const char *arg = ctx->argv[ctx->argi];
    if(arg[0] == '-') {
        ++arg;
    }
    int arg_len = strlen(arg);
    switch(opt.kind) {
        case OPTK_BOOL: {
            if(arg_amount < arg_len && (arg[arg_amount] == '=' || arg[arg_amount] == ':')) {
                snprintf(
                    ctx->err, CP_PARSE_ERR_LEN,
                    "At argument near %d: Argument of type `bool` takes no argument.", ctx->argi
                );
                return false;
            }
            *(bool*)(opt.holder) = true;
        } break;
        case OPTK_STRING: {
            if(arg_amount > 1) {
                snprintf(
                    ctx->err, CP_PARSE_ERR_LEN,
                    "At argument near %d: Short opts can only have an argument if isolated.", ctx->argi
                );
                return false;
            }
            if(arg_amount >= arg_len) {
                // the value is on the next argument
                if(ctx->argi+1 >= ctx->argc) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected argument but got nothing.", ctx->argi
                    );
                    return false;
                }
                arg = ctx->argv[++ctx->argi];
                *(char**)(opt.holder) = (char*)arg;
                return true;
            }
            char *assign;
            if((assign = strchr(arg, '=')) == NULL) {
                if((assign = strchr(arg, ':')) == NULL) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected either '=', ':' or ' ' to set value. E.g. `flag=this flag:that`.", ctx->argi
                    );
                    return false;
                }
            }
            ++assign;
            *(char**)(opt.holder) = assign;
            return true;
        } break;
        case OPTK_NUMBER: {
            if(arg_amount > 1) {
                snprintf(
                    ctx->err, CP_PARSE_ERR_LEN,
                    "At argument near %d: Short opts can only have an argument if isolated.", ctx->argi
                );
                return false;
            }
            if(arg_amount >= arg_len) {
                // the value is on the next argument
                if(ctx->argi+1 >= ctx->argc) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected argument but got nothing.", ctx->argi
                    );
                    return false;
                }
                arg = ctx->argv[++ctx->argi];
                double value;
                if(sscanf(arg, "%lf", &value) != 1) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected a number literal.", ctx->argi
                    );
                    return false;
                }
                *(double*)(opt.holder) = value;
                return true;
            }
            char *assign;
            if((assign = strchr(arg, '=')) == NULL) {
                if((assign = strchr(arg, ':')) == NULL) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected either '=', ':' or ' ' to set value. E.g. `flag=this flag:that`.", ctx->argi
                    );
                    return false;
                }
            }
            ++assign;
            double value;
            if(sscanf(assign, "%lf", &value) != 1) {
                snprintf(
                    ctx->err, CP_PARSE_ERR_LEN,
                    "At argument near %d: Expected a number literal.", ctx->argi
                );
                return false;
            }
            *(double*)(opt.holder) = value;
            return true;
        } break;
        default: {
            strncpy(ctx->err, "Internal: Unknown option kind.", CP_PARSE_ERR_LEN);
            return false; // in theory, unreachable
        } break;
    }

    return true;
}

// returns where it stopped parsing `ctx->argv`, 0-indexed, or -1 for parsing error
int cp_parseUntil(Cp_Ctx *ctx, uintmax_t subcommandc, const char *subcommandv[]) {
    for(; ctx->argi < ctx->argc; ++ctx->argi) {
        const char *arg = ctx->argv[ctx->argi];
        if(ctx->dashdash_halt && cp__streq(arg, "--")) {
            if(ctx->argumentc+1 >= ctx->argumentcap) {
                return ctx->argi;
            }
            for(; ctx->argi < ctx->argc && ctx->argumentc < ctx->argumentcap; ++ctx->argi) {
                arg = ctx->argv[ctx->argi];
                ctx->argumentv[ctx->argumentc++] = (char*)arg;
            }
            return ctx->argi;
        } else {
            for(size_t j = 0; j < subcommandc; ++j) {
                if(cp__streq(subcommandv[j], arg)) {
                    return ctx->argi;
                }
            }
        }

        if(cp__strHasPrefix(arg, "--")) {
            arg+=2;
            if(strlen(arg) != 0) { 
                bool match = false;
                for(size_t j = 0; j < ctx->optc; ++j) {
                    if(cp__strHasPrefix(arg, ctx->optv[j].name)) {
                        int name_len = strlen(ctx->optv[j].name);
                        if(arg[name_len]!=0 && arg[name_len]!=':' && arg[name_len]!='=') {
                            snprintf(
                                ctx->err, CP_PARSE_ERR_LEN,
                                "At argument near %d: Unknown long argument: '%s'.", ctx->argi, arg
                            );
                            return -1;
                        }
                        match = true;
                        if(!cp__parseLongOpt(ctx, ctx->optv[j])) {
                            return -1;
                        }
                    }
                    if(match) {
                        break;
                    }
                }
                if(!match) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Unknown long argument: '%s'.", ctx->argi, arg
                    );
                    return -1;
                }
            }
        } else if(arg[0] == '-') {
            ++arg;
            int arg_amount = 0;
            bool match;
            for(int j = 0; arg[j] != '\0'; ++j) {
                match = false;
                ++arg_amount;
                for(size_t k = 0; k < ctx->optc; ++k) {
                    if(arg[j] == ctx->optv[k].short_name) {
                        match = true;
                        if(!cp__parseShortOpt(ctx, ctx->optv[k], arg_amount)) {
                            return -1;
                        }
                    }
                    if(match) {
                        break;
                    }
                }
                if(!match) {
                    snprintf(
                        ctx->err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Unknown short argument in arg: '%s'.", ctx->argi, arg
                    );
                    return -1;
                }
                if(
                    arg[arg_amount] == '=' ||
                    arg[arg_amount] == ':'
                ) {
                    if(arg_amount > 1) {
                        snprintf(
                            ctx->err, CP_PARSE_ERR_LEN,
                         "At argument near %d: Short opts can only have an argument if isolated.", ctx->argi
                        );
                        return -1;
                    }
                    break;
                }
            }
        } else {
            if(ctx->argumentc+1 < ctx->argumentcap) {
                ctx->argumentv[ctx->argumentc++] = (char*)arg;
            }
        }
    }

    return ctx->argi;
}

int cp_parse(Cp_Ctx *ctx) {
    return cp_parseUntil(ctx, 0, NULL);
}

#endif

#endif
