#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "f.h"

struct file *file_create(void) {
    struct file *f = (struct file*)calloc(1, sizeof(struct file));
    if (!f) return NULL;
    f->code = NULL;
    f->code_capacity = 0;
    f->code_lenth = 0;
    f->memory_lenth = 0;
    return f;
}

void file_destroy(struct file *f) {
    if (!f) return;
    free(f->code);
    f->code = NULL;
    f->code_capacity = 0;
    f->code_lenth = 0;
}


/* 确保 code 至少能容纳 need 条指令 */
static int ensure_code_capacity(struct file *f, unsigned int need) {
    if (!f) return -1;
    if (need <= f->code_capacity) return 0;

    unsigned int newcap = (f->code_capacity == 0) ? 64 : f->code_capacity;
    while (newcap < need) {
        unsigned int old = newcap;
        newcap *= 2;
        if (newcap < old) return -1;
    }

    ST *p = (ST*)realloc(f->code, newcap * sizeof(ST));
    if (!p) return -1;

    f->code = p;
    f->code_capacity = newcap;
    return 0;
}

/*
//现在的主程序可以打开示例文件并且将其显示出来
int main(int argc, char**argv){
	char path[100];
	char filename[11]="\\test.jian";
	get_dir(path,argv);
	strcat(path,filename);
	printf("文件位置：%s\n",path);

	printfile(read_file(path));

	return 10086;
}
*/

//read_file函数，用于读取一个二进制文件。
//path为文件路径。
struct file* read_file(char*path){
	FILE*fp=fopen(path,"rb");
	unsigned char code_lenth=0,memory_lenth=0;
	if(!fp){
		printf("未能找到文件：\n\t%s\n",path);
		return 0;
	}
    struct file* file = file_create();
    if (!file) { fclose(fp); printf("内存不足\n"); return NULL; }

    if (fread(&code_lenth, 1, 1, fp) != 1 ||
        fread(&memory_lenth, 1, 1, fp) != 1) {
        printf("二进制文件非法：文件头不足\n");
        fclose(fp);
        file_destroy(file);
        return NULL;
    }

    unsigned char *code = (unsigned char*)calloc(code_lenth,1);
    if (fread(code, 1, code_lenth, fp) != code_lenth) {
        printf("二进制文件非法：代码段长度不足（可能文件被截断）\n");
        free(code);
        fclose(fp);
        file_destroy(file);
        return NULL;
    }

    unsigned char *memory = (unsigned char*)calloc(memory_lenth,1);
    if (fread(memory, 1, memory_lenth, fp) != memory_lenth) {
        printf("二进制文件非法：数据段长度不足（可能文件被截断）\n");
        free(code);
        free(memory);
        fclose(fp);
        file_destroy(file);
        return NULL;
    }

    if (ensure_code_capacity(file, (unsigned int)code_lenth) != 0) {
        printf("内存不足：无法为二进制指令分配空间\n");
        fclose(fp);
        file_destroy(file);
        return NULL;
    }

