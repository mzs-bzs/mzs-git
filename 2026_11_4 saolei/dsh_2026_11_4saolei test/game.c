#include "game.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * 同 test.c：让 MSVC 把中文字符串按 UTF-8 保存，
 * 保证程序输出和控制台的 UTF-8 编码一致，避免乱码。
 * 项目里的 /utf-8 选项是第一道保险，这里是第二道。
 */
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#ifdef _WIN32
#include <windows.h>
#endif

/*
 * ============================================================================
 *  扫雷游戏（控制台版）—— game.c
 * ============================================================================
 *
 *  【这个文件负责什么？】
 *  游戏的所有核心逻辑：
 *      · 初始化棋盘
 *      · 随机布雷（带首击保护）
 *      · 翻开格子（空白区域自动扩散）
 *      · 插旗 / 取消旗
 *      · 判断胜利和失败
 *      · 把棋盘打印到屏幕上
 *
 *  【两个棋盘】
 *      mine[][] —— 答案：记录哪里有雷，玩家看不到
 *      show[][] —— 卷面：记录玩家看到的内容，打印时只看它
 *
 *  【坐标约定】
 *  函数内部的 row、col 都从 0 开始数；
 *  玩家输入时从 1 开始数，test.c 会帮玩家减 1 再传进来。
 * ============================================================================
 */

/* ==========================================================================
 * 一、控制台颜色
 * ==========================================================================
 * Windows 控制台用一个整数表示颜色：
 *      1 = 蓝，2 = 绿，4 = 红，8 = 高亮（更亮）
 * 把需要的数字加起来就是组合色，例如：
 *      9  = 1 + 8  亮蓝色
 *      12 = 4 + 8  亮红色
 *      14 = 2 + 4 + 8  黄色
 * 数字颜色只是为了让棋盘更直观，不影响游戏规则。
 * ========================================================================== */

#define COLOR_DEFAULT       7    /* 默认的灰白色 */
#define COLOR_GRAY          8    /* 灰色（未翻开的格子） */
#define COLOR_LIGHT_BLUE    9    /* 亮蓝（数字 1） */
#define COLOR_LIGHT_GREEN  10    /* 亮绿（数字 2） */
#define COLOR_LIGHT_CYAN   11    /* 亮青（数字 6） */
#define COLOR_LIGHT_RED    12    /* 亮红（数字 3、地雷） */
#define COLOR_LIGHT_MAGENTA 13   /* 亮紫（数字 4） */
#define COLOR_YELLOW       14    /* 黄色（小旗、数字 5） */
#define COLOR_WHITE        15    /* 白色（数字 7） */

#ifdef _WIN32
/* Windows 下真正修改文字颜色 */
static void set_color(int color)
{
    HANDLE screen = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(screen, (WORD)color);
}
#else
/* 其他系统不做任何事，只是占位，保证代码能编译通过 */
static void set_color(int color)
{
    (void)color;   /* 告诉编译器“这个参数我故意不用”，避免警告 */
}
#endif

/*
 * 根据格子里的字符，决定它用什么颜色显示。
 * 数字 1~8 的颜色参考经典扫雷的配色：数字越大，颜色越醒目。
 */
static int cell_color(char ch)
{
    switch (ch) {
    case CELL_HIDDEN: return COLOR_GRAY;
    case CELL_FLAG:   return COLOR_YELLOW;
    case CELL_MINE:   return COLOR_LIGHT_RED;
    case CELL_WRONG:  return COLOR_LIGHT_RED;
    case CELL_EMPTY:  return COLOR_GRAY;
    case '1':         return COLOR_LIGHT_BLUE;
    case '2':         return COLOR_LIGHT_GREEN;
    case '3':         return COLOR_LIGHT_RED;
    case '4':         return COLOR_LIGHT_MAGENTA;
    case '5':         return COLOR_YELLOW;
    case '6':         return COLOR_LIGHT_CYAN;
    case '7':         return COLOR_WHITE;
    case '8':         return COLOR_GRAY;
    default:          return COLOR_DEFAULT;
    }
}

/* ==========================================================================
 * 二、初始化棋盘
 * ========================================================================== */

