%{
#include <stdio.h>
#include <cstring>
#include <stdbool.h>
#include <iostream>
#include "scanType.h" // TokenData type
#include "treeUtil.h"
#include "printTree.h"
#include "ourgetopt.h"
#include "semantic.h"
#include "yyerror.h"
#include "symbolTable.h"

double vars[26];

extern int yylex();
extern int yyparse();
//extern int yylineno;
extern FILE *yyin;
//int numErrors;
int numWarns;



/*void addSibling(TreeNode *t1, TreeNode *t2)
{
    if(t1 != NULL) 
    {
        if(t1->sibling == NULL)
        {
            t1->sibling = t2;
        }
        else
        {
            addSibling(t1->sibling, t2);
        }
    }
    else
    {  printf("error2\n");
     //addSibling(t1->sibling, t2); 
     }
}*/

TreeNode *addSibling(TreeNode *t, TreeNode *s)
{
    if (s==NULL && numErrors==0) {
        printf("ERROR(SYSTEM): never add a NULL to a sibling list.\n");
        exit(1);
    }
    if (t!=NULL) { 
        TreeNode *tmp;

        tmp = t;
        while (tmp->sibling!=NULL) tmp = tmp->sibling;
        tmp->sibling = s; 
        return t;
    }
    return s;
}

void setType(VarData v, TreeNode *t1)
{
    if(t1 != NULL)
    {
        if(t1->expType == UndefinedType)
        {
            t1->expType = v.expType;
            t1->isStatic = v.isStatic;

            setType(v, t1->sibling);
        }
        else
        { return; }
    }
}

TreeNode* root;

%}

%union{
    VarData vardata;
    TokenData *tokenData;
    TreeNode *tree;
}

%token <tokenData> ID BOOLCONST NUMCONST CHARCONST STRINGCONST BOOL CHAR
%token <tokenData> STATIC IF ASGN ADD SUB MUL DIV MOD EQ
%token <tokenData> COMMA LT GT LEQ GEQ NEQ ADDASS SUBASS DIVASS MULASS ELSE THEN RETURN
%token <tokenData> OPENPAREN CLOSEPAREN OPENBRACKET CLOSEBRACKET OPENCURL CLOSECURL ';' COLON
%token <tokenData> INT QUESTION DEC INC TO DO BREAK BY AND OR NOT WHILE FOR

%type <vardata> typeSpec
%type <tokenData> unaryop mulop sumop relop

%type <tree>  declList decl varDecl funDecl scopedVarDecl varDeclList varDeclInit varDeclId
%type <tree> parms parmList parmTypeList parmIdList parmId
%type <tree> stmt expStmt compoundStmt localDecls stmtList selectStmt iterStmt iterRange returnStmt breakStmt
%type <tree> exp assignop simpleExp andExp unaryRelExp relExp sumExp mulExp unaryExp
%type <tree> factor mutable immutable call args argList constant matched unmatched

%define parse.error verbose

%%

