#include <stdio.h>
#include "bst.h"

/***********************************
 * Tree structure
 * defined in bst.h
 ***********************************

*
* NODE
*
typedef struct _node_struct{
        int key;
        int data;
        int left;
        int right;
	int parent;
}NODE,*pNODE;

*
* TREE
*
typedef struct _tree_struct{
        int root;
	int last;
        int size;
        int depth;
        int diff;
        NODE list[MAX_NUM];
}TREE,*pTREE;

************************************/

#define MAX_NODE_COUNT MAX_NUM	/* 100000 */
#define MAX_NODE_Q_COUNT (MAX_NODE_COUNT/2)
void print_test(pTREE ptree);
pNODE nodeq[MAX_NODE_Q_COUNT];
int qcnt = 0;
int rear = 0;
int front = 0;

int bst_tree_rebuild(pTREE ptree,pNODE p)
{
	return 1;
}

pNODE pop()
{
	pNODE p;

	if( qcnt == 0 && rear == front ){
		printf("NODE Q empty\n");
		return NULL;
	}


	p = nodeq[front];

/*
	if( p != NULL )
	printf("\nPOP  [ qcnt = %d, front = %d, rear = %d ] VALUE[ key = %d, data = %d, left = %d, right = %d ]\n",
		qcnt, front, rear, p->key, p->data, p->left, p->right );
	else
	printf("\nPOP NULL\n");
*/

	front = ++front%MAX_NODE_Q_COUNT;
	qcnt--;

	return p;
	
}

int push(pNODE p)
{
	int ret = rear;

/*
	if( p != NULL )
	printf("\nPUSH [ qcnt = %d, front = %d, rear = %d ] VALUE[ key = %d, data = %d, left = %d, right = %d ]\n",
		qcnt, front, rear, p->key, p->data, p->left, p->right );
	else
	printf("\nPUSH NULL\n");
*/

	if( (rear+1)%MAX_NODE_Q_COUNT== front ){
		printf("NODE Q Overflow[ queue count = %d ]\n",qcnt);
		return -1;
	}

	nodeq[rear] = p;

	rear = ++rear%MAX_NODE_Q_COUNT;

	qcnt++;
	
	return ret;
}

int print_node(pNODE p)
{
	if( p != NULL ){
		printf("[%d:%d][%d:%d] ",
			p->key,p->data,p->left,p->right);
		return 1;
	}
	else{
		printf("x ");
		return -1;
	}
}

int bst_view(pTREE ptree)
{
	int i;
	int ret;
	int cnt;
	int subcnt;
	int depth;
	pNODE p;

	rear = front = 0;
	if( ptree->size > 0){
	
		p = &(ptree->list[ptree->root]);
		cnt = ptree->depth+1;
		subcnt = 1;
		depth = 0;
		i = 0;
		ret = push(p);
		if( ret < 0 ){
			printf("failed push root-node into nodeq\n");
			return -1;
		}
		printf(" depth\t\t [index:value][left:right]\n");
		printf(" --------------------------------\n");

		while(cnt > 0){

			printf("%d ",i++);
			while( subcnt > 0 ){
				subcnt--;
				p = pop();
				print_node(p);

				if( p == NULL ){
					push((pNODE)NULL);
					push((pNODE)NULL);
					continue;
				}
				else
					depth = p->depth;

				if( p->left < 0 ){
					push((pNODE)NULL);
				}
				else
					push(&(ptree->list[p->left]));

				if( p->right< 0 ){
					push((pNODE)NULL);
				}
				else
					push(&(ptree->list[p->right]));

			}
			subcnt = 2<<depth;
			printf("\n");

			cnt--;
		}

	}
	else{
		printf("No data[ tree list empty ]\n");
	}
	return 1;

}

void print_test(pTREE ptree)
{
	int i;
	pNODE p;
	printf("\tindex \tkey \tdata (left:right)\n");
	printf("-----------------------------------------------\n");
	for( i=0;i<ptree->size;i++){
		p = &(ptree->list[i]);
		printf("\t%d \t%d \t%d (%d:%d)\n",
		i, p->key, p->data, p->left, p->right );
	}
	return;
}

pNODE seek_tree(pTREE ptree,pNODE p)
{
	int cnt;
	pNODE pivot;

	cnt = ptree->depth+1;
	pivot = &(ptree->list[ptree->root]);

	while(cnt > 0){
		if( pivot->data > p->data ){
			if( pivot->left < 0 ){
				return pivot;
			}
			pivot = &(ptree->list[pivot->left]);
		}
		else if( pivot->data < p->data ){
			if( pivot->right < 0 ){
				return pivot;
			}
			pivot = &(ptree->list[pivot->right]);
		}
		else if( pivot->data == p->data ){
			return pivot;
		}
		cnt--;
	}
	return NULL;
}

