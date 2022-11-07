#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "treeNodes.h"
#include "symbolTable.h"
#include "semantic.h"

SymbolTable *st = new SymbolTable();
treeNode *currentFunction;

//extern int numErrors;
//extern int numWarns;
int numLoops;
bool siblingFlg = false;
bool returnFlg = false;
bool loopFlg = false;
const char* types[] = {"type void", "type int", "type bool", "type char", "type char", "equal", "undefined type", "error"};



void semantic(TreeNode *syntaxTree)
{

    ioSetup("output", Void, Integer);
    ioSetup("outputb", Void, boolean);
    ioSetup("outputc", Void, Char);
    ioSetup("input", Integer, Void);
    ioSetup("inputb", boolean, Void);
    ioSetup("inputc", Char, Void);
    ioSetup("outnl", Void, Void);

    insertNode(syntaxTree);
    st->applyToAll(checkUse);
    TreeNode *temp = st->lookup("main");

    if(temp == NULL){
        printf("ERROR(LINKER):  A function named 'main' with no parameters must be defined.\n");
        numErrors++;
    }
    else
    {
        if(temp->nodekind == DeclK && temp->kind.decl != FuncK){
            printf("ERROR(LINKER): A function named 'main' with no parameters must be defined.\n");
            numErrors++;
        }
    }
}

