#define CLI_PARSER_IMPLEMENTATION
#include "cli-parser.h"

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
    bool test;
    char *file = NULL;
    char *name = NULL;
    double numb = CP_OPTK_NUMBER_INVALID;
    Cp_Opt opts[] = {
        {&test, OPTK_BOOL, "test", 't', "Sick test."},
        {&file, OPTK_STRING, "file", 0, "File to print.", "File which the program will print, whilst not removing its contents. Can be useful as a replacement for `cat`. Upon printing, exits the program successfully."},
        {&name, OPTK_STRING, "name", 'n', "Your name.", "Prints your name to the terminal screen."},
        {&numb, OPTK_NUMBER, "number", 'N', "Number to print."}
    };

    char *hi_name = NULL;
    const char *scmd_hi_name = "hi";
    Cp_Opt scmd_hi[] = {
        {&hi_name, OPTK_STRING, "name", 'n'}
    };

    char *say_say = NULL;
    const char *scmd_say_name = "say";
    Cp_Opt scmd_say[] = {
        {&say_say, OPTK_STRING, "name"}
    };
  
    char **argumentv = alloca( argc * sizeof(char*));
    Cp_Ctx *ctx = cp_newCtx(argc, argv, sizeof(opts)/sizeof(*opts), opts, argc, argumentv);
    if(ctx == NULL) {
        return 1;
    }
    
    int stopped = 0;
    const char *scmd_main[] = {scmd_hi_name, scmd_say_name};
    if((stopped = cp_parseUntil(ctx, 2, scmd_main)) == -1) {
        printf("ERROR: %s\n", cp_parse_opt_err);
        cp_freeCtx(ctx);
        return 1;
    }

    if(test) {
        printf("This is a very sick test\n");
    }
    if(numb == numb) {
        printf("Number: %lf\n", numb);
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

    if(stopped == argc) {
        printf("Success.\n");
        cp_freeCtx(ctx);
        return 0;
    }

    if(cp__streq(argv[stopped], scmd_hi_name)) {
        cp_freeCtx(ctx);
        ctx = cp_newCtx(argc - stopped, argv + stopped, sizeof(scmd_hi)/sizeof(*scmd_hi), scmd_hi, 0, NULL);
        if(ctx == NULL) {
            return -1;
        }

        if(cp_parse(ctx) == -1) {
            printf("ERROR: %s\n", cp_parse_opt_err);
            cp_freeCtx(ctx);
            return -1;
        }

        if(hi_name != NULL) {
            printf("Hello, %s! ", hi_name);
        }
        printf("How are you doing today?\n");
    } else { // we can assume `scmd_say_name` here
        printf("\n`say` started.\n\n");
        cp_freeCtx(ctx);

        argumentv = alloca( (argc - stopped) * sizeof(char*));
        ctx = cp_newCtx(argc - stopped, argv + stopped, sizeof(scmd_say)/sizeof(*scmd_say), scmd_say, argc - stopped, argumentv);
        if(ctx == NULL) {
            return -1;
        }

        if(cp_parse(ctx) == -1) {
            printf("ERROR: %s\n", cp_parse_opt_err);
            cp_freeCtx(ctx);
            return -1;
        }

        if(say_say != NULL) {
            printf("Hello, %s!\n", say_say);
        }
        for(int i = 1; i < ctx->argumentc; ++i) {
            const char *arg = ctx->argumentv[i];
            printf("Arg %d: %s\n", i, arg);
        }
    }

    cp_freeCtx(ctx);
    return 0;
}
