#include "mmcr.h"

FILE    *fp;
int	totalNodeCnt;
int open_conf_file(char *file, FILE **fp, char *mode);
int create_new_conf_list(DLinkedList *head);
int make_conf_list(char *fname, DLinkedList *head);
DLinkedList *new_node(DLinkedList *prev);
int seperate_columns(char *buff, char wbuff[MAX_COL_COUNT][MAX_COL_SIZE]);
void trim_trail_line(char *line);
DLinkedList *search_list_node(char srchStr[MAX_COL_COUNT][MAX_COL_SIZE],int startIndex, int compsize, DLinkedList *head);
DLinkedList *search_start_conf(DLinkedList *head);
DLinkedList *search_end_conf(DLinkedList *tail);
DLinkedList *insert_list_node(char insitem[MAX_COL_COUNT][MAX_COL_SIZE], int colsize, DLinkedList *tail);
void delete_list_node(DLinkedList *node);
DLinkedList *next_conf_node(DLinkedList *node);
DLinkedList *next_conf_node_compare(DLinkedList * node,char srchStr[MAX_COL_COUNT][MAX_COL_SIZE], int startIndex, int compsize, int paraCnt );
void fill_line_buff(char rbuff[MAX_COL_COUNT][MAX_COL_SIZE], char *wbuff, int rsize);
int flush_list_to_file(DLinkedList *head, char *fname);
void print_list_node(DLinkedList *head);
int getTotalNodeCnt();


//	file open

int open_conf_file(char *file, FILE **fp, char *mode)
{
    if((*(fp) = fopen(file, mode)) == NULL){
        //do logging
        return -1;
    }

	rewind(*(fp));

	return 0;
}


//	new conf list - start node, end node 
int create_new_conf_list(DLinkedList *head)
{
	DLinkedList *node, *prev = head;

	if((node = new_node(prev)) == NULL){
		//fail to allocate new node
		return -1;
	}

	// @START
	node->item.gubun = TOK_START_LINE;
	strncpy(node->item.line, CONF_START_LINE, BUFSIZ);

	prev = node;
	if((node = new_node(prev)) == NULL){
		//fail to allocate new node
		return -1;
	}

	// @END
	node->item.gubun = TOK_END_LINE;
	strncpy(node->item.line, CONF_END_LINE, BUFSIZ);

	return 0;

}

