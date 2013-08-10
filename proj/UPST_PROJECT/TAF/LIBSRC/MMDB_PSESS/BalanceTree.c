/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : Jiyoon Chung and Sangwoo Lee
   Section  : 
   SCCS ID  : 
   Date     : 
   Revision History : 
        '01.  9. 21     Initial
		'02. 12.		miheeh : 데이터 구조를 pointer에서 index로 변환
	 
   Description:
		MMDB

   Copyright (c) ABLEX 2001
***********************************************************/

/**A.1*  File Inclusion ***********************************/

#include "mmdb_psess.h"

/**B.1*  Definition of New Constants *********************/

#define LEFTSIDE	1
#define RIGHTSIDE	(-1)

/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

int avl_insert_psess( PPSESS_KEY key, long long *body, int *root )
{
    PSESS_TYPE *Q, *F, *A, *P, *B, *C;
	PSESS_TYPE *insert;
	int		Pi, Ai, Bi, Ci;
	int		index;
	int		i;
    short 	d;	/* Balace factor 용 */

    /* Phase 1: Locate insertion point for min */
    //F = 0; A = *T_root; P = *T_root; Q = 0;
    F = 0; A = 0; P = 0; Q = 0;
	Ai = *root; Pi = *root;
    
	while(Pi != -1){  /*  min를 인서트할 T를 찾는다 */
		P = get_psess(Pi);
        if(P->bf != 0) {
            A = P;
			Ai = Pi;
            F = Q;
        } 
        if( memcmp( key, &P->key, sizeof( PSESS_KEY ) ) < 0 ) {  
            Q = P;  /* Q는 P의 부모노드가 된다 */

			if( Pi == P->left )	/* not initialized */
				return -2;
			Pi = P->left;
        }
        else if( memcmp( key, &P->key, sizeof( PSESS_KEY ) ) > 0 ) {  
            Q = P;
			if( Pi == P->right )	/* not initialized */
				return -2;
			Pi = P->right;
        }
        else 
		{
            return 1;	/* Exist */
		}
    }

    /* Phase 2: Insert and Rebalance */
	index = psess_alloc();
	if( index == -1 )
		return -1;

	insert = get_psess(index);

	for (i = 0; i < JMM_PSESS_BODY_LEN;i++) {
		insert->body[i] = body[i];
	}

    insert->key = *key;
	//memcpy( &insert->key, key, sizeof( PSESS_KEY ) ); 
    insert->left = insert->right = -1;
    insert->bf = 0;

	if (Q == 0) {		/* T_root가 0인 특수한 경우 */
		*root = index;
		return 0;
    } 
	else if( memcmp( key, &Q->key, sizeof( PSESS_KEY ) ) < 0 )
        Q->left = index;  /* insert as left child */
    else 
		Q->right = index; /* insert as right child */
    
    /* A에서 Q에 이르는 노드들의 balance factor를 조정한다 */
	A = get_psess(Ai);
    if( memcmp( key, &A->key, sizeof( PSESS_KEY ) ) > 0 ){  
        P = get_psess(A->right);
        B = P;
		Bi = A->right;
        d = RIGHTSIDE;
    }
    else {
        P = get_psess(A->left);
        B = P;
		Bi = A->left;
        d = LEFTSIDE;
    }

    while(P != insert) {
    	if( memcmp( key, &P->key, sizeof( PSESS_KEY ) ) > 0 ){  
           	P->bf = RIGHTSIDE;             /* right의 높이가 1만큼 증가 */ 
           	P = get_psess(P->right);
        }
        else {
            P->bf = LEFTSIDE;              /* left의 높이가 1만큼 증가 */
            P = get_psess(P->left);
        }
		if( P == 0)
			return -1;
    }
    
    /* tree가 unbalance인가 ? */
    if(A->bf == 0) {        /* tree still balanced */
        A->bf = d;
        return 0;
    }
    if((A->bf + d) == 0) {  /* tree is balanced */
        A->bf = 0;
        return 0;
    }
	
    /* tree는 unbalance하다. rotation type 결정 */
	B = get_psess(Bi);
    if( d == LEFTSIDE) {  /* left imbalance */
        if (B->bf == LEFTSIDE) {	/* LL */
			A->left = B->right;
            B->right = Ai;
            A->bf = B->bf = 0;

		} else {		/* LR */
			Ci = B->right;
            C = get_psess(Ci);
            B->right = C->left;
            A->left = C->right;
            C->left = Bi;
            C->right = Ai;
            switch(C->bf){
                case LEFTSIDE:
                    A->bf = RIGHTSIDE; B->bf = 0;
                    break;
                case RIGHTSIDE:
                    B->bf = LEFTSIDE; A->bf = 0;
                    break;
                default:
                    B->bf = 0; A->bf = 0;       
            }
            C->bf = 0; 
			Bi = Ci;  /* B is new root */
		}
	
	/* d == RIGHTSIDE */
    } 
	else { /* right imbalance */
        if (B->bf == RIGHTSIDE) { /* rotation type  RR */
           	A->right = B->left;
           	B->left = Ai;
           	A->bf = B->bf = 0;
		} 
		else {	/* RL	*/
			Ci = B->left;
            C = get_psess(Ci);
            B->left = C->right;
            A->right = C->left;
            C->right = Bi;
            C->left = Ai;
            switch(C->bf){
                case RIGHTSIDE:
                    A->bf = LEFTSIDE; B->bf = 0;
                    break;
                case  LEFTSIDE:
                    B->bf = RIGHTSIDE; A->bf = 0;
                    break;
                default:
                    B->bf = 0; A->bf = 0;       
            }
            C->bf = 0; 
			Bi = Ci;  /* B is new root */
        }
    }
    
    /* subtree with root B has been rebalanced and is the new subtree */
    /* of F. The original subtree of F had root A */
    if(F == 0)
        *root = Bi;
    else if(Ai == F->left)
        F->left = Bi;
    else if(Ai == F->right)
        F->right = Bi;

	return 0;
}

