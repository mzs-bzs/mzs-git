/*
 * 让 Visual Studio 不再因为使用 sscanf 而报“不安全函数”的警告。
 * 这个宏必须写在所有 #include 之前。
 */
#define _CRT_SECURE_NO_WARNINGS

/*
 * 让 MSVC 把中文字符串按 UTF-8 保存到 exe 里。
 *
 * 之前出现乱码的原因：
 *   程序运行时会调用 SetConsoleOutputCP(CP_UTF8)，让控制台按 UTF-8 显示；
 *   但如果没有 /utf-8 编译选项，MSVC 会把源码里的中文按系统编码（GBK）
 *   存进 exe。控制台用 UTF-8 去解释 GBK 字节，就变成了乱码。
 *
 * 项目里四个配置都已经加了 /utf-8；这里再加一道保险，
 * 即使以后项目设置被改掉，这个文件里的中文也始终按 UTF-8 保存。
 */
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"

#ifdef _WIN32
#include <windows.h>
#endif

/*
 * ============================================================================
 *  扫雷游戏（控制台版）—— test.c
 * ============================================================================
 *
 *  【这个文件负责什么？】
 *  它是程序入口，负责“和玩家打交道”：
 *      · 显示主菜单，让玩家选择难度
 *      · 读取玩家输入（选择操作、输入行列坐标）
 *      · 调用 game.c 里的函数完成游戏逻辑
 *      · 显示胜利或失败的信息
 *
 *  【游戏基本流程】
 *      开始 → 选难度 → 循环{ 打印棋盘 → 读操作 → 翻开/插旗 }
 *           → 胜利或踩雷 → 显示结果 → 回到主菜单
 * ============================================================================
 */

/*
 * 把控制台的输入、输出编码都设置为 UTF-8。
 * 本项目源文件使用 UTF-8 保存，编译时也加了 /utf-8 选项，
 * 两边保持一致，中文才不会显示成乱码。
 */
static void init_console(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

/*
 * 读取一整行输入，存进 buf 里。
 * 返回值：1 = 读取成功，0 = 输入结束（例如按了 Ctrl+Z）。
 *
 * 为什么用 fgets 而不是 scanf("%s")？
 *   fgets 会把空格也一起读进来，而且可以限制最多读多少个字符，
 *   不会发生“输入太长把内存写坏”的情况。
 */
static int read_line(char *buf, int size)
{
    int len;

    if (fgets(buf, size, stdin) == NULL) {
        return 0;   /* 文件结束或读取失败 */
    }

    len = (int)strlen(buf);

    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';   /* 去掉行尾的换行符 */
    } else {
        /*
         * 这一行太长，缓冲区没有装下完整的行。
         * 把剩下的字符读掉并丢弃，避免它们影响下一次输入。
         */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* 空循环，只负责丢弃字符 */
        }
    }

    return 1;
}

/*
 * 显示提示语，读取一个整数。
 * 返回值：1 = 成功读到整数，0 = 输入结束。
 *
 * 如果玩家输入的不是整数（例如输入了字母），会提示后重新读取，
 * 而不会让程序陷入死循环，也不会直接退出。
 */
static int read_int(const char *prompt, int *value)
{
    char line[64];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);   /* 立刻显示提示，不要等到换行 */

        if (!read_line(line, (int)sizeof(line))) {
            return 0;     /* 输入结束 */
        }

        if (sscanf(line, "%d", value) == 1) {
            return 1;     /* 成功读到一个整数 */
        }

        printf("输入无效，请输入一个整数。\n");
    }
}

/* 打印主菜单 */
static void print_menu(void)
{
    printf("\n");
    printf("============================================\n");
    printf("               扫  雷  游  戏\n");
    printf("============================================\n");
    printf("   1. 初级    9 x 9     10 颗雷\n");
    printf("   2. 中级   16 x 16    40 颗雷\n");
    printf("   3. 高级   16 x 30    99 颗雷\n");
    printf("   0. 退出游戏\n");
    printf("--------------------------------------------\n");
}

/* 打印棋盘上各种符号的含义 */
static void print_help(void)
{
    printf("\n------------------ 符号说明 ------------------\n");
    printf("  #  未翻开          F  已插旗\n");
    printf("  .  已翻开的空白    *  地雷（游戏结束时显示）\n");
    printf("  X  插错的旗（游戏结束时显示）\n");
    printf("  1~8  这个格子周围 8 格中一共有几颗雷\n");
    printf("  如果翻开的格子周围没有雷，会自动向外扩散。\n");
    printf("  把除地雷外的所有格子都翻开，就获胜了！\n");
    printf("----------------------------------------------\n");
}

