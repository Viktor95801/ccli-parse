/*
    Copyright © 2025 Viktor Hugo Caetano M. Goulart

    This software is licensed under the "Revised Anyone But Richard M. Stallman" (RABRMS) license, described below.

    ===--------------------------------------------------===
    | The "Revised Anyone But Richard M. Stallman" license |
    ===--------------------------------------------------===

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Conditions
    ----------

    1. Redistributions of source code must retain the above copyright notice, this list of conditions, 
        and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions, and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors may be used to 
        endorse or promote products derived from this software without specific prior written permission.

    4. You can relicense under any license that meets the first 3 conditions and isn't copylefted, 
        as long as you aren't Richard M. Stallman.

    5. Richard M. Stallman (the guy behind GNU, etc.) may not make use of, redistribute, 
        nor relicense this program or any of its derivatives.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, 
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#define CP_OPTK_NUMBER_INVALID (0.0/0.0)

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
    char short_name;
    const char *desc;
    const char *long_desc;
} Cp_Opt;

typedef struct {
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
    int argumentc;
    int argumentcap;
} Cp_Ctx;
Cp_Ctx *cp_newCtx(int argc, char *argv[], uintmax_t optc, Cp_Opt optv[], int argumentcap, char *argumentv[]);
void cp_freeCtx(Cp_Ctx *ctx);

#define CP_PARSE_ERR_LEN 1024
extern char cp_parse_opt_err[CP_PARSE_ERR_LEN];
// internal usage
bool cp__parseLongOpt(Cp_Ctx *ctx, int *argv_index, Cp_Opt opt);

int cp_parseUntil(Cp_Ctx *ctx, uintmax_t subcommandc, const char **subcommandv);
int cp_parse(Cp_Ctx *ctx);

void cp_usage(Cp_Ctx *ctx, FILE *file);

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

    ctx->argc = --argc;
    ctx->argv = ++argv;

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

char cp_parse_opt_err[CP_PARSE_ERR_LEN];
bool cp__parseLongOpt(Cp_Ctx *ctx, int *argv_index, Cp_Opt opt) {
    const char *arg = ctx->argv[*argv_index];
    switch(opt.kind) {
        case OPTK_BOOL: {
            if(strlen(opt.name) != strlen(arg)) {
                snprintf(
                    cp_parse_opt_err, CP_PARSE_ERR_LEN,
                    "At argument near %d: Argument of type `bool` takes no argument.", *argv_index
                );
                return false;
            }
            *(bool*)(opt.holder) = 1;
        } break;
        case OPTK_STRING: {
            int name_len = strlen(opt.name);
            int arg_len = strlen(arg);
            if(name_len == arg_len) {
                // the value is on the next argument
                if((*argv_index)+1 >= ctx->argc) {
                    snprintf(
                        cp_parse_opt_err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected argument but got nothing.", *argv_index
                    );
                    return false;
                }
                arg = ctx->argv[++*argv_index];
                *(char**)(opt.holder) = (char*)arg;
                break;
            }
            char *assign;
            if((assign = strchr(arg, '=')) == NULL) {
                if((assign = strchr(arg, ':')) == NULL) {
                    snprintf(
                        cp_parse_opt_err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected either '=', ':' or ' ' to set value. E.g. `flag=this flag:that`.", *argv_index
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
                if((*argv_index)+1 >= ctx->argc) {
                    snprintf(
                        cp_parse_opt_err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected argument but got nothing.", *argv_index
                    );
                    return false;
                }
                arg = ctx->argv[++*argv_index];
                double value;
                if(sscanf(arg, "%lf", &value) != 1) {
                    snprintf(
                        cp_parse_opt_err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected a number literal.", *argv_index
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
                        cp_parse_opt_err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Expected either '=', ':' or ' ' to set value. E.g. `flag=this flag:that`.", *argv_index
                    );
                    return false;
                }
            }
            ++assign;
            double value;
            if(sscanf(assign, "%lf", &value) != 1) {
                snprintf(
                    cp_parse_opt_err, CP_PARSE_ERR_LEN,
                    "At argument near %d: Expected a number literal.", *argv_index
                );
                return false;
            }
            *(double*)(opt.holder) = value;
        } break;
        default: {
            strncpy(cp_parse_opt_err, "Unknown option kind.", CP_PARSE_ERR_LEN);
            return false; // in theory, unreachable
        } break;
    }

    return true;
}

// returns where it stopped parsing `ctx->argv`, 0-indexed, or -1 for parsing error
int cp_parseUntil(Cp_Ctx *ctx, uintmax_t subcommandc, const char **subcommandv) {
    int i = 0;
    for(; i < ctx->argc; ++i) {
        const char *arg = ctx->argv[i];
        if(cp__streq(arg, "--")) {
            return i;
        } else {
            for(size_t j = 0; j < subcommandc; ++j) {
                if(cp__streq(subcommandv[j], arg)) {
                    return i;
                }
            }
        }

        if(cp__strHasPrefix(arg, "--")) {
            arg+=2;
            bool match = false;
            for(size_t j = 0; j < ctx->optc; ++j) {
                if(cp__strHasPrefix(arg, ctx->optv[j].name)) {
                    match = true;
                    if(!cp__parseLongOpt(ctx, &i, ctx->optv[j])) {
                        return -1;
                    }
                }
                if(match) {
                    break;
                }
            }
            if(!match) {
                snprintf(
                    cp_parse_opt_err, CP_PARSE_ERR_LEN,
                    "At argument near %d: Unknown long argument: '%s'.", i, arg
                );
                return -1;
            }
        } else if(arg[0] == '-') {
            ++arg;
            int arg_amount = 0,
                arg_len = strlen(arg);
            bool match, halt;
            for(int j = 0; arg[j] != '\0'; ++j) {
                match = false;
                ++arg_amount;
                for(size_t k = 0; k < ctx->optc; ++k) {
                    if(arg[j] == ctx->optv[k].short_name) {
                        match = true;
                        switch(ctx->optv[k].kind) {
                            case OPTK_BOOL: {
                                if(j+1 < arg_len && (arg[j+1] == '=' || arg[j+1] == ':')) {
                                    snprintf(
                                        cp_parse_opt_err, CP_PARSE_ERR_LEN,
                                        "At argument near %d: Argument of type `bool` takes no argument.", i
                                    );
                                    return -1;
                                }
                                *(bool*)(ctx->optv[k].holder) = 1;
                            } break;
                            case OPTK_STRING: {
                                if(arg_amount > 1) {
                                    snprintf(
                                        cp_parse_opt_err, CP_PARSE_ERR_LEN,
                                        "At argument near %d: Short args can only have an argument if isolated.", i
                                    );
                                    return -1;
                                }
                                if(j+1 >= arg_len) {
                                    // the value is on the next argument
                                    if(i+1 >= ctx->argc) {
                                        snprintf(
                                            cp_parse_opt_err, CP_PARSE_ERR_LEN,
                                            "At argument near %d: Expected argument but got nothing.", i
                                        );
                                        return -1;
                                    }
                                    arg = ctx->argv[++i];
                                    *(char**)(ctx->optv[k].holder) = (char*)arg;
                                    halt = true;
                                    break;
                                }
                                char *assign;
                                if((assign = strchr(arg, '=')) == NULL) {
                                    if((assign = strchr(arg, ':')) == NULL) {
                                        snprintf(
                                            cp_parse_opt_err, CP_PARSE_ERR_LEN,
                                            "At argument near %d: Expected either '=', ':' or ' ' to set value. E.g. `flag=this flag:that`.", i
                                        );
                                        return -1;
                                    }
                                }
                                ++assign;
                                *(char**)(ctx->optv[k].holder) = assign;
                                halt = true;
                            } break;
                            case OPTK_NUMBER: {
                                if(arg_amount > 1) {
                                    snprintf(
                                        cp_parse_opt_err, CP_PARSE_ERR_LEN,
                                        "At argument near %d: Short args can only have an argument if isolated.", i
                                    );
                                    return -1;
                                }
                                if(j+1 >= arg_len) {
                                    // the value is on the next argument
                                    if(i+1 >= ctx->argc) {
                                        snprintf(
                                            cp_parse_opt_err, CP_PARSE_ERR_LEN,
                                            "At argument near %d: Expected argument but got nothing.", i
                                        );
                                        return -1;
                                    }
                                    arg = ctx->argv[++i];
                                    double value;
                                    if(sscanf(arg, "%lf", &value) != 1) {
                                        snprintf(
                                            cp_parse_opt_err, CP_PARSE_ERR_LEN,
                                            "At argument near %d: Expected a number literal.", i
                                        );
                                        return -1;
                                    }
                                    *(double*)(ctx->optv[k].holder) = value;
                                    halt = true;
                                    break;
                                }
                                char *assign;
                                if((assign = strchr(arg, '=')) == NULL) {
                                    if((assign = strchr(arg, ':')) == NULL) {
                                        snprintf(
                                            cp_parse_opt_err, CP_PARSE_ERR_LEN,
                                            "At argument near %d: Expected either '=', ':' or ' ' to set value. E.g. `flag=this flag:that`.", i
                                        );
                                        return -1;
                                    }
                                }
                                ++assign;
                                double value;
                                if(sscanf(assign, "%lf", &value) != 1) {
                                    snprintf(
                                        cp_parse_opt_err, CP_PARSE_ERR_LEN,
                                        "At argument near %d: Expected a number literal.", i
                                    );
                                    return -1;
                                }
                                *(double*)(ctx->optv[k].holder) = value;
                                halt = true;
                            } break;
                            default: {
                                strncpy(cp_parse_opt_err, "Unknown option kind.", CP_PARSE_ERR_LEN);
                                return -1; // in theory, unreachable
                            } break;
                        }
                    }
                    if(match) {
                        break;
                    }
                }
                if(halt) {
                    break;
                }
                if(!match) {
                    snprintf(
                        cp_parse_opt_err, CP_PARSE_ERR_LEN,
                        "At argument near %d: Unknown short argument: '%s'.", j, arg
                    );
                    return -1;
                }
            }
        } else {
            if(ctx->argumentc+1 < ctx->argumentcap) {
                ctx->argumentv[ctx->argumentc++] = (char*)arg;
            }
        }
    }

    return i;
}
//TODO: Support strings and numbers with optional argument
int cp_parse(Cp_Ctx *ctx) {
    return cp_parseUntil(ctx, 0, NULL);
}

//TODO: TODO
void cp_usage(Cp_Ctx *ctx, FILE *file) {
    fprintf(file, "Hello, World!\n");
}
#endif

#endif