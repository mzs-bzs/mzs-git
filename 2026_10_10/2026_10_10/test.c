#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>

int Fac(int n)
{
	int ret = 1;
	int i = 0;
	for (i = 1;i <= n;i++)
	{
		ret *= i;
	}
	return ret;
}

int main()
{
	int n = 0;
	scanf("%d", &n);
	int r = Fac(n);
	printf("%d", r);
	return 0;
}