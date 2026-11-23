# 2026_11_23 C language project

C 语言练习工程（Visual Studio，`C language project.slnx`）。
读入 4 个整数，输出它们的**和**与**平均值**。

## 代码

`C language project/test.c`：

```c
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>   // 引入标准输入输出库：printf 就是这里提供的，没有它就不能打印

//int main()           // 主函数：整个程序从这里开始执行（一个程序只能有一个 main）
//{
//	int i;           // 整数变量 i，当计数器用：表示"现在算到第几个数"
//	double s = 0;    // 小数变量 s 存累加结果，初值 0。必须用 double，
//	                 // 因为 1/2 这类结果带小数，用 int 会丢掉小数部分
//
//	// for 循环 (初始值; 继续条件; 每轮结尾的动作)
//	// i 从 1 开始，只要 i < 4 就继续，每轮结束 i 加 1
//	// 所以 i 依次是 1、2、3，循环体一共执行 3 次
//	for (i = 1;i < 4;i++)
//	{
//		// 把 1/i 累加到 s 上；先算除法，再算加法
//		// 注意写 1.0 而不是 1：整数 1/2 结果是 0，写成 1.0/2 才是 0.5
//		s = s + 1.0 / i;
//
//		// 打印当前进度：%d 填入整数 i，%.3f 填入 s 并保留 3 位小数
//		// \n 是换行；引号后的 i、s 必须和前面的 %d、%.3f 按顺序一一对应
//		printf("i=%d#s=%.3f\n", i, s);
//	}
//
//	// 循环结束后 i 已经被加到 4（先 i++，再判断 4 < 4 不成立才退出）
//	// 这行打印最终结果：i=4#s=1.833
//	printf("i=%d#s=%.3f\n", i, s);
//
//	return 0;        // 返回 0 告诉操作系统：程序正常结束、没有出错
//}
int plus(int x, int y, int z, int j)
{
	int num;
	num = x + y + z + j;

	return num;
}
int AVE(int x, int y, int z, int j)
{
	int sum = 0;
	sum = (x + y + z + j) / 4;
	return sum;
}
int main()
{
	int a, b, c, d;
	double average;
	scanf("%d %d %d %d", &a, &b, &c, &d);
	plus(a,b,c,d);
	average = AVE(a, b, c, d);
	printf("%d\n", plus(a,b,c,d));
	printf("%.1f\n", average);
	return 0;

}
```

## 运行结果

输入 `1 2 3 4`，输出：

```
10
2.0
```

## 代码要点

| 函数 | 作用 |
|---|---|
| `plus` | 四个整数相加，返回 `int` |
| `AVE` | `(x + y + z + j) / 4` 求平均，返回 `int` |

- `AVE` 里除数两边都是整数，除法会**丢掉小数**：`10 / 4` 得 `2` 而不是 `2.5`，所以平均值打印成 `2.0`。想要 `2.5`，要写成 `/ 4.0` 并把返回类型改成 `double`。
- `scanf("%d %d %d %d", ...)` 格式串结尾**不要加 `\n`**，否则输完 4 个数它还会继续等下一个字符，程序看起来像卡住。
- 源文件保存为 **UTF-8 带 BOM**，避免 MSVC 在中文代码页下报 `C4819` 以及连带的语法错误。

## 编译

Visual Studio 打开 `C language project.slnx`，选 x64/Debug 按 F5；或命令行 `cl test.c`。