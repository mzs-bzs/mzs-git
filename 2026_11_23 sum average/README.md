# 2026_11_23 sum average

读入 4 个整数，输出它们的和与平均值。

## 代码

见 [`test.c`](test.c)。文件里保留了之前练习时写的注释块（已注释掉，不参与编译），方便回看当时推导的过程。

真正生效的是文件末尾的 `main`：

```c
int main()
{
	int a, b, c, d;
	int sum = 0;
	double average;

	scanf("%d%d%d%d", &a, &b, &c, &d);   // 先读入

	sum = a + b + c + d;                 // 再求和
	average = sum / 4.0;                 // 再求平均：写成 4.0 才是小数除法

	printf("sum=%d\n", sum);
	printf("average = %.1f\n", average);
	return 0;
}
```

## 运行结果

输入 `1 2 3 4`：

```
sum=10
average = 2.5
```

## 要点

- **顺序**：先 `scanf` 读入，再算 `sum`，再算 `average`，最后 `printf`。算在输入之前只会拿未初始化的随机值。
- `average = sum / 4.0;`：除数写成 `4` 会做**整数除法**（`10 / 4` 得 `2`，平均值变 `2.0`）；写成 `4.0` 才是小数除法，得到 `2.5`。
- `double average;` 接收小数结果，`%.1f` 保留 1 位小数。
- 源文件保存为 **UTF-8 带 BOM**，否则 MSVC 在中文代码页下会报 `C4819` 并连带语法错误。

## 编译

`cl test.c`，运行 `test.exe`。