int left_rotation_psess( int index, int *pindex )
{
	PSESS_TYPE *grand_child, *child, *parent;
	PSESS_TYPE *node;
	int left, right;
	
	node = get_psess(index);
	left = node->left;
  	child = get_psess(left);

	if (child->bf == LEFTSIDE) {
   	 /* single rotation */
	 /* 윗단의 노드에도 영향이 있음 */
   	    node->left = child->right;
   	    child->right = index;
		node->bf = 0;
   		*pindex = left;
		parent = &psess_tbl->tuple[*pindex];
  		parent->bf = 0;
		return 1;

    } else if(child->bf == RIGHTSIDE) {
     /* double rotation */
	 /* 윗단의 노드에도 영향이 있음 */
	 	right = child->right;
  	    grand_child = &psess_tbl->tuple[right];
   		child->right = grand_child->left;
     	grand_child->left = left;
        node->left = grand_child->right;
     	grand_child->right = index;

     	switch (grand_child->bf) {
          	case LEFTSIDE: 
					node->bf = RIGHTSIDE; child->bf = 0;
           	        break;
          	case RIGHTSIDE: 
					node->bf = 0; child->bf = LEFTSIDE;
					break;
          	default: 
					node->bf = child->bf = 0;
           	        break;
    	}
     	*pindex = right;
  		parent = &psess_tbl->tuple[*pindex];
		parent->bf = 0;
		return 1;

  	} else {
	 /* single rotation */
	 /* 윗단의 노드에는 영향이 없음 */
   	    node->left = child->right;
   	    child->right = index;
   	    node->bf = 1;
   		*pindex = left;
		parent = &psess_tbl->tuple[*pindex];
  		parent->bf = -1;
		return 0;
	} 
}

int right_rotation_psess( int index, int *pindex )
{
	PSESS_TYPE *grand_child, *child, *parent;
	PSESS_TYPE *node;	
	int left, right;
	
	node = &psess_tbl->tuple[index];
	right = node->right;
	child = &psess_tbl->tuple[right];

	if (child->bf == RIGHTSIDE) {
   	/* single rotation */
	/* 윗단의 노드에도 영향이 있음 */
   		node->right = child->left;
   	    child->left = index;
   	    node->bf = 0;
   		*pindex = right;
		parent = &psess_tbl->tuple[*pindex];
  		parent->bf = 0;
		return 1;

    } else if(child->bf == LEFTSIDE) {
     /* double rotation */
	 /* 윗단의 노드에도 영향이 있음 */
	 	left = child->left;
  	    grand_child = &psess_tbl->tuple[left];
   		child->left = grand_child->right;
     	grand_child->right = node->right;
        node->right = grand_child->left;
     	grand_child->left = index;

     	switch (grand_child->bf) {
          	case RIGHTSIDE: 
				node->bf = LEFTSIDE; child->bf = 0;
           	    break;
          	case LEFTSIDE: 
				node->bf = 0; child->bf = RIGHTSIDE;
				break;
          	default: 
				node->bf = child->bf = 0;
    	}
     	*pindex = left;
		parent = &psess_tbl->tuple[*pindex];
  		parent->bf = 0;
		return 1;

  	} else {
	 /* single rotation */
	 /* 윗단의 노드에는 영향이 없음 */
   	    node->right = child->left;
   	    child->left = index;
   	    node->bf = -1;
   		*pindex = right;
		parent = &psess_tbl->tuple[*pindex];
  		parent->bf = 1;
		return 0;
	} 
}