	for(int i=0;i<code_lenth;i++){
		if(code[i]<0b01000000){
			file->code[i].cmd=ADD;
			file->code[i].op1=code[i]>>4&0b11;
			file->code[i].op2=code[i]>>2&0b11;
			file->code[i].op3=code[i]>>0&0b11;
		}else if(code[i]<0b10000000){
			file->code[i].cmd=AND;
			file->code[i].op1=code[i]>>4&0b11;
			file->code[i].op2=code[i]>>2&0b11;
			file->code[i].op3=code[i]>>0&0b11;
		}else if(code[i]<0b10100000){
			file->code[i].cmd=LODI;
			file->code[i].op1=code[i]>>3&0b11;
			file->code[i].op2=code[i]>>0&0b111;
			file->code[i].op3=0;
		}else if(code[i]<0b11000000){
			file->code[i].cmd=STOI;
			file->code[i].op1=code[i]>>3&0b11;
			file->code[i].op2=code[i]>>0&0b111;
			file->code[i].op3=0;
		}else if(code[i]<0b11010000){
			file->code[i].cmd=LOD;//原版多打了一个I，改了
			file->code[i].op1=code[i]>>2&0b11;
			file->code[i].op2=code[i]>>0&0b11;
			file->code[i].op3=0;
		}else if(code[i]<0b11100000){
			file->code[i].cmd=STO;//原版多打了一个I，改了
			file->code[i].op1=code[i]>>2&0b11;
			file->code[i].op2=code[i]>>0&0b11;
			file->code[i].op3=0;
		}else if(code[i]<0b11110000){
			file->code[i].cmd=BR;
			file->code[i].op1=code[i]>>2&0b11;
			file->code[i].op2=code[i]>>0&0b11;
			file->code[i].op3=0;
		}else if(code[i]<0b11110100){
			file->code[i].cmd=NOT;
			file->code[i].op1=code[i]&0b11;
			file->code[i].op2=0;
			file->code[i].op3=0;
		}else if(code[i]<0b11111000){
			file->code[i].cmd=JMP;
			file->code[i].op1=code[i]&0b11;
			file->code[i].op2=0;
			file->code[i].op3=0;
		}else if(code[i]<0b11111100){
			file->code[i].cmd=PRT;
			file->code[i].op1=code[i]&0b11;
			file->code[i].op2=0;
			file->code[i].op3=0;
		}else{
			file->code[i].cmd=RET;
			file->code[i].op1=code[i]&0b11;
			file->code[i].op2=0;
			file->code[i].op3=0;
		}
		/*说明： 0b代表二进制，而0代表八进制，0x代表十六进制。
 		 *>>和&都是位运算。>>指右移，&是位与。
		 *如：0b10101010右移动3位则可以得到0b00010101。
		 *如：0b10101010和0b11110000按位求与则可以得到0b10100000。
 		 *通过这两个操作，我们可以轻松提取出想要的若干位。
 		 */
	}
	free(code);	//用完内存即使释放是一个好习惯！

	int m = memory_lenth;
    if (m > 256)  m = 256;
    for (int i = 0; i < m; i++)  file->memory[i] = memory[i];
    file->memory_lenth = (unsigned short)m;

	free(memory);	//释放内存
    fclose(fp);

	file->code_lenth=(unsigned short int)code_lenth;

	return file;
}

//printfile函数，用于打印一个已经读取的代码文件。
//file指向一个已经代码文件。
void printfile(struct file*file){
	char CMD[12][5] =
	{
		"",	//因为我设定的指令里add编号从1开始，因此第0位置空
		"add", "and", "not", "lodi", "stoi","lod",
		"sto", "br", "jmp", "prt", "ret"
	};
	printf("文件内容：\n");
	printf("-----------以下是代码-----------\n");
	for(int i=0;i<file->code_lenth;i++){
		printf("%s\t",CMD[file->code[i].cmd]);
		printf("r%d\t",file->code[i].op1);
		if(file->code[i].cmd==ADD||file->code[i].cmd==AND||
		file->code[i].cmd==LOD||file->code[i].cmd==STO||
		file->code[i].cmd==BR)
			printf("r");
		if(file->code[i].cmd<=8&&file->code[i].cmd!=NOT)
			printf("%d\t",file->code[i].op2);
		if(file->code[i].cmd==ADD||file->code[i].cmd==AND)
			printf("r%d",file->code[i].op3);
		printf("\n");
	}
	printf("-----------以下是内存-----------\n");
	for(int i=0;i<file->memory_lenth;i++){
		printf("%d\t",(signed char)file->memory[i]);
		//我必须将它转化成有符号的，否则打印的结果可能会很奇怪（你可以试试看）
		if(i%4==3) printf("\n");
	}
	printf("\n");
}

