#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "f.h"

int main(int argc, char**argv) {
    char dir[100];
    char path_bin[100];
    char path_txt[100];
    char path_out[100];

    struct file prog;     // 用来存文本解析出来的代码
    struct file *f = NULL; // 用来存 read_file 出来的指针

    get_dir(dir, argv);
    printf("程序所在目录：%s\n", dir);

    int choice;

    while (1) {
        printf("\n===== 菜单 =====\n");
        printf("1. 从 test.jian 读取并打印\n");
        printf("2. 从 test.jian 读取并运行\n");
        printf("3. 从 source.txt 读取代码并打印\n");
        printf("4. 从 source.txt 读取代码并生成 test.jian\n");
        printf("5. 从 source.txt 读取代码并直接运行\n");
        printf("请输入选项：");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {                               // 1. 从二进制文件 test.jian 读取并打印
                strcpy(path_bin, dir);
                strcat(path_bin, "\\test.jian");

                printf("二进制文件：%s\n", path_bin);
                f = read_file(path_bin);
                if (f) {
                    printfile(f);
                    free(f);
                }
                break;
            }
            case 2:{                                // 2. 从二进制文件 test.jian 读取并运行
                strcpy(path_bin, dir);
                strcat(path_bin, "\\test.jian");

                printf("二进制文件：%s\n", path_bin);
                f = read_file(path_bin);
                if (f) {
                    int ret = run(f);
                    printf("\n程序运行完毕，返回值 = %d\n", ret);
                    free(f);
                }
                break;
            }
            case 3:{                               // 3. 从文本文件 source.txt 读取代码并打印
                strcpy(path_txt, dir);
                strcat(path_txt, "\\source.txt");

                printf("源代码文件：%s\n", path_txt);
                if (read_code(path_txt, &prog) == 0) {
                    printfile(&prog);
                } else {
                    printf("读取源代码失败。\n");
                }
                break;
            }
            case 4:{
                strcpy(path_txt, dir);
                strcat(path_txt, "\\source.txt");

                strcpy(path_out, dir);
                strcat(path_out, "\\test.jian");

                printf("源代码文件：%s\n", path_txt);
                printf("输出二进制：%s\n", path_out);

                if (read_code(path_txt, &prog) == 0) {
                    if (save(&prog, path_out) == 0) {
                        printf("保存成功！\n");
                    } else {
                        printf("保存失败。\n");
                    }
                } else {
                    printf("读取源代码失败，无法保存。\n");
                }
                break;
            }
            case 5:{
                strcpy(path_txt, dir);
                strcat(path_txt, "\\source.txt");

                printf("源代码文件：%s\n", path_txt);

                if (read_code(path_txt, &prog) == 0) {
                    int ret = run(&prog);
                    printf("\n程序运行结束，返回值 = %d\n", ret);
                } else {
                    printf("读取源代码失败，无法运行。\n");
                }
                break;
            default:
                printf("没有这个选项，请重新输入。\n");
                break;
            }
        }
    }
}
