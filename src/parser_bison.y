%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include "../includes/parser_data_structure.h"
    #include "../includes/webspacemanager.h"
    #include "../includes/httpHelper.h"
    #include "../includes/main.h"
%}

%union {
    char* str;
    struct OptionList *optionList;
    struct CommandList *commandList;
}

%token COMMA COLONS
%token <str> WORD HTTP_METHOD
%type <optionList> param_list
%type <commandList> cmd_list

%%

input: input '\n' line
    | line
    ;
line: /* empty */
    | cmd_list COLONS param_list   {cmdList->tail->optionList = *$3;free($3); }
    | COLONS param_list {fprintf(stderr, "Erro: sem comando.\n"); freeOptionList($2); free($2);}
    | HTTP_METHOD WORD WORD            {addCommand(cmdList, $1);free($1);
                                        addOption(&(cmdList->tail->optionList), $2); free($2);
                                        addOption(&(cmdList->tail->optionList), $3); free($3);
                                        }
    ;
cmd_list:   cmd_list COMMA WORD        {addCommand(cmdList, $3);free($3);}
        |   WORD                       {addCommand(cmdList, $1);free($1);}
        ;
param_list  : /* empty */               {$$ = createOptionList();}
            | param_list COMMA WORD     {$$ = $1; addOption($$, $3); free($3);}
            | WORD                    {$$ = createOptionList(); addOption($$, $1); free($1);}
            ;


%%

void yyerror(const char* s) {
    fprintf(stderr, "Erro: %s\n", s);
}