#ifndef TREE_UTIL_H_
#define TREE_UTIL_H_

#include "treeNodes.h"
#include "scanType.h"

treeNode *newExpNode(ExpKind);

treeNode *newStmtNode(StmtKind);

treeNode *newDeclNode(DeclKind);



#endif