void game_init(Game *game, int rows, int cols, int mines)
{
    int r, c;

    /*
     * 1. 参数保护
     * 即使调用者传入了奇怪的数字，也不能让数组越界。
     *    - 最小 5×5：太小的棋盘放不下首击保护需要的安全区；
     *    - 最大不超过 MAX_ROWS / MAX_COLS。
     */
    if (rows < 5) rows = 5;
    if (cols < 5) cols = 5;
    if (rows > MAX_ROWS) rows = MAX_ROWS;
    if (cols > MAX_COLS) cols = MAX_COLS;

    /*
     * 2. 雷数保护
     * 第一次翻开的格子以及它周围 8 格都不能有雷（共 9 格），
     * 所以雷最多只能有“总格数 - 9”颗，否则根本没法安全布雷。
     */
    if (mines > rows * cols - 9) mines = rows * cols - 9;
    if (mines < 1) mines = 1;

    /* 3. 保存这一局的设置 */
    game->rows = rows;
    game->cols = cols;
    game->mine_count = mines;
    game->opened_count = 0;
    game->flag_count = 0;
    game->first_dig = 1;            /* 还没翻过格子，下一手要触发首击保护 */
    game->state = STATE_PLAYING;

    /*
     * 4. 清空两个棋盘
     * mine 全部设为 '0'（没有雷），show 全部设为 '#'（未翻开）。
     * 注意这里只清洗实际使用的区域，没有用到的格子不会参与游戏。
     */
    for (r = 0; r < rows; ++r) {
        for (c = 0; c < cols; ++c) {
            game->mine[r][c] = '0';
            game->show[r][c] = CELL_HIDDEN;
        }
    }
}

/* ==========================================================================
 * 三、随机布雷（只在第一次翻开格子时调用）
 * ========================================================================== */

/*
 * 布雷，同时保证第一次点击的位置和它周围 8 格是安全的。
 *
 * 所用的“Fisher-Yates 洗牌”思路：
 *   1. 把所有允许放雷的格子编号，收进 candidates 数组；
 *   2. 从前往后，每次随机选一个还没被选过的位置，换到当前最前面；
 *   3. 洗牌结束后，取前 mine_count 个位置放雷。
 *
 * 这样每个候选位置成为地雷的概率完全相等，雷的分布是均匀的，
 * 不会出现“雷总爱挤在左上角”的问题。
 */
static void place_mines(Game *game, int safe_row, int safe_col)
{
    int candidates[MAX_ROWS * MAX_COLS];  /* 存放所有候选位置的编号 */
    int count = 0;                        /* 候选位置的数量 */
    int r, c, i;

    /* 第 1 步：收集候选位置，排除首击安全区（3×3 的范围） */
    for (r = 0; r < game->rows; ++r) {
        for (c = 0; c < game->cols; ++c) {
            int dr = r - safe_row;   /* 行方向的距离 */
            int dc = c - safe_col;   /* 列方向的距离 */
            int in_safe_zone = (dr >= -1 && dr <= 1 && dc >= -1 && dc <= 1);

            if (!in_safe_zone) {
                /* 二维坐标 (r, c) 压缩成一个整数，方便存进一维数组 */
                candidates[count] = r * game->cols + c;
                count++;
            }
        }
    }

    /* 第 2 步：只洗前 mine_count 个位置，洗到谁就让谁当雷 */
    for (i = 0; i < game->mine_count; ++i) {
        /*
         * rand() % (count - i) 得到 0 ~ (count-i-1) 的随机数，
         * 加上 i 之后，就是从“还没洗过”的区间里随机挑一个下标。
         */
        int j = i + rand() % (count - i);
        int tmp = candidates[i];

        candidates[i] = candidates[j];
        candidates[j] = tmp;

        /* 第 3 步：把选中的位置在 mine 棋盘上标记为雷 */
        {
            int pos = candidates[i];
            game->mine[pos / game->cols][pos % game->cols] = '1';
        }
    }
}

/* ==========================================================================
 * 四、翻开格子相关的功能
 * ========================================================================== */

/*
 * 数一数 (row, col) 周围 8 个格子中一共有几颗雷。
 * 用 dr、dc 表示“行偏移”和“列偏移”，遍历 -1、0、1 的组合：
 * 正好覆盖左上、上、右上、左、右、左下、下、右下 8 个方向。
 */
static int count_mines_around(const Game *game, int row, int col)
{
    int dr, dc;
    int count = 0;

    for (dr = -1; dr <= 1; ++dr) {
        for (dc = -1; dc <= 1; ++dc) {
            int r, c;

            if (dr == 0 && dc == 0) continue;  /* 跳过自己 */

            r = row + dr;
            c = col + dc;

            /* 先确认没有跑出棋盘，再检查是不是雷 */
            if (r >= 0 && r < game->rows && c >= 0 && c < game->cols
                && game->mine[r][c] == '1') {
                count++;
            }
        }
    }
    return count;
}

/*
 * 递归翻开一片区域。
 *
 * “递归”就是函数自己调用自己。递归最怕忘记停止条件，所以这里写清楚：
 *   · 位置超出棋盘        → return
 *   · 格子已经翻开或插旗  → return
 *   · 格子周围有雷        → 只翻开它，不再继续扩散
 *
 * 如果格子周围没有雷（数量为 0），就把它显示成 '.'，
 * 再对周围 8 个格子分别调用自己，像水波一样自动向外扩散。
 * 这就是扫雷里“点一下开一大片”的效果。
 */