//run函数，用于运行一个已经读取的代码文件。
//file指向一个已经代码文件。
/*
int run(struct file*file){
	char r[4]={0};	//即r0,r1,r2,r3
	char memory[256];
	signed char current = 0;
	for(int i=0;i<256;i++) memory[i]=file->memory[i];	//将内存拷贝一份（防止运行过程损坏原数据）
	for(int ip=0;ip>=0&&ip<=255;ip++){	//ip表示接下来需要运行哪一行代码
		switch(file->code[ip].cmd){
            case ADD:
                r[file->code[ip].op1]=r[file->code[ip].op2]+r[file->code[ip].op3];
                break;
            case AND:
                r[file->code[ip].op1]=r[file->code[ip].op2]&r[file->code[ip].op3];
                break;
            case LODI:
                r[file->code[ip].op1]=memory[3];
                break;
            case STOI:
                memory[7]=r[file->code[ip].op1];
                break;
            case LOD:
                current = (signed char)r[file->code[ip].op2]; // 正确语法
                r[file->code[ip].op1] = memory[(signed char)current];
                break;
            case STO:
                current=(signed char)r[file->code[ip].op2];
                memory[(signed char)current]=r[file->code[ip].op1];
                break;
			case BR:
				if((signed char)r[file->code[ip].op1]>=0)
					ip+=(signed char)r[file->code[ip].op2];
				break;
			case JMP:
				ip=(unsigned char)r[file->code[ip].op1]-1;
				//因为后面还会有ip++，所以这里需要-1。br则不需要（详见br的说明）。
				break;
			case PRT:
				printf("%6d",r[file->code[ip].op1]);
				break;
			case RET:
				return r[file->code[ip].op1];
            case NOT:                                          //原来老师给的程序里面没有写，补上了
                r[file->code[ip].op1] = ~r[file->code[ip].op1];  // 按位取反
                break;
		}
	}
	printf("运行时出现错误：ip指针越界。\n");
	return -130;
}

*/

//这里我对run函数进行了一些小小的修改，为了使我的程序上下流畅性更好
int run(struct file *file) {
    signed char r[4] = {0, 0, 0, 0};
    signed char memory[256] = {0};
    signed char current = 0;

    // 把文件里的内存拷贝一份（没用到的地方保持 0）
    for (int i = 0; i < file->memory_lenth && i < 256; i++) {
        memory[i] = (signed char)file->memory[i];
    }

    // ip = 指令指针，从 0 开始
    for (int ip = 0; ip >= 0 && ip < (int)file->code_lenth; ip++) {
        ST ins = file->code[ip];

        switch (ins.cmd) {
        case ADD:
            r[ins.op1] = (signed char)(r[ins.op2] + r[ins.op3]);
            break;

        case AND:
            r[ins.op1] = (signed char)(r[ins.op2] & r[ins.op3]);
            break;

        case LODI:{
            r[ins.op1] = (signed char)ins.op2;
            break;
        }

        case STOI:
            memory[(unsigned char)ins.op2] = r[ins.op1];
            break;

        case LOD:
            current = r[ins.op2];
            r[ins.op1] = memory[(unsigned char)current];
            break;

        case STO:
            current = r[ins.op2];
            memory[(unsigned char)current] = r[ins.op1];
            break;

        case BR:
            if (r[ins.op1] >= 0) {
                ip += (signed char)r[ins.op2];
            }
            break;

        case JMP:
            ip = (unsigned char)r[ins.op1] - 1;
            break;

        case PRT:
            printf("%6d", r[ins.op1]);
            break;

        case RET:
            return r[ins.op1];

        case NOT:
            r[ins.op1] = ~r[ins.op1];
            break;
        }
    }

    printf("运行时出现错误：ip指针越界。\n");
    return -130;
}



//问题3.实现将代码从文本文件读取
//我定义的文本格式就是分为三部分：第一部分——准备区
//                                                           第二部分——分割线“---”
//                                                           第三部分——代码区
//                                                           #表明了后面是注释

void trim(char *s) {               //去掉行尾换行的换行符，为了每一行使用strcmp函数判断以及分割线的读取
    int len=(int)strlen(s);
    while (len>0 && s[len-1]=='\n') {
        s[--len]= '\0';
    }
}

int parse_reg(const char *tok) {             //把r0、r1转化成数字
    if ((tok[0] == 'r' || tok[0] == 'R') && tok[1] >= '0' && tok[1] <= '9') {
        return tok[1] - '0';
    }
    return atoi(tok);
}

