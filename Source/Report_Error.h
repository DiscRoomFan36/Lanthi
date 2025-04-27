
#ifndef REPORT_ERROR_H_
#define REPORT_ERROR_H_

#include "tokenizer.h"


void cleanup_and_exit(int status);

void report_Tokenizer_error(Tokenizer *t, SV line_in_question, const char *reason, const char *maybe_help_text);

// TODO the ast must have a way to get good error info, maybe
// void report_AST_error(Tokenizer *t, SV line_in_question, const char *reason, const char *maybe_help_text);


#endif // REPORT_ERROR_H_
