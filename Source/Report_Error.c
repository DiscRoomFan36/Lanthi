
#include <stdio.h>
#include <stdlib.h>

#include "Report_Error.h"

#include "String_View.h"
#include "arena.h"
#include "context.h"


// TODO maybe make a common file to define this common thing
#define local static


// ANSI Terminal coloring options

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


// Helper function for print line, cuz the ANSI stuff is pretty verbose.
local void print_file_and_line(FILE *__restrict__ __stream, const char *file, s64 line, s64 col_num) {
    fprintf(__stream, ANSI_COLOR_RED"%s:"ANSI_COLOR_YELLOW"%ld:%ld:"ANSI_COLOR_RESET, file, line, col_num);
}


void cleanup_and_exit(int status) {
    // TODO put this under a verbose flag or something
    printf("\nSafely cleaning up before exit.\n");

    Context *context = get_context();

    Arena_free(&context->ast_arena);
    Arena_free(&context->string_arena);

    SV_free(&context->file);

    exit(status);
}


void report_Tokenizer_error(Tokenizer *t, SV line_in_question, const char *reason, const char *maybe_help_text) {
    // report the error
    print_file_and_line(stderr, t->filename, t->line_num, t->col_num);
    fprintf(stderr, " "ANSI_COLOR_RED"Tokenizer Error:"ANSI_COLOR_RESET" %s\n", reason);

    // TODO maybe highlight bad char in question? use ^ trick with padding
    fprintf(stderr, "%4ld |"SV_Fmt"\n", t->line_num, SV_Arg(line_in_question));
    fprintf(stderr, "     |%*s\n", (int)t->col_num, "^");

    if (maybe_help_text) {
        fprintf(stderr, ANSI_COLOR_CYAN"note:"ANSI_COLOR_RESET" %s\n", maybe_help_text);
    }

    // clean up safely
    cleanup_and_exit(1);
}


