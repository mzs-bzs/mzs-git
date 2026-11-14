#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>


#include"game.h"

//函数的定义
void InintBoard(char board[ROWS][COLS], int r, int c,char set)
{
	int i = 0;
	for (i = 0; i < r; i++)
	{
		int j = 0;
		for (j = 0; j < c; j++)
		{
			board[i][j] = set;
		}
	}
}

void DisplayBoard(char board[ROWS][COLS], int r, int c)
{
	int i = 0;
	int j = 0;
	//列号打印
	for (j = 0;j <= c;j++)
	{
		printf("%d ", j);
	}
	printf("\n");
	for (i = 1; i <= r; i++)
	{
		printf("%d ", i);
		for (j = 1; j <= c; j++) 
		{
			printf("%c ", board[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void SetMine(char mine[ROWS][COLS], int r, int c)
{
	int count = EASY_COUNT;
	while (count)//>=EASY_COUNT
	{
		int x = rand() % r + 1;
		int y = rand() % c + 1;
		if (mine[x][y] != '1')
		{
			mine[x][y] = '1';
			count--;
		}
	}
}

//这个函数仅仅支持FindMine函数的实现而存在的
//这个函数仅仅内部使用
//static size_t GetMineCount(char mine[ROWS][COLS], int x, int y)
//{
//	return mine[x - 1][y] + mine[x - 1][y - 1] + mine[x][y - 1] + mine[x + 1][y - 1] + mine[x + 1][y] +
//		mine[x + 1][y + 1] + mine[x][y + 1] + mine[x - 1][y + 1] - 8 * '0';
//}
static size_t GetMineCount(char mine[ROWS][COLS], int x, int y)
{
	int i = 0;
	int j = 0;
	size_t count = 0;
	for (i = -1;i <= 1;i++)
	{
		for (j = -1;j <= 1;j++)
		{
			count += (mine[x + i][y + j] - '0');
		}
	}
	return count;
}

void FindMine(char mine[ROWS][COLS], char show[ROWS][COLS], int r, int c)
{
	int x = 0;
	int y = 0;
	int win = 0;
	//判断输入坐标合法性
	while (win < r*c-EASY_COUNT)
	{
		printf("请输入要排查的坐标:");
		scanf("%d%d", &x, &y);
		if (x >= 1 && x <= r && y >= 1 && y <= c)
		{
			//判断该位置是否被排查
			if (show[x][y] == '*')
			{
				//该位置是不是雷
				if (mine[x][y] == '0')//不是雷
				{
					size_t count = GetMineCount(mine, x, y);//统计mine数组中x，y周围雷的个数
					show[x][y] = (char)(count + '0');
					DisplayBoard(show, ROW, COL);
					win++;
				}
				else
				{
					printf("很遗憾，你被炸死了\n");
					DisplayBoard(mine, ROW, COL);
					break;
				}
			}
			else
			{
				printf("该坐标已经被排查过，请重新输入坐标\n");
			}
		}
		else
		{
			printf("输入坐标非法，请重新输入正确坐标\n");
		}
	}
	if (win == r * c - EASY_COUNT)
	{
		printf("恭喜你，排雷成功\n");
		DisplayBoard(mine, ROW, COL);
	}
}