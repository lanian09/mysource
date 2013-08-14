#include <stdio.h>

enum status{
	dont_used = 0x00,
	active    = 0x01,
	completed = 0x02
};

char g_activated_mdbu_list;

struct st_activated_mdbu {
	unsigned char mdbu1:2;
	unsigned char mdbu2:2;
	unsigned char mdbu3:2;
	unsigned char mdbu4:2;

};


int set_activated_mdbu_list()
{

	struct st_activated_mdbu *pstmdbu;

	g_activated_mdbu_list = 0; //initialize

	pstmdbu = (struct st_activated_mdbu*)&g_activated_mdbu_list;

	//ex) enable mdbu 1,3,4
	pstmdbu->mdbu1 = active;
	pstmdbu->mdbu3 = active;
	pstmdbu->mdbu4 = active;
	
	return 1;
}

char *get_activated_mdbu_status(int mdbu_status)
{
	switch(mdbu_status){
		case dont_used : return "Don't used";
		case active :    return "activated mdbu";
		default :        return "Completed mdbu.set";
	}
	return "unknown status";
}

#define SRDIA_STRUCT_OF(s,i)        (void *)((char *)(s) + sizeof(*(s)) * (i))

void mdbu_set_callback(void *tag[3])
{
	int  *index;
	struct st_activated_mdbu *pstmdbu;

	pstmdbu = (struct st_activated_mdbu*)&g_activated_mdbu_list;

	index = (int*)tag[0];
	/*
	if (*index == 1) {
		printf("pstmdbu->mdbu1=0x%02x ", pstmdbu->mdbu1);
		pstmdbu->mdbu1 |= completed;
		printf("=> 0x%02x[active=0x%02x, completed=0x%02x, or=0x%02x]\n", pstmdbu->mdbu1, active, completed, active | completed);
	}
	*/

	if (*index == 1) pstmdbu->mdbu1 |= completed;
	else if (*index == 2) pstmdbu->mdbu2 |= completed;
	else if (*index == 3) pstmdbu->mdbu3 |= completed;
	else if (*index == 4) pstmdbu->mdbu4 |= completed;
	
}

int main()
{
	struct st_activated_mdbu *pstmdbu;
	void  *usertag[3];
	int    index;

	pstmdbu = (struct st_activated_mdbu*)&g_activated_mdbu_list;

	set_activated_mdbu_list();

	if (pstmdbu->mdbu1) {
		printf("mdbu1's status=%s\n", get_activated_mdbu_status(pstmdbu->mdbu1)); 
		index = 1;
		usertag[0] = (void*)&index;
		mdbu_set_callback(usertag);
		usertag[0] = 0;
	}
	if (pstmdbu->mdbu2) {
		printf("mdbu2's status=%s\n", get_activated_mdbu_status(pstmdbu->mdbu2)); 
		index = 2;
		usertag[0] = (void*)&index;
		mdbu_set_callback(usertag);
		usertag[0] = 0;
	}
	if (pstmdbu->mdbu3) {
		printf("mdbu3's status=%s\n", get_activated_mdbu_status(pstmdbu->mdbu3)); 
		index = 3;
		usertag[0] = (void*)&index;
		mdbu_set_callback(usertag);
		usertag[0] = 0;
	}
	if (pstmdbu->mdbu4) {
		printf("mdbu4's status=%s\n", get_activated_mdbu_status(pstmdbu->mdbu4)); 
	}

	printf("mdbu1 status = 0x%02x\n", pstmdbu->mdbu1);
	printf("mdbu2 status = 0x%02x\n", pstmdbu->mdbu2);
	printf("mdbu3 status = 0x%02x\n", pstmdbu->mdbu3);
	printf("mdbu4 status = 0x%02x\n", pstmdbu->mdbu4);

	return;
}