ExpType insertNode(TreeNode *t)
{
    int i, errType;
    ExpType c1, c2, c3, returns;
    bool scoped = false; 
    bool arr1F = false, arr2F = false, n1 = true, n2 = true;
    bool func = false;
    bool loop = false;
    TreeNode *temp, *temp2;

    
    if(t == NULL)
    {printf("this is an error");}

    //VALID
    if(t->nodekind == DeclK)
    {
        switch(t->kind.decl)
        {
            case VarK:
            if(t->child[0] != NULL){
                t->isInit = true;
                t->child[0]->isChecked = true;

                c1 = insertNode(t->child[0]);

                if(t->child[0]->nodekind == ExpK){
                    if(t->child[0]->kind.exp == ConstantK)
                        {   
                            if(t->child[0]->expType != t->expType)
                            {
                                printf("ERROR(%d): Variable '%s' is of %s but is being initialized with an expression of %s.\n", t->lineno, t->attr.name, types[t->expType], types[t->child[0]->expType]);
                                numErrors++;
                            }  
                        }
                    else if(t->child[0]->kind.exp == OpK)
                        {
                            if(t->child[0]->op == 7 && strncmp(t->child[0]->attr.name, "*", 1) == 0)
                            {
                                printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n", t->lineno, t->attr.name);
                                numErrors++;
                            }
                            else if(c1 != t->expType)
                            {
                                printf("ERROR(%d): Variable '%s' is of %s but is being initialized with an expression of %s.\n", t->lineno, t->attr.name, types[t->expType], types[temp->expType]);
                                numErrors++;
                            }
                        }
                    else{
                        if(t->child[0]->kind.exp == IdK)
                            {
                                temp2 = st->lookup(t->child[0]->attr.name);

                                if(temp2 != NULL)
                                { temp2->isUsed = true; }
                            }

                            printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n", t->lineno, t->attr.name);
                            numErrors++;
                    }    
                }
            }

                if(!st->insert(t->attr.name, t))        
                {
                    printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", t->lineno, t->attr.name,
                    (TreeNode*)st->lookup(t->attr.name)->lineno);
                    numErrors++;
                }

                if(st->depth() == 1){
                    t->var = Global;
                }
                else if(st->depth() > 1){
                    t->var = Local;
                    if(t->isStatic){
                        t->var = LocalStatic;
                    }
                }

                /*if(t->child[0] != NULL)              
                {
                    t->isInit = true;
                }*/

                returns = t->expType;
                break;
            case FuncK:                                  
                if(!st->insert(t->attr.name, t))         
                {
                    printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", t->lineno, t->attr.name, st->lookup(t->attr.name)->lineno);
                    numErrors++;
                }
                if(strncmp(t->attr.name, "main", 4) == 0){ 
                    t->isUsed = true; 
                }

                if(t->child[1] != NULL && t->child[1]->kind.stmt == CompoundK) 
                {
                    t->child[1]->enteredScope = true;
                }

                st->enter(t->attr.name);   
                currentFunction = t;             
                scoped = true;                          
                returns = t->expType;
                break;

            case ParamK:
            t->var = Parameter;
                if(!st->insert(t->attr.name, t))         
                {
                    printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", t->lineno, t->attr.name, st->lookup(t->attr.name)->lineno);
                    numErrors++;
                }
                t->isInit = true; 
                returns = t->expType;
                break;
            default:
                break;
        }
    }
    //INVALID
    else if(t->nodekind == ExpK)
    {
        switch(t->kind.exp)
        {
            case OpK:

                if(t->child[0] != NULL)
                {
                    t->child[0]->isChecked = true; 

                    if(t->child[0]->kind.exp == IdK)
                    {
                        temp = st->lookup(t->child[0]->attr.name);
                        //printf("YEERRRRRR\n");

                        if(temp != NULL) 
                        {
                            
                            n1 = false;
                            if(temp->isArray == true)
                            { arr1F = true; }
                        }
                    }
                }
                if(t->child[1] != NULL)
                {
                    t->child[1]->isChecked = true; 

                    if(t->child[1]->kind.exp == IdK)
                    {
                        temp2 = st->lookup(t->child[1]->attr.name);
                         //printf("YOOOOOO\n");

                        if(temp2 != NULL) 
                        {
                           
                            n2 = false;
                            if(temp2->isArray == true)
                            { arr2F = true; }
                        }
                    }
                }
                switch(t->op)
                {
                    case 1:     //or
                    case 2:     //and
                        c1 = insertNode(t->child[0]);
                        c2 = insertNode(t->child[1]);

                        if(c1 == UndefinedType)
                        {  }
                        else if(c1 != boolean)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", t->lineno, t->attr.name, types[2], types[c1]);
                            numErrors++;
                        }

                        if(c2 == UndefinedType)
                        { }
                        else if(c2 != boolean)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but rhs is of %s.\n", t->lineno, t->attr.name, types[2], types[c2]);
                            numErrors++;
                        }

                        if(arr1F || arr2F)
                        {
                            printf("ERROR(%d): The operation '%s' does not work with arrays.\n", t->lineno, t->attr.name);
                            numErrors++;  
                        }
                        t->expType = boolean;
                        break;

                    case 3:     //not
                        c1 = insertNode(t->child[0]);
                        if(c1 == UndefinedType)
                        {}
                        else if(c1 != boolean)
                        {
                            printf("ERROR(%d): Unary '%s' requires an operand of %s but was given %s.\n", t->lineno, t->attr.name, types[2], types[c1]);
                            numErrors++;
                        }

                        if(arr1F == true)
                        {
                            printf("ERROR(%d): The operation '%s' does not work with arrays.\n", t->lineno, t->attr.name);
                            numErrors++;
                        }

                        t->expType = boolean;
                        break;

                    case 4:     //relop
                        c1 = insertNode(t->child[0]);
                        c2 = insertNode(t->child[1]);
                        if(strncmp(t->attr.name, "==", 2)== 0 || strncmp(t->attr.name, "!=", 2)== 0)
                        {
                            if(c1 == Void || c2 == Void)
                            {
                                if(c1 == Void)
                                {
                                    printf("ERROR(%d): '%s' requires operands of type bool, char, or int but lhs is of %s.\n", t->lineno, t->attr.name, types[c1]);
                                    numErrors++;
                                }
                                if(c2 == Void)
                                {
                                    printf("ERROR(%d): '%s' requires operands of type bool, char, or int but rhs is of %s.\n", t->lineno, t->attr.name, types[c2]);
                                    numErrors++;
                                }
                            }
                            else
                            {
                                if(c1 == UndefinedType || c2 == UndefinedType)
                                {  }
                                else if(c1 != c2)                      
                                { 
                                    printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n", t->lineno, t->attr.name, types[c1], types[c2]);
                                    numErrors++;
                                } 
                            }
                            t->expType = boolean;
                        }
                        else    // <=, <, >=, >
                        {
                            if(arr1F == true || arr2F == true)
                            {
                                printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n", t->lineno, t->attr.name, types[c1], types[c2]);
                                numErrors++;
                            }
                            else
                            {
                                if(c1 == Void || c2 == Void || c1 == boolean || c2 == boolean)
                                {
                                    if(c1 == Void || c1 == boolean)
                                    {
                                        printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n", t->lineno, t->attr.name, types[c1], types[c2]);
                                        numErrors++;

                                    }

                                    if(c2 == Void || c2 == boolean)
                                    {
                                        printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n", t->lineno, t->attr.name, types[c1], types[c2]);
                                        numErrors++;
                                    }
                                }
                                else
                                {
                                    if(c1 == UndefinedType || c2 == UndefinedType)
                                    { }
                                    else if(c1 != c2)
                                    {
                                        printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n", t->lineno, t->attr.name, types[c1], types[c2]);
                                        numErrors++;
                                    }
                                }
                            }
                            t->expType = boolean;
                        }
                        break;
                    case 5:     //sumop
                    case 6:     //mulop
                        c1 = insertNode(t->child[0]);
                        c2 = insertNode(t->child[1]);
                        if(c1 == UndefinedType)
                        {}
                        else if(c1 != Integer)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", t->lineno, t->attr.name, types[1], types[c1]);
                            numErrors++;
                        }

                        if(c2 == UndefinedType)
                        {}
                        else if(c2 != Integer)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but rhs is of %s.\n", t->lineno, t->attr.name, types[1], types[c2]);
                            numErrors++;
                        }

                        if(arr1F == true || arr2F == true)
                        {
                            printf("ERROR(%d): The operation '%s' does not work with arrays.\n", t->lineno, t->attr.name);
                            numErrors++;
                        }
                        t->expType = Integer;
                        break;
                    case 7:     //unaryop
                        c1 = insertNode(t->child[0]);

                        if(c1 == UndefinedType)
                        {}
                        else if(strncmp(t->attr.name, "*", 1)== 0)
                        {
                            if(arr1F == false)
                            {
                                printf("ERROR(%d): The operation '%s' only works with arrays.\n", t->lineno, t->attr.name);
                                numErrors++;
                            }
                        }
                        else    // - & ?
                        {
                            if(c1 != Integer)
                            {
                                printf("ERROR(%d): Unary '%s' requires an operand of %s but was given %s.\n", t->lineno, t->attr.name, types[1], types[c1]);
                                numErrors++;
                            }

                            if(arr1F == true)
                            {
                                printf("ERROR(%d): The operation '%s' does not work with arrays.\n", t->lineno, t->attr.name);
                                numErrors++;
                            }
                        }

                        t->expType = Integer;
                        break;
                    case 8:     // [
                        if(t->isInit)
                        {
                            if(!n1){ 
                                temp->isInit = true;
                                if(temp->nodekind == DeclK && temp->kind.decl == FuncK){
                                    func = true;
                                } 
                            }
                            else if(strncmp(t->child[0]->attr.name, "[", 1)== 0)
                            {
                                t->child[0]->isInit = true;
                            }

                        }
                        c1 = insertNode(t->child[0]);
                        c2 = insertNode(t->child[1]);
                        if(strncmp(t->child[0]->attr.name, "[", 1)== 0)
                        {
                            printf("ERROR(%d): Cannot index nonarray.\n",t->lineno);
                            numErrors++;
                        }
                        else if(func || c1 == UndefinedType){

                        }
                        else if((!n1 && !arr1F) || c1 == UndefinedType) 
                        {
                            printf("ERROR(%d): Cannot index nonarray '%s'.\n",t->lineno, t->child[0]->attr.name);
                            numErrors++;
                        }
                        if(!n2)
                        {
                            if(c2 == UndefinedType)
                            {}
                            else if(c2 != Integer)
                            {
                                printf("ERROR(%d): Array '%s' should be indexed by type int but got %s.\n", t->lineno, t->child[0]->attr.name, types[c2]);
                                numErrors++;
                            }  
                            if(arr2F && !temp2->isIndexed)
                            {
                                printf("ERROR(%d): Array index is the unindexed array '%s'.\n", t->lineno, temp2->attr.name);
                                numErrors++;
                            }
                        }
                        else
                        {
                            if(t->child[1]->expType == UndefinedType)
                            {  }
                            else if(t->child[1]->expType != Integer)
                            {
                                printf("ERROR(%d): Array '%s' should be indexed by type int but got %s.\n", t->lineno, t->child[0]->attr.name, types[t->child[1]->expType]);
                                numErrors++;
                            }  
                        }
                        t->child[0]->isIndexed = true;
                        t->expType = t->child[0]->expType; //lhs
                        break;
                    default:
                        break;
                }
                returns = t->expType;
                break;
            case ConstantK:
                returns = t->expType;
                break;
            case IdK:
                temp = st->lookup(t->attr.name);         
                if(temp == NULL)                            
                {
                    t->expType = UndefinedType;           
                    printf("ERROR(%d): Symbol '%s' is not declared.\n", t->lineno, t->attr.name);
                    numErrors++;
                }
                else                                      
                {
                    temp->isUsed = true;
                    if(temp->kind.decl == FuncK)            
                    {
                        temp->isFlagged = true;
                        t->expType = UndefinedType;
                        printf("ERROR(%d): Cannot use function '%s' as a variable.\n", t->lineno, t->attr.name);
                        numErrors++;
                    }
                    else                                
                    {
                        t->expType = temp->expType;
                    }  
                    if(!temp->isInit && !temp->isFlagged && temp->var != Global && temp->var != LocalStatic)
                    {
                        if(!t->isInit){
                                                    temp->isFlagged = true;
                       printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", t->lineno, temp->attr.name);
                       // printf("");
                        numWarns++;
                        }
                    } 
                }
               /* if(t != NULL && t->isExp == true)
                {
                    if(temp != NULL)
                    {
                        temp->isUsed = true;
                    }
                }*/
                returns = t->expType;
                break;
            case AssignK:  
                if(t->child[0] != NULL)
                {
                    t->child[0]->isChecked = true; 

                    if(t->child[0]->kind.exp == IdK)
                    {
                        temp = st->lookup(t->child[0]->attr.name);

                        if(temp != NULL) 
                        {

                            //printf("YEERRRRRR\n");
                            n1 = false;
                            if(temp->isArray == true)
                            { arr1F = true; }
                        }
                    }
                }
                if(t->child[1] != NULL)
                {
                    t->child[1]->isChecked = true;
                    if(t->child[1]->kind.exp == IdK)
                    {
                        temp2 = st->lookup(t->child[1]->attr.name);
                        if(temp2 != NULL)  
                        {
                            //printf("YEERRRRRR\n");
                            n2 = false;
                            if(temp2->isArray == true)
                            { arr2F = true; }
                        }
                        else
                        {
                            //printf("YEERRRRRR\n");
                            temp2 = t->child[0];   
                        }
                    }
                }
                switch(t->op)
                {
                    case 1:     // =
                        if(!n1)
                        { temp->isInit = true; }
                        else
                        { t->child[0]->isInit = true; }
                        c1 = insertNode(t->child[0]);
                        c2 = insertNode(t->child[1]);

                        if(!n1){
                            temp->isInit = true;
                        }
                        else{
                            t->child[0]->isInit = true;
                        }

                        if(c1 == UndefinedType || c2 == UndefinedType)
                        { }
                        else if(c1 == Void){
                            printf("ERROR(%d): '%s' requires operands of type bool, char, or int but lhs is of %s.\n", t->lineno, t->attr.name, types[c1]);
                            numErrors++;
                        }
                        else if(c2 == Void){
                            printf("ERROR(%d): '%s' requires operands of type bool, char, or int but rhs is of %s.\n", t->lineno, t->attr.name, types[c2]);
                            numErrors++;
                        }
                        else if(c1 != c2) 
                        {
                            printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n", t->lineno, t->attr.name, types[c1], types[c2]);
                            numErrors++;
                        }
                        t->expType = c1;
                        break;
                    case 2:     // +=
                    case 3:     // -=
                    case 4:     // *=
                    case 5:     // /=
                        c1 = insertNode(t->child[0]);
                        c2 = insertNode(t->child[1]);  
                        if(c1 == UndefinedType)
                        { }
                        else if(c1 != Integer)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", t->lineno, t->attr.name, types[1], types[c1]);
                            numErrors++;
                        }
                        if(c2 == UndefinedType)
                        {}
                        else if(c2 != Integer)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but rhs is of %s.\n", t->lineno, t->attr.name, types[1], types[c2]);
                            numErrors++;
                        }
                        t->expType = Integer;
                        break;
                    case 6:     // ++
                    case 7:     // --
                        c1 = insertNode(t->child[0]);
                        if(c1 != Integer)
                        {
                            printf("ERROR(%d): Unary '%s' requires an operand of %s but was given %s.\n", t->lineno, t->attr.name, types[1], types[c1]);
                            numErrors++;
                        }
                        t->expType = Integer;
                        break;
                }
                returns = t->expType;
                break;
            case CallK:
                temp = st->lookup(t->attr.name);         
                if(temp == NULL)                     
                {   
                    t->expType = UndefinedType;
                    printf("ERROR(%d): Symbol '%s' is not declared.\n", t->lineno, t->attr.name);
                    numErrors++;
                }
                else
                {
                    if(temp->kind.decl != FuncK)           
                    {
                        t->expType = temp->expType;
                        printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", t->lineno, t->attr.name);
                        numErrors++;
                    }
                    else
                    {
                        temp->isUsed = true;
                        t->expType = temp->expType;

                        if(t->child[0] != NULL){
                            t->child[0]->isChecked = true;
                        }


                        checkParams(temp, t, temp->child[0], t->child[0], 1);
                    }
                }
                returns = t->expType;
                break;
            default:
                break;
        }
    }
    //INVALID
    else if(t->nodekind == StmtK)
    {
        switch(t->kind.stmt)
        {
            case IfK:
                c1 = insertNode(t->child[0]);

                if(c1 == UndefinedType){

                }
                else if(c1 != boolean){
                    printf("ERROR(%d): Expecting Boolean test condition in %s statement but got %s.\n", t->lineno, getStmtKind(t->kind.stmt).c_str(), types[c1]);
                    numErrors++;
                }

                if(t->child[0] != NULL)
                { 
                    t->child[0]->isChecked = true; 
                    if(t->child[0]->nodekind == ExpK && t->child[0]->kind.exp == IdK)
                    { 
                        temp = st->lookup(t->child[0]->attr.name); 

                        if(temp != NULL && temp->isArray)
                        {
                            printf("ERROR(%d): Cannot use array as test condition in %s statement.\n", t->lineno, getStmtKind(t->kind.stmt).c_str());
                            numErrors++;
                        }
                    }
                }
                returns = Void;
                break;
            case WhileK:
                c1 = insertNode(t->child[0]);

                if(c1 == UndefinedType)
                { /*Do Nothing*/ }
                else if(c1 != boolean)
                {
                    printf("ERROR(%d): Expecting Boolean test condition in %s statement but got %s.\n", t->lineno, getStmtKind(t->kind.stmt).c_str(), types[c1]);
                    numErrors++;
                }

                if(t->child[0] != NULL)
                { 
                    t->child[0]->isChecked = true; 
                    if(t->child[0]->nodekind == ExpK && t->child[0]->kind.exp == IdK)
                    { 
                        temp = st->lookup(t->child[0]->attr.name); 

                        if(temp != NULL && temp->isArray)
                        {
                            printf("ERROR(%d): Cannot use array as test condition in %s statement.\n", t->lineno, getStmtKind(t->kind.stmt).c_str());
                            numErrors++;
                        }
                    }
                }

                loop = loopFlg = true;
                numLoops++;
                returns = Void;
                break;
            case CompoundK:
                if(!t->enteredScope)                  
                {
                    st->enter("Compound Scope"); 
                    scoped = true;
                }
                returns = Void;
                break;
            case RangeK:
                c1 = insertNode(t->child[0]);

                if(t->child[0] != NULL)
                { t->child[0]->isChecked = true; }

                if(t->child[1] != NULL)
                { t->child[1]->isChecked = true; }

                if(c1 == UndefinedType)
                { /* Do Nothing*/ }
                else if(c1 != Integer)
                {
                    printf("ERROR(%d): Expecting integer in range for loop statement but got %s.\n", t->lineno, types[c1]);
                    numErrors++;
                }

                c2 = insertNode(t->child[1]);

                if(c2 == UndefinedType)
                { /* Do Nothing*/ }
                else if(c2 != Integer)
                {
                    printf("ERROR(%d): Expecting integer in range for loop statement but got %s.\n", t->lineno, types[c2]);
                    numErrors++;
                }

                switch(t->op)
                {
                    //case 1:     ASSIGN simpleExpression RANGE simpleExpression
                    case 2:     //ASSIGN simpleExpression RANGE simpleExpression COLON simpleExpression
                        c3 = insertNode(t->child[2]);

                        if(t->child[2] != NULL)
                        { t->child[2]->isChecked = true; }

                        if(c3 == UndefinedType)
                        { /* Do Nothing*/ }
                        else if(c3 != Integer)
                        {
                            printf("ERROR(%d): Expecting integer in range for loop statement but got %s.\n", t->lineno, types[c3]);
                            numErrors++;
                        }

                        break;

                    default:
                        break;
                }
                returns = Void;
                break;
            case ReturnK:
                if(t->child[0] != NULL)
                {
                    if(t->child[0]->kind.exp == IdK)
                    {
                        temp = st->lookup(t->child[0]->attr.name);

                        if(temp != NULL && temp->isArray == true)
                        {
                            printf("ERROR(%d): Cannot return an array.\n",t->lineno);
                            numErrors++;
                        }
                    }
                }
                c1 = insertNode(t->child[0]);
 
                if(currentFunction != NULL)
                {
                    if(t->child[0] != NULL)
                    {
                        if(c1 == UndefinedType)
                        { /* Do Nothing */}
                        else if(currentFunction->expType == Void)
                        {
                            printf("ERROR(%d): Function '%s' at line %d is expecting no return value, but return has return value.\n", t->lineno, currentFunction->attr.name, currentFunction->lineno);
                            numErrors++;
                        }
                        else if(c1 != currentFunction->expType)
                        {
                            printf("ERROR(%d): Function '%s' at line %d is expecting to return %s but got %s.\n", t->lineno, currentFunction->attr.name, currentFunction->lineno, types[currentFunction->expType], types[c1]);
                            numErrors++;
                        }
                    }
                    else
                    {
                        if(currentFunction->expType != Void)
                        {
                            printf("ERROR(%d): Function '%s' at line %d is expecting to return %s but return has no return value.\n", t->lineno, currentFunction->attr.name, currentFunction->lineno, types[currentFunction->expType]);
                            numErrors++;  
                        }
                    }
                }

                returns = Void;
                returnFlg = true;
                break;
            case BreakK:
                if(!loopFlg)
                {
                    printf("ERROR(%d): Cannot have a break statement outside of loop.\n", t->lineno);
                    numErrors++;
                }            
                returns = Void;
                break;
            default:
                break;
         }
    }
    //VALID
    for(i = 0; i < MAXCHILDREN; i++)
    {
        if(t->child[i] != NULL && t->child[i]->isChecked == false)
        {
            insertNode(t->child[i]);
        }
    }
    //VALID
    if(scoped)                          
    {
        if(strncmp(currentFunction->attr.name, st->scope().c_str(), 10) == 0)
        {
            if(!returnFlg && currentFunction->expType != Void)
            {
                printf("WARNING(%d): Expecting to return %s but function '%s' has no return statement.\n", t->lineno, types[currentFunction->expType], currentFunction->attr.name);
                numWarns++;
            }
            else
           

        st->applyToAll(checkUse);
        st->leave();
    }
    
    if(loop)
    {
        numLoops--;
    }

    if(numLoops == 0)
    {
        loopFlg = false;
    }


    //VALID
    if(t->sibling != NULL)
    {
        insertNode(t->sibling); 
    }
    return returns;
    }
}

