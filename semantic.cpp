#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "treeNodes.h"
#include "symbolTable.h"
#include "semantic.h"

#define CONSTANTSIZE 1 

SymbolTable st;
TreeNode *currFunc;
int numLoops;
int Goffset = 0;
int Loffset;
bool returnFlg = false, siblingFlg = true, loopFlg = false, errFlg = false;
const char* types[] = {"type void", "type int", "type bool", "type char", "type char", "equal", "undefined type", "error"};

void semantic(TreeNode *syntaxTree)
{
    ioSetup("output", Void, Integer);
    ioSetup("outputb", Void, Boolean);
    ioSetup("outputc", Void, Char);
    ioSetup("input", Integer, Void);
    ioSetup("inputb", Boolean, Void);
    ioSetup("inputc", Char, Void);
    ioSetup("outnl", Void, Void);

    //st.print(pointerPrintAddr);

    insertNode(syntaxTree);
    st.applyToAll(checkUse);
    TreeNode *temp = st.lookupNode("main");

    if(temp == NULL)
    {
        printf("ERROR(LINKER): Procedure main is not declared.\n");
        numErrors++;
    }
    else
    {
        if(temp->nodekind == DeclK && temp->kind.decl != FuncK)
        {
            printf("ERROR(LINKER): Procedure main is not declared.\n");
            numErrors++; 
        }
    }
}

