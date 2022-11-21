%{
#include "scanType.h" // TokenData Type
#include "treeNode.h"
#include "parser.tab.h"

char *lastToken;
int line = 1;
int numErrors;    // ERR err count
//int numWarns;


static int setVal(int linenum, int tokenClass, char *svalue){

    // create pass-back data space
    yylval.tokenData = new TokenData;

    // fills up data space
    yylval.tokenData->linenum = linenum;
    yylval.tokenData->tokenstr = strdup(svalue); //duplicates string

    if (tokenClass == NUMCONST){
      yylval.tokenData->nvalue = atoi(svalue);
    }
    else if (tokenClass == ID){
      yylval.tokenData->svalue = strdup(svalue);
    }
    else if (tokenClass == BOOLCONST){
      if(strcmp(svalue, "true") == 0){
          yylval.tokenData->nvalue = 1;
          yylval.tokenData->svalue = strdup(svalue);
        }
        else{
          yylval.tokenData->nvalue = 0;
          yylval.tokenData->svalue = strdup(svalue);
        }
    }
    else if(tokenClass == CHARCONST){
      yylval.tokenData->cvalue = strdup(svalue);
    }
    else if(tokenClass == STRINGCONST){
      yylval.tokenData->svalue = strdup(svalue);
      yylval.tokenData->nvalue = strlen(svalue) - 2;
    }
    return tokenClass;
}

%}

%option noyywrap
%option yylineno

%%

"true"                  {return setVal(line, BOOLCONST, yytext);} 
"false"                 {return setVal(line, BOOLCONST, yytext);}
"bool"                  {return setVal(line, BOOL, yytext);}
"char"                  {return setVal(line, CHAR, yytext);}
"if"                    {return setVal(line, IF, yytext);}
"else"                  {return setVal(line, ELSE, yytext);}
"then"                  {return setVal(line, THEN, yytext);}
"return"                {return setVal(line, RETURN, yytext);}
"int"                   {return setVal(line, INT, yytext);}
"to"                    {return setVal(line, TO, yytext);}
"do"                    {return setVal(line, DO, yytext);}
"break"                 {return setVal(line, BREAK, yytext);}
"by"                    {return setVal(line, BY, yytext);}
"and"                   {return setVal(line, AND, yytext);}
"or"                    {return setVal(line, OR, yytext);}
"not"                   {return setVal(line, NOT, yytext);}
"while"                 {return setVal(line, WHILE, yytext);}
"for"                   {return setVal(line, FOR, yytext);}
"static"                {return setVal(line, STATIC, yytext);}
[a-zA-Z][a-zA-Z0-9]*    {return setVal(line, ID, yytext);}
[0-9]+                 {return setVal(line, NUMCONST, yytext);}
\'(\\.|[^'\n])\'                  {return setVal(line, CHARCONST, yytext);}
\"(\\.|[^"\n])*\"    {return setVal(line, STRINGCONST, yytext);}
=                       {return setVal(line, ASGN, yytext);}
\+                      {return setVal(line, ADD, yytext);}
\-                      {return setVal(line, SUB, yytext);}
\*                      {return setVal(line, MUL, yytext);}
\/                      {return setVal(line, DIV, yytext);}
%                       {return setVal(line, MOD, yytext);}
"=="                    {return setVal(line, EQ, yytext);}
"+="                    {return setVal(line, ADDASS, yytext);}
"-="                    {return setVal(line, SUBASS, yytext);}
"/="                    {return setVal(line, DIVASS, yytext);}
"*="                    {return setVal(line, MULASS, yytext);}
"--"                    {return setVal(line, DEC, yytext);}
"++"                    {return setVal(line, INC, yytext);}
","                     {return setVal(line, COMMA, yytext);}
\<                      {return setVal(line, LT, yytext);}
\>                      {return setVal(line, GT, yytext);}
"<="                    {return setVal(line, LEQ, yytext);}
">="                    {return setVal(line, GEQ, yytext);}
"!="                    {return setVal(line, NEQ, yytext);}
"("                     {return setVal(line, OPENPAREN, yytext);}
")"                     {return setVal(line, CLOSEPAREN, yytext);}
"["                     {return setVal(line, OPENBRACKET, yytext);}
"]"                     {return setVal(line, CLOSEBRACKET, yytext);}
"{"                     {return setVal(line, OPENCURL, yytext);}
"}"                     {return setVal(line, CLOSECURL, yytext);}
";"                     {return setVal(line, ';', yytext);}
":"                     {return setVal(line, COLON, yytext);}
"?"                     {return setVal(line, QUESTION, yytext);}


[ \t]                   ; //ignore white space
\/\/.*                  ;
\n                      {line++;}
.                       {printf("ERROR(%d): Invalid or misplaced input character: '%c'. Character Ignored.\n", line, yytext[0], numErrors++);}

%%
