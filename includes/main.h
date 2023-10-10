#ifndef MAIN_H
#define MAIN_H

#include "essentials.h"

#define DEBUG 1
#define LOG 0

extern FILE *yyin;
int yylex();
void yyerror(const char* s);
int yylex_destroy(void);

int main(int argc, char *argv[]);

#endif