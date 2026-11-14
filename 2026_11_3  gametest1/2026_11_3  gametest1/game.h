#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define ROW 9
#define COL 9
#define ROWS ROW+2
#define COLS COL+2

#define EASY_COUNT 10



//初始化游戏区域函数
void InintBoard(char board[ROWS][COLS],int r,int c, char set);

//打印棋盘函数
void DisplayBoard(char board[ROWS][COLS], int r, int c);

//布置雷
void SetMine(char mine[ROWS][COLS], int r, int c);

//排查雷
void FindMine(char mine[ROWS][COLS],char show[ROWS][COLS],int r, int c);

//统计mine数组中x，y周围雷的个数
