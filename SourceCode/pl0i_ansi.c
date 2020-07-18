/*
* MIT License
* 
* Copyright (c) 2020 leaving-voider
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include "pl0.h"
#include "string.h"

#define stacksize 500
#define readlength 120

int get_the_second(char* str); 
int get_the_third(char* str);
float get_the_fourth(char* str);
void get_the_fifth(char* str, char* fifth);

int main(int argc,char** argv)
{
    // 定义用于for循环遍历
    int iter;
    
    // 打开存放中间代码的文件
    FILE* pFile = fopen("virCode.tmp", "r");
    if(!pFile)
        return;
    // 定义用于存放文件字符串的数组
    char str[readlength] = { 0 };
    // 给存放虚拟机代码的code数组赋初值为nan, nan是我自定义的虚拟机代码, 代表不是任何可执行的代码
    for(iter=0 ; iter<cxmax; iter++){
        code[iter].f = nan;
    }
    // num记录虚拟机代码在code中的index
    int num = 0;
    char* a;
    while ( a = fgets(str, readlength, pFile) )
    {
        // 获取虚拟机代码的操作码
        char op[4] = "000";
        op[3] = 0;
        for(iter=0 ; iter<3 ; iter++){
            op[iter] = str[iter];
        }
        // 判断是什么类型的虚拟机指令，并保存
        if(strcmp(op,"lit")==0){
            code[num].f = lit;
        }else if(strcmp(op,"opr")==0){
            code[num].f = opr;
        }else if(strcmp(op,"lod")==0){
            code[num].f = lod;
        }else if(strcmp(op,"sto")==0){
            code[num].f = sto;
        }else if(strcmp(op,"cal")==0){
            code[num].f = cal;
        }else if(strcmp(op,"int")==0){
            code[num].f = inte;
        }else if(strcmp(op,"jmp")==0){
            code[num].f = jmp;
        }else if(strcmp(op,"jpc")==0){
            code[num].f = jpc;
        }else if(strcmp(op,"lda")==0){
            code[num].f = lda;
        }else if(strcmp(op,"sta")==0){
            code[num].f = sta;
        }else if(strcmp(op,"lpl")==0){
            code[num].f = lpl;
        }else if(strcmp(op,"lmi")==0){
            code[num].f = lmi;
        }else if(strcmp(op,"end")==0){
            code[num].f = end;
        }else if(strcmp(op,"mov")==0){
            code[num].f = mov;
        }else{
            printf("nan\n");
        }

        // 获取第二、三个操作数
        int second = 0;
        int third = 0;
        float fourth = 0;
        char fifth[strmax] = {0};
        // 读取到code中
        second = get_the_second(str);
        third = get_the_third(str);
        code[num].l = second;
        code[num].a = third;

        // 只有opr指令且其A的值为17和19时，才会涉及到第3个参数是以字符串的形式读取
        if(code[num].f==opr && (code[num].a==17 || code[num].a==19)){
            get_the_fifth(str, code[num].str);
        }else{
            fourth = get_the_fourth(str);
            // 记录到code数组
            code[num].real = fourth;
        }
        num+=1;
    }
    // 关闭文件
    fclose(pFile);

    /*
    int length = sizeof(code)/sizeof(code[0]);
    printf("\nlength: %d", length);
    printf("\n\nCode:\n");
    for(iter=0 ; iter<length ; iter++){
        switch(code[iter].f){
            case 0:
                printf("lit %d %d\n", code[iter].l, code[iter].a);
                break;
            case 1:
                printf("opr %d %d\n", code[iter].l, code[iter].a);
                break;
            case 2:
                printf("lod %d %d\n", code[iter].l, code[iter].a);
                break;
            case 3:
                printf("sto %d %d\n", code[iter].l, code[iter].a);
                break;
            case 4:
                printf("cal %d %d\n", code[iter].l, code[iter].a);
                break;
            case 5:
                printf("inte %d %d\n", code[iter].l, code[iter].a);
                break;
            case 6:
                printf("jmp %d %d\n", code[iter].l, code[iter].a);
                break;
            case 7:
                printf("jpc %d %d\n", code[iter].l, code[iter].a);
                break;
            default:
                printf("wrong\n");
                break;
        }
    }*/

	interpret();
    return 0;
}

// 用于获取第1个操作数的函数
int get_the_second(char* str){
	int ins;
	int second = 0;
		for(ins=4 ; ins<readlength ; ins++){
            // 32表示空格
			if((int)str[ins]!=32){
				second = second*10 + (int)(str[ins])-48;
			}else{
				break;
			}
		}
	return second;
}