int bst_insert(pTREE ptree, int data)
{
	pNODE p;
	pNODE pivot;
	int last;

	if( ptree->size >= MAX_NODE_COUNT -1 ){
		printf("list overflow!!\n");
		return -1;
	}

	p = &(ptree->list[ptree->last]); 
	p->data = data;
	p->key  = ptree->last;


	pivot = seek_tree(ptree,p);

	if( ptree->size != 0 ){
		if( pivot->data > data )
			pivot->left = p->key;
		else if( pivot->data < data )
			pivot->right = p->key;
		else{
			printf("Same value is exist[ data = %d, key = %d ]\n",data,p->key);
			return -2;
		}

		if( pivot->depth == ptree->depth ){
			p->depth = pivot->depth+1;
			ptree->depth = p->depth;
		}
		else
			p->depth = ptree->depth;

		p->parent = pivot->key;
	}
	else{
		p->depth = 0;
		ptree->last = 0;
		ptree->depth = 0;
	}
	ptree->size++;
	ptree->last = p->right;
	p->right = -1;

	print_test(ptree);
	return p->key;
}

pNODE seek_min(pTREE ptree, pNODE pnode)
{
	pNODE p;
	p = pnode;
	while(p->left > 0 ){
		p = &(ptree->list[p->left]);	
	}
	return p;
}

pNODE seek_max(pTREE ptree, pNODE pnode)
{
	pNODE p;
	p = pnode;
	while(p->right> 0 ){
		p = &(ptree->list[p->right]);	
	}
	return p;
}

int seek_side(pTREE ptree, pNODE p){
	int ret;
	pNODE parent;
	parent = &(ptree->list[p->parent]);
	if( parent->data < p->data && parent->right == p->key )
		ret = 1;
	else if( parent->data > p->data && parent->left == p->key )
		ret = -1;
	return ret;
}

int tree_inorder(pTREE ptree, pNODE p){
	int cnt = 0;
	if( p->left != -1 ){
		cnt += tree_inorder(ptree,&(ptree->list[p->left]));
	}
	if( p->depth == ptree->depth ){
		/* p->depth is MAX depth */
		cnt++;
	}
	if( p->right != -1 ){
		cnt += tree_inorder(ptree,&(ptree->list[p->right]));
	}
	return cnt;
	
}

int max_depth_leap_cnt(pTREE ptree){
	
	return tree_inorder(ptree,&(ptree->list[ptree->root]));
}

void rm_leap(pTREE ptree, pNODE pnode)
{
	int max_depth_cnt;

	if( seek_side(ptree,pnode) < 0 ){
		/* left-child */
		ptree->list[pnode->parent].left = -1;
	}
	else{
		/* right-child */
		ptree->list[pnode->parent].right = -1;
	}

	/* tree - depth */
	if( pnode->depth == ptree->depth ){
		max_depth_cnt = max_depth_leap_cnt(ptree);
		if( max_depth_cnt == 0 ){
			ptree->depth--;
		}
	}

	ptree->size--;

	return;
}

void clear_node(pTREE ptree, pNODE pnode)
{
	pNODE old,new;

	/* last-node */
	old = &(ptree->list[ptree->last]);
	new = pnode;
	new->parent = old->parent;
	new->depth = -1;
	new->right = ptree->last;
	new->left = old->left = -1;
	old->parent = new->key;
	ptree->last = new->key;

	return;
}

void cp_node(pNODE target, pNODE src)
{
	/* setting link only */
	target->data = src->data;
	return;
}

int bst_delete(pTREE ptree, int data)
{
	NODE node;
	pNODE pivot;
	pNODE p;
	int depth_cnt;

	if( ptree->size == 0 ){
		printf("list underflow[ tree empty ]\n");
		return -1;
	}

	node.data = data;

	pivot = seek_tree(ptree,&node);

	if( pivot->data != node.data ){
		printf("data = %d is not exist in tree\n",node.data);
		return -2;
	}
	
	if( ptree->size > 1 ){
		printf("removed data [%d:%d]\n",pivot->key,pivot->data);
		if( pivot->left < 0 && pivot->right < 0 ){
			
			rm_leap(ptree,pivot);
			clear_node(ptree,pivot);
		}
		else if( pivot->key == ptree->root ){
			if( pivot->key == ptree->root ){
				/* root */
				ptree->root = p->key;
			}
		}
		else{
			if( pivot->left < 0 ){
				/* default - search min node in right-tree */
				p = seek_min(ptree,pivot);
			}
			else{
				p = seek_max(ptree,pivot);
			}
			/* p is leap ! */
			rm_leap(ptree,p);
			cp_node(pivot);
			clear_node(ptree,p);
		}
	}
	else if( ptree->size == 1 ){
		printf("removed data [%d%d], list empty.\n",pivot->key, pivot->data);
		bst_init_tree(ptree);
	}

	return pivot->key;
}

int bst_init_tree(pTREE ptree)
{
	int i;
	int cnt;
	pNODE p;

	cnt = sizeof(ptree->list)/sizeof(NODE);

	/* node list */
	for( i=0; i<cnt; i++ ){
		p = &(ptree->list[i]);
		p->key  = -1;
		p->data = -1;
		p->left = -1;
		p->right = i+1 ;
		p->parent = i-1;
		p->depth = -1;
	}
	
	/* first node */
	ptree->root = 0;

	/* final node */
	ptree->last= 0;
	
	/* list size */
	ptree->size = 0;
	
	/* depth */
	ptree->depth = -1;
	ptree->diff = 0;

	return 1;
}/* init */

