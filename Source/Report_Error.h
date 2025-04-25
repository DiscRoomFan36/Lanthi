
#ifndef REPORT_ERROR_H_
#define REPORT_ERROR_H_

#include "tokenizer.h"


void cleanup_and_exit(int status);

// maybe add a help string?
void report_tokenizer_error(Tokenizer *t, SV line_in_question, const char *reason, const char *maybe_help_text);



#endif // REPORT_ERROR_H_
