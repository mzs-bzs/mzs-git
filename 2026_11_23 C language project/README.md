# 2026_11_23 C language project

C 语言练习工程（Visual Studio，`C language project.slnx`）。
累加 `1.0 / i`（i = 1..3），每一步打印当前的和，循环结束后再打印一次。

## 代码

`C language project/test.c`：

```c
int main()
{
    int i;
    double s = 0;
    for (i = 1; i < 4; i++)
    {
        s = s + 1.0 / i;
        printf("i=%d#s=%.3f\n", i, s);
    }
    printf("i=%d#s=%.3f\n", i, s);
    return 0;
}
```

## 运行结果

```
i=1#s=1.000
i=2#s=1.500
i=3#s=1.833
i=4#s=1.833
```

## 编译

用 Visual Studio 打开 `C language project.slnx`，选择 x64/Debug，按 F5 运行；也可以用命令行：

```
cl test.c
```

`.gitignore` 只跟踪源码与工程文件，`.vs/`、`x64/`、`*.user` 等构建产物不入库。