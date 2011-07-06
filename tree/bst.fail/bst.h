#ifndef _BST_H_
#define _BST_H_

#define MAX_NUM 100000

typedef struct _node_struct{
	int key;
	int data;
	int left;
	int right;
	int parent;	/* parent node */
	int depth;
}NODE,*pNODE;

typedef struct _tree_struct{
	int root;
	int last;
	int size;
	int depth;
	int diff;
	NODE list[MAX_NUM];
}TREE,*pTREE;

#endif /* _BST_H_ */