/*
tokenlist : token tokenlist
          | token
          ;

  token     : ID          {printf("Line %d Token: ID Value: %s\n", $1->linenum, $1->svalue);}
            | BOOLCONST   {printf("Line %d Token: BOOLCONST Value: %d  Input: %s\n", $1->linenum, $1->nvalue, $1->svalue);}
            | NUMCONST    {printf("Line %d Token: NUMCONST Value: %d  Input: %d\n", $1->linenum, $1->nvalue, $1->nvalue);}
            | CHARCONST   {printf("Line %d Token: CHARCONST Value: %s  Input: %s\n", $1->linenum, $1->cvalue, $1->cvalue);}
            | STRINGCONST {printf("Line %d Token: STRINGCONST Value: %s  Len: %d  Input: %s\n", $1->linenum, $1->svalue, $1->nvalue, $1->svalue);}
            | BOOL        {printf("Line %d Token: BOOL\n", $1->linenum, $1->svalue);}
            | CHAR        {printf("Line %d Token: CHAR\n", $1->linenum, $1->svalue);}
            | STATIC      {printf("Line %d Token: STATIC\n", $1->linenum, $1->svalue);}
            | IF          {printf("Line %d Token: IF\n", $1->linenum, $1->svalue);}
            | ASGN        {printf("Line %d Token: =\n", $1->linenum, $1->svalue);}
            | ADD         {printf("Line %d Token: +\n", $1->linenum, $1->svalue);}
            | SUB         {printf("Line %d Token: -\n", $1->linenum, $1->svalue);}
            | MUL         {printf("Line %d Token: *\n", $1->linenum, $1->svalue);}
            | DIV         {printf("Line %d Token: /\n", $1->linenum, $1->svalue);}
            | MOD         {printf("Line %d Token: %%\n", $1->linenum, $1->svalue);}
            | EQ          {printf("Line %d Token: EQ\n", $1->linenum, $1->svalue);}
            | COMMA       {printf("Line %d Token: ,\n", $1->linenum, $1->svalue);}
            | LT          {printf("Line %d Token: <\n", $1->linenum, $1->svalue);}
            | GT          {printf("Line %d Token: >\n", $1->linenum, $1->svalue);}
            | LEQ         {printf("Line %d Token: LEQ\n", $1->linenum, $1->svalue);}
            | GEQ         {printf("Line %d Token: GEQ\n", $1->linenum, $1->svalue);}
            | NEQ         {printf("Line %d Token: NEQ\n", $1->linenum, $1->svalue);}
            | ADDASS      {printf("Line %d Token: ADDASS\n", $1->linenum, $1->svalue);}
            | SUBASS      {printf("Line %d Token: SUBASS\n", $1->linenum, $1->svalue);}
            | DIVASS      {printf("Line %d Token: DIVASS\n", $1->linenum, $1->svalue);}
            | MULASS      {printf("Line %d Token: MULASS\n", $1->linenum, $1->svalue);}
            | ELSE        {printf("Line %d Token: ELSE\n", $1->linenum, $1->svalue);}
            | THEN        {printf("Line %d Token: THEN\n", $1->linenum, $1->svalue);}
            | RETURN      {printf("Line %d Token: RETURN\n", $1->linenum, $1->svalue);}
            | OPENPAREN   {printf("Line %d Token: (\n", $1->linenum, $1->svalue);}
            | CLOSEPAREN  {printf("Line %d Token: )\n", $1->linenum, $1->svalue);}
            | OPENBRACKET  {printf("Line %d Token: [\n", $1->linenum, $1->svalue);}
            | CLOSEBRACKET  {printf("Line %d Token: ]\n", $1->linenum, $1->svalue);}
            | OPENCURL      {printf("Line %d Token: \{\n", $1->linenum, $1->svalue);}
            | CLOSECURL     {printf("Line %d Token: }\n", $1->linenum, $1->svalue);}
            | ';'     {printf("Line %d Token: ;\n", $1->linenum, $1->svalue);}
            | COLON         {printf("Line %d Token: :\n", $1->linenum, $1->svalue);}
            | INT           {printf("Line %d Token: INT\n", $1->linenum, $1->svalue);}
            | QUESTION      {printf("Line %d Token: ?\n", $1->linenum, $1->svalue);}
            | DEC           {printf("Line %d Token: DEC\n", $1->linenum, $1->svalue);}
            | INC         {printf("Line %d Token: INC\n", $1->linenum, $1->svalue);}
            | TO          {printf("Line %d Token: TO\n", $1->linenum, $1->svalue);}
            | DO          {printf("Line %d Token: DO\n", $1->linenum, $1->svalue);}
            | BREAK       {printf("Line %d Token: BREAK\n", $1->linenum, $1->svalue);}
            | BY          {printf("Line %d Token: BY\n", $1->linenum, $1->svalue);}
            | AND         {printf("Line %d Token: AND\n", $1->linenum, $1->svalue);}
            | OR          {printf("Line %d Token: OR\n", $1->linenum, $1->svalue);}
            | NOT         {printf("Line %d Token: NOT\n", $1->linenum, $1->svalue);}
            | WHILE       {printf("Line %d Token: WHILE\n", $1->linenum, $1->svalue);}
            | FOR         {printf("Line %d Token: FOR\n", $1->linenum, $1->svalue);}
            ; */

program : declList {root = $1;}
        ;

declList : declList decl {if($1 == NULL){$$=$2;}else{addSibling($1, $2); $$ = $1;}}
         | decl
         ;

