
#ifndef REPORT_ERROR_H_
#define REPORT_ERROR_H_

#include <stdnoreturn.h>

#include "tokenizer.h"


noreturn void cleanup_and_exit(int status);

noreturn void report_Tokenizer_error(Tokenizer *t, const char *reason, const char *maybe_help_text);

// TODO the ast must have a way to get good error info, maybe
noreturn void report_AST_error(Tokenizer *t, const char *reason, const char *maybe_help_text);

noreturn void report_AST_unexpected_token(Tokenizer *t, const char *where_its_currently_parseing, Token unexpected, const char *maybe_help_text);


#endif // REPORT_ERROR_H_
