#define CLI_PARSER_IMPLEMENTATION
#include "cli-parser.h"

#include <stdio.h>

void usage(Cp_Ctx *ctx, FILE *file) {
    int maxlen = 0;
    for (int i = 0; i < ctx->optc; ++i) {
        int len = strlen(ctx->optv[i].name);
        if (ctx->optv[i].short_name != '\0') {
            len += 2; // for ", X"
        }
        if (len > maxlen) maxlen = len;
    }

    printf("OPTIONS:\n");
    for(int i = 0; i < ctx->optc; ++i) {
        Cp_Opt opt = ctx->optv[i];
        printf("  %s", opt.name);
        if(opt.short_name != '\0') {
            printf(", %c", opt.short_name);
        }
        int printed_len = (opt.short_name != '\0')
            ? (int)strlen(opt.name) + 3  // "name, x"
            : (int)strlen(opt.name);
        int padding = maxlen - printed_len + 2;
        printf("%-*s", padding, "\0");
        printf(" : %s\n", opt.short_desc);
    }
}

int main(int argc, char *argv[]) {
    bool help;
    bool test;
    char *file = NULL;
    char *name = NULL;
    double numb = CP_OPTK_NUMBER_INVALID;
    Cp_Opt opts[] = {
        {&help, OPTK_BOOL, "help", 'h', "Prints this help message. Upon doing so, exits the program successfully."},
        {&test, OPTK_BOOL, "test", 't', "Sick test."},
        {&file, OPTK_STRING, "file", 0, "File to print.", "File which the program will print, whilst not removing its contents. Can be useful as a replacement for `cat`. Upon printing, exits the program successfully."},
        {&name, OPTK_STRING, "name", 'n', "Your name.", "Prints your name to the terminal screen."},
        {&numb, OPTK_NUMBER, "number", 'N', "Number to print."}
    };
    
    char **argumentv = alloca( argc * sizeof(char*));
    Cp_Ctx *ctx = cp_newCtx(argc, argv, sizeof(opts)/sizeof(*opts), opts, argc, argumentv);
    if(ctx == NULL) {
        return 1;
    }
    
    if(cp_parse(ctx) == -1) {
        printf("ERROR: %s\n", ctx->err);
        cp_freeCtx(ctx);
        return 1;
    }
    
    if(test) {
        printf("This is a very sick test\n");
    }
    if(numb == numb) {
        printf("Number: %lf\n", numb);
    }
    if(help) {
        usage(ctx, stdout);
        cp_freeCtx(ctx);
        return 0;
    }
    if(name != NULL) {
        printf("Hi, %s!\n", name);
    }
    if(file != NULL) {
        printf("File name: %s\n", file);
    }
    for(int i = 0; i < ctx->argumentc; ++i) {
        printf("Argument[%d] = %s\n", i, ctx->argumentv[i]);
    }

    cp_freeCtx(ctx);
    return 0;
}