int parse_reg_checked(const char *tok,int* output){        //这里完成了拓展2，通过检验二进制下是否在0~3之间检验是否合法
    int r=parse_reg(tok);
    if (r<0 || r>3) return -1;
    *output=r;
    return 0;
}

int parse_code_line(const char *line, ST *out) {            //主函数
    char buf[128];
    strcpy(buf, line);

    char *p=strchr(buf,'#');            //这里完成了拓展1，结尾为#表明了后面是注释
    if (p) *p = '\0';

    trim(buf);

    if (buf[0] == '\0') {
        return 0;   // 约定：0 表示这行是空
    }

    char op[16], a[16], b[16], c[16];
    op[0] = a[0] = b[0] = c[0] = '\0';

    int n = sscanf(buf, "%10s %10s %10s %10s", op, a, b, c);      // 把一行拆成：指令名 + 3 个参数

    if (strcmp(op, "add") == 0) {
        int r1,r2,r3;
        if (parse_reg_checked(a, &r1) < 0 || parse_reg_checked(b, &r2) < 0 || parse_reg_checked(c, &r3) < 0) {
            printf("寄存器编号非法：%s\n", line);
            return -1;
        }
        out->cmd = ADD;
        out->op1 = r1;
        out->op2 = r2;
        out->op3 = r3;
    } else if (strcmp(op, "and") == 0) {
        int r1,r2,r3;
        if (parse_reg_checked(a, &r1) < 0 || parse_reg_checked(b, &r2) < 0 || parse_reg_checked(c, &r3) < 0) {
            printf("寄存器编号非法：%s\n", line);
            return -1;
        }
        out->cmd = AND;
        out->op1 = r1;
        out->op2 = r2;
        out->op3 = r3;
    } else if (strcmp(op, "not") == 0) {
        int r1;
        if (parse_reg_checked(a, &r1) < 0) {
            printf("寄存器编号非法：%s\n", line);
            return -1;
        }
        out->cmd = NOT;
        out->op1 = r1;
        out->op2 = 0;
        out->op3 = 0;
    } else if (strcmp(op, "lodi") == 0) {
        int r1;
        if (parse_reg_checked(a, &r1) < 0) {
            printf("寄存器编号非法：%s\n", line);
            return -1;
        }
        int n = atoi(b);
        if (n < 0 || n > 7) {
            printf("lodi 指令中立即数超出范围(0~7)：%s\n", line);
            return -1;
        }
        out->cmd = LODI;
        out->op1 = (short)r1;
        out->op2 = (short)n;
        out->op3 = 0;
    } else if (strcmp(op, "stoi") == 0) {
        int r1;
        if (parse_reg_checked(a, &r1) < 0) {
            printf("寄存器编号非法：%s\n", line);
            return -1;
        }
        int n = atoi(b);
        if (n < 0 || n > 7) {
            printf("stoi 指令中立即数超出范围(0~7)：%s\n", line);
            return -1;
        }
        out->cmd = STOI;
        out->op1 = (short)r1;
        out->op2 = (short)n;
        out->op3 = 0;
    } else if (strcmp(op, "lod") == 0) {
        int r1,r2;
        if (parse_reg_checked(a, &r1) < 0 || parse_reg_checked(b, &r2) < 0) {
            printf("寄存器编号非法：%s\n", line);
            return -1;
        }
        out->cmd = LOD;
        out->op1 = r1;
        out->op2 = r2;
        out->op3 = 0;
    } else if (strcmp(op, "sto") == 0) {
        int r1,r2;
        if (parse_reg_checked(a, &r1) < 0 || parse_reg_checked(b, &r2) < 0) {
            printf("寄存器编号非法：%s\n", line);
            return -1;
        }
        out->cmd = STO;
        out->op1 = r1;
        out->op2 = r2;
        out->op3 = 0;
    } else if (strcmp(op, "br") == 0) {
        int r1,r2;
        if (parse_reg_checked(a, &r1) < 0 || parse_reg_checked(b, &r2) < 0) {
            printf("寄存器编号非法：%s\n", line);
            return -1;
        }
        out->cmd = BR;
        out->op1 = r1;
        out->op2 = r2;
        out->op3 = 0;
    } else if (strcmp(op, "jmp") == 0) {
        int r1;
        if (parse_reg_checked(a, &r1) < 0) {
            printf("寄存器编号非法：%s\n", line);
            return -1;
        }
        out->cmd = JMP;
        out->op1 = r1;
        out->op2 = 0;
        out->op3 = 0;
    } else if (strcmp(op, "prt") == 0) {
        int r1;
        if (parse_reg_checked(a, &r1) < 0) {
            printf("寄存器编号非法：%s\n", line);
            return -1;
        }
        out->cmd = PRT;
        out->op1 = r1;
        out->op2 = 0;
        out->op3 = 0;
    } else if (strcmp(op, "ret") == 0) {
        int r1;

        if (n >= 2) {               //如果说输入的参数个数>=2，直接读op1，否则默认r3
            if (parse_reg_checked(a, &r1) < 0) {
                printf("ret 指令中寄存器非法：%s\n", line);
                return -1;        // 行解析失败
            }
        } else {
            r1 = 3;
        }

        out->cmd = RET;
        out->op1 = (short)r1;
        out->op2 = 0;
        out->op3 = 0;
    } else {
        return -1;
    }
    return 1;
}

