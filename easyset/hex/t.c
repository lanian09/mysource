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

void test_MDBU()
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

#define SET_ROU_END(p)  ((p) |= 0x01 << 7)
#define SET_RDU_END(p)  ((p) |= 0x01 << 6)

void test_ROU()
{
	unsigned char a1 = 0x01;
	unsigned char a2 = 0x02;
	unsigned char a3 = 0x03;

	printf(" [before]\n");
	printf("rou1 = 0x%02x\n", (a1));
	printf("rou2 = 0x%02x\n", (a2));
	printf("rou3 = 0x%02x\n", (a3));

	printf(" [after]\n");
	printf("rou1 = 0x%02x\n", SET_ROU_END(a1));
	printf("rou2 = 0x%02x\n", SET_ROU_END(a2));
	printf("rou3 = 0x%02x\n", SET_ROU_END(a3));

	printf("rdu1 = 0x%02x\n", SET_RDU_END(a1));
	printf("rdu2 = 0x%02x\n", SET_RDU_END(a2));
	printf("rdu3 = 0x%02x\n", SET_RDU_END(a3));
}

void test_hex()
{
	unsigned char a;
	int i;

	for (i = 0, a = 0x01; i < 8; i++) {
		printf("a%d=0x%02x\n", i, a << i);
	}
}

void test_struct()
{
	struct stt {
		unsigned char a1:2;
		unsigned char a2:2;
		unsigned char a3:2;
		unsigned char a4:2;
	} a;

	struct stt2 {
		char a;
		char b;
		int  c;
	} c;
	unsigned char b;

	printf("sizeof(a)=%d\n", sizeof(a));
	printf("sizeof(b)=%d\n", sizeof(b));
	printf("sizeof(c)=%d\n", sizeof(c));

	a.a1 = 0x01;
	a.a2 = 0x02;
	a.a3 = 0x02;
	a.a4 = 0x01;

	printf("a.a1=0x%02x\n", a.a1);
	printf("a.a2=0x%02x\n", a.a2);
	printf("a.a3=0x%02x\n", a.a3);
	printf("a.a4=0x%02x\n", a.a4);
	printf("a   =0x%02x\n", a);
}

int get_test_ptr(int type, int *bi[2])
{
	int ret = 1;
	switch (type) {
		case 1: case 3: *bi[0] = 100; break;;
		case 2: case 4: *bi[0] = 200; *bi[1] = 300; ret = 2;
				break;
		default:
				return 0;
	}
	return ret;
}

void test_ptr()
{
	int *bi[2];
	int a,b;
	int ret;
	int i;

	bi[0] = &a;
	bi[1] = &b;

	for (i = 0; i < 6; i++) {
		ret = get_test_ptr(i, bi);
		if (!ret) continue;

		if (ret == 2)      printf("[%d]bi[0]=%d, bi[1]=%d\n", i, *bi[0], *bi[1]);
		else if (ret == 1) printf("[%d]bi[0]=%d\n", i, *bi[0]);
	}
}

void ttt(void *ii[2])
{
	int a;
	int b;

	a = (int)ii[0];
	b = (int)ii[1];

	printf("a=%d\n", a);
	printf("b=%d\n", b);
}
void test_ptr2()
{
	int *a;
	int *b;
	void *tag[2];

	a = (int *)11;
	b = (int *)7;
	tag[0] = (void *)a;
	tag[1] = (void *)b;

	ttt(tag);
}

struct st1 {
	int a;
	struct st1 *pst;
};

void t2(struct st1 *a[2])
{
}

void test_st()
{
	struct st1 *a[2];
	struct st1 b1, b2;

	a[0] = &b1;
	a[1] = &b2;

	t2(a);
}

int main()
{

	test_MDBU();
	test_ROU();
	test_hex();
	test_struct();
	test_ptr();
	test_ptr2();
	test_st();
	return;
}
