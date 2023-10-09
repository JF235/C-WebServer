%{
    #include "parser_bison.tab.h"
%}

%s options

word                [^:,\r\n \t]+
special_word        [^, \r\n\t][^,\r\n\t]+[^, \r\n\t]
comentario          ^#.*
dnt_except          DNT
http_methods        [A-Z]+
comma               [ ]*,[ ]*
colons              [ ]*:[ ]*

%%

{comentario}                    {;}
[ \t]+                          {;}
{dnt_except}                    {yylval.str = strdup(yytext);return WORD;}  
{http_methods}                  {yylval.str = strdup(yytext);return HTTP_METHOD;}
{word}                          {yylval.str = strdup(yytext);return WORD;}
<options>{special_word}         {yylval.str = strdup(yytext);return WORD;}
[\r]?\n                         {BEGIN(0);return '\n';}
{comma}                         {return COMMA;}
{colons}                        {BEGIN(options);return COLONS;}

%%