int read_code(const char *path, struct file *file) {          //对文本进行操作
    FILE *fp = fopen(path, "r");
    if (!fp) {
        printf("无法打开源文件：%s\n", path);
        return -1;
    }

    // 清空旧内容
    file->code_lenth = 0;
    file->memory_lenth = 0;

    char line[256];
    int in_code_section = 0;  // 0 = 准备区，1 = 代码区
    int ok=1;                  //判断整体是否合法

    while (fgets(line, sizeof(line), fp)) {           //对每一行进行读取，只要不是空行
        if (strncmp(line, "---", 3) == 0) {           // 遇到 --- 说明进入代码区
            in_code_section = 1;
            continue;
        }
        if (!in_code_section) {
            continue;
        }

        ST ins;
        int r=parse_code_line(line, &ins);        //把文本里的这一行指令解析为机器语言
        if (r==1) {
            if (ensure_code_capacity(file, (unsigned int)file->code_lenth + 1) != 0) {
                printf("内存不足：无法继续扩容指令数组\n");
                ok=0;
                break;
            }
            file->code[file->code_lenth++] = ins;
        } else if (r == -1) {
            printf("无法识别的指令行：%s", line);
            ok=0;
        }
    }
    fclose(fp);

    if (!in_code_section || file->code_lenth == 0) {
        printf("没有找到 '---' 或没有任何指令，程序不合法\n");
        ok = 0;
    }

    return ok ? 0 : -1;
}




//问题4

typedef unsigned char uint8;

uint8 encode_add(short op1, short op2, short op3) {
    uint8 code=0;
    code |=(op1 & 0b11)<<4;
    code |=(op2 & 0b11)<<2;
    code |=(op3 & 0b11)<<0;
    return code;
}

uint8 encode_and(short op1, short op2, short op3) {
    uint8 code=0;
    code |=(op1 & 0b11)<<4;
    code |=(op2 & 0b11)<<2;
    code |=(op3 & 0b11)<<0;
    code |=0b01000000;
    return code;
}

uint8 encode_not(short op1) {
    uint8 code=0;
    code |=(op1 & 0b11);
    code |=0b11110000;
    return code;
}

uint8 encode_lodi(short op1,int n) {
    uint8 code=0;
    code |=(n & 0b111);
    code |=(op1 & 0b11)<<3;
    code |=0b10000000;
    return code;
}

uint8 encode_stoi(short op1,int n) {
    uint8 code=0;
    code |=(n & 0b111);
    code |=(op1 & 0b11)<<3;
    code |=0b10100000;
    return code;
}

