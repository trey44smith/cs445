#include <stdlib.h>
#include <stdio.h> 
#include "printTree.h"
#include <string.h>

static void printSpaces(int spaces)
{
    int i;
    for(i = 0; i < spaces; i++)
    {
        printf(".   ");
    }
}

void printTree(TreeNode *tree, int indent, int sibling)
{
    int i;

    if(tree->nodekind == DeclK)
    {
        switch(tree->kind.decl)
        {
            case VarK:
                switch(tree->expType)
                {
                    case Integer:
                        if(tree->isArray)
                        {
                            printf("Var: %s of array of type int [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        else
                        {
                            printf("Var: %s of type int [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        break;
                    case boolean:
                        if(tree->isArray)
                        {
                            printf("Var: %s of array of type bool [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        else
                        {
                            printf("Var: %s of type bool [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        break;
                    case Char:
                        if(tree->isArray)
                        {
                            printf("Var: %s of array of type char [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        else
                        {
                            printf("Var: %s of type char [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        break;
                }
                break;
            case FuncK:
                switch(tree->expType)
                {
                    case Void:
                        printf("Func: %s returns type void [line: %d]\n", tree->attr.name, tree->lineno);
                        break;
                    case Integer:
                        printf("Func: %s returns type int [line: %d]\n", tree->attr.name, tree->lineno);
                        break;
                    case boolean:
                        printf("Func: %s returns type bool [line: %d]\n", tree->attr.name, tree->lineno);
                        break;
                    case Char:
                        printf("Func: %s returns type char [line: %d]\n", tree->attr.name, tree->lineno);
                        break;
                   case UndefinedType:
                        printf("Func: %s returns type void [line: %d]\n", tree->attr.name, tree->lineno);
                        break;
                    default:
                        printf("ERROR FUNC: Undefined Type [line: %d]\n",tree->lineno);
                        break;
                }
                break;
            case ParamK:
                switch(tree->expType)
                {
                    case Integer:
                        if(tree->isArray)
                        {
                            printf("Parm: %s of array of type int [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        else
                        {
                            printf("Parm: %s of type int [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        break;
                    case boolean:
                        if(tree->isArray)
                        {
                            printf("Parm: %s of array of type bool [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        else
                        {
                            printf("Parm: %s of type bool [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        break;
                    case Char:
                        if(tree->isArray)
                        {
                            printf("Parm: %s of array of type char [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        else
                        {
                            printf("Parm: %s of type char [line: %d]\n", tree->attr.name, tree->lineno);
                        }
                        break;
                }
                break;
            default:
                printf("error\n");
                break;
        }
    }
    else if(tree->nodekind == ExpK)
    {
        switch(tree->kind.exp)
        {
            case OpK:
                printf("Op: %s [line: %d]\n", tree->attr.name, tree->lineno);
                break;
            case ConstantK:
                switch (tree->expType)
                {
                    case Char:
                        if(tree->op == 1)
                        {
                            printf("Const %s [line: %d]\n", tree->attr.cvalue, tree->lineno);
                        }
                        else
                        {
                            printf("Const \"%s\" [line: %d]\n", tree->attr.string, tree->lineno);
                        }
                        break;
                    default:
                        printf("Const %s [line: %d]\n", tree->attr.name, tree->lineno);
                        break;
                }
                break;
            case IdK:
                printf("Id: %s [line: %d]\n", tree->attr.name, tree->lineno);
                break;
            case AssignK:
                printf("Assign: %s [line: %d]\n", tree->attr.name, tree->lineno);
                break;
            case CallK:
                printf("Call: %s [line: %d]\n", tree->attr.name, tree->lineno);
                break;
            default:
                printf("error\n");
                break;
        }
    }
    else if(tree->nodekind == StmtK)
    {
        switch(tree->kind.stmt)
        {
            case IfK:
                printf("If [line: %d]\n", tree->lineno);
                break;
            case WhileK:
                printf("While [line: %d]\n", tree->lineno);
                break;
            case ForK:
                printf("For [line: %d]\n", tree->lineno);
                break;
            case CompoundK:
                printf("Compound [line: %d]\n", tree->lineno);
                break;
            case RangeK:
                printf("Range [line: %d]\n", tree->lineno);
                break;
            case ReturnK:
                printf("Return [line: %d]\n", tree->lineno);
                break;
            case BreakK:
                printf("Break [line: %d]\n", tree->lineno);
                break;
            default:
                printf("error\n");
                 break;
         }
    }
    else
    {
        printf("ERROR: Unkown Node\n");
    }

    for(i = 0; i < MAXCHILDREN; i++)
    {
        if(tree->child[i] != NULL)
        {
            printSpaces(indent);
            printf("Child: %d ", i);
            printTree(tree->child[i], indent+1, 0);
        }
    }
    
    if(tree->sibling != NULL)
    {
        printSpaces(indent-1);
        printf("Sibling: %d ", sibling+1);
        printTree(tree->sibling, indent, sibling+1); 
    }
}

void semanticPrintTree(TreeNode *tree, int indent, int sibling, bool memTyping)
{
    int i;

    if(tree->nodekind == DeclK)
    {
        switch(tree->kind.decl)
        {
            case VarK:
                switch(tree->expType)
                {
                    case Integer:
                        if(tree->isArray)
                        {
                            printf("Var: %s is array of type int", tree->attr.name);
                            if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        else
                        {
                            printf("Var: %s of type int", tree->attr.name);
                            if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        break;
                    case boolean:
                        if(tree->isArray)
                        {
                            printf("Var: %s is array of type bool", tree->attr.name);
                            if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        else
                        {
                            printf("Var: %s of type bool", tree->attr.name);
                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        break;
                    case Char:
                        if(tree->isArray)
                        {
                            printf("Var: %s is array of type char", tree->attr.name);
                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        else
                        {
                            printf("Var: %s of type char", tree->attr.name);
                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        break;
                }
                break;
            case FuncK:
                switch(tree->expType)
                {
                    case Void:
                        printf("Func: %s returns type void", tree->attr.name);
                         if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        break;
                    case Integer:
                        printf("Func: %s returns type int", tree->attr.name);
                         if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        break;
                    case boolean:
                        printf("Func: %s returns type bool", tree->attr.name);
                         if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        break;
                    case Char:
                        printf("Func: %s returns type char", tree->attr.name);
                         if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        break;
                   case UndefinedType:
                        printf("Func: %s returns type void", tree->attr.name);
                         if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        break;
                    default:
                        printf("ERROR FUNC: Undefined Type [line: %d]\n",tree->lineno);
                        break;
                }
                break;
            case ParamK:
                switch(tree->expType)
                {
                    case Integer:
                        if(tree->isArray)
                        {
                            printf("Parm: %s is array of type int", tree->attr.name);
                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        else
                        {
                            printf("Parm: %s of type int", tree->attr.name);
                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        break;
                    case boolean:
                        if(tree->isArray)
                        {
                            printf("Parm: %s is array of type bool", tree->attr.name);
                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        else
                        {
                            printf("Parm: %s of type bool", tree->attr.name);
                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        break;
                    case Char:
                        if(tree->isArray)
                        {
                            printf("Parm: %s is array of type char", tree->attr.name);
                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        else
                        {
                            printf("Parm: %s of type char", tree->attr.name);
                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        break;
                }
                break;
            default:
                printf("error\n");
                break;
        }
    }
    else if(tree->nodekind == ExpK)
    {
        switch(tree->kind.exp)
        {
            case OpK:
                printf("Op: %s of type ", tree->attr.name); getExpType(tree->expType); 
                 if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                printf(" [line: %d]\n", tree->lineno);
                break;
            case ConstantK:
                switch (tree->expType)
                {
                    case Char:
                        if(tree->op == 1)
                        {
                            printf("Const %s of type", tree->attr.cvalue);
                                      if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        else
                        {
                            printf("Const \"%s\" of type", tree->attr.string);
                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        }
                        break;
                    default:
                        printf("Const %s of type ", tree->attr.name); getExpType(tree->expType);
                        if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                        break;
                }
                break;
            case IdK:
                printf("Id: %s of type ", tree->attr.name); getExpType(tree->expType);
                                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                break;
            case AssignK:
                printf("Assign: %s of type int", tree->attr.name);
                                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                break;
            case CallK:
                printf("Call: %s of type ", tree->attr.name); getExpType(tree->expType);
                                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                break;
            default:
                printf("error\n");
                break;
        }
    }
    else if(tree->nodekind == StmtK)
    {
        switch(tree->kind.stmt)
        {
            case IfK:
                printf("If");
                                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                break;
            case WhileK:
                printf("While");
                                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                break;
            case ForK:
                printf("For");
                                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                break;
            case CompoundK:
                printf("Compound");
                                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                break;
            case RangeK:
                printf("Range");
                                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                break;
            case ReturnK:
                printf("Return");
                                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                break;
            case BreakK:
                printf("Break");
                                             if(memTyping){
                                printf(" [mem: %s loc: %d size: %d]", returnMemKind(tree->memKind), tree->offset, tree->memSize);
                            }
                            printf(" [line: %d]\n", tree->lineno);
                break;
            default:
                printf("error\n");
                 break;
         }
    }
    else
    {
        printf("ERROR: Unkown Node\n");
    }

    for(i = 0; i < MAXCHILDREN; i++)
    {
        if(tree->child[i] != NULL)
        {
            printSpaces(indent);
            printf("Child: %d ", i);
            semanticPrintTree(tree->child[i], indent+1, 0, memTyping);
        }
    }
    
    if(tree->sibling != NULL)
    {
        printSpaces(indent-1);
        printf("Sibling: %d ", sibling+1);
        semanticPrintTree(tree->sibling, indent, sibling+1, memTyping); 
    }
}

void getExpType(ExpType t){

    switch(t){
        case Void:
            printf("void");
            break;
        case Integer:
            printf("int");
            break;
        case boolean:
            printf("bool");
            break;
        case Char:
            printf("char");
            break;
        case Equal:
            printf("Equal");
            break;
        case UndefinedType:
            printf("void");
            break;
        default: 
            printf("expType not found");
            break;
    }
}

char *returnMemKind(VarKind kind){
    char *str;

    switch(kind){
        case None:
            str = strdup("None");
            break;
        case Local:
            str = strdup("Local");
            break;
        case Global:
            str = strdup("Global");
            break;
        case Parameter:
            str = strdup("Parameter");
            break;
        case LocalStatic:
            str = strdup("LocalStatic");
            break;
        default:
            break;
    }

    return str;
}
