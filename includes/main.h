#ifndef MAIN_H
#define MAIN_H

#include "../includes/parser_data_structure.h"
#include <stdio.h>

struct CommandList *cmdList;

extern FILE *yyin;

int yylex();
void yyerror(const char* s);
int yylex_destroy(void);

int main(int argc, char *argv[]);

#endif