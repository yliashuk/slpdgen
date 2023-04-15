%{
#include <stdio.h>
#include <string.h>
#include "parser.tab.h"
extern YYSTYPE yylval;
extern int curr_line;
%}
%x comment
%%
"SECTION"      {return KW_SECTION;}
"END"          {return KW_SECTION_END;}
"HEADER"       {return KW_HEADER;}
"TYPE"         {return KW_TYPE;}
"CODE"         {return KW_CODE;}
"enum"         {return KW_ENUM;}
"struct"       {return KW_STRUCT;}

"BASE"         {return KW_S_BASE;}
"DEFINITIONS"  {return KW_S_DEFINITIONS;}
"MESSAGES"     {return KW_S_MESSAGES;}
"RULES"        {return KW_S_RULES;}

"from"         {return FROM;}
"to"           {return TO;}
"empty"        {return EMPTY;}
"reverse"      {return REVERSE;}
"none"         {return NONE;}
"local"        {return LOCAL;}
"remote"       {return REMOTE;}
[_0-9a-zA-Z][_a-zA-Z0-9]*  {yylval.str=strdup(yytext); return WORD;}

"{"             {return ST_BLOCK;}
"}"             {return END_BLOCK;}
"["             {return LSQB;}
"]"             {return RSQB;}
";"             {return SEPARATOR;}
":"             {return COLON;}
"::"            {return DOUBLE_COLON;}
"="             {return EQUAL;}
"=>"            {return THEREFORE;}
\n\r|\r\n|\n|\r {curr_line++;}


"/*"            BEGIN(comment);
                <comment>[^*\n]*
                <comment>"*"+[^*/\n]*
                <comment>"*"+"/"        BEGIN(INITIAL);

.
%%