// 用于获取第2个操作数的函数
int get_the_third(char* str){
	int ins;
	int third = 0;
		for(ins=6 ; ins<readlength ; ins++){
            // 32表示空格
			if((int)str[ins]!=32){
				third = third*10 + (int)(str[ins])-48;
			}else{
				break;
			}
		}
	return third;
}

// 以浮点数的形式读取第3个参数
float get_the_fourth(char* str){
    int ins;
    float fourth = 0;
    int flag = 0;	// flag=1指示目前读到了小数点之后 
    float afterDot = 0.1;	// 用于加上小数点后的数 
        for(ins=9 ; ins<readlength ; ins++){
            // 10表示行尾
            if((int)str[ins]!=10){
                if((int)str[ins]!=46){
                	// 首先判断是否已经是小数位 
                	if(flag==0){
                		// 还在整数位 
                		fourth = fourth*10 + (int)(str[ins])-48;
					}else{
						// 已经是小数位 
						fourth = fourth + ((int)(str[ins])-48)*afterDot;
						afterDot = afterDot * 0.1;
					}
				}else{
					flag = 1;
				}
                
            }else{
                break;
            }
        }
    return fourth;
}

// 以字符串的形式读取第3个参数
void get_the_fifth(char* str, char* fifth){
    int ins;
    for(ins=0 ; ins<strmax ; ins++){
        fifth[ins] = 0;
    }
    int index = 0;
    for(ins=9 ; ins<readlength ; ins++){
        // 10表示行尾
        if((int)str[ins]!=10){
            fifth[index] = str[ins];
            index++;
        }else{
            break;
        }
    }
}

// read格式化输入处理函数
int FormatRead(struct instruction* ins){
    int num = 0,iter;
    // 遍历字符串str
    for(iter=0 ; iter<strmax ; iter++){
        // 读到尾部则退出
        if(ins->str[iter]==0)
            break;
        // 如果不等于%，则直接装入tempstr暂存
        if(ins->str[iter]!='%'){
            tempstr[index_temp] = ins->str[iter];
            index_temp++; 
        }else{
            // 若读到了%
            tempstr[index_temp] = '%';
            index_temp++;
            iter++;
            // 直接按浮点数类型读入，之后再按类型保存
            tempstr[index_temp] = 'f';
            // 先暂时读到数组里，全部读完后再倒序放到栈顶
            scanf(tempstr, &(tempfloat[num]));
            num++;
            // 重置tempstr
            memset(tempstr,0,strmax);
            index_temp = 0;
        }
    }
    // 重置，以便下次使用
    memset(tempstr,0,strmax);
    index_temp = 0;
    return num;
}

// 格式化输出处理函数
void PrintStr(struct instruction* ins){
	int iter;
    // 同样遍历str每个字符
    for(iter=0 ; iter<strmax ; iter++){
        // 读到尾部则退出
        if(ins->str[iter]==0)
            break;
        // 只要读到的不是转义字符时，直接存入tempstr
        if(ins->str[iter]!='\\'){
            tempstr[index_temp] = ins->str[iter];
            index_temp++; 
        }else{
            // 读到了转义字符，则将之前的输出，并初始化tempstr
            printf("%s",tempstr);
            memset(tempstr,0,strmax);
            index_temp = 0;
            iter++;
            // 判断下一个是不是n，若是，表明这是个换行符
            if(ins->str[iter]=='n'){
                printf("\n");
            }else{  // 若不是n，则按原来的进行输出
                printf("%c%c",ins->str[iter-1],ins->str[iter]);
            }
        }
    }
    // 输出最后的tempstr
    printf("%s",tempstr);
    // 重置
    memset(tempstr,0,strmax);
    index_temp = 0;
}