decl     : varDecl
         | funDecl
         | error {$$ = NULL;}
         ;

varDecl  : typeSpec varDeclList ';' {setType($1, $2); $$=$2; yyerrok;}
         | error varDeclList ';' {$$ = NULL; yyerrok;}
         | typeSpec error ';' {$$ = NULL; yyerrok;}
         ;

scopedVarDecl : STATIC typeSpec varDeclList ';'  {setType($2, $3);$$ = $3; $$->isArray = true; yyerrok;} //handle the static
              | typeSpec varDeclList ';' {setType($1, $2); $$=$2; yyerrok;}
              ;

varDeclList : varDeclList COMMA varDeclInit {
                                        if($1 == NULL)
                                                {$$ = $3;}
                                        else
                                        {addSibling($1, $3);$$ = $1;
                                        }
                                        yyerrok;}
            | varDeclInit
            | varDeclList COMMA error {$$ = NULL;} 
            | error {$$ = NULL;}
            ;

varDeclInit : varDeclId
            | varDeclId COLON simpleExp {$$ = $1;$$->child[0] = $3;}
            | error COLON simpleExp {$$ = NULL; yyerrok;}
            ;

varDeclId : ID  {$$ = newDeclNode(VarK);$$->lineno = $1->linenum; $$->attr.name = $1->tokenstr;$$->expType = UndefinedType;}
          | ID OPENBRACKET NUMCONST CLOSEBRACKET {$$ = newDeclNode(VarK);$$->lineno = $1->linenum; $$->attr.name = $1->tokenstr;$$->isArray = true;$$->expType = UndefinedType;}
          | ID OPENBRACKET error {$$ = NULL;}
          | error CLOSEBRACKET {$$ = NULL; yyerrok;}
          ;

typeSpec : BOOL {$$.linenum = $1->linenum;$$.expType = boolean;}
         | CHAR {$$.linenum = $1->linenum;$$.expType = Char;}
         | INT  {$$.linenum = $1->linenum;$$.expType = Integer;}
         ;

funDecl : typeSpec ID OPENPAREN parms CLOSEPAREN compoundStmt {$$ = newDeclNode(FuncK);$$->child[0] = $4;$$->child[1] = $6;$$->expType = $1.expType; $$->attr.name = $2->tokenstr;$$->lineno = $1.linenum;}
        | ID OPENPAREN parms CLOSEPAREN compoundStmt {$$ = newDeclNode(FuncK);$$->child[0] = $3;$$->child[1] = $5;$$->attr.name = $1->tokenstr;$$->expType = UndefinedType;$1 -> linenum;$$->lineno = $1->linenum;}
        | typeSpec error {$$ = NULL;}
        | typeSpec ID OPENPAREN error {$$ = NULL;}
        | ID OPENPAREN error {$$ = NULL;}
        | ID OPENPAREN parms CLOSEPAREN error {$$ = NULL;}
        ;

parms : parmList  
      | { $$ = NULL; }
      ;

parmList : parmList ';' parmTypeList {
                                if($1 == NULL)
                                {
                                    $$ = $3;
                                }
                                else
                                {
                                    $$ = $1;
                                    addSibling($1, $3);
                                }
                            }
         | parmTypeList
         | parmList ';' error {$$ = NULL;}
         | error {$$ = NULL;}
         ;

parmTypeList : typeSpec parmIdList {setType($1, $2);$$ = $2;}
             | typeSpec error {$$ = NULL;}
             ;

parmIdList : parmIdList COMMA parmId {
                                if($1 == NULL)
                                {
                                    $$ = $3;
                                }
                                else
                                {
                                    addSibling($1, $3);
                                    $$ = $1;
                                }
                            }
           | parmId
           | parmIdList COMMA parmId {$$ = addSibling($1, $3); $$ = NULL;}
           | parmIdList COMMA error {$$ = NULL;}
           | error {$$ = NULL;}
           ;

parmId : ID {$$ = newDeclNode(ParamK); $$->attr.name = $1->tokenstr;$$->lineno = $1->linenum;$$->expType = UndefinedType;}
       | ID OPENBRACKET CLOSEBRACKET {$$ = newDeclNode(ParamK);$$->attr.name = $1->tokenstr;$$->isArray = true;$$->lineno = $1->linenum;$$->expType = UndefinedType;}
       ;

