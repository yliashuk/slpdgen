#ifndef ANALYZER_H
#define ANALYZER_H

#include "Formater.h"

// Fields are declared in parser.y

extern bool isSuccess();
extern int yyparse(void);
extern "C" FILE *yyin;

extern Formater formater;

#endif // ANALYZER_H