// 虚拟机代码解释函数
void interpret()
{
	int iter,num;
    printf("start interpret:\n");
    int ins_addr, base_addr, top_addr;
    struct instruction ins;				// 指令寄存器
    float s[stacksize];					// 运行栈
    memset(s,-1,stacksize);
    top_addr = 0;						// 栈顶寄存器
    base_addr = 0;						// 基址寄存器
    ins_addr = 0;						// 指令地址寄存器，存放指令在code[]数组里的位置
    s[0] = s[1] = s[2] = 0;
    do {
        // 读当前指令
        ins = code[ins_addr];
        ins_addr++;
        // code的格式：F L A，F是操作码，L是引用层与声明层的层差、不起作用就为0，A则根据指令而不同
        switch (ins.f)
        {
        case lit:                                               // 将a的值取到栈顶，栈顶指针t+1
            switch (ins.l)
            {
                case 0:     // 立即数A存入top所指单元，top+1
                    //printf("整数:%d\n", ins.a);
					s[top_addr] = ins.a;
					//printf("整数:%d\n", ins.a);
					//printf("此时栈顶:%d\n", s[top_addr]);
                    top_addr++;
                    break;
                case 1:     // 浮点数F存入top所指单元，top+1
                    s[top_addr] = ins.real;
                    //printf("此时栈顶:%d\n", s[top_addr]);
                    top_addr++;
                    break;
            }
            
            break;
        case opr:                                               // 数学、逻辑运算
            switch (ins.a)
            {
            case 0:                                         // 结束被调用过程，返回调用点并退栈
                // 栈顶位置指向现在过程的基址，作为退栈操作
                top_addr = base_addr;
                // 下一条指令指针p 指向当前过程的 返回地址
                ins_addr = s[top_addr+2];
                // 基址 指向当前过程的 动态链所标识的位置
                base_addr = s[top_addr+1];
                break;
            case 1:                                         // 求栈顶元素相反数，并保存在栈顶
                s[top_addr-1] = -s[top_addr-1];
                break;
            case 2:                                         // 次栈顶与栈顶相加，存入次栈顶，t-1
                top_addr--;
                s[top_addr-1] = s[top_addr-1]+s[top_addr];
                break;
            case 3:                                         // 次栈顶减去栈顶，存入次栈顶，t-1
                top_addr--;
                s[top_addr-1] = s[top_addr-1]-s[top_addr];
                break;
            case 4:                                         // 次栈顶 × 栈顶，存入次栈顶，t-1
                top_addr--;
                s[top_addr-1] = s[top_addr-1]*s[top_addr];
                break;
            case 5:                                         // 次栈顶 除以 栈顶，存入次栈顶，t-1
                top_addr--;
                s[top_addr-1] = s[top_addr-1]/s[top_addr];
                break;
            case 6:                                         // 栈顶内容若为奇数 则变为1，为偶数变为0
                s[top_addr-1] = (int)(s[top_addr-1])%2;
                break;
            case 8:                                         // 次栈顶与栈顶内容相等，1存于次栈顶，t-1
                top_addr--;
                s[top_addr-1] = (s[top_addr-1] == s[top_addr]);
                break;
            case 9:                                         // 次栈顶与栈顶内容不相等，1存于次栈顶，t-1
                top_addr--;
                s[top_addr-1] = (s[top_addr-1] != s[top_addr]);
                break;
            case 10:                                        // 次栈顶内容小于栈顶内容，1存于次栈顶，t-1
                top_addr--;
                s[top_addr-1] = (s[top_addr-1] < s[top_addr]);
                break;
            case 11:                                        // 次栈顶内容 ≥ 栈顶内容，1存于次栈顶，t-1
                top_addr--;
                s[top_addr-1] = (s[top_addr-1] >= s[top_addr]);
                break;
            case 12:                                        // 次栈顶内容 大于 栈顶内容，1存于次栈顶，t-1
                top_addr--;
                s[top_addr-1] = (s[top_addr-1] > s[top_addr]);
                break;
            case 13:                                        // 次栈顶内容 ≤ 栈顶内容，1存于次栈顶，t-1
                top_addr--;
                s[top_addr-1] = (s[top_addr-1] <= s[top_addr]);
                break;
            case 14:                                        // 栈顶输出控制台(判断float和int)，t-1
                if ((int)(s[top_addr-1]*10000)%10000==0)
                    printf("%d", (int)s[top_addr-1]);
                else
                    printf("%f", s[top_addr-1]);
                top_addr--;
                break;
            case 15:                                        // 控制台屏幕输出一个换行
                printf("\n");
                break;
            case 16:                                        // 从控制台读入一行输入，存入栈顶，t+1
                printf("\n>>>");
                scanf("%f", &(s[top_addr]));
                top_addr++;
                break;
            case 17:                                        // 输出字符串, 下面的判断是为了获取到换行符
                PrintStr(&ins);
                break;
            case 18:                                         // 强制以某种格式输出栈顶
                if((int)ins.real==0){
                    printf("%d",(int)s[top_addr-1]);
                }else if((int)ins.real==1){
                    printf("%f",(float)s[top_addr-1]);
                }else{
                    printf("运行时错误！！");
                }
                top_addr--;
                break;
            case 19:
                printf("\n>>>");
                num=FormatRead(&ins);
                // 此时再从数组后往前 往栈顶放, 如此一来最开时读的就在栈顶，可直接获取到
                for(iter=(num-1) ; iter>=0 ; iter--){
                    s[top_addr] = tempfloat[iter];
                    top_addr++;
                }
                break;
            }
            break;
        case lod:                                               // 取相对当前过程 层差为L 数据偏移量为A 的存储单元的值到栈顶, t+1
            s[top_addr] = s[base(ins.l,s,base_addr)+ins.a];
            top_addr++;
            //printf("此时栈顶:%d\n", s[top_addr-1]);
            break;
        case sto:                                               // 栈顶的值存到 相对当前过程层差为L 数据偏移量为A 的存储单元, t-1
            //printf("此时栈顶:%d\n", s[top_addr-1]);
            //printf("判断:%d\n", ((int)ins.real)%10);
            switch ((((int)ins.real)%10)){
                case 0:
                    top_addr--;
                    s[base(ins.l, s, base_addr) + ins.a] = (int)s[top_addr];
                    break;
                case 1:
                    top_addr--;
                    s[base(ins.l, s, base_addr) + ins.a] = s[top_addr];
                    break;
            }
            break;
        case cal:                                               // 调用子过程
            // 将父过程基地址入栈（静态链）
            s[top_addr] = base(ins.l, s, base_addr);
            // 将本过程基地址入栈，此两项用于base函数（动态链）
            s[top_addr+1] = base_addr;
            // 将当前指令指针入栈 （返回地址）
            s[top_addr+2] = ins_addr; 
            // 改变基地址指针值为新过程的基地址
            base_addr = top_addr;
            // 跳转
            ins_addr = ins.a;
            break;
        case inte:                                              // 栈顶开辟A个存储单元
            top_addr += ins.a;
            break;
        case jmp:                                               // 直接跳转
            ins_addr = ins.a;
            break;
        case jpc:                                               // 条件跳转，若栈顶为0，跳转至code的地址A
            top_addr--;
            if (s[top_addr] == 0)
            {
                ins_addr = ins.a;
            }
            break;
        case lda:                                               // 取数组中对应位置的值到当前栈顶，top指针不变
            s[top_addr-1] = s[base(ins.l, s, base_addr) + ins.a + (int)s[top_addr-1]];
            break;
        case sta:
            switch ((((int)ins.real)%10)){
                case 0:
                    top_addr--;
                    s[base(ins.l, s, base_addr) + ins.a + (int)s[top_addr-1]] = (int)s[top_addr];
                    break;
                case 1:
                    top_addr--;
                    s[base(ins.l, s, base_addr) + ins.a + (int)s[top_addr-1]] = s[top_addr];
                    break;
                default:
                    printf("sta wrong!\n");
                    break;
            top_addr--;
            }
            break;
        case lpl:           // 取相对当前过程 层差为L 数据偏移量为A+次栈顶的值 的存储单元的值 加上 当前栈顶的值并覆盖栈顶, t不变
            s[top_addr-1] = s[base(ins.l, s, base_addr) + ins.a + (int)s[top_addr-2]] + s[top_addr-1];
            break;
        case lmi:           // 取相对当前过程 层差为L 数据偏移量为A+次栈顶的值 的存储单元的值 减去 当前栈顶的值并覆盖栈顶, t不变
            s[top_addr-1] = s[base(ins.l, s, base_addr) + ins.a + (int)s[top_addr-2]] - s[top_addr-1];
            break;
        case end:           // 取相对当前过程 层差为L 数据偏移量为A+次栈顶的值 的存储单元的值 减去 当前栈顶的值并覆盖栈顶, t不变
            printf("\nThe program exited...\n");
            break;
        case mov:           // 将栈顶A个元素往上移3个位置，为过程的链数据腾出位置，t-A
            for(iter=ins.a ; iter>0 ; iter--){
                s[top_addr+2] = s[top_addr-1];
                top_addr--;
            }
            break;
        }
        if (ins.f == end){
            break;
        }
    } while (ins_addr != 0);
    
}

// 计算基址的函数
int base(int l, float* s, int base_addr)
{
    int b1;
    b1 = base_addr;
    while (l > 0)
    {
        b1 = s[b1];
        l--;
    }
    return b1;
}
