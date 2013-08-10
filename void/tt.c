#include <stdio.h>
#define SRDIA_STRUCT_OF(a,b) (void *)((char *)(a) + sizeof(*(a)) * (b))
struct dou_alarm {
	char pd1;
	char pd2;
	char pd3;
};
struct dou_ {
	struct dou_alarm alm;
	char pd1_pwr;
	char pd2_pwr;
	char pd3_pwr;
	short pd1_att;
	short pd2_att;
	short pd3_att;
};

int main()
{
	struct dou_ dou;
	void *addr_of;

	int i;
	int *di;

	di = (int *)&dou.pd1_pwr;

	dou.alm.pd1 = 1;
	dou.alm.pd1 = 0;
	dou.alm.pd1 = 1;

	dou.pd1_pwr = 23;
	dou.pd2_pwr = 33;
	dou.pd3_pwr = 53;

	dou.pd1_att = 125;
	dou.pd2_att = 225;
	dou.pd3_att = 325;

	printf("address &dou=%p\n", &dou);
	printf("address &dou.pd1_pwr=%p(=%d:di(=%p)=%d)\n", &dou.pd1_pwr, *(&dou.pd1_pwr), di, *(char *)di);
	printf("address &dou.pd2_pwr=%p\n", &dou.pd2_pwr);
	printf("address &dou.pd3_pwr=%p\n", &dou.pd3_pwr);

	for (i = 0; i < 3; i++) {
		//addr_of = (void *)((char *)(&dou->pd1_pwr) + sizeof(*(&dou->pd1_pwr)) * (i));
		addr_of = SRDIA_STRUCT_OF(&dou.pd1_pwr,i);
		printf("%d]pd_pwr:addr_of(%p)=%d\n", i, addr_of, *(char *)addr_of);

		addr_of = SRDIA_STRUCT_OF(&dou.alm.pd1,i);
		printf("%d]pd_alm:addr_of(%p)=%d\n", i, addr_of, *(char *)addr_of);

		addr_of = SRDIA_STRUCT_OF(&dou.pd1_att,i);
		printf("%d]pd_att:addr_of(%p)=%d\n", i, addr_of, *(short *)addr_of);
	}
	/*
	for (i = 0; i < 4; i++) {
	addr_of = (void *)((char *)(&dou->pd1_pwr) + sizeof(*(&dou->pd1_pwr)) * (i));
	sprintf(path, "%s.p%d.pdPwr2", jsonKey,i+1);
	ijson_set_string(data, path, Hbase_step01cvt((int)addr_of));

	addr_of = (void *)((char *)(&dou->alm.pd1) + sizeof(*(&dou->al
	*/
}
