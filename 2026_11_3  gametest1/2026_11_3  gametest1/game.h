#pragma once
#include<stdio.h>
#define ROW 9
#define COL 9
#define ROWS ROW+2
#define COLS COL+2



//初始化游戏区域函数
void InintBoard(char board[ROWS][COLS],int r,int c, char set);

//打印棋盘函数
void DisplayBoard(char board[ROWS][COLS], int r, int c);