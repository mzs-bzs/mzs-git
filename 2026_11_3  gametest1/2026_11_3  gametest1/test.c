#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include"game.h"

void menu()
{
	printf("------------------------\n");
	printf("--------  1.play  ------\n");
	printf("--------  0.exit  ------\n");
	printf("------------------------\n");

}

void game()
{
	//存储数据的数组
	char mine[ROWS][COLS];//布置好的雷的信息
	char show[ROWS][COLS];//排查出的雷的信息
	//初始化游戏区域
	InintBoard(mine, ROWS, COLS, '0');//'0'
	InintBoard(show, ROWS, COLS, '*');//'*'
	//打印棋盘游戏区
	DisplayBoard(mine, ROW, COL);
	DisplayBoard(show, ROW, COL);
	
}

int main()
{
	int input = 0;
	do {
		menu();
		printf("请选择：");
		scanf("%d", &input);
		switch (input)
		{
		case 1:
			game();//真正的扫雷游戏
			break;
		case 0:
			printf("退出游戏\n");
			break;
		default:
			printf("选择错误，请重新选择\n");
			break;
		}

	} while (input);

	return 0;
}