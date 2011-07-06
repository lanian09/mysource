#include <stdio.h>
#include "bst.h"

#define CMD_LIMIT 2

void init_tree(pTREE ptree);
void insert(pTREE ptree);
void search(pTREE ptree);
void delete(pTREE ptree);
void display(pTREE ptree);
int  input_data();

TREE tree;

int main()
{
	char ch,ch2;
	int eflag;

	printf("Binary Search Tree Start\n");

	/* init tree */
	//reserved :: 
	init_tree(&tree);

	eflag = 1;
	while(1){

		printf("\n1. insert\n");
		printf("2. search\n");
		printf("3. delete\n");
		printf("4. view Tree\n");
		printf("5. exit\n");
		printf("------------------\n");
		printf("command : ");
		ch = getchar();
		ch2 = getchar();
		printf("\n");

		switch(ch){
			case '1': case 'i': case 'I':
				insert(&tree); 
				break;

			case '2': case 's': case 'S':
				search(&tree);
				break;

			case '3': case 'd': case 'D':
				delete(&tree);
				break;

			case '4': case 'v': case 'V':
				display(&tree);
				break;

			case '5': case 'e': case 'E': case 'q': case 'Q':
				printf("Terminated by user\n");
				//reserved :: 
				eflag = -1;
				break;
			default:
				break;
		}
		

		if( eflag != 1 )
			break;

	}

	printf("Finished Program\n\n");
	return 0;
}

void init_tree(pTREE ptree)
{
	bst_init_tree(ptree);
}/* init tree */

void insert(pTREE ptree)
{
	int data;
	int ret;

	printf("-> BST_INSERT_FUNC\n");

	data = input_data();

/*
	ret = bst_insert(ptree,data);
	if( ret < 0 ){
		printf("FAILED bst_insert()[%d]\n",data);	
	}
	else
		printf("SUCCESS bst_insert()[%d:%d]\n",ret,data);
*/
	
}/* insert */

void search(pTREE ptree)
{
	int data;
	printf("-> BST_SEARCH_FUNC\n");
	data = input_data();

}/* search */

void delete(pTREE ptree)
{
	int data;
	int ret;

	printf("-> BST_DELETE_FUNC\n");

	data = input_data();

/*
	ret =  bst_delete(ptree,data);
	if( ret < 0 ){
		printf("FAILED bst_delete()[%d]\n",data);
	}
	else
		printf("SUCCESS bst_delete()[%d:%d]\n",ret,data);
*/
	
}/* delete */

void display(pTREE ptree)
{
	int i;
	printf("-> BST_DISPLAY_FUNC\n");
/*
	bst_view(ptree);
*/

}/* display */

int input_data()
{
	int data;
	char value[11];
	int i;

	printf("Input value : ");
	if( fgets(value,11,stdin) < 0 ){
		printf("FAILED input value(fgets)\n");
		return;
	}
	
	for( i=0;i<strlen(value);i++){
		if( value[i] == '\n' )
			break;
		if( value[i] > 57 || value[i] < 48 ){
			printf("Value is not NUMBER[%s]\n",value);
			return;
		}
	}

	data = atoi(value);

	return data;
}
