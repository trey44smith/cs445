#ifndef PRINT_TREE_H_
#define PRINT_TREE_H_

#include "treeNode.h"
#include "scanType.h"

void printTree(TreeNode *, int, int);

void semanticPrintTree(TreeNode *, int, int, bool);

void getExpType(ExpType t);

char *returnMemKind(VarKind kind);

#endif