ExpType insertNode(TreeNode *t)
{
    int i, errType, tmpLoffset;
    ExpType c1, c2, c3, returns;
    bool scoped = false;    //Boolean to help recursively leave scopes
    bool arr1F = false, arr2F = false, n1 = true, n2 = true, func = false, loop = false;
    const char* stmt[] = { "null", "elsif", "if", "while" };
    TreeNode *temp, *temp2;

    //Check if Null -- return Error if true
    if(t == NULL)
    {return Error;}

    //VALID
    if(t->nodekind == DeclK)
    {
        switch(t->kind.decl)
        {
            case VarK:

                if(t->child[0] != NULL)                 //If Initializing
                {
                    t->isInit = true;
                    t->child[0]->isChecked = true;

                    c1 = insertNode(t->child[0]);


                    if(t->child[0]->nodekind == ExpK)
                    {
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
                        else
                        {
                            if(t->child[0]->kind.exp == IdK)
                            {
                                temp2 = st.lookupNode(t->child[0]->attr.name);

                                if(temp2 != NULL)
                                { temp2->isUsed = true; }
                            }

                            printf("ERROR(%d): Initializer for variable '%s' is not a constant expression.\n", t->lineno, t->attr.name);
                            numErrors++;
                        }
                    }
                    
                }

                if(st.depth() == 1)
                { t->var = Global;}
                else if(st.depth() > 1)
                {
                    if(t->isStatic)
                    { t->var = LocalStatic; }
                    else
                    { t->var = Local; } 
                }

                if(t->isArray)
                { t->size = CONSTANTSIZE + t->size;}
                else
                { t->size = CONSTANTSIZE; }

                if(!st.insert(t->attr.name, t))         //Already declared
                {
                    printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", t->lineno, t->attr.name, st.lookupNode(t->attr.name)->lineno);
                    numErrors++;
                }
                else{
                    if(t->var == Local)
                    {
                        t->offset = Loffset;
                        //printf("VAR %d off: %d\n", t->size, Loffset);
                        Loffset -= t->size;
                        //printf("VAR %d off: %d\n", t->size, Loffset);
                    }
                    else if(t->var == LocalStatic || t->var == Global)
                    {
                        t->offset = Goffset;
                        Goffset -= t->size;
                    }
                }


                if(t->isArray)
                { t->offset--; }
                returns = t->expType;
                break;

            case FuncK:                                  
                if(!st.insert(t->attr.name, t))         //Already declared
                {
                    printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", t->lineno, t->attr.name, st.lookupNode(t->attr.name)->lineno);
                    numErrors++;
                }
                
                if(strncmp(t->attr.name, "main", 4) == 0)
                { t->isUsed = true; }

                if(t->child[1] != NULL && t->child[1]->kind.stmt == CompoundK) //Set the enteredScope bool to true for the following compound statement
                {
                    t->child[1]->enteredScope = true;
                }
                
                t->var = Global;
                Loffset -= 2;
                st.enter(t->attr.name);                 //Enter a new scope
                currFunc = t;
                scoped = true;                          //Entered scope bool set
                returns = t->expType;
                break;

            case ParamK:
                t->var = Parameter;
                t->size = CONSTANTSIZE;
                if(!st.insert(t->attr.name, t))         //Already declared
                {
                    printf("ERROR(%d): Symbol '%s' is already declared at line %d.\n", t->lineno, t->attr.name, st.lookupNode(t->attr.name)->lineno);
                    numErrors++;
                }
                else
                {
                    t->offset = Loffset;
                    Loffset -= t->size;
                }
                t->isInit = true; //Technically initialized since being passed in
                returns = t->expType;
                break;

            default:
                returns = Error;
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
                    t->child[0]->isChecked = true;  //CAUTION

                    if(t->child[0]->kind.exp == IdK)
                    {
                        //t->child[0]->isChecked = true;  //CAUTION
                        temp = st.lookupNode(t->child[0]->attr.name);

                        if(temp != NULL)  //&& temp->kind.exp == IdK
                        {
                            n1 = false;
                            if(temp->isArray == true)
                            { arr1F = true; }
                        }
                    }
                }

                if(t->child[1] != NULL)
                {
                    t->child[1]->isChecked = true;  //CAUTION

                    if(t->child[1]->kind.exp == IdK)
                    {
                        temp2 = st.lookupNode(t->child[1]->attr.name);

                        if(temp2 != NULL)  //&& temp2->kind.exp == IdK
                        {
                            n2 = false;
                            if(temp2->isArray == true)
                            { arr2F = true; }
                        }
                    }
                }

                switch(t->op)
                {
                    case 1:     //OR
                    case 2:     //AND
                        c1 = insertNode(t->child[0]);
                        c2 = insertNode(t->child[1]);

                        if(c1 == UndefinedType)
                        { /*Do Nothing*/ }
                        else if(c1 != Boolean)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", t->lineno, t->attr.name, types[2], types[c1]);
                            numErrors++;
                        }

                        if(c2 == UndefinedType)
                        { /*Do Nothing*/ }
                        else if(c2 != Boolean)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but rhs is of %s.\n", t->lineno, t->attr.name, types[2], types[c2]);
                            numErrors++;
                        }

                        //Check if array
                        if(arr1F || arr2F)
                        {
                            printf("ERROR(%d): The operation '%s' does not work with arrays.\n", t->lineno, t->attr.name);
                            numErrors++;  
                        }

                        t->expType = Boolean;
                        break;

                    case 3:     //NOT
                        c1 = insertNode(t->child[0]);

                        if(c1 == UndefinedType)
                        {/*Do nothing*/}
                        else if(c1 != Boolean)
                        {
                            printf("ERROR(%d): Unary '%s' requires an operand of %s but was given %s.\n", t->lineno, t->attr.name, types[2], types[c1]);
                            numErrors++;
                        }

                        //Check if array
                        if(arr1F == true)
                        {
                            printf("ERROR(%d): The operation '%s' does not work with arrays.\n", t->lineno, t->attr.name);
                            numErrors++;
                        }
                        
                        t->expType = Boolean;
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
                                { /* Do nothing? */ }
                                else if(c1 != c2)                       //if they are not equal
                                { 
                                    printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n", t->lineno, t->attr.name, types[c1], types[c2]);
                                    numErrors++;
                                } 
                            }
                            t->expType = Boolean;
                        }
                        else    // <=, <, >=, >
                        {
                             //else
                            {
                                if(c1 == Void || c2 == Void || c1 == Boolean || c2 == Boolean)
                                {
                                    if(c1 == Void || c1 == Boolean)
                                    {
                                        printf("ERROR(%d): '%s' requires operands of type char or type int but lhs is of %s.\n", t->lineno, t->attr.name, types[c1]);
                                        numErrors++;
                                    }

                                    if(c2 == Void || c2 == Boolean)
                                    {
                                        printf("ERROR(%d): '%s' requires operands of type char or type int but rhs is of %s.\n", t->lineno, t->attr.name, types[c2]);
                                        numErrors++;
                                    }
                                }
                                else
                                {
                                    if(c1 == UndefinedType || c2 == UndefinedType)
                                    { /* Do nothing? */ }
                                    else if(c1 != c2)
                                    {
                                        printf("ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n", t->lineno, t->attr.name, types[c1], types[c2]);
                                        numErrors++;
                                    }
                                }
                            }

                            if(arr1F == true || arr2F == true)
                            {
                                printf("ERROR(%d): The operation '%s' does not work with arrays.\n", t->lineno, t->attr.name);
                                numErrors++;
                            }
                            t->expType = Boolean;
                        }
                        break;

                    case 5:     //sumop
                    case 6:     //mulop
                        c1 = insertNode(t->child[0]);
                        c2 = insertNode(t->child[1]);

                        if(c1 == UndefinedType)
                        {/*Do nothing*/}
                        else if(c1 != Integer)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", t->lineno, t->attr.name, types[1], types[c1]);
                            numErrors++;
                        }

                        if(c2 == UndefinedType)
                        {/*Do nothing*/}
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
                        {/*Do nothing*/}
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
                            if(!n1)
                            { 
                                temp->isInit = true; 
                                if(temp->nodekind == DeclK && temp->kind.decl == FuncK)
                                { func = true; }
                            }
                            else if(strncmp(t->child[0]->attr.name, "[", 1)== 0)
                            {
                                t->child[0]->isInit = true;
                            }

                        }

                        c1 = insertNode(t->child[0]);
                        c2 = insertNode(t->child[1]);

                        //Check that child[0] is an array
                        if(strncmp(t->child[0]->attr.name, "[", 1)== 0)
                        {
                            printf("ERROR(%d): Cannot index nonarray.\n",t->lineno);
                            numErrors++;
                        }
                        else if (func || c1 == UndefinedType)
                        { /*Do Nothing */ }
                        else if((!n1 && !arr1F)) //Careful
                        {
                            printf("ERROR(%d): Cannot index nonarray '%s'.\n",t->lineno, t->child[0]->attr.name);
                            numErrors++;
                        }

                        if(!n2)
                        {
                            //Ensure array is being indexed by an Integer
                            if(c2 == UndefinedType)
                            {/*Do Nothing */}
                            else if(c2 != Integer)
                            {
                                printf("ERROR(%d): Array '%s' should be indexed by type int but got %s.\n", t->lineno, t->child[0]->attr.name, types[c2]);
                                numErrors++;
                            }  

                            //Check if array is being used for index, it is indexed
                            if(arr2F && !temp2->isIndexed)
                            {

                                printf("ERROR(%d): Array index is the unindexed array '%s'.\n", t->lineno, temp2->attr.name);
                                numErrors++;
                            }
                        }
                        else
                        {
                            if(t->child[1]->expType == UndefinedType)
                            { /* Do nothing */ }
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
                temp = st.lookupNode(t->attr.name);         //Assign return of lookupNode to temporary TreeNode
                if(temp == NULL)                            //Not declared
                {
                    t->size = CONSTANTSIZE;
                    t->expType = UndefinedType;             //Set to undefined type
                    printf("ERROR(%d): Symbol '%s' is not declared.\n", t->lineno, t->attr.name);
                    numErrors++;
                }
                else                                        //Is declared
                {
                    temp->isUsed = true;
                    t->var = temp->var;
                    t->offset = temp->offset;
                    t->size = temp->size;
                    if(temp->kind.decl == FuncK)            //Error in calling a function as a variable
                    {
                        temp->isFlagged = true;
                        t->expType = UndefinedType;
                        printf("ERROR(%d): Cannot use function '%s' as a variable.\n", t->lineno, t->attr.name);
                        numErrors++;
                    }
                    else                                    //Assign the ID with a type   
                    {
                        t->expType = temp->expType;
                    }  

                    if(!temp->isInit && !temp->isFlagged && temp->var != Global && temp->var != LocalStatic)
                    {
                        if(!t->isInit)
                        {
                            temp->isFlagged = true;
                            printf("WARNING(%d): Variable '%s' may be uninitialized when used here.\n", t->lineno, temp->attr.name);
                            numWarnings++;
                        }
                    } 
                }

                returns = t->expType;
                break;

            case AssignK:  
                if(t->child[0] != NULL)
                {
                    t->child[0]->isChecked = true;  //CAUTION

                    if(t->child[0]->kind.exp == IdK)
                    {
                        temp = st.lookupNode(t->child[0]->attr.name);

                        if(temp != NULL)  //&& temp->kind.exp == IdK
                        {
                            n1 = false;
                            if(temp->isArray == true)
                            { arr1F = true; }
                        }
                    }
                }

                if(t->child[1] != NULL)
                {
                    t->child[1]->isChecked = true;  //CAUTION

                    if(t->child[1]->kind.exp == IdK)
                    {
                        temp2 = st.lookupNode(t->child[1]->attr.name);

                        if(temp2 != NULL)  //&& temp2->kind.exp == IdK
                        {
                            n2 = false;
                            if(temp2->isArray == true)
                            { arr2F = true; }
                        }
                        else
                        {
                            temp2 = t->child[0];     //Catch '[' case
                        }
                    }
                }

                switch(t->op)
                {
                    case 1:     // =
                        //Child 1 Initialized
                        t->child[0]->isInit = true;
                        c1 = insertNode(t->child[0]);
                        c2 = insertNode(t->child[1]);

                        if(!n1)
                        { temp->isInit = true; }
                        else
                        { t->child[0]->isInit = true; }

                        if(c1 == UndefinedType || c2 == UndefinedType)
                        { /* Do nothing? */ }
                        else if(c1 == Void)
                        {
                            printf("ERROR(%d): '%s' requires operands of type bool, char, or int but lhs is of %s.\n", t->lineno, t->attr.name, types[c1]);
                            numErrors++;
                        }
                        else if(c2 == Void)
                        {
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
                        {/* Do Nothing*/ }
                        else if(c1 != Integer)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but lhs is of %s.\n", t->lineno, t->attr.name, types[1], types[c1]);
                            numErrors++;
                        }

                        if(c2 == UndefinedType)
                        {/* Do Nothing*/ }
                        else if(c2 != Integer)
                        {
                            printf("ERROR(%d): '%s' requires operands of %s but rhs is of %s.\n", t->lineno, t->attr.name, types[1], types[c2]);
                            numErrors++;
                        }

                        if(arr1F || arr2F)
                        {
                            printf("ERROR(%d): The operation '%s' does not work with arrays.\n", t->lineno, t->attr.name);
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

                        if(arr1F == true)
                        {
                            printf("ERROR(%d): The operation '%s' does not work with arrays.\n", t->lineno, t->attr.name);
                            numErrors++;
                        }
                        t->expType = Integer;
                        break;
                }
                returns = t->expType;
                break;

            case CallK:
                temp = st.lookupNode(t->attr.name);         //Assign return of lookupNode to temporary TreeNode

                if(temp == NULL)                            //Not declared
                {  
                    t->expType = UndefinedType;
                    printf("ERROR(%d): Symbol '%s' is not declared.\n", t->lineno, t->attr.name);
                    numErrors++;
                }
                else
                {
                    if(temp->kind.decl != FuncK)            //Error in calling a function as a variable
                    {
                        t->expType = temp->expType;
                        printf("ERROR(%d): '%s' is a simple variable and cannot be called.\n", t->lineno, t->attr.name);
                        numErrors++;
                    }
                    else        //is a function
                    {
                        temp->isUsed = true;
                        t->expType = temp->expType;
                        
                        if(t->child[0] != NULL)
                        { t->child[0]->isChecked = true; }

                        checkParams(temp, t, temp->child[0], t->child[0], 1);
                    }
                }
                returns = t->expType;
                break;

            default:
                returns = Error;
                break;
        }
    }
    //INVALID
    else if(t->nodekind == StmtK)
    {
        switch(t->kind.stmt)
        {
            case ElsifK:
            case IfK:
                c1 = insertNode(t->child[0]);

                if(c1 == UndefinedType)
                { /*Do Nothing*/ }
                else if(c1 != Boolean)
                {
                    printf("ERROR(%d): Expecting Boolean test condition in %s statement but got %s.\n", t->lineno, stmt[t->kind.stmt], types[c1]);
                    numErrors++;
                }

                if(t->child[0] != NULL)
                { 
                    t->child[0]->isChecked = true; 
                    if(t->child[0]->nodekind == ExpK && t->child[0]->kind.exp == IdK)
                    { 
                        temp = st.lookupNode(t->child[0]->attr.name); 

                        if(temp != NULL && temp->isArray)
                        {
                            printf("ERROR(%d): Cannot use array as test condition in %s statement.\n", t->lineno, stmt[t->kind.stmt]);
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
                else if(c1 != Boolean)
                {
                    printf("ERROR(%d): Expecting Boolean test condition in %s statement but got %s.\n", t->lineno, stmt[t->kind.stmt], types[c1]);
                    numErrors++;
                }

                if(t->child[0] != NULL)
                { 
                    t->child[0]->isChecked = true; 
                    if(t->child[0]->nodekind == ExpK && t->child[0]->kind.exp == IdK)
                    { 
                        temp = st.lookupNode(t->child[0]->attr.name); 

                        if(temp != NULL && temp->isArray)
                        {
                            printf("ERROR(%d): Cannot use array as test condition in %s statement.\n", t->lineno, stmt[t->kind.stmt]);
                            numErrors++;
                        }
                    }
                }

                loop = loopFlg = true;
                numLoops++;
                returns = Void;
                break;

            case LoopK:
                if(t->child[2] != NULL)                     //Apparently Null checking makes this work if it is not a compound????
                {
                    if(t->child[2]->kind.stmt == CompoundK) //Set the enteredScope bool to true for the following compound statement
                    {
                        //compoundFlg = true;
                        t->child[2]->enteredScope = true;
                        // tmpLoffset = Loffset;
                        //Loffset = -2;
                    }
                }

                st.enter("Loop");
                tmpLoffset = Loffset;
                c1 = insertNode(t->child[0]);

                if(t->child[0] != NULL)
                {
                    t->child[0]->isChecked = true;

                    temp = st.lookupNode(t->child[0]->attr.name);

                    if(temp != NULL)
                    {
                        temp->isInit = true;
                    }
                }

                loop = loopFlg = true;
                numLoops++;
                scoped = true;
                returns = Void;
                break;

            case LoopForeverK:
                loop = loopFlg = true;
                numLoops++;
                returns = Void;
                break;

            case CompoundK:
                if(!t->enteredScope)                    //Check that it is not a function scope before
                {
                    st.enter("Compound Scope"); 
                    // compoundFlg = true;
                    scoped = true;
                    tmpLoffset = Loffset;
                    // Loffset = -2;
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
                    t->child[0]->isChecked = true;
                    if(t->child[0]->kind.exp == IdK)
                    {
                        temp = st.lookupNode(t->child[0]->attr.name);

                        if(temp != NULL && temp->isArray == true)
                        {
                            printf("ERROR(%d): Cannot return an array.\n",t->lineno);
                            numErrors++;
                        }
                    }
                }

                c1 = insertNode(t->child[0]);
 
                if(currFunc != NULL)
                {
                    if(t->child[0] != NULL)
                    {
                        if(c1 == UndefinedType)
                        { /* Do Nothing */}
                        else if(currFunc->expType == Void)
                        {
                            printf("ERROR(%d): Function '%s' at line %d is expecting no return value, but return has return value.\n", t->lineno, currFunc->attr.name, currFunc->lineno);
                            numErrors++;
                        }
                        else if(c1 != currFunc->expType)
                        {
                            printf("ERROR(%d): Function '%s' at line %d is expecting to return %s but got %s.\n", t->lineno, currFunc->attr.name, currFunc->lineno, types[currFunc->expType], types[c1]);
                            numErrors++;
                        }
                    }
                    else
                    {
                        if(currFunc->expType != Void)
                        {
                            printf("ERROR(%d): Function '%s' at line %d is expecting to return %s but return has no return value.\n", t->lineno, currFunc->attr.name, currFunc->lineno, types[currFunc->expType]);
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
                returns = Error;
                break;
         }
    }
    //printf("%d off: %d\n", t->lineno, Loffset);

    //VALID
    for(i = 0; i < MAXCHILDREN; i++)
    {
        if(t->child[i] != NULL && t->child[i]->isChecked == false)
        {
            insertNode(t->child[i]);
        }
    }

    //VALID
    if(scoped)                          //Leaves the scope as recursive function backtracks
    {
        if(strncmp(currFunc->attr.name, st.scope().c_str(), 10) == 0)
        {
            temp = st.lookupNode(currFunc->attr.name);
            if(temp != NULL && temp->nodekind == DeclK && temp->kind.decl == FuncK)
            { temp->size = Loffset;}
            Loffset = 0;
            if(!returnFlg && currFunc->expType != Void)
            {
                printf("WARNING(%d): Expecting to return %s but function '%s' has no return statement.\n", t->lineno, types[currFunc->expType], currFunc->attr.name);
                numWarnings++;
            }
            else
            { returnFlg = false; }
        }
        else 
        { 
            Loffset = tmpLoffset;
        }
        
        st.applyToAll(checkUse);
        st.leave();
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
    if(t->sibling != NULL && siblingFlg)
    {
        insertNode(t->sibling); 
    }

    return returns;
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
        t->attr.name = funcName;
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

        if(!st.insert(t->attr.name, t))
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
        { /* Do Nothing */ }
        else if(funcParam->expType != callParam->expType)
        {
            printf("ERROR(%d): Expecting %s in parameter %d of call to '%s' declared on line %d but got %s.\n", callNode->lineno, types[funcParam->expType], paramNum, funcNode->attr.name, funcNode->lineno, types[callParam->expType]);
            numErrors++;
        }

        if(callParam->nodekind == ExpK && (callParam->kind.exp == IdK || callParam->kind.exp == CallK || callParam->kind.exp == OpK))
        {
            TreeNode *temp = st.lookupNode(callParam->attr.name);

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
            else if(callParam->kind.exp == OpK && funcParam->isArray)
            {
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

void checkUse(std::string sym, void* t)
{
    TreeNode *temp;
    temp = st.lookupNode(sym.c_str());

    if(temp != NULL)
    {
        if(temp->isUsed == false)
        {
            if(temp->nodekind == DeclK && temp->kind.decl == FuncK)
            {
                printf("WARNING(%d): The function '%s' seems not to be used.\n", temp->lineno, temp->attr.name);
                numWarnings++;
            }
            else if(temp->nodekind == DeclK && temp->kind.decl == ParamK)
            {
                printf("WARNING(%d): The parameter '%s' seems not to be used.\n", temp->lineno, temp->attr.name);
                numWarnings++;
            }
            else
            {
                printf("WARNING(%d): The variable '%s' seems not to be used.\n", temp->lineno, temp->attr.name);
                numWarnings++;  
            }
        }
    }
}
