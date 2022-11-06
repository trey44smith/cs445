#ifndef SEMANTIC_H_
#define SEMANTIC_H_
#include <stdio.h>
#include <stdlib.h>
#include <string>

extern int numErrors, numWarns;

void semantic(TreeNode *syntaxTree);

static void traverse(TreeNode *t, void (*preProcedure) (TreeNode *), void (*postProcedure) (TreeNode *));

static void nullProcedure(TreeNode *t);

ExpType insertNode(TreeNode *t);

void checkUse(std::string, void*);

ExpType typeCheck(TreeNode *t);

int getOpKind(TreeNode *t);

void ioSetup(const char*, ExpType, ExpType);

void checkParams(TreeNode *, TreeNode *, TreeNode *, TreeNode *, int);

//8:13AM version

#endif