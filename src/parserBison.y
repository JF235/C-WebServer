%{
    #include "../includes/essentials.h"

    extern CommandList *globalCmdList;
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
    | cmd_list COLONS param_list   {globalCmdList->tail->optionList = *$3;free($3); }
    | COLONS param_list {fprintf(stderr, "Erro: sem comando.\n"); freeOptionList($2); free($2);}
    | HTTP_METHOD WORD WORD            {addCommand(globalCmdList, $1);free($1);
                                        addOption(&(globalCmdList->tail->optionList), $2); free($2);
                                        addOption(&(globalCmdList->tail->optionList), $3); free($3);
                                        }
    ;
cmd_list:   cmd_list COMMA WORD        {addCommand(globalCmdList, $3);free($3);}
        |   WORD                       {addCommand(globalCmdList, $1);free($1);}
        ;
param_list  : /* empty */               {$$ = createOptionList();}
            | param_list COMMA WORD     {$$ = $1; addOption($$, $3); free($3);}
            | WORD                    {$$ = createOptionList(); addOption($$, $1); free($1);}
            ;


%%

void yyerror(const char* s) {
    fprintf(stderr, "Erro: %s\n", s);
}