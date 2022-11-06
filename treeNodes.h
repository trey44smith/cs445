#ifndef TREENODE_H
#define TREENODE_H


typedef int OpKind;
enum NodeKind {DeclK, StmtK, ExpK};
enum DeclKind {VarK, FuncK, ParamK};
enum StmtKind {NullK, IfK, WhileK, ForK, CompoundK, ReturnK, BreakK, RangeK};
enum ExpKind {OpK, ConstantK, IdK, AssignK, InitK, CallK};
enum ExpType {Void, Integer, boolean, Char, Equal, UndefinedType};
enum VarKind {None, Local, Global, Parameter, LocalStatic};

const int MAXCHILDREN = 3;

typedef struct treeNode
{
    struct treeNode *child[MAXCHILDREN];   
    struct treeNode *sibling;            

    int lineno;                          
    NodeKind nodekind;   
    VarKind var;
    OpKind op;     
   //TokenData *tokenData;  
    //char *tokenstr;   
    //int linenum;   

    union                               
    {
	    DeclKind decl;                    
	    StmtKind stmt;                    
	    ExpKind exp;                      
    } kind;
    
    union                                 
    {               
        OpKind op;          
	    int value;                        
        char *cvalue;           
	    char *string;                     
	    char *name;                        
    } attr;  

    ExpType expType;		      
    bool isArray;                          
    bool isStatic;    
    bool isChild;
    bool isChecked;
    bool isInit;
    //bool isIO;
    bool isUsed;
    bool isFlagged;
    bool isIndexed;
    bool enteredScope;
    bool isExp;


} TreeNode;

typedef struct varData 
{
    int linenum;
    ExpType expType;
    bool isStatic; 
} VarData;


#endif