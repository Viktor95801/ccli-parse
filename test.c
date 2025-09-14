#define CLI_PARSER_IMPLEMENTATION
#include "cli-parser.h"

#include <stdio.h>

int main(int argc, char *argv[]) {
    bool help;
    char *file = NULL;
    char *name = NULL;
    double num = CP_OPTK_NUMBER_INVALID;
    Cp_Opt opts[] = {
        {&help, OPTK_BOOL, "help", 'h', "Prints this help message. Upon doing so, exits the program successfully."},
        {&file, OPTK_STRING, "file", 0, "File to print.", "File which the program will print, whilst not removing its contents. Can be useful as a replacement for `cat`. Upon printing, exits the program successfully."},
        {&name, OPTK_STRING, "name", 'n', "Your name.", "Prints your name to the terminal screen."},
        {&num, OPTK_NUMBER, "number", 'N', "Number to print."}
    };
    
    char *argumentv[argc];
    Cp_Ctx *ctx = cp_newCtx(argc, argv, sizeof(opts)/sizeof(*opts), opts, argc, argumentv);
    if(ctx == NULL) {
        return 1;
    }
    
    if(cp_parse(ctx) == -1) {
        printf("ERROR: %s\n", cp_parse_opt_err);
        cp_freeCtx(ctx);
        return 1;
    }
    
    if(help) {
        cp_usage(ctx, stdout);
        cp_freeCtx(ctx);
        return 0;
    }
    if(name != NULL) {
        printf("Hi, %s!\n", name);
    }
    if(file != NULL) {
        printf("File name: %s\n", file);
    }
    if(num == num) {
        printf("Number: %lf\n", num);
    }
    
    for(int i = 0; i < ctx->argumentc; ++i) {
        printf("Argument[%d] = %s\n", i, ctx->argumentv[i]);
    }

    cp_freeCtx(ctx);
    return 0;
}
