#ifndef F_H_INCLUDED
#define F_H_INCLUDED

#include <stdio.h>

/* 指令编号（和你原来 printfile 里的 CMD 下标要对应） */
#define ADD   1
#define AND   2
#define NOT   3
#define LODI  4
#define STOI  5
#define LOD   6
#define STO   7
#define BR    8
#define JMP   9
#define PRT   10
#define RET   11

/* 一条指令 */
typedef struct {
    short cmd;
    short op1;
    short op2;
    short op3;
} ST;

/* 一个代码文件：指令区 + 内存区 */
struct file {
    ST *code;
    unsigned int code_capacity;
    unsigned char memory[256];
    unsigned short code_lenth;
    unsigned short memory_lenth;
};

/* 对外提供的函数声明 */

void get_dir(char *path, char **argv);

struct file *file_create(void);
void file_destroy(struct file *file);

struct file *read_file(char *path);
void printfile(struct file *file);
int  run(struct file *file);

int  read_code(const char *path, struct file *file);
int  save(struct file *file, const char *filename);

#endif // F_H_INCLUDED
