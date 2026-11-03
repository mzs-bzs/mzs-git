#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>


int main()
{
	int arr[3][5] = {0};
	int j = 0;
	for (j = 0;j < 3;j++)
	{
		int i = 0;
		for (i = 0;i < 5;i++)
		{
			scanf("%d", &arr[j][i]);
		}
	}
	for (j = 0;j < 3;j++)
	{
		int i = 0;
		for (i = 0;i < 5;i++)
		{
			printf("%d", arr[j][i]);
		}
		printf("\n");
	}

	return 0;
}