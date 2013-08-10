/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : Jiyoon Chung and Sangwoo Lee
   Section  : 
   SCCS ID  : 
   Date     : 
   Revision History : 
        '01.  9. 21     Initial

   Description:
		MMDB

   Copyright (c) ABLEX 2001
***********************************************************/

/**A.1*  File Inclusion ***********************************/

#include "mmdb_destip.h"

/**B.1*  Definition of New Constants *********************/

#define LEFTSIDE	1
#define RIGHTSIDE	(-1)

/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/
/**D.1*  Definition of Functions  *************************/
/**D.2*  Definition of Functions  *************************/

//int avl_insert_destip( PDESTIP_KEY key, long long *body, DESTIP_TYPE **T_root )
int avl_insert_destip( PDESTIP_KEY key, long long *body, int *root )
{
    DESTIP_TYPE *Q, *F, *A, *P, *B, *C;
	DESTIP_TYPE *insert;
	int		Pi, Ai, Bi, Ci;
	int		index;
	int		i;
    short 	d;	/* Balace factor �� */

    /* Phase 1: Locate insertion point for min */
    //F = 0; A = *T_root; P = *T_root; Q = 0;
    F = 0; A = 0; P = 0; Q = 0;
	Ai = *root; Pi = *root;
    
	while(Pi != -1){  /*  min�� �μ�Ʈ�� T�� ã�´� */
		P = get_destip(Pi);
        if(P->bf != 0) {
            A = P;
			Ai = Pi;
            F = Q;
        } 
        if( memcmp( key, &P->key, sizeof( DESTIP_KEY ) ) < 0 ) {  
            Q = P;  /* Q�� P�� �θ��尡 �ȴ� */
            //P = P->left;
			Pi = P->left;
        }
        else if( memcmp( key, &P->key, sizeof( DESTIP_KEY ) ) > 0 ) {  
            Q = P;
            //P = P->right;
			Pi = P->right;
        }
        else 
		{
            return 1;	/* Exist */
		}
    }

    /* Phase 2: Insert and Rebalance */
	index = destip_alloc();
	if( index == -1 )
		return -1;

	insert = get_destip(index);

	for (i = 0; i < JMM_DESTIP_BODY_LEN;i++) {
		insert->body[i] = body[i];
	}

    insert->key = *key;
	//memcpy( &insert->key, key, sizeof( DESTIP_KEY ) ); 
    insert->left = insert->right = -1;
    insert->bf = 0;

	if (Q == 0) {		/* T_root�� 0�� Ư���� ��� */
		*root = index;
		return 0;
    } 
	else if( memcmp( key, &Q->key, sizeof( DESTIP_KEY ) ) < 0 )
        Q->left = index;  /* insert as left child */
    else 
		Q->right = index; /* insert as right child */
    
    /* A���� Q�� �̸��� ������ balance factor�� �����Ѵ� */
	A = get_destip(Ai);
    if( memcmp( key, &A->key, sizeof( DESTIP_KEY ) ) > 0 ){  
        P = get_destip(A->right);
        B = P;
		Bi = A->right;
        d = RIGHTSIDE;
    }
    else {
        P = get_destip(A->left);
        B = P;
		Bi = A->left;
        d = LEFTSIDE;
    }

    while(P != insert){
    	if( memcmp( key, &P->key, sizeof( DESTIP_KEY ) ) > 0 ){  
           	P->bf = RIGHTSIDE;             /* right�� ���̰� 1��ŭ ���� */ 
           	P = get_destip(P->right);
        }
        else {
            P->bf = LEFTSIDE;              /* left�� ���̰� 1��ŭ ���� */
            P = get_destip(P->left);
        }
    }
    
    /* tree�� unbalance�ΰ� ? */
    if(A->bf == 0) {        /* tree still balanced */
        A->bf = d;
        return 0;
    }
    if((A->bf + d) == 0) {  /* tree is balanced */
        A->bf = 0;
        return 0;
    }
	
    /* tree�� unbalance�ϴ�. rotation type ���� */
	B = get_destip(Bi);
    if( d == LEFTSIDE) {  /* left imbalance */
        if (B->bf == LEFTSIDE) {	/* LL */
			A->left = B->right;
            B->right = Ai;
            A->bf = B->bf = 0;

		} else {		/* LR */
			Ci = B->right;
            C = get_destip(Ci);
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
            C = get_destip(Ci);
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

//int left_rotation_destip( DESTIP_TYPE *node, DESTIP_TYPE **parent )
int left_rotation_destip( int index, int *pindex )
{
	DESTIP_TYPE *grand_child, *child, *parent;
	DESTIP_TYPE *node;
	int left, right;
	
	node = get_destip(index);
	left = node->left;
  	child = get_destip(left);

	if (child->bf == LEFTSIDE) {
   	 /* single rotation */
	 /* ������ ��忡�� ������ ���� */
   	    node->left = child->right;
   	    child->right = index;
		node->bf = 0;
   		*pindex = left;
		parent = &destip_tbl->tuple[*pindex];
  		parent->bf = 0;
		return 1;

    } else if(child->bf == RIGHTSIDE) {
     /* double rotation */
	 /* ������ ��忡�� ������ ���� */
	 	right = child->right;
  	    grand_child = &destip_tbl->tuple[right];
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
  		parent = &destip_tbl->tuple[*pindex];
		parent->bf = 0;
		return 1;

  	} else {
	 /* single rotation */
	 /* ������ ��忡�� ������ ���� */
   	    node->left = child->right;
   	    child->right = index;
   	    node->bf = 1;
   		*pindex = left;
		parent = &destip_tbl->tuple[*pindex];
  		parent->bf = -1;
		return 0;
	} 
}

//int right_rotation_destip( DESTIP_TYPE *node, DESTIP_TYPE **parent )
int right_rotation_destip( int index, int *pindex )
{
	DESTIP_TYPE *grand_child, *child, *parent;
	DESTIP_TYPE *node;	
	int left, right;
	
	node = &destip_tbl->tuple[index];
	right = node->right;
	child = &destip_tbl->tuple[right];

	if (child->bf == RIGHTSIDE) {
   	/* single rotation */
	/* ������ ��忡�� ������ ���� */
   		node->right = child->left;
   	    child->left = index;
   	    node->bf = 0;
   		*pindex = right;
		parent = &destip_tbl->tuple[*pindex];
  		parent->bf = 0;
		return 1;

    } else if(child->bf == LEFTSIDE) {
     /* double rotation */
	 /* ������ ��忡�� ������ ���� */
	 	left = child->left;
  	    grand_child = &destip_tbl->tuple[left];
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
		parent = &destip_tbl->tuple[*pindex];
  		parent->bf = 0;
		return 1;

  	} else {
	 /* single rotation */
	 /* ������ ��忡�� ������ ���� */
   	    node->right = child->left;
   	    child->left = index;
   	    node->bf = -1;
   		*pindex = right;
		parent = &destip_tbl->tuple[*pindex];
  		parent->bf = 1;
		return 0;
	} 
}

//DESTIP_TYPE *avl_search_destip( DESTIP_TYPE *tree, PDESTIP_KEY key )
DESTIP_TYPE *avl_search_destip( int tindex, PDESTIP_KEY key )
{
	DESTIP_TYPE *tree;

	while(tindex != -1) {
		tree = &destip_tbl->tuple[tindex];
{
    //int i;
    //char    tbuf[sizeof(DESTIP_KEY)];

    //memcpy(tbuf, &tree->key, sizeof(DESTIP_KEY));
    //fprintf(stderr, "MEM -> (");
    //for(i = 0; i < sizeof(DESTIP_KEY); i++){
    //    fprintf(stderr, "%.02x|", tbuf[i]);
    //}
    //fprintf(stderr, ")\n");
}
    	if( memcmp( key, &tree->key, sizeof( DESTIP_KEY ) ) < 0 )
		{
			tindex = tree->left;
		}
    	else if( memcmp( key, &tree->key, sizeof( DESTIP_KEY ) ) > 0 )
		{
			tindex = tree->right;
		}
		else
		{
			return tree;
		}
	}
	return 0;
} 

/*
DESTIP_TYPE *avl_search_destip_seq( int tindex, short seq)
{
    DESTIP_TYPE *tree;
    PDESTIP_DATA pdata;
    short         keyseq = 0;

    while(tindex != -1) {
        tree = &destip_tbl->tuple[tindex];
        pdata = (PDESTIP_DATA)tree;
        keyseq = (pdata->ucGroupID & 0x0F);
        keyseq = (keyseq << 8) + pdata->ucSerial;
        if(keyseq == seq){
            return tree;
        }else{
            tindex = tree->right;
        }
    }
    return 0;

}
*/

//int avl_delete_destip( DESTIP_TYPE **thenode, PDESTIP_KEY delkey )
int avl_delete_destip( int *root, PDESTIP_KEY delkey )
{
	DESTIP_TYPE *thenode, *left_node, *right_node;
	DESTIP_TYPE *pre_delnode, *delnode, *rotate;
	//DESTIP_TYPE *stack[100];
	int del_left, del_right, dindex, rindex;
	int stack[100];
	int i, top, side = 0;

	top = 0;
	
	stack[top++] = -1;
	dindex = *root;
	//delnode = &destip_tbl->tuple[dindex];
   	//if( memcmp( delkey, &delnode->key, sizeof( DESTIP_KEY ) ) < 0 )
	//	side = LEFTSIDE;	/* LEFT*/
	//else
	//	side = RIGHTSIDE;	/* RIGHT or MID*/

	pre_delnode = 0;

	/* ���� ��带 ã�� �������� delnode�� ���ÿ� ����*/
	while(dindex != -1){
        /* parent�� path�� ����ϱ����� stack�� ���� */
		stack[top++] = dindex;
		delnode = get_destip(dindex);
   		if( memcmp( delkey, &delnode->key, sizeof( DESTIP_KEY ) ) < 0 ){  
			pre_delnode = delnode;
            dindex = delnode->left;
   		} 
		else if( memcmp( delkey, &delnode->key, sizeof( DESTIP_KEY ) ) > 0 ){  
			pre_delnode = delnode;
            dindex = delnode->right;
		} 
		else 
			break;
    }

	/* ��������ϴ� ���Ұ� ���� ��� delnode�� 0�� ������.*/
    if (dindex == -1) 
        return 1;	/* eNotFound */

	/* Step 1. Non leaf�� ��� leaf node deletion�� ������ ������ �����.*/

	/* ���� ��尡 leaf�� ���*/
	delnode = get_destip(dindex);
	if(delnode->left == -1 && delnode->right == -1){
        /* ���� ��尡 ��Ʈ�� Ư���� ���*/
        if (pre_delnode == 0){
            *root = -1;
            destip_dealloc(dindex); 
			return 0; 
   		} 
		else if( memcmp( delkey, &pre_delnode->key, sizeof( DESTIP_KEY ) ) > 0 ){  
			/* side�� rotation�� �� �ʿ��� ����: ���⼱ ������ ����  */
			side = RIGHTSIDE;
   		    pre_delnode->right = -1;

    	}
		else{
			/* side�� rotation�� �� �ʿ��� ����: ���⼱ ���� ����  */
			side = LEFTSIDE;
       		pre_delnode->left = -1;
		}

        /* leaf node�� �������µ� ���ÿ� �����Ƿ� �����Ѵ�.*/
        top--;
    	destip_dealloc(dindex);
	}

	/* ���� ��尡 2���� sub Tree�� ������ ���*/
	else if(delnode->left != -1 && delnode->right != -1) {
		del_left = delnode->left;		
		del_right = delnode->right;		

		/* ���� Ʈ���� ���� ū ���� ã�´� */
		left_node = get_destip(del_left);
		right_node = get_destip(del_right);

		if(left_node->right != -1){
			while(left_node->right != -1){
				pre_delnode = get_destip(del_left);
                stack[top++] = del_left;
                del_left = left_node->right;
				left_node = get_destip(del_left);
			}

			/* pre_delnode�� ���� ������ del_left�� �θ�*/
        	pre_delnode->right = left_node->left;

			/* ��������ϴ� ���(delnode)�� ���� �������� ���(del_left)�� ���� ġȯ*/
			delnode->key  = left_node->key;
			for (i = 0;i < JMM_DESTIP_BODY_LEN;i++) 
				delnode->body[i] = left_node->body[i];

			side = RIGHTSIDE; /* side �� ������*/
        	destip_dealloc(del_left);
		}
		/* ���� Ʈ���� ���� ū ���� ������*/
		/* ���� ����� ���� Ʈ���� ���� ���� ���� ã�´�	*/
		else if(right_node->left != -1){
 			while(right_node->left != -1){
				pre_delnode = get_destip(del_right);
                stack[top++] = del_right;
                del_right = right_node->left;
				right_node = get_destip(del_right);
			}
			/* pre_delnode�� ���� ������ del_left�� �θ�*/
            pre_delnode->left = right_node->right;

			/* ��������ϴ� ���(delnode)�� ���� �������� ���(del_right)�� ���� ġȯ*/
			delnode->key  = right_node->key;

			for (i = 0;i < JMM_DESTIP_BODY_LEN;i++)
				delnode->body[i]  = right_node->body[i];

			side = LEFTSIDE; /* side�� ����*/
        	destip_dealloc(del_right);
		}

		/* ���� ����� �� ���� Ʈ���� ���� ū�� �Ǵ� ���� ���� ���� ���*/
		/* ���� ����� ���� ��带 ���Ѵ� */
		/* ���ܿ� ������ ���� �����Ƿ� �ٷ� return �Ѵ�*/
		else{
			/* bf������ ������־�� �Ѵ�.*/
			thenode = get_destip(*root);

			if (pre_delnode == 0) {
				if (thenode->bf >= 0){
      	 	 		thenode->key = (&destip_tbl->tuple[del_left])->key;
					for (i = 0;i < JMM_DESTIP_BODY_LEN;i++)
						thenode->body[i] = left_node->body[i];

					thenode->left = left_node->left;
					thenode->bf = thenode->bf - 1;
        			destip_dealloc(del_left);
				} 
				else {
      	 	 		thenode->key = right_node->key;
					for (i = 0;i < JMM_DESTIP_BODY_LEN;i++)
						thenode->body[i] = right_node->body[i];

					thenode->right = right_node->right;
					thenode->bf = thenode->bf + 1;
        			destip_dealloc(del_right);
				}
				return 0; 
			} 
			else{ /* predelnode�� 0�� �ƴϸ� */

				if (delnode->bf > 0){
					delnode->key = left_node->key;
					for (i = 0;i < JMM_DESTIP_BODY_LEN;i++)
						delnode->body[i] = left_node->body[i];

					delnode->left = left_node->left; 
					delnode->bf = delnode->bf  - 1; 
        			destip_dealloc(del_left);
				}
				else if(delnode->bf == 0){
					delnode->key = left_node->key;
					for (i = 0;i < JMM_DESTIP_BODY_LEN;i++)
						delnode->body[i] = left_node->body[i];

					delnode->left = left_node->left; 
					delnode->bf = delnode->bf  - 1; 
        			destip_dealloc(del_left);
					return 0;
				}
				else {
					delnode->key = right_node->key;
					for (i = 0;i < JMM_DESTIP_BODY_LEN;i++)
						delnode->body[i] = right_node->body[i];

					delnode->right = right_node->right; 
					delnode->bf = delnode->bf + 1; 
        			destip_dealloc(del_right);
				}
   				if( memcmp( delkey, &pre_delnode->key, sizeof( DESTIP_KEY ) ) > 0 )
					side = RIGHTSIDE;
				else 
					side = LEFTSIDE;
				top--;

			} /* pre_delnode�� 0�� �ƴҶ� */
		}	/* ���� ��� ���ʿ� ũ�ų� ���� ���� ������ */	
	}	/* ���� ��� �纯�� sub tree�� 2�� ������ */

	/* ���� ��尡 1������ sub tree�� ���� ���*/
	/* ���� ��尡 root�� ��� pre_delnode�� 0�� ���´�. �̰�� ���� ���δ�*/
	else{
		if (pre_delnode == 0) {
        	if(delnode->right != -1) 
           	 	*root = delnode->right;
    		else 
            	*root = delnode->left;
   		} 
		else if( memcmp( delkey, &pre_delnode->key, sizeof( DESTIP_KEY ) ) > 0 ) {
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
        /* delnode�ڽ��� �����Ǿ����Ƿ� ���ÿ��� �����Ѵ�.*/
        top--;
        destip_dealloc(dindex);
	}

	/* Step 3. Root���� ���鼭 rotate�� �����Ѵ�.*/
    while(top > 1){
		rindex = stack[--top];
		rotate = get_destip(rindex);
		pre_delnode = get_destip(stack[top-1]);	/* Parent */
		
		if (side < 0) {
            switch (rotate->bf) {
                case LEFTSIDE:
					if(pre_delnode == 0){
   	                	if(left_rotation_destip(rindex, root) == 0)
							return 0;
						else
						    break;
					}
					else if(pre_delnode->right == rindex){
   	                	if(left_rotation_destip(rindex, &pre_delnode->right) == 0)
							return 0;
						else
						    break;
					} 
					else if(pre_delnode->left == rindex){
   	                	if(left_rotation_destip(rindex, &pre_delnode->left) == 0)
							return 0;
						else
						    break;
					}
                case  0: rotate->bf = LEFTSIDE;
                     return 0;
                case RIGHTSIDE:		/* ��ü height�� 1���� ���*/
                     rotate->bf = 0;
                     break;
            }
			if (top <= 1 || pre_delnode == 0) return 0;
			/* side �� 0���� ũ�� left ������ right */
			side = memcmp( &pre_delnode->key, &rotate->key, sizeof( DESTIP_KEY ) );

		} 
		else {  /* parent�� left child�� ������ */
            switch (rotate->bf) {
                case LEFTSIDE: rotate->bf = 0;
                    break;
                case  0: rotate->bf = RIGHTSIDE;
                    return 0;
                case RIGHTSIDE: 
                    if(pre_delnode == 0){
                        if(right_rotation_destip(rindex, root) == 0)
                            return 0;
                        else
                            break;
                    } 
					else if(pre_delnode->right == rindex){
                        if(right_rotation_destip(rindex, &pre_delnode->right) == 0)
                            return 0;
                        else
                            break;
                    } 
					else if (pre_delnode->left == rindex){
                        if(right_rotation_destip(rindex, &pre_delnode->left) == 0)
                            return 0;
                        else
                            break;
					}
            }
			/* side �� 0���� ũ�� left ������ right */
			if (top <= 1 || pre_delnode == 0) return 0;
			side = memcmp( &pre_delnode->key, &rotate->key, sizeof( DESTIP_KEY ) );
        } 
    }
	return 0;
} 

/******
    return one key which is greater than first_key and less than last_key
******/
//DESTIP_TYPE *avl_select_destip( DESTIP_TYPE *tree, PDESTIP_KEY first_key, PDESTIP_KEY last_key)
DESTIP_TYPE *avl_select_destip( int index, PDESTIP_KEY first_key, PDESTIP_KEY last_key)
{
	DESTIP_TYPE *tree;
    int stack[100];
    int top;

    if( index == -1 || memcmp(first_key, last_key, sizeof(DESTIP_KEY)) >= 0)
        return 0;

    top = 0;
    stack[top++] = -1;

    while(index != -1)
    {
		tree = &destip_tbl->tuple[index];
        if (memcmp(first_key, &tree->key, sizeof(DESTIP_KEY)) < 0)
        {
            stack[top++] = index;
            index = tree->left;
        }
        else if (memcmp(first_key, &tree->key, sizeof(DESTIP_KEY)) > 0)
        {
            index = tree->right;
        }
        else
        {
            index = tree->right;
			tree = get_destip(index);
            while(index != -1)
            {
                stack[top++] = index;
                index = tree->left;
				tree = get_destip(index);
            }
            break;
        }
    }

	if (stack[--top] != -1)
	{
		tree = &destip_tbl->tuple[stack[top]];
    	if (memcmp(&tree->key, last_key, sizeof(DESTIP_KEY)) >= 0)
        	return 0;
	}

	//���⼭�� stack[top]==-1 �� ��쵵 ����ؾ� �ϹǷ�
	//&destip_tbl->tuple[stack[top]] �� �Ἥ�� �ȵȴ�.
	//get_destip�Լ��� �Ἥ, stack[top]==-1 �϶��� return ������ 0�� ������ �Ѵ�.
	tree = get_destip( stack[top] );
    return tree;
}

int avl_update_destip( DESTIP_TYPE *tree, long long *body )
{
    int i = 0;

    for (i = 0; i < JMM_DESTIP_BODY_LEN;i++) {
        tree->body[i] = body[i];
    }

    return 0;
}

DESTIP_TYPE *get_destip( int index )
{
	if( index >= 0 && index < JMM_DESTIP_RECORD )
		return (&destip_tbl->tuple[index]);
	else
		return 0;
}