static void open_area(Game *game, int row, int col)
{
    int n, dr, dc;

    if (row < 0 || row >= game->rows || col < 0 || col >= game->cols) return;
    if (game->show[row][col] != CELL_HIDDEN) return;   /* 已翻开或已插旗 */

    n = count_mines_around(game, row, col);

    if (n == 0) {
        game->show[row][col] = CELL_EMPTY;
    } else {
        /* 把数字 n 转成字符，例如 3 → '3' */
        game->show[row][col] = (char)('0' + n);
    }
    game->opened_count++;

    if (n != 0) return;   /* 有雷的邻居，不再向外扩散 */

    /* 周围 8 格都尝试翻开（递归） */
    for (dr = -1; dr <= 1; ++dr) {
        for (dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            open_area(game, row + dr, col + dc);
        }
    }
}

int game_dig(Game *game, int row, int col)
{
    /* 1. 先做各种合法性检查，任何一项不通过都不能翻 */
    if (game->state != STATE_PLAYING) return DIG_INVALID;
    if (row < 0 || row >= game->rows || col < 0 || col >= game->cols) {
        return DIG_INVALID;
    }
    if (game->show[row][col] != CELL_HIDDEN) return DIG_INVALID;

    /*
     * 2. 如果是本局第一次翻开，现在才真正布雷。
     * 把玩家点的位置作为安全区中心，所以第一下一定不会踩雷。
     */
    if (game->first_dig) {
        place_mines(game, row, col);
        game->first_dig = 0;
    }

    /* 3. 踩到地雷：把这颗雷显示出来，游戏结束 */
    if (game->mine[row][col] == '1') {
        game->show[row][col] = CELL_MINE;
        game->state = STATE_LOST;
        return DIG_EXPLODE;
    }

    /* 4. 安全：翻开这个格子，必要时自动扩散 */
    open_area(game, row, col);

    /*
     * 5. 判断胜利
     * 除了地雷以外的格子总数 = rows * cols - mine_count。
     * 如果已经翻开的数量等于它，说明所有安全格子都翻完了，玩家获胜。
     */
    if (game->opened_count == game->rows * game->cols - game->mine_count) {
        game->state = STATE_WON;
    }

    return DIG_OK;
}

int game_flag(Game *game, int row, int col)
{
    if (game->state != STATE_PLAYING) return -1;
    if (row < 0 || row >= game->rows || col < 0 || col >= game->cols) {
        return -1;
    }

    if (game->show[row][col] == CELL_HIDDEN) {
        /* 未翻开 → 插旗 */
        game->show[row][col] = CELL_FLAG;
        game->flag_count++;
        return 1;
    }
    if (game->show[row][col] == CELL_FLAG) {
        /* 已插旗 → 取消旗 */
        game->show[row][col] = CELL_HIDDEN;
        game->flag_count--;
        return 1;
    }

    /* 已经翻开的格子（数字或 '.'）不能插旗 */
    return 0;
}

/* ==========================================================================
 * 五、结束与显示
 * ========================================================================== */

void game_reveal_all(Game *game)
{
    int r, c;

    for (r = 0; r < game->rows; ++r) {
        for (c = 0; c < game->cols; ++c) {
            if (game->show[r][c] == CELL_FLAG) {
                /*
                 * 已经插旗的格子：
                 *   如果下面真有雷，保留 'F'（说明插对了）；
                 *   如果下面没雷，标成 'X'（说明插错了）。
                 */
                if (game->mine[r][c] != '1') {
                    game->show[r][c] = CELL_WRONG;
                }
            } else if (game->mine[r][c] == '1') {
                /* 没插旗的地雷，全部显示出来 */
                game->show[r][c] = CELL_MINE;
            }
        }
    }
}

void game_print(const Game *game)
{
    int r, c;

    /*
     * 打印列号。
     * 下面的行号格式是 " %2d |"，一共占 5 个字符；
     * 所以这里先打印 5 个空格，让列号和棋盘格子对齐。
     */
    printf("\n     ");
    for (c = 0; c < game->cols; ++c) {
        printf("%2d", c + 1);
    }
    printf("\n");

    for (r = 0; r < game->rows; ++r) {
        /* 行号：例如 "  1 |"、" 10 |" */
        printf(" %2d |", r + 1);

        for (c = 0; c < game->cols; ++c) {
            char ch = game->show[r][c];

            set_color(cell_color(ch));   /* 先换颜色 */
            printf(" %c", ch);           /* 再打印这个格子 */
        }

        set_color(COLOR_DEFAULT);        /* 一行结束，恢复默认颜色 */
        printf("\n");
    }

    /*
     * 剩余地雷 = 地雷总数 - 已插旗数，这是一个“估计值”：
     * 如果玩家插旗插多了，这里可能显示负数，属于正常现象。
     * 已翻开数量的分母是“所有非雷格子”，翻满就赢了。
     */
    printf("\n剩余地雷：%d    已翻开：%d/%d\n",
           game->mine_count - game->flag_count,
           game->opened_count,
           game->rows * game->cols - game->mine_count);
}
