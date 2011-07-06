
typedef struct st_TT{
	int a;
	char b;
}st_T;

st_T stt[1000];

int main()
{
	st_T stt[1000];

	printf("stt:%p\n",stt);
	printf("&stt:%p\n",&stt);
	printf("&stt[0]:%p\n",&stt[0]);

	return 0;
}
