#include<stdio.h>
#include<string.h>
//int main()
//{
//	int arr[] = {1,2,3,4,5,6,7,8,9,10};
//	int left = 0;
//	int right = sizeof(arr) / sizeof(arr[0]) - 1;
//	int key = 7;//要找的数字
//	int mid = 0;//记录元素下标
//	int find = 0;
//	while (left <= right)
//	{
//		mid = (left + right) / 2;
//		if (arr[mid] > key)
//		{
//			right = mid - 1;
//
//		}
//		else if (arr[mid] < key)
//		{
//			left = mid - 1;
//		}
//		else
//		{
//			find = 1;
//			break;
//		}
//	}
//	if (1 == find)
//		printf("找到了，下标是%d\n", mid);
//	else
//		printf("找不到\n");
//	return 0;
//}
#include<windows.h>
int main()
{
	char arr1[] = "I Love My Home -- GuiZzhou!!!";
	char arr2[] = "#############################";
	size_t left = 0;
	size_t right = strlen(arr1) - 1;
		
	while (left <= right)
	{
		arr2[left] = arr1[left];
		arr2[right] = arr1[right];
		printf("%s\n", arr2);
		Sleep(1000);
		system("cls");
		left++;
		right--;
	}
	return 0;

}