//INVALID?
void checkUse(std::string sym, void* t)
{
  TreeNode *temp;
    temp = st->lookup(sym.c_str());

    if(temp != NULL)
    {
        if(temp->isUsed == false)
        {
            if(temp->nodekind == DeclK && temp->kind.decl == FuncK)
            {
                printf("WARNING(%d): The function '%s' seems not to be used.\n", temp->lineno, temp->attr.name);
                //printf("CHECK\n");
                numWarns++;
            }
            else if(temp->nodekind == DeclK && temp->kind.decl == ParamK)
            {
                printf("WARNING(%d): The parameter '%s' seems not to be used.\n", temp->lineno, temp->attr.name);
                //printf("CHECK1\n");
                numWarns++;
            }
            else
            {
                printf("WARNING(%d): The variable '%s' seems not to be used.\n", temp->lineno, temp->attr.name);
                //printf("CHECK2\n");
                numWarns++;  
            }
        }
    }
}

void ioSetup(const char *funcName, ExpType returnType, ExpType paramType)
{
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    TreeNode *c = (TreeNode *) malloc(sizeof(TreeNode));

    if(t != NULL)
    {
        for(int i = 0; i < MAXCHILDREN; i++)
        { 
            t->child[i] = NULL;
        }

        t->sibling = NULL;
        t->nodekind = DeclK;
        t->kind.decl = FuncK;
        t->attr.name = (char *) funcName;
        t->expType = returnType;
        t->isUsed = true;
        t->lineno = -1;

        if(paramType != Void && c != NULL)
        {
            for(int i = 0; i < MAXCHILDREN; i++)
            { 
                c->child[i] = NULL;
            }

            c->sibling = NULL;
            c->nodekind = DeclK;
            c->kind.decl = ParamK;
            c->attr.name = "*dummy*";
            c->expType = paramType;
            c->lineno = -1;

            t->child[0] = c;
        }

        if(!st->insert(t->attr.name, t))
        {printf("Error inserting IO function %s \n", t->attr.name);}
    }    
}

