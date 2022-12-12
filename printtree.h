#ifndef _PRINT_TREE_H_
#define _PRINT_TREE_H_

#include "treeNodes.h"
#include "scantype.h"

/* descr: Print the saved AST tree
* param: TreeNode -> Pointer to the treeNode
* param: int -> Contains the amount of indents needed
* param: int -> Contains the Sibling number
* return: void
*/
void printTree(TreeNode *, int, int);

/* descr: Print the saved AST tree with semantic additions
* param: TreeNode -> Pointer to the treeNode
* param: int -> Contains the amount of indents needed
* param: int -> Contains the Sibling number
* return: void
*/
void semanticPrintTree(TreeNode *, int, int);

#endif