#include <stdio.h>

int main()
{
	long long   rateloadval = 0;
	int succcnt = 32;
	int trialcnt = 90;
	rateloadval = (float)succcnt / (float)trialcnt * 100.0;
	printf("%u , %lld\n", rateloadval, rateloadval);
	return 0;
}