void checkParams(TreeNode *funcNode, TreeNode *callNode, TreeNode *funcParam, TreeNode *callParam, int paramNum)
{
    if(funcParam != NULL && callParam != NULL)
    {
        bool tempFlg = siblingFlg;
        siblingFlg = false;
        insertNode(callParam);
        siblingFlg = tempFlg;      //fixes call within call error with params

        if(callParam->expType == UndefinedType)
        { }
        else if(funcParam->expType != callParam->expType)
        {
            printf("ERROR(%d): Expecting %s in parameter %d of call to '%s' declared on line %d but got %s.\n", callNode->lineno, types[funcParam->expType], paramNum, funcNode->attr.name, funcNode->lineno, types[callParam->expType]);
            numErrors++;
        }

        if(callParam->nodekind == ExpK && (callParam->kind.exp == IdK || callParam->kind.exp == CallK || callParam->kind.exp == OpK))
        {
            TreeNode *temp = st->lookup(callParam->attr.name);

            if(temp != NULL)
            {
                if(temp->isArray && !funcParam->isArray)
                {
                    printf("ERROR(%d): Not expecting array in parameter %d of call to '%s' declared on line %d.\n", callNode->lineno, paramNum, funcNode->attr.name, funcNode->lineno);
                    numErrors++;
                }   
                else if(!temp->isArray && funcParam->isArray)
                {
                    printf("ERROR(%d): Expecting array in parameter %d of call to '%s' declared on line %d.\n", callNode->lineno, paramNum, funcNode->attr.name, funcNode->lineno);
                    numErrors++;
                }
            }
            else if(callParam->kind.exp == OpK && funcParam->isArray){
                printf("ERROR(%d): Expecting array in parameter %d of call to '%s' declared on line %d.\n", callNode->lineno, paramNum, funcNode->attr.name, funcNode->lineno);
                numErrors++;
            }
        }

        paramNum++;
        checkParams(funcNode, callNode, funcParam->sibling, callParam->sibling, paramNum); 
    }
    else if(funcParam == NULL && callParam != NULL)
    {
        printf("ERROR(%d): Too many parameters passed for function '%s' declared on line %d.\n", callNode->lineno, funcNode->attr.name, funcNode->lineno);
        numErrors++;  
        insertNode(callParam); 
    }
    else if (callParam == NULL && funcParam != NULL)
    {
        printf("ERROR(%d): Too few parameters passed for function '%s' declared on line %d.\n", callNode->lineno, funcNode->attr.name, funcNode->lineno);
        numErrors++;
    }
}

std::string getStmtKind(StmtKind t){

    switch(t){
       // std::string str;
        case NullK:
            //str = "NullK";
            std::cout << "NullK"; 
            break;
        case IfK:
            std::cout << "IfK";
            break;
        case WhileK:
            std::cout << "WhileK";
            break;
        case ForK:
            std::cout << "ForK";
            break;
        case CompoundK:
            std::cout << "CompoundK";
            break;
        case ReturnK:
             std::cout << "ReturnK";
            break;
        case BreakK:
              std::cout << "BreakK";
            break;
        case RangeK:
           std::cout << "RangeK";
            break;        
        default: 
            printf("stmtKind not found");
            break;
    }
    return NULL;
}