uint8 encode_lod(short op1,short op2) {
    uint8 code=0;
    code |=(op1 & 0b11)<<2;
    code |=(op2 & 0b11);
    code |=0b11000000;
    return code;
}

uint8 encode_sto(short op1,short op2) {
    uint8 code=0;
    code |=(op1 & 0b11)<<2;
    code |=(op2 & 0b11);
    code |=0b11010000;
    return code;
}

uint8 encode_br(short op1,short op2) {
    uint8 code=0;
    code |=(op1 & 0b11)<<2;
    code |=(op2 & 0b11);
    code |=0b11100000;
    return code;
}

uint8 encode_jmp(short op1) {
    uint8 code=0;
    code |=(op1 & 0b11);
    code |=0b11110100;
    return code;
}

uint8 encode_prt(short op1) {
    uint8 code=0;
    code |=(op1 & 0b11);
    code |=0b11111000;
    return code;
}

uint8 encode_ret(short op1) {
    uint8 code=0;
    code |=(op1 & 0b11);
    code |=0b11111100;
    return code;
}

int save(struct file*file,const char* filename){    //主函数
    if (file==NULL || filename==NULL){
        printf("错误\n");
        return -1;
    }

    if (file->code_lenth > 255) {
        printf("save失败：原.jian格式仅支持<=255条指令（当前=%u）\n", (unsigned)file->code_lenth);
        return -1;
    }

    uint8 code_bytes[256];
    int  code_cnt=0;

    for (int i=0; i<file->code_lenth && code_cnt<256; i++) {
        ST *ins = &file->code[i];

        switch (ins->cmd) {
            case ADD:
                code_bytes[code_cnt++] = encode_add(ins->op1, ins->op2, ins->op3);
                break;
            case AND:
                code_bytes[code_cnt++] = encode_and(ins->op1, ins->op2, ins->op3);
                break;
            case NOT:
                code_bytes[code_cnt++] = encode_not(ins->op1);
                break;
            case LODI:
                code_bytes[code_cnt++] = encode_lodi(ins->op1, ins->op2);
                break;
            case STOI:
                code_bytes[code_cnt++] = encode_stoi(ins->op1, ins->op2);
                break;
            case LOD:
                code_bytes[code_cnt++] = encode_lod(ins->op1, ins->op2);
                break;
            case STO:
                code_bytes[code_cnt++] = encode_sto(ins->op1, ins->op2);
                break;
            case BR:
                code_bytes[code_cnt++] = encode_br(ins->op1, ins->op2);
                break;
            case JMP:
                code_bytes[code_cnt++] = encode_jmp(ins->op1);
                break;
            case PRT:
                code_bytes[code_cnt++] = encode_prt(ins->op1);
                break;
            case RET:
                code_bytes[code_cnt++] = encode_ret(ins->op1);
                break;
            default:
                printf("第 %d 条指令 cmd=%d 有误\n", i , ins->cmd);
                break;
        }
    }

    uint8 code_len = (uint8)code_cnt;
    uint8 data_len = (uint8)file->memory_lenth;

    FILE *fp=fopen(filename,"wb");
    if(!fp){
        printf("无法创建文本\n");
        return -1;
    }

    // 1. 写头两个字节
    fwrite(&code_len, 1, 1, fp);
    fwrite(&data_len, 1, 1, fp);

    // 2. 写代码段
    fwrite(code_bytes, 1, code_len, fp);

    // 3. 写数据段
    fwrite(file->memory, 1, data_len, fp);

    fclose(fp);
    return 0;
}


/*
 *	你的任务：
 *	1.将二进制代码读取的任务已经完成了，请自行阅读并尝试理解。（如有必要，你可以自行修改）完成
 *	2.利用C语言模拟代码的运行。（已经完成了一半，请补全代码）                                                    完成
 *	3.实现将代码从文本文件读取（你可以自己制定一条规则存放程序所需要用的参数）。                  完成
 *	4.实现将代码保存为二进制文件。                                                                                                      完成
 *	5.创建一个菜单，使得上述功能可以方便地实现并完成。
 */

