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
	for (i = 1; i <= r; i++)
	{
		int j = 0;
		for (j = 1; j <= c; j++)
		{
			printf("%c", board[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}