stmt : expStmt
     | compoundStmt
     | selectStmt
     | iterStmt
     | returnStmt
     | breakStmt
     ;

matched : IF error {$$ = NULL;}
        | IF error ELSE matched {$$ = NULL; yyerrok;}
        | IF error THEN matched ELSE matched {$$ = NULL; yyerrok;}
        | WHILE error DO matched {$$ = NULL; yyerrok;}
        | WHILE error {$$ = NULL;}
        | FOR ID ASGN error DO matched{$$ = NULL; yyerrok;}
        | FOR error {$$ = NULL;}
        ;

stmtList : stmtList stmt {if($1==NULL)
                                {$$ = $2;}
                        else{addSibling($1,$2); 
                                $$=$1;}
                                }
         | {$$=NULL;}
         ;

expStmt : exp ';' {$$ = $1;}
        | ';' {$$=NULL;}
        | error ';' {$$ = NULL; yyerrok;}
        ;

compoundStmt : OPENCURL localDecls stmtList CLOSECURL {$$ = newStmtNode(CompoundK);$$->child[0] = $2;$$->child[1] = $3;$$->lineno = $1->linenum; yyerrok;}
             ;

localDecls : localDecls scopedVarDecl {
                                if($1 == NULL)
                                {
                                    $$ = $2;
                                }
                                else
                                {
                                    addSibling($1, $2);
                                    $$ = $1;
                                }
                            }
           | {$$ = NULL;}
           ;

selectStmt : IF simpleExp THEN stmt {$$ = newStmtNode(IfK); $$->child[0] = $2; $$->child[1] = $4; $$->lineno = $1->linenum;}
           | IF simpleExp THEN stmt ELSE stmt {$$= newStmtNode(IfK); $$->child[0] = $2; $$->child[1] = $4; $$->child[2] = $6; $$->lineno = $1->linenum;}
           ;

iterStmt : WHILE simpleExp DO stmt {$$ = newStmtNode(WhileK); $$->child[0] = $2; $$->child[1] = $4; $$->lineno = $1->linenum;}  
         | FOR ID ASGN iterRange DO stmt {$$ = newStmtNode(ForK); $$->lineno = $2->linenum;
                                        treeNode *id = newExpNode(IdK); id->attr.name = $2->tokenstr; id->lineno = $2->linenum; $$->attr.name = $2->name;
                                        $$->child[0] = id; 
                                        $$->child[1] = $4; 
                                        $$->child[2] = $6; } // lil bit of a doozy
         ;

iterRange : simpleExp TO simpleExp {$$ = newStmtNode(RangeK); $$->child[0] = $1; $$->child[1] = $3; $$->lineno = $2->linenum;}
          | simpleExp TO simpleExp BY simpleExp {$$ = newStmtNode(RangeK); $$->child[0] = $1; $$->child[1] = $3; $$->child[2] = $5; $$->lineno = $2->linenum;}
          | simpleExp TO error {$$ = NULL;}
          | error BY error {$$ = NULL; yyerrok;}
          | simpleExp TO simpleExp BY error {$$ = NULL;}
          ;

unmatched : IF error THEN stmt {$$ = NULL; yyerrok;}
          | IF error THEN matched ELSE unmatched {$$ = NULL; yyerrok;}

returnStmt : RETURN ';' { $$ = newStmtNode(ReturnK); $$->lineno = $1->linenum;}
           | RETURN exp ';' {$$ = newStmtNode(ReturnK); $$->child[0] = $2; $$->lineno = $1->linenum; yyerrok;}
           | RETURN error ';' {$$ = NULL; yyerrok;}
           ;

breakStmt : BREAK ';' {$$ = newStmtNode(BreakK);$$->lineno = $1->linenum;}
          ;