/*
 * 玩一局游戏。
 *   rows / cols / mines —— 本局的棋盘大小和雷数
 * 返回值：1 = 正常结束，回到主菜单；0 = 输入结束，程序退出。
 */
static int play_round(int rows, int cols, int mines)
{
    Game game;          /* 一局游戏的全部数据 */
    char line[128];
    int action;         /* 玩家的操作：1=翻开，2=插旗，0=返回 */
    int row, col;       /* 玩家输入的坐标（从 1 开始） */

    game_init(&game, rows, cols, mines);

    printf("\n================ 新 的 一 局 ================\n");
    printf("棋盘：%d 行 × %d 列，共 %d 颗地雷\n",
           game.rows, game.cols, game.mine_count);
    print_help();

    /* 只要游戏还在进行，就不断“打印棋盘 → 等玩家操作” */
    while (game.state == STATE_PLAYING) {
        game_print(&game);

        if (!read_int("请选择操作（1=翻开  2=插旗/取消旗  0=返回菜单）：", &action)) {
            return 0;
        }

        if (action == 0) {
            printf("本局结束，返回主菜单。\n");
            return 1;
        }
        if (action != 1 && action != 2) {
            printf("无效的操作，只能输入 0、1 或 2。\n");
            continue;
        }

        /* 读取行号和列号 */
        printf("请输入行 列（都从 1 开始，例如：3 5）：");
        fflush(stdout);

        if (!read_line(line, (int)sizeof(line))) {
            return 0;
        }
        if (sscanf(line, "%d %d", &row, &col) != 2) {
            printf("输入格式不对，请输入两个数字，中间用空格隔开。\n");
            continue;
        }

        row -= 1;   /* 玩家从 1 开始数，数组从 0 开始数 */
        col -= 1;

        if (action == 1) {
            int result = game_dig(&game, row, col);

            if (result == DIG_INVALID) {
                printf("不能翻开这个格子：可能超出范围、已经翻开，或者插了小旗。\n");
            } else if (result == DIG_OK) {
                printf("已翻开第 %d 行第 %d 列。\n", row + 1, col + 1);
            }
            /*
             * 如果 result 是 DIG_EXPLODE（踩雷），game.state 已经变成
             * STATE_LOST，循环条件不成立，会自然退出，然后统一显示结果。
             */
        } else {
            int result = game_flag(&game, row, col);

            if (result == 1) {
                printf("第 %d 行第 %d 列的旗子状态已切换。\n", row + 1, col + 1);
            } else if (result == 0) {
                printf("这个格子已经翻开，不能插旗。\n");
            } else {
                printf("不能操作这个位置：可能超出了棋盘范围。\n");
            }
        }
    }

    /*
     * 走到这里说明游戏已经结束（胜利或失败）。
     * 先揭开所有地雷、标记插错的旗，再打印最终棋盘。
     */
    game_reveal_all(&game);
    game_print(&game);

    if (game.state == STATE_WON) {
        printf("\n恭喜你！所有安全格子都翻开了，你赢了！\n");
    } else {
        printf("\n很遗憾，踩到地雷了，游戏失败！\n");
        printf("提示：'X' 表示插错的旗，'*' 表示地雷。\n");
    }

    /* 暂停一下，让玩家看清楚结果，再回主菜单 */
    printf("\n按回车键返回主菜单……");
    fflush(stdout);

    if (!read_line(line, (int)sizeof(line))) {
        return 0;
    }

    return 1;
}

int main(void)
{
    int choice;

    init_console();

    /*
     * 设置随机数种子。
     * time(NULL) 返回当前时间的秒数，每次运行都不一样，
     * 所以每局的地雷位置都不同。
     * 整个程序只需要调用一次 srand()，之后用 rand() 取随机数。
     */
    srand((unsigned int)time(NULL));

    printf("欢迎来到控制台扫雷游戏！\n");

    for (;;) {
        print_menu();

        if (!read_int("请选择难度（0-3）：", &choice)) {
            break;   /* 输入结束，跳出循环 */
        }

        switch (choice) {
        case 0:
            printf("感谢游玩，再见！\n");
            return 0;
        case 1:
            if (!play_round(9, 9, 10)) return 0;
            break;
        case 2:
            if (!play_round(16, 16, 40)) return 0;
            break;
        case 3:
            if (!play_round(16, 30, 99)) return 0;
            break;
        default:
            printf("没有这个选项，请输入 0、1、2 或 3。\n");
            break;
        }
    }

    printf("\n输入已结束，游戏退出。\n");
    return 0;
}