//total node list
int make_conf_list(char * fname, DLinkedList *head)
{
	char 	tbuff[BUFSIZ];
	DLinkedList *node, *prev = head;
	FILE	*fp;
	if(open_conf_file(fname, &fp, "r") < 0){
         fprintf (stdout, "[make_conf_list] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
		//fclose(fp);
        return -1;
    }
	totalNodeCnt=0;
	memset(tbuff, 0x00, BUFSIZ);
	// read line , limit BUFSIZ
	while(fgets(tbuff, BUFSIZ, fp)){
		if((node = new_node(prev)) == NULL){
			//fail to allocate new node
			fclose(fp);
			return -1;
		}
		// copy the buffer to node->item.line
		strncpy(node->item.line, tbuff, BUFSIZ);
		// define the node->item.gubun( TOK_COMMENT,TOK_START_LINE,TOK_END_LINE,TOK_UNKNOWN,TOK_EMPTY_LINE,TOK_CONF_LINE )
		if(node->item.line[0] == '#'){
			node->item.gubun = TOK_COMMENT;
		}else if(node->item.line[0] == '@'){
			if(!strncmp(node->item.line, CONF_START_LINE,strlen(CONF_START_LINE))){
				node->item.gubun = TOK_START_LINE;
			}else if(!strncmp(node->item.line, CONF_END_LINE, strlen(CONF_END_LINE))){
				node->item.gubun = TOK_END_LINE;
			}else{
				node->item.gubun = TOK_UNKNOWN;
			}
		}else if(node->item.line[0] == '\n'){
			node->item.gubun = TOK_EMPTY_LINE;
		}else{
			node->item.gubun = TOK_CONF_LINE;
			trim_trail_line(node->item.line);
			seperate_columns(node->item.line, node->item.columns);
			totalNodeCnt++;
		}

		prev = node;
		memset(tbuff, 0x00, BUFSIZ);
	}

    // Sort exception
    // UDR_FILE and UAWAPANA_GW_FILE are not sorted.
	/* jjinri 05.30
    if(!strstr(fname, UDR_FILE) && !strstr(fname, UAWAPANA_GW_FILE))
        sort_asc_conf_nodes(head);
	*/
	fclose(fp);
	return 0;
}

int getTotalNodeCnt()
{
	return totalNodeCnt;
}

DLinkedList *new_node(DLinkedList *prev)
{
	DLinkedList *node;

	node = (DLinkedList *)calloc(sizeof(DLinkedList), 1);

	if(!node){
		//memory allocation error
		return NULL;
	}

	//initialization
	node->prev = prev;
	node->next = prev->next;
	node->next->prev = node;
	prev->next = node;

	return node;
}

int seperate_columns(char *buff, char wbuff[MAX_COL_COUNT][MAX_COL_SIZE])
{
	int i = 0, j, col = -1;
	int flagNewCol = 1;

	while(buff[i] != '\n'){
		if(buff[i] == ' ' || buff[i] == '\t'){
			i++;
			flagNewCol = 1; // Prepare a New column
			continue;
		}
		if(flagNewCol){
			j = 0;
			col++;
			flagNewCol = 0; //free New column flag
		}
		if(j < MAX_COL_SIZE)
			wbuff[col][j] = buff[i];
		j++;
		i++;
	}
	return col;
}

void trim_trail_line(char *line)
{
	char *p = line+(strlen(line)-2);

	while(p != line){
		if(*p == '\t' || *p == ' ' || *p == '\n'){
			*p = '\n';
			*(p+1) = '\0';
		}else
			break;
		p--;
	}

//fprintf(stderr, "line - %s", line);

}

DLinkedList *search_list_node(char srchStr[MAX_COL_COUNT][MAX_COL_SIZE],int startIndex, int compsize, DLinkedList *head)
{
	int			i, found = 0;
	DLinkedList *node = head;
	while(node->next){
		node = node->next;

		if(node->item.gubun != TOK_CONF_LINE){
			continue;
		}
		for(i = startIndex; i < startIndex+compsize; i++){
			fprintf( stdout, "index : %d , srcStr : %s, destStr : [%s] \n", i, srchStr[i], node->item.columns[i] );
			if( !strncasecmp(srchStr[i], node->item.columns[i], MAX_COL_SIZE)){
				found = 1;
			}else{
				found = 0;
				break;
			}
		}
		if(found)
			break;
	}
	if(found)
		return node;
	else
		return NULL;

}

DLinkedList *search_node(char *srchStr, int pos, DLinkedList *head)
{
	DLinkedList        *found = NULL;
	static DLinkedList *node = NULL;

    if(head){
        node = head;
    }

	while(node && node->next){
		node = node->next;

		if(node->item.gubun != TOK_CONF_LINE){
			continue;
		}
		//fprintf(stderr, "node->item.columns[pos]: %s\n", node->item.columns[pos]);
		//`fprintf(stderr, "srchStr: %s\n",srchStr);	
		if(!strcasecmp(srchStr, node->item.columns[pos])){
            found = node;
            break;
		}
	}

    return found;
}

DLinkedList *search_node3(char *srchStr, int pos, DLinkedList *head)
{
	DLinkedList        *found = NULL;
    static DLinkedList *node = NULL;
	int idx=0;

    if(head){
        node = head;
    }

    while(node && node->next){
       	if(idx > 0) 
			node = node->next;

        if(node->item.gubun != TOK_CONF_LINE){
            continue;
        }
        //fprintf(stderr, "node->item.columns[pos]: %s\n", node->item.columns[pos]);
        if(!strcasecmp(srchStr, node->item.columns[pos])){
            found = node;
            break;
        }
		idx++;
    }
    return found;
}

DLinkedList *search_node2(char *srchStr1, int pos1,DLinkedList *head)
{
	DLinkedList        *found = NULL;
   	static DLinkedList *node = NULL;

    if(head){
    	node = head;
    }

    if(node->item.gubun != TOK_CONF_LINE){
       	;
    }

	//fprintf(stderr, "srchStr1: %s\n",srchStr1);
    if(!strcasecmp(srchStr1, node->item.columns[pos1])){
      	found = node;
    }

    return found;
}

DLinkedList *delete_search_node(int startIndex, DLinkedList *head)
{
	int		compVal;
	DLinkedList *result = NULL, *node = head;
	while(node->next){
		node = node->next;

		if(node->item.gubun != TOK_CONF_LINE){
			continue;
		}
		if(!result){
			result = node;
		}else{
			compVal = strcasecmp(node->item.columns[startIndex], result->item.columns[startIndex]);
			if(compVal < 0){
				result = node;
			}
		}
		
	}
	return result;
}

DLinkedList *next_conf_node(DLinkedList * node ) {
	//static int i = 0;
	DLinkedList *cur_node = node;

	while(cur_node->next){
		cur_node = cur_node->next;
		if(cur_node->item.gubun == TOK_CONF_LINE)
			return cur_node;
	}

	return NULL;
}

DLinkedList *next_conf_node_compare(DLinkedList * node,char srchStr[MAX_COL_COUNT][MAX_COL_SIZE], int startIndex, int compsize, int paraCnt ) {
    int         i, found = 0;
    DLinkedList *cur_node = node;

    if( paraCnt > 0 ){
    while(cur_node->next){
        cur_node = cur_node->next;

        if(cur_node->item.gubun != TOK_CONF_LINE){
            continue;
        }
        for(i = startIndex; i < startIndex+compsize; i++){
        //  fprintf( stdout, "index : %d , srcStr : %s, destStr : [%s] \n", i, srchStr[i], cur_node->item.columns[i] );
            if( !strncasecmp(srchStr[i], cur_node->item.columns[i], MAX_COL_SIZE)){
                found = 1;
            }else{
                found = 0;
                break;
            }
        }
        if(found)
            break;
    }
    if(found)
        return cur_node;
    else
        return NULL;

    } else 
        cur_node = next_conf_node( node );

    return cur_node;

}


DLinkedList *search_start_conf(DLinkedList *head)
{
	DLinkedList *node = head;

	while(node->next){
		node = node->next;
		if(node->item.gubun == TOK_START_LINE)
			break;
	}

	if(!node->next)
		return NULL;
	else
		return node;
}

DLinkedList *search_end_conf(DLinkedList *tail)
{
	DLinkedList *node = tail;
	 //fprintf( stdout, "search node gubun : %d \n", node->item.gubun );
	while(node->prev){
		node = node->prev;
		//fprintf( stdout, "search whilenode gubun : %d \n", node->item.gubun );
		if(node->item.gubun == TOK_END_LINE)
			break;
	}

	//fprintf( stdout, "while end \n" );

	if(!node->prev)
		return NULL;
	else
		return node;
}

DLinkedList *insert_list_node(char insitem[MAX_COL_COUNT][MAX_COL_SIZE], int colsize, DLinkedList *tail)
{
	int		i;
	DLinkedList *node, *prev, *confend;
	confend = search_end_conf(tail);
	if ( confend==NULL )
		return NULL;

	prev = confend->prev;
	if( (node = new_node(prev)) == NULL){
		//fail to allocate new node
		return NULL;
	}

	node->item.gubun = TOK_CONF_LINE;
	for(i = 0; i < colsize; i++){
		strncpy(node->item.columns[i], insitem[i], MAX_COL_SIZE);
	}

	fill_line_buff(node->item.columns, node->item.line, colsize);

	return node;

}

void delete_list_node(DLinkedList *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;

	free(node);
}

void print_list_node(DLinkedList *head)
{
	DLinkedList *node = head;

	while(node->next){
		node = node->next;
		//printf("%s", node->item.line);
	}
}

void fill_line_buff(char rbuff[MAX_COL_COUNT][MAX_COL_SIZE], char *wbuff, int rsize)
{
    int     i, len = 0;
    char    *tbuff = wbuff;

    for(i = 0; i < rsize; i++){
        len = strlen(rbuff[i]);
        if(len > 0){
            strcpy(tbuff, rbuff[i]);
            tbuff += len;
        }else{
            strcpy(tbuff, "-"); // jjinri  06.07
            tbuff += 1;
        }
        strcat(tbuff, "\t");
        tbuff += 1;
    }

    wbuff[strlen(wbuff)-1] = '\n';
    wbuff[strlen(wbuff)] = '\0';
}

int flush_list_to_file(DLinkedList *head, char * fname)
{
	DLinkedList *node = head;
	FILE    *fp;
    if(open_conf_file(fname, &fp, "w") < 0){
         fprintf (stdout, "[flush_list_to_file] fopen fail[%s]; err=%d(%s)\n", fname, errno, strerror(errno));
        //fclose(fp);
        return -1;
    }

	//rewind(fp);
    // Sort exception
    // UDR_FILE and UAWAPANA_GW_FILE are not sorted.
	/* jjinri 05.30
    if(!strstr(fname, UDR_FILE) && !strstr(fname, UAWAPANA_GW_FILE))
        sort_asc_conf_nodes(head);
	*/

	while(node->next){
		node = node->next;
        if(strlen(node->item.line) == 0) continue;
		if(EOF == fputs(node->item.line, fp)){
			fprintf( stdout, "Eof \n" );
			fclose(fp);
			return -1;
		}
	}
	fflush(fp);
	fclose(fp);

	return 0;
}

void set_head_tail(DLinkedList *head, DLinkedList *tail)
{
    memset(head, 0x00, sizeof(DLinkedList));
    memset(tail, 0x00, sizeof(DLinkedList));

	head->prev = NULL;
	head->next = tail;
	tail->prev = head;
	tail->next = NULL;
}

void delete_all_list_node(DLinkedList *head, DLinkedList *tail)
{
	DLinkedList *temp = head;

	while(head->next != tail){
		temp = head->next;
		head->next = temp->next;
		temp->next->prev = head;
		free(temp);
	}
}

void sort_asc_conf_nodes(DLinkedList *head)
{
    int         compRes;
    DLinkedList *startnode, *polenode, *compnode, *objnode;
    DLinkedList *pNode, *indexnode;

    pNode = head->next;

    while((pNode->item.gubun != TOK_START_LINE) && (pNode->next != NULL))
        pNode = pNode->next;

    if(pNode->next == NULL) // we meet the tail node, list is not standard.
        return;

    startnode = pNode;

    if(startnode->next->item.gubun == TOK_END_LINE)  //list is empty.
        return;

    polenode = startnode->next;
    if(polenode->next->item.gubun == TOK_END_LINE)   // list has only one node.
        return;


    /*
     --       --       --
    |  | --> |  | --> |  |
     --       --       --
    pole     comp     index
    (obj)
    1. compare pole with comp.
    2. index represents next nodes to compare with.
    */
    compnode = polenode->next;
    indexnode = compnode->next;
    objnode = polenode;
    while(compnode->item.gubun != TOK_END_LINE){
        while(objnode != compnode){             // it meets itself.
            compRes = compare_nodes(objnode, compnode);
            if(compRes > 0){
                // disconnect compnode's current connnection list
                compnode->prev->next = compnode->next;  // set previous node's next of compnode's to its current node's next.
                compnode->next->prev = compnode->prev;  // set next node's prev of compnode's to its current node's prev.
                compnode->prev = objnode->prev;         // set compnode's prev to objnode's prev.
                compnode->prev->next = compnode;        // set previous node's next of compnode's to compnode.
                compnode->next = objnode;
                objnode->prev = compnode;

                if(objnode == polenode){
                    polenode = compnode;
                }
                break;
            }
            objnode = objnode->next;
        }
        compnode = indexnode;
        indexnode = indexnode->next;
        objnode = polenode;
    }

}

int compare_nodes(const DLinkedList *node1, const DLinkedList *node2)
{
    int     i, compRes;

    for(i = 0; i < MAX_COL_COUNT; i++){
        if(strlen(node1->item.columns[i]) == 0) break;

        if(node1->item.columns[i][0] == '$')
            return -1;
        else if(node2->item.columns[i][0] == '$')
            return 1; 

        compRes = strcasecmp(node1->item.columns[i], node2->item.columns[i]);
        if(compRes != 0)
            return compRes;
    }

    return 0;
}