exp : mutable assignop exp {$$ = newExpNode(AssignK); $$->child[0] = $1; $$->child[1] = $3;  $$->op = $2->op; $$->attr.name = $2->attr.name; $$->lineno = $2->lineno; }
    | mutable INC          {$$ = newExpNode(AssignK);$$->child[0] = $1;$$->op = 6;$$->attr.name = $2->tokenstr; $$->lineno = $2->linenum;}
    | mutable DEC          {$$ = newExpNode(AssignK);$$->child[0] = $1;$$->op = 7;$$->attr.name = $2->tokenstr; $$->lineno = $2->linenum;}
    | simpleExp
    | error assignop exp {$$ = NULL; yyerrok;}
    | mutable assignop error {$$ = NULL;}
    | error INC {$$ = NULL; yyerrok;}
    | error DEC {$$ = NULL; yyerrok;}
    ;

assignop : ASGN   {$$ = newExpNode(AssignK); $$->op = 1; $$->attr.name = $1->tokenstr; $$->lineno = $1->linenum;}
         | ADDASS {$$ = newExpNode(AssignK); $$->op = 3; $$->attr.name = $1->tokenstr; $$->lineno = $1->linenum;}
         | SUBASS {$$ = newExpNode(AssignK); $$->op = 3; $$->attr.name = $1->tokenstr; $$->lineno = $1->linenum;}
         | MULASS {$$ = newExpNode(AssignK); $$->op = 3; $$->attr.name = $1->tokenstr; $$->lineno = $1->linenum;}
         | DIVASS {$$ = newExpNode(AssignK); $$->op = 3; $$->attr.name = $1->tokenstr; $$->lineno = $1->linenum;}
         ;

simpleExp : simpleExp OR andExp {$$ = newExpNode(OpK); $$->child[0] = $1;$$->child[1] = $3;  $$->op = 1;$$->attr.name = $2->tokenstr; $$->lineno = $2->linenum;}
          | andExp
          | simpleExp OR error {$$ = NULL;}
          ;

andExp : andExp AND unaryRelExp {$$ = newExpNode(OpK);$$->child[0] = $1;$$->child[1] = $3;  $$->op = 2;$$->attr.name = $2->tokenstr; $$->lineno = $2->linenum; }
       | unaryRelExp
       | andExp AND error {$$ = NULL;}
       ;

unaryRelExp : NOT unaryRelExp {$$ = newExpNode(OpK);$$->child[0] = $2;$$->op = 3;$$->attr.name = $1->tokenstr; $$->lineno = $1->linenum;}
            | relExp
            | NOT error {$$ = NULL;}
            ;

relExp : sumExp relop sumExp  {$$ = newExpNode(OpK);$$->child[0] = $1;$$->child[1] = $3;  $$->op = 4;$$->attr.name = $2->tokenstr; $$->lineno = $2->linenum;}
       | sumExp
       ;

relop : GT 
      | GEQ 
      | LT 
      | LEQ 
      | EQ 
      | NEQ 
      ;

sumExp : sumExp sumop mulExp {$$ = newExpNode(OpK);$$->child[0] = $1;$$->child[1] = $3;  $$->op = 5;$$->attr.name = $2->tokenstr; $$->lineno = $2->linenum;}
      | mulExp
      | sumExp sumop error {$$ = NULL;}
      ;

sumop : ADD 
      | SUB
      ;

mulExp : mulExp mulop unaryExp  {$$ = newExpNode(OpK);$$->child[0] = $1;$$->child[1] = $3;  $$->op = 6;$$->attr.name = $2->tokenstr; $$->lineno = $2->linenum;}
       | unaryExp
       | mulExp mulop error {$$ = NULL;}
       ;

mulop : MUL 
      | DIV 
      | MOD 
      ;

unaryExp : unaryop unaryExp {$$ = newExpNode(OpK); $$->child[0] = $2;$$->op = 7;$$->attr.name = $1->tokenstr;$$->lineno = $1->linenum;}
         | factor
         |unaryop error {$$ = NULL;}
         ;

unaryop : SUB
        | MUL 
        | QUESTION 
        ;

factor : mutable
       | immutable
       ;

mutable : ID {$$ = newExpNode(IdK);  $$->attr.name = $1->tokenstr; $$->lineno = $1->linenum;}
        | ID OPENBRACKET exp CLOSEBRACKET {$$ = newExpNode(OpK); treeNode *id = newExpNode(IdK); id->attr.name = $1->tokenstr; id->lineno = $1->linenum;
                                                                $$->attr.name = $2->name;
                                                                $$->child[0] = id;
                                                                $$->child[1] = $3;$$->op = 8;$$->attr.name = $2->tokenstr;$$->lineno = $2->linenum;}
        ;

