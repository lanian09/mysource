#include <stdio.h>

#define GAML  g_activated_mdbu_list
#define SET_MDBU(i,v) (GAML |= ((v) << ((4-(i))*2)))
#define GET_MDBU(i)   ((GAML >> ((4-(i))*2)) & 0x03)  

enum status{
	dont_used = 0x00,
	active    = 0x01,
	completed = 0x02
};

char g_activated_mdbu_list;

struct st_activated_mdbu {
	unsigned char mdbu4:2;
	unsigned char mdbu3:2;
	unsigned char mdbu2:2;
	unsigned char mdbu1:2;

};


int set_activated_mdbu_list()
{

	g_activated_mdbu_list = 0; //initialize

	SET_MDBU(1, active);
	SET_MDBU(3, active);
	SET_MDBU(4, active);

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

	SET_MDBU(*index, completed);
}

int main()
{
	struct st_activated_mdbu *pstmdbu;
	void  *usertag[3];
	int    index;
	int    i;

	pstmdbu = (struct st_activated_mdbu*)&g_activated_mdbu_list;

	set_activated_mdbu_list();

	printf("==== before ====\n");

	for (i = 0; i < 4; i++){

		index = i+1;
		if (GET_MDBU(index)) {
			printf("mdbu%d status = 0x%02x\n", index, GET_MDBU(index));
			if (4 == index) continue;
			usertag[0] = (void*)&index;
			mdbu_set_callback(usertag);
			usertag[0] = 0;
		}
	}

	printf(" === after ===\n");

	printf("mdbu1 status = 0x%02x\n", GET_MDBU(1));
	printf("mdbu2 status = 0x%02x\n", GET_MDBU(2));
	printf("mdbu3 status = 0x%02x\n", GET_MDBU(3));
	printf("mdbu4 status = 0x%02x\n", GET_MDBU(4));

	return;
}