PSESS_TYPE *avl_search_psess( int tindex, PPSESS_KEY key )
{
	PSESS_TYPE *tree;

	while(tindex != -1) {
		tree = &psess_tbl->tuple[tindex];
    	if( memcmp( key, &tree->key, sizeof( PSESS_KEY ) ) < 0 )
		{
			if( tindex == tree->left )
				return 0;	/*  not initialized */
			tindex = tree->left;
		}
    	else if( memcmp( key, &tree->key, sizeof( PSESS_KEY ) ) > 0 )
		{
			if( tindex == tree->right )
				return 0;	/* not initialize */
			tindex = tree->right;
		}
		else
		{
			return tree;
		}
	}
	return 0;
} 

int avl_delete_psess( int *root, PPSESS_KEY delkey )
{
	PSESS_TYPE *thenode, *left_node, *right_node;
	PSESS_TYPE *pre_delnode, *delnode, *rotate;
	//PSESS_TYPE *stack[100];
	int del_left, del_right, dindex, rindex;
	int stack[100];
	int i, top, side = 0;

	top = 0;
	
	stack[top++] = -1;
	dindex = *root;
	//delnode = &psess_tbl->tuple[dindex];
   	//if( memcmp( delkey, &delnode->key, sizeof( PSESS_KEY ) ) < 0 )
	//	side = LEFTSIDE;	/* LEFT*/
	//else
	//	side = RIGHTSIDE;	/* RIGHT or MID*/

	pre_delnode = 0;

	/* 지울 노드를 찾는 과정에서 delnode를 스택에 저장*/
	while(dindex != -1){
        /* parent의 path를 기억하기위해 stack에 저장 */
		stack[top++] = dindex;
		delnode = get_psess(dindex);
   		if( memcmp( delkey, &delnode->key, sizeof( PSESS_KEY ) ) < 0 ){
			if( dindex == delnode->left )
				return -2;	/* not initialized */  
			pre_delnode = delnode;
            dindex = delnode->left;
   		} 
		else if( memcmp( delkey, &delnode->key, sizeof( PSESS_KEY ) ) > 0 ){
			if( dindex == delnode->right )
				return -2;	/* not initialize */  
			pre_delnode = delnode;
            dindex = delnode->right;
		} 
		else 
			break;
    }

	/* 지우고자하는 원소가 없는 경우 delnode는 0을 가진다.*/
    if (dindex == -1) 
        return 1;	/* eNotFound */

	/* Step 1. Non leaf인 경우 leaf node deletion과 동일한 구조로 만든다.*/

	/* 지울 노드가 leaf인 경우*/
	delnode = get_psess(dindex);
	if(delnode->left == -1 && delnode->right == -1){
        /* 지울 노드가 루트인 특수한 경우*/
        if (pre_delnode == 0){
            *root = -1;
            psess_dealloc(dindex); 
			return 0; 
   		} 
		else if( memcmp( delkey, &pre_delnode->key, sizeof( PSESS_KEY ) ) > 0 ){  
			/* side는 rotation시 꼭 필요한 변수: 여기선 오른쪽 방향  */
			side = RIGHTSIDE;
   		    pre_delnode->right = -1;

    	}
		else{
			/* side는 rotation시 꼭 필요한 변수: 여기선 왼쪽 방향  */
			side = LEFTSIDE;
       		pre_delnode->left = -1;
		}

        /* leaf node가 지워졌는데 스택에 있으므로 삭제한다.*/
        top--;
    	psess_dealloc(dindex);
	}

	/* 지울 노드가 2개의 sub Tree를 가지는 경우*/
	else if(delnode->left != -1 && delnode->right != -1) {
		del_left = delnode->left;		
		del_right = delnode->right;		

		/* 좌측 트리중 가장 큰 값을 찾는다 */
		left_node = get_psess(del_left);
		right_node = get_psess(del_right);

		if(left_node->right != -1){
			while(left_node->right != -1){
				pre_delnode = get_psess(del_left);
                stack[top++] = del_left;
                del_left = left_node->right;
				left_node = get_psess(del_left);
			}

			/* pre_delnode는 실제 지워질 del_left의 부모*/
        	pre_delnode->right = left_node->left;

			/* 지우고자하는 노드(delnode)와 실제 지워지는 노드(del_left)의 내용 치환*/
			delnode->key  = left_node->key;
			for (i = 0;i < JMM_PSESS_BODY_LEN;i++) 
				delnode->body[i] = left_node->body[i];

			side = RIGHTSIDE; /* side 는 오른쪽*/
        	psess_dealloc(del_left);
		}
		/* 좌측 트리중 가장 큰 값이 없으면*/
		/* 지울 노드의 우측 트리중 가장 작은 값을 찾는다	*/
		else if(right_node->left != -1){
 			while(right_node->left != -1){
				pre_delnode = get_psess(del_right);
                stack[top++] = del_right;
                del_right = right_node->left;
				right_node = get_psess(del_right);
			}
			/* pre_delnode는 실제 지워질 del_left의 부모*/
            pre_delnode->left = right_node->right;

			/* 지우고자하는 노드(delnode)와 실제 지워지는 노드(del_right)의 내용 치환*/
			delnode->key  = right_node->key;

			for (i = 0;i < JMM_PSESS_BODY_LEN;i++)
				delnode->body[i]  = right_node->body[i];

			side = LEFTSIDE; /* side는 왼쪽*/
        	psess_dealloc(del_right);
		}

		/* 지울 노드의 좌 우측 트리가 가장 큰값 또는 작은 값이 없을 경우*/
		/* 지울 노드의 왼쪽 노드를 택한다 */
		/* 윗단에 영향을 주지 않으므로 바로 return 한다*/
		else{
			/* bf문제를 고려해주어야 한다.*/
			thenode = get_psess(*root);

			if (pre_delnode == 0) {
				if (thenode->bf >= 0){
      	 	 		thenode->key = (&psess_tbl->tuple[del_left])->key;
					for (i = 0;i < JMM_PSESS_BODY_LEN;i++)
						thenode->body[i] = left_node->body[i];

					thenode->left = left_node->left;
					thenode->bf = thenode->bf - 1;
        			psess_dealloc(del_left);
				} 
				else {
      	 	 		thenode->key = right_node->key;
					for (i = 0;i < JMM_PSESS_BODY_LEN;i++)
						thenode->body[i] = right_node->body[i];

					thenode->right = right_node->right;
					thenode->bf = thenode->bf + 1;
        			psess_dealloc(del_right);
				}
				return 0; 
			} 
			else{ /* predelnode가 0이 아니면 */

				if (delnode->bf > 0){
					delnode->key = left_node->key;
					for (i = 0;i < JMM_PSESS_BODY_LEN;i++)
						delnode->body[i] = left_node->body[i];

					delnode->left = left_node->left; 
					delnode->bf = delnode->bf  - 1; 
        			psess_dealloc(del_left);
				}
				else if(delnode->bf == 0){
					delnode->key = left_node->key;
					for (i = 0;i < JMM_PSESS_BODY_LEN;i++)
						delnode->body[i] = left_node->body[i];

					delnode->left = left_node->left; 
					delnode->bf = delnode->bf  - 1; 
        			psess_dealloc(del_left);
					return 0;
				}
				else {
					delnode->key = right_node->key;
					for (i = 0;i < JMM_PSESS_BODY_LEN;i++)
						delnode->body[i] = right_node->body[i];

					delnode->right = right_node->right; 
					delnode->bf = delnode->bf + 1; 
        			psess_dealloc(del_right);
				}
   				if( memcmp( delkey, &pre_delnode->key, sizeof( PSESS_KEY ) ) > 0 )
					side = RIGHTSIDE;
				else 
					side = LEFTSIDE;
				top--;

			} /* pre_delnode가 0이 아닐때 */
		}	/* 지울 노드 양쪽에 크거나 작은 값이 없을때 */	
	}	/* 지울 노드 양변에 sub tree가 2개 있을때 */

	/* 지울 노드가 1개만의 sub tree를 갖는 경우*/
	/* 지울 노드가 root인 경우 pre_delnode는 0을 갖는다. 이경우 직접 붙인다*/
	else{
		if (pre_delnode == 0) {
        	if(delnode->right != -1) 
           	 	*root = delnode->right;
    		else 
            	*root = delnode->left;
   		} 
		else if( memcmp( delkey, &pre_delnode->key, sizeof( PSESS_KEY ) ) > 0 ) {
			if(delnode->right != -1)
	            pre_delnode->right = delnode->right;
			else 
	            pre_delnode->right = delnode->left;
			side = RIGHTSIDE;	
		}
    	else{ 
			if(delnode->right != -1)
	            pre_delnode->left = delnode->right;
			else 
	            pre_delnode->left = delnode->left;
			side = LEFTSIDE;
		}
        /* delnode자신이 삭제되었으므로 스택에서 제거한다.*/
        top--;
        psess_dealloc(dindex);
	}

	/* Step 3. Root까지 가면서 rotate를 수행한다.*/
    while(top > 1){
		rindex = stack[--top];
		rotate = get_psess(rindex);
		pre_delnode = get_psess(stack[top-1]);	/* Parent */
		
		if (side < 0) {
            switch (rotate->bf) {
                case LEFTSIDE:
					if(pre_delnode == 0){
   	                	if(left_rotation_psess(rindex, root) == 0)
							return 0;
						else
						    break;
					}
					else if(pre_delnode->right == rindex){
   	                	if(left_rotation_psess(rindex, &pre_delnode->right) == 0)
							return 0;
						else
						    break;
					} 
					else if(pre_delnode->left == rindex){
   	                	if(left_rotation_psess(rindex, &pre_delnode->left) == 0)
							return 0;
						else
						    break;
					}
                case  0: rotate->bf = LEFTSIDE;
                     return 0;
                case RIGHTSIDE:		/* 전체 height가 1줄은 경우*/
                     rotate->bf = 0;
                     break;
            }
			if (top <= 1 || pre_delnode == 0) return 0;
			/* side 가 0보다 크면 left 작으면 right */
			side = memcmp( &pre_delnode->key, &rotate->key, sizeof( PSESS_KEY ) );

		} 
		else {  /* parent가 left child를 가질때 */
            switch (rotate->bf) {
                case LEFTSIDE: rotate->bf = 0;
                    break;
                case  0: rotate->bf = RIGHTSIDE;
                    return 0;
                case RIGHTSIDE: 
                    if(pre_delnode == 0){
                        if(right_rotation_psess(rindex, root) == 0)
                            return 0;
                        else
                            break;
                    } 
					else if(pre_delnode->right == rindex){
                        if(right_rotation_psess(rindex, &pre_delnode->right) == 0)
                            return 0;
                        else
                            break;
                    } 
					else if (pre_delnode->left == rindex){
                        if(right_rotation_psess(rindex, &pre_delnode->left) == 0)
                            return 0;
                        else
                            break;
					}
            }
			/* side 가 0보다 크면 left 작으면 right */
			if (top <= 1 || pre_delnode == 0) return 0;
			side = memcmp( &pre_delnode->key, &rotate->key, sizeof( PSESS_KEY ) );
        } 
    }
	return 0;
} 