immutable : OPENPAREN exp CLOSEPAREN {$$=$2; yyerrok;}
          | call
          | constant
          | OPENPAREN error {$$ = NULL;}
          ;

call : ID OPENPAREN args CLOSEPAREN {$$ = newExpNode(CallK); $$->attr.name= $1->name;
                                        //$$->child[0] = id; 
                                        $$->child[0] = $3;
                                        $$->op = 8; 
                                        $$->attr.name = $1->tokenstr; 
                                        $$->lineno = $1->linenum; }
     | error OPENPAREN {$$ = NULL; yyerrok;}
     ;

args : argList 
     | { $$ = NULL; }
     ;

argList : argList COMMA exp {
                                if($1 == NULL)
                                {
                                    $$ = $3;
                                }
                                else
                                {
                                    addSibling($1, $3);
                                    $$ = $1;
                                }
                            yyerrok;}
        | exp
        | argList COMMA error {$$ = NULL;}
        ;

constant : NUMCONST { $$ = newExpNode(ConstantK); $$->attr.name = $1->tokenstr;$$->expType = Integer;$$->lineno = $1->linenum;}        
         | CHARCONST  { $$ = newExpNode(ConstantK);  $$->attr.cvalue = $1->cvalue;$$->expType = Char;$$->op = 1; $$->lineno = $1->linenum;}      
         | STRINGCONST { $$ = newExpNode(ConstantK); $$->attr.string = $1->svalue;$$->expType = Char;$$->lineno = $1->linenum;}   
         | BOOLCONST  {$$ = newExpNode(ConstantK); $$->attr.value = $1->nvalue;$$->attr.name = $1->tokenstr;$$->expType = boolean;$$->lineno = $1->linenum; }        
        ; 
%%




int main(int argc, char **argv)
{
    extern int opterr;
    extern int optind;
    extern char *optarg;
    int c; 
    int dflg = 0; 
    int pflg = 0; 
    int filerr = 0; 
    int Pflg = 0;
    int hflg = 0;
    char *oarg = NULL;
    FILE *filename;


    while ((c = ourGetopt(argc, argv, (char *)":dpPh")) != EOF)
    {
        switch(c)
        {
            case 'd':
                ++dflg;
                break;
            case 'p':
                ++pflg;
                break;
            case 'P':
                ++Pflg;
                break;
            case 'h':
                ++hflg;
                printf("usage: -c  [options] [sourcefile]\n");
                printf("options:\n");  
                printf("-d              - turn on parser debugging\n");
                printf("-D              - turn on symbol table debugging\n");
                printf("-h              - print this usage message\n");
                printf("-p              - print the abstract syntax tree\n");
                printf("-P              - print the abstract syntax tree plus type information\n");   
                break;        
            case '?':
                printf("Error\n");
                exit(1);
                break;
            default:
                printf("default\n");
                break;
        }
    }

    if(dflg) 
    {
        yydebug = 1;
    }

    if (optind < argc) 
    {
        oarg = argv[optind];
        filerr++;
        optind++;
    }

    if(filerr == 1)
    {
        filename = fopen(oarg, "r");

        if(filename == NULL)
        {
            printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", oarg);
            printf("Number of warnings: %d\n", numWarns);
            printf("Number of errors: %d\n", numErrors);
            exit(1);
        }
        else
        {
            yyin = filename;
        }
    }
    else
    {
        yyin = stdin;
    }

    //numErrors = 0;
    //numWarns = 0;

    initErrorProcessing();

    yyparse();
    semantic(root);
    //std::cout << "CHECK HERE" << std::endl;
    //std::cout << numErrors << std::endl;

    if(numErrors == 0 )
    {
        if(pflg) 
        {
            printTree(root, 1, 0);
        }

        //symbolTable = new SymbolTable(); 

        if(Pflg) 
        {
          
            semanticPrintTree(root, 1, 0);
        }
    }

    
    printf("Number of warnings: %d\n", numWarns);
    printf("Number of errors: %d\n", numErrors);

    return 0;
}