/******
    return one key which is greater than first_key and less than last_key
******/
PSESS_TYPE *avl_select_psess( int index, PPSESS_KEY first_key, PPSESS_KEY last_key)
{
	PSESS_TYPE *tree;
    int stack[100];
    int top;

    if( index == -1 || memcmp(first_key, last_key, sizeof(PSESS_KEY)) >= 0)
        return 0;

    top = 0;
    stack[top++] = -1;

    while(index != -1)
    {
		tree = &psess_tbl->tuple[index];
        if (memcmp(first_key, &tree->key, sizeof(PSESS_KEY)) < 0)
        {
			if( index == tree->left )
				return 0;	/* not initialized */
            stack[top++] = index;
            index = tree->left;
        }
        else if (memcmp(first_key, &tree->key, sizeof(PSESS_KEY)) > 0)
        {
			if( index == tree->right )
				return 0;	/* not initialized */
            index = tree->right;
        }
        else
        {
            index = tree->right;
			tree = get_psess(index);
            while(index != -1)
            {
                stack[top++] = index;
                index = tree->left;
				tree = get_psess(index);
            }
            break;
        }
    }

	if (stack[--top] != -1)
	{
		tree = &psess_tbl->tuple[stack[top]];
    	if (memcmp(&tree->key, last_key, sizeof(PSESS_KEY)) >= 0)
        	return 0;
	}

	//여기서는 stack[top]==-1 인 경우도 고려해야 하므로
	//&psess_tbl->tuple[stack[top]] 을 써서는 안된다.
	//get_psess함수를 써서, stack[top]==-1 일때는 return 값으로 0이 가도록 한다.
	tree = get_psess( stack[top] );
    return tree;
}

int avl_update_psess( PSESS_TYPE *tree, long long *body )
{
    int i = 0;
    for (i = 0; i < JMM_PSESS_BODY_LEN;i++) {
        tree->body[i] = body[i];
    }

    return 0;
}

PSESS_TYPE *get_psess( int index )
{
	if( index >= 0 && index < JMM_PSESS_RECORD )
		return (&psess_tbl->tuple[index]);
	else
		return 0;
}
