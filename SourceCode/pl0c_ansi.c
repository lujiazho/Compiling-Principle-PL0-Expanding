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

/* 解释执行时使用的栈 */
#define stacksize 500

int main(int argc,char** argv)
{
	// symnum为32, 代表32类符号
    bool nxtlev[symnum];

    // 打开一个文件, 返回文件指针: FILE类型的指针变量, 指向该文件
    Pl0_SourceCode = fopen(argv[1], "r");

    // 如果这文件存在则Ok
    if (Pl0_SourceCode)
    {
        init();     /* 初始化 */

        err = 0;    // 计算已发现的错误个数
        // cc、ll: getch使用的计数器, cc表示当前字符
        // cx虚拟机代码指针, 取值范围[0, cxmax-1]
        cc = cx = ll = 0;
        // 缓冲区中获取字符，getch 使用, 初始化为一个空格
        ch = ' ';

        // 首次读一个字符, 后面的在block里完成, 其是递归程序
        if(-1 != getsym())
        {
            // nxtlev：长度为32的bool数组, declbegsys：代表一个声明开始的符号集合，statbegsys代表语句开始的符号集合
            // 最终nxtlev里就存了3种声明和6种语句的开始符号, 是其中一种就是true
            addset(nxtlev, declbegsys, statbegsys, symnum);
            // 再加一个程序结束符 .
            nxtlev[period] = true;

            // 4个参数, 1: 当前分程序所在层, 2: 名字表当前尾指针, 第三个就是标识了3种声明和6种语句的bool数组
            // 4: param_num参数个数 
            if(-1 == block(0, 0, nxtlev, 0))   /* 调用编译程序 */
            {
                fclose(Pl0_SourceCode);
                printf("\n");
                return 0;
            }

            // 如果最后一个符号不是period, 就表示没有正确结束
            if (sym != period)
            {
                error(9);	// 程序结尾不是 . 非正常结束
            }

            // 如果只有0个错误, 就可以调用解释程序开始运行了
            if (err == 0)
            {
            	// 将虚拟机代码存入文件
            	StoreToFile();
            }
            else // 否则报错, 无法用解释程序来运行
            {
                printf("Errors in pl/0 program");
            }
        }

        fclose(Pl0_SourceCode); // 关闭源文件
    }
    else	// 文件都无法正常打开
    {
        printf("Can't open file!\n");
    }

    printf("\n");
    // 程序结束暂停一下
    // system("pause");
    return 0;
}

/*
* 初始化，设置符号值——符号辨识，类型归类——类型辨识
*/
void init()
{
    int i;

    /* 设置单字符符号 */
    for (i=0; i<=255; i++)
    {
        ssym[i] = nul;      // nul = 0
    }
    for(i=0 ; i<cxmax; i++){
    	code[i].f = nan;
    }
    // ASCII范围(0–31控制字符, 32–126 分配给了能在键盘上找到的字符\
    数字127代表 DELETE 命令, 后128个是扩展ASCII打印字符) 因此共256个
    // 设置14个 单字符的符号值 以ASCII码为索引
    ssym['+'] = plus;       // 3
    ssym['-'] = minus;      // 4
    ssym['*'] = times;      // 5
    ssym['/'] = slash;      // 6
    ssym['('] = lparen;     // 14
    ssym[')'] = rparen;     // 15
    ssym['='] = eql;        // 8
    ssym[','] = comma;      // 16
    ssym['.'] = period;     // 18
    ssym['#'] = neq;        // 9 不等于
    ssym[';'] = semicolon;  // 17 分号
    ssym[':'] = colon;  	// 33 冒号
    ssym['['] = lbrakt;     // 39 左中括号
    ssym[']'] = rbrakt;     // 40 右中括号

    /* 设置保留字名字,按照字母顺序，便于折半查找 */
    // 共19个保留字，word即代表保留字，word的第2个[]是存放保留字中一个个的字母，最多10个
    strcpy(&(word[0][0]), "begin");
    strcpy(&(word[1][0]), "break");     // break语句声明标志
    strcpy(&(word[2][0]), "call");
    strcpy(&(word[3][0]), "const");
    strcpy(&(word[4][0]), "do");
    strcpy(&(word[5][0]), "end");
    strcpy(&(word[6][0]), "exit");		// exit语句声明标志
    strcpy(&(word[7][0]), "for");       // for语句声明的标志
    strcpy(&(word[8][0]), "if");
    strcpy(&(word[9][0]), "odd");
    strcpy(&(word[10][0]), "procedure");
    strcpy(&(word[11][0]), "read");
    strcpy(&(word[12][0]), "real");		// 浮点声明的标志
    strcpy(&(word[13][0]), "step");     // step的标志
    strcpy(&(word[14][0]), "then");
    strcpy(&(word[15][0]), "until");    // until的标志
    strcpy(&(word[16][0]), "var");
    strcpy(&(word[17][0]), "while");
    strcpy(&(word[18][0]), "write");
    

    /* 设置保留字符号 */
    // 设置19个保留字的符号值
    wsym[0] = beginsym;     // 20 begin
    wsym[1] = breaksym;     // 41 break声明
    wsym[2] = callsym;      // 28 call
    wsym[3] = constsym;     // 29 const
    wsym[4] = dosym;        // 27 do
    wsym[5] = endsym;       // 21 end
    wsym[6] = exitsym;		// 44 exit声明
    wsym[7] = forsym;       // 36 for声明
    wsym[8] = ifsym;        // 22 if
    wsym[9] = oddsym;       // 7 odd
    wsym[10] = procsym;      // 31 procedure
    wsym[11] = readsym;      // 26 read
	wsym[12] = realsym;     // 34 real声明
    wsym[13] = stepsym;     // 37 step
    wsym[14] = thensym;     // 23 then
    wsym[15] = untilsym;    // 38 until
    wsym[16] = varsym;      // 30 var
    wsym[17] = whilesym;    // 24 while
    wsym[18] = writesym;    // 25 write


    /* 设置符号集 */
    // symnum = 32，代表32个符号(除去类P_CODE指令)
    for (i=0; i<symnum; i++)
    {
        declbegsys[i] = false;  // 表示声明开始的符号集合
        statbegsys[i] = false;  // 表示语句开始的符号集合
        facbegsys[i] = false;   // 表示因子开始的符号集合
    }

    // 9个保留字
    /* 设置声明开始符号集 */
    declbegsys[constsym] = true;    // 常数声明
    declbegsys[varsym] = true;      // 变量声明
    declbegsys[procsym] = true;     // 过程声明

    /* 设置语句开始符号集 */
    statbegsys[beginsym] = true;    // 复合语句的开始begin
    statbegsys[callsym] = true;     // 过程调用语句
    statbegsys[ifsym] = true;       // if条件语句
    statbegsys[whilesym] = true;    // while循环语句
    statbegsys[readsym] = true;     // 读语句
    statbegsys[writesym] = true;    // 写语句
    statbegsys[forsym] = true;      // for循环语句
    statbegsys[breaksym] = true;    // break语句
    statbegsys[exitsym] = true;    // break语句

    /* 设置因子(即等式右边的部分)开始符号集 */
    facbegsys[ident] = true;        // 标识符
    facbegsys[number] = true;       // 无符号整数
    facbegsys[lparen] = true;       // 左括号，left parenthesis 左圆括号
    facbegsys[real] = true;			// 浮点数real
}

/*
* 用数组实现集合的集合运算
*/
int inset(int e, bool* s)
{
    return s[e];
}

// sr: 长度为32的bool数组, s1: 代表一个声明开始的符号集合，s2: 代表语句开始的符号集合, n = 32
// 最终sr里就存了3种声明和6种语句的开始符号, 是其中一种就是true
int addset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]||s2[i];
    }
    return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]&&(!s2[i]);
    }
    return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]&&s2[i];
    }
    return 0;
}

/*
*   出错处理，打印出错位置 和 错误编码n
*/
void error(int n)
{
    char space[81];     // 这个81和读取行缓冲区是一致的, 也是81个字符
    // 初始化space, 用于表示错误在一个Line里出现的位置
    memset(space,32,81);    // 将这81个char(占1个字节)的位置全部填上32: 二进制为0010 0000, 其ASCII代表 空格

    // 赋值为0, 表示'\0'结束, 因此空格的长度(因为第一个空格位置是0)就是出错的位置(在最后一个空格后面, 为cc-1)
    space[cc-1]=0; //出错时当前符号已经读完，所以cc-1

    printf("****%s!%d\n", space, n);
    switch(n)
    {
        case 1 :printf("常量说明中不可以用\":=\". \n");break;
        case 2 :printf("常量说明中的\"=\"后应该是数字.  \n");break;
        case 3 :printf("缺少\"=\". \n");break;
        case 4 :printf("const ，var ，procedure后应为标识符. \n");break;
        case 5 :printf("漏掉了\",\"或者是\";\". \n");break;
        case 6 :printf("过程说明后的符号不正确(应是句子的开始符,过程定义符)。\n");break;
        case 7 :printf("声明顺序有误，应为[<变量说明部分>][<常量说明部分>] [<过程说明部分>]<语句>。\n");break;
        case 8 :printf("程序体内的语句部分的符不正确。\n");break;
        case 9 :printf("程序的末尾丢掉了句号\".\"。\n");break;
        case 10 :printf("句子之间漏掉了\";\"。\n");break;
        case 11 :printf("标识符未说明.\n");break;
        case 12 :printf("赋值号左端应为变量。\n");break;
        case 13 :printf("应为 \":=/+=/-=\" 之一。\n");break;
        case 14 :printf("call 后应为标识符。\n");break;
        case 15 :printf("call 后标识符属性应是过程。\n");break;
        case 16 :printf("缺少\"then\"。\n");break;
        case 17 :printf("缺少\"end\"或\";\"。\n");break;
        case 18 :printf("do while 型循环语句缺少do。\n");break;
        case 19 :printf("语句后的标号不正确。\n");break;
        case 20 :printf("应为关系运算符。\n");break;
        case 21 :printf("表达式内的标识符属性不能是无返回值的过程。\n");break;
        case 22 :printf("表达式中漏掉右括号。\n");break;
        case 23 :printf("非法符号。\n");break;
        case 24 :printf("表达式的开始符为非法符号符号。\n");break;
        case 25 :printf("运算符的后边是常量。\n");break;
        case 26 :printf("不存在此操作符\n");break;
        case 27 :printf("变量定义的长度应为常量或者是常数\n");break;
        case 28 :printf("变量定义重复\n");break;
        case 29 :printf("未找到对应过程名\n");break;
        case 30 :printf("不支持过程的判断\n");break;
        case 31 :printf("超过地址上界。\n");break;
        case 32 :printf("超过最大允许过程嵌套声明层数。\n");break;
        case 33 :printf("缺少）");break;
        case 34 :printf("read内应当为变量值\n");break;
        case 35 :printf("read里没标识符，或标识符未声明，或者标识符不是变量\n");break;
        case 36 :printf("未知变量类型\n");break;
        case 37 :printf("字符串过长\n");break;
        case 38 :printf("write中字符串后应为逗号\n");break;
        case 39 :printf("write格式化输出不对应\n");break;
        case 40 :printf("块注释未正常结束\n");break;
        case 41 :printf("for后 应为标识符\n");break;
        case 42 :printf("缺少step\n");break;
        case 43 :printf("缺少until\n");break;
        case 44 :printf("缺少do\n");break;
        case 45 :printf("数组大小必须是常量或立即数\n");break;
        case 46 :printf("数组声明中括号不完整\n");break;
        case 47 :printf("数组访问错误\n");break;
        case 48 :printf("数组访问缺少右括号\n");break;
        case 49 :printf("break不在循环语句内\n");break;
        case 50 :printf("循环超过规定最大层数\n");break;
        case 51 :printf("格式化输入参数错误\n");break;
        case 52 :printf("read后缺少左括号 (\n");break;
        case 53 :printf("未知格式化类型\n");break;
        case 54 :printf("read读入变量超过限制\n");break;
        case 55 :printf("过程传参语法错误\n");break;
        case 56 :printf("过程调用参数不一致\n");break;
        default :printf("找不到这种错误\n");
    }

    err++;
}

void StoreToFile(){
	virCode = fopen("virCode.tmp", "w");
 	int length = sizeof(code)/sizeof(code[0]);
 	int iter;
	for(iter=0 ; iter<length ; iter++){
		switch(code[iter].f){
			case 0:
                if(code[iter].a/10>0)
				    fprintf(virCode,"lit %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"lit %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
				break;
			case 1:
				if(code[iter].a/10>0){
                    if(code[iter].a==17 || code[iter].a==19){
                        fprintf(virCode,"opr %d %d %s\n", code[iter].l, code[iter].a, code[iter].str);
                    }else{
                        fprintf(virCode,"opr %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                    }
                }
                else
                    fprintf(virCode,"opr %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
                break;
			case 2:
				if(code[iter].a/10>0)
                    fprintf(virCode,"lod %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"lod %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
                break;
			case 3:
				if(code[iter].a/10>0)
                    fprintf(virCode,"sto %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"sto %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
                break;
			case 4:
				if(code[iter].a/10>0)
                    fprintf(virCode,"cal %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"cal %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
                break;
			case 5:
				if(code[iter].a/10>0)
                    fprintf(virCode,"int %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"int %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
                break;
			case 6:
				if(code[iter].a/10>0)
                    fprintf(virCode,"jmp %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"jmp %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
                break;
			case 7:
				if(code[iter].a/10>0)
                    fprintf(virCode,"jpc %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"jpc %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
                break;
            case 9:
                if(code[iter].a/10>0)
                    fprintf(virCode,"lda %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"lda %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
                break;
            case 10:
                if(code[iter].a/10>0)
                    fprintf(virCode,"sta %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"sta %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
                break;
            case 11:
            	if(code[iter].a/10>0)
                    fprintf(virCode,"lpl %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"lpl %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
            	break;
            case 12:
            	if(code[iter].a/10>0)
                    fprintf(virCode,"lmi %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"lmi %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
            	break;
            case 13:
                fprintf(virCode,"end %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
            	break;
            case 14:
                if(code[iter].a/10>0)
                    fprintf(virCode,"mov %d %d %f\n", code[iter].l, code[iter].a, code[iter].real);
                else
                    fprintf(virCode,"mov %d %d  %f\n", code[iter].l, code[iter].a, code[iter].real);
            	break;
			default:
				break;
		}
	}
    fclose(virCode);
}

void print_nameTable(int tx0, int tx){
    printf("TABLE:\n");
    int i;
    if (tx0+1 > tx)
    {
        printf("    NULL\n");
    }
    printf("number \tkind \tname \tval/lev \taddr \tsize");
    for (i=1; i<=tx; i++)
    {
        switch (table[i].kind)
        {
        case constant:
            printf("\n%d \tconst \t%s \t", i, table[i].name);
            printf("val=%d \taddr=%d \ttype=%c", table[i].val, table[i].adr, table[i].type);
            break;
        case variable:
            printf("\n%d \tvar \t%s \t", i, table[i].name);
            printf("lev=%d \taddr=%d \ttype=%c", table[i].level, table[i].adr, table[i].type);
            break;
        case procedur:
            printf("\n%d \tproc \t%s \t", i, table[i].name);
            printf("lev=%d \taddr=%d \tsize=%d", table[i].level, table[i].adr, table[i].size);
            break;
        case array:
            printf("\n%d \tarray \t%s \t", i, table[i].name);
            printf("lev=%d \taddr=%d \ttype=%c \tsize=%d", table[i].level, table[i].adr, table[i].type, table[i].size);
            break;
        }
    }
    printf("\n\n");
}

/*
* 漏掉空格，读取一个字符。
*
* 每次读一行，存入line缓冲区，line被getsym取空后再读一行
*
* 被函数getsym调用。
*/
int getch()
{
    // ll、cc 均初始化为0, cc表示当前字符(ch)的位置
    // 如果相等了, 说明上次从流中读出的一行字符(在line里)已经被读完了, 就得再读一行出来 
    if (cc == ll)
    {
        // feof: 检测流上的文件是否已结束，如果光标后啥也没有，则返回非0值，光标后有东西(包括EOF), 返回0
        // Pl0_SourceCode: 源代码文件的指针
        if (feof(Pl0_SourceCode))
        {
            printf("program incomplete");
            return -1;
        }
        // 重新赋值为0
        ll=0;
        cc=0;
        // printf("%d ", cx);          // cx, 虚拟机代码指针(其实是个数字, 表示这是第几行虚拟机代码)
        ch = ' ';               // 空格, 对应ASCII 32
        while (ch != 10)        // 刚开始32肯定不等于10, 则进入循环, 10是换行符line feed(意思是把整行读完)
        {
            // 如果是EOF文件结束符, 则line中该位置为'\0'串尾
            if (EOF == fscanf(Pl0_SourceCode,"%c", &ch))   // 从流中读取一个%c即单个字符到ch
            {
                line[ll] = 0;
                break;
            }
            // 代表是行注释 
            if(ch=='/' && ll!=0 && line[ll-1]=='/'){
                ll--;
                // 这里进来的肯定也不等于10，只为了把剩下的读完
                while (ch != 10){
                	// EOF的意思是，读取错误，即下一个没读到东西 
                    if (EOF == fscanf(Pl0_SourceCode,"%c", &ch))   // 从流中读取一个%c即单个字符到ch
		            {
		            	// 这里我们可以让line[0]的位置为0，反正是注释 
		                line[ll] = 0;
		                break;
		            }
                }
                break;
            }else if(ch=='*' && ll!=0 && line[ll-1]=='/'){			// 代表是块注释 
            	ll--;
            	char tempch;			// tempch作为当前ch的前一个值 
            	fscanf(Pl0_SourceCode,"%c", &ch);
            	// 读到 */ 为止 
            	do{
            		tempch = ch; 
            		if (EOF == fscanf(Pl0_SourceCode,"%c", &ch))   // 从流中读取一个%c即单个字符到ch
		            {
		                line[ll] = 0;
		                break;
		            }
				}while(!(ch == '/' && tempch=='*')); 
                break;
			} 
            //end richard
            line[ll] = ch;
            ll++;
        }
    }
    // line: 读取行缓冲区(存着源代码中的一行), 最多81个char字符(包括'\0')
    ch = line[cc];  // 把当前cc指向的读取行缓冲区中的字符给ch
    cc++;           // cc 加1, 指向下一个位置, 以便下次读取
    // 如果ll=0代表是行注释或块注释且在第一个位置，则把这个 ch也得消掉，不然会错读 
	if(ll==0){
    	ch = ' ';
    	cc=0;
	}
    return 0;
}

/*
* 词法分析，获取一个符号(词)
*/
int getsym()
{
    int i,j,k;

    /* the original version lacks "\r", thanks to foolevery */
    // 检查上一次最后读到的字符, 如果不是如下4种, 则不读新的
    while (ch==' ' || ch==10 || ch==13 || ch==9)  /* 忽略空格、10:换行、13:回车(Carriage Return)和9:TAB(水平制表符) */
    {
        getchdo; // 抓取一个字符到ch
    }
    /* 名字(标识符)或保留字以a..z开头 */
    if (ch>='a' && ch<='z')
    {           
        k = 0; // 表示当前这个词已经有多少个字符了
        do {
            if(k<al) // al: 符号的最大长度, 为10
            {
                // a: 用于临时读取的一个词(符号)
                a[k] = ch;
                k++;
            }
            // else: 如果后面还有字母数字, k却已经>=al, 则读来丢(目前如此认为)
            getchdo;    // 再取一个字符
            //printf("\n-%c ",ch);
        } while (ch>='a' && ch<='z' || ch>='0' && ch<='9');
        a[k] = 0;       // 最后以'\0'结尾
        strcpy(id, a);  // 暂时copy到id中, id: 存放当前标识符
        i = 0;
        j = norw-1;     // norw: 关键字个数, 13个
        do {    /* 搜索当前符号是否为保留字, 折半查找 */
            k = (i+j)/2; // 先检查中间那个
            // ASCII:   A 65    a 97    0 48
            if (strcmp(id,word[k]) <= 0) // 当id<word[k]时，返回为负数(按ASCII值大小比较)
            {
                // 表示 即便是保留字, 也是前半部分的, 因为word中的保留字是按字母a-z从0开始排的(目前只有小写)
                j = k - 1;
            }
            if (strcmp(id,word[k]) >= 0) // 当id>word[k]时，返回为正数
            {
                i = k + 1;
            }
        } while (i <= j); // 一旦识别到相等, i=k+1一定大于j=k-1,且相差大于1, 不相等的话,只会相差1
        // 如果i比j大至少2, 说明是保留字
        if (i-1 > j)
        {
            // sym记录为相应保留字的标识
            sym = wsym[k]; // sym, 存放当前符号的标识(数字)
        }
        else // 是名字
        {
            // sym记录为相应标识符(ident)的标识, 为ident=1
            sym = ident; /* 搜索失败，是名字 */
        }
    }
    else // 不是字母开头, 则只能是数字或符号
    {
        if (ch>='0' && ch<='9')
        {           /* 检测是否为数字：以0..9开头 */
            k = 0;          // 表示这个数字已有几位数
            num = 0;        // 记录该数字
            sym = number;   // 数字的标识, 为2
            do {
                num = 10*num + ch - '0';    // 累加
                k++;
                getchdo;    // 再读一个, 不是数字则退出循环
            } while (ch>='0' && ch<='9'); /* 获取数字的值 */
            
            // 继续判断是不是浮点数
            if (ch=='.'){
            	// 进入则表示是浮点数
            	sym = real;
				getchdo;
				float temp = 0.1;
				float_ = (float)num;
				while(ch>='0'&& ch<='9'){
					float_ = float_ + temp*(ch - '0');
					temp = temp*0.1;
					k++;
					getchdo;
				}
            }
            // k--; 删掉，不合理
            if (k > nmax)   // nmax: number的最大位数, 为14
            {
                error(30);  // 错误编码为30, 数字过长!!!
            }
        }
        else // 是符号, 可能是单字符, 也可能是双字符
        {
            if (ch == ':')      /* 检测赋值符号 */
            {
                getchdo;  // 读取一个字符
                if (ch == '=')  // 如果是=, 说明整个是 := 赋值
                {
                    sym = becomes;  // 赋值符号的标识, 为19
                    getchdo;    // 再读一个, 下次直接检测, 和上面的同步
                }
                else
                {
                    sym = colon;  /* 为冒号 */
                }
            }
            else    // 非赋值符号
            {
                if (ch == '<')      /* 检测小于或小于等于符号 */
                {
                    getchdo;
                    if (ch == '=')
                    {
                        sym = leq;  // 小于等于符号标识为11
                        getchdo;    // 再读一个, 下次直接检测, 和上面的同步
                    }
                    else
                    {
                        sym = lss;  // 小于符号标识为10
                    }
                }
                else    // 不是小于开头的符号
                {
                    if (ch=='>')        /* 检测大于或大于等于符号 */
                    {
                        getchdo;
                        if (ch == '=')
                        {
                            sym = geq;  // 大于等于符号标识为13
                            getchdo;    // 再读一个, 下次直接检测, 和上面的同步
                        }
                        else
                        {
                            sym = gtr;  // 大于, 标识为12
                        }
                    }
                    else
                    {
                        // 检查字符串
                        if (ch=='"')
                        {
                            for(k=0 ; k<strmax ; k++){
                                str_[k] = '\0';
                            }
                            k=0;
                            sym = str;
                            getchdo;
                            while(ch!='"')
                            {
                                str_[k] = ch;
                                k++;
                                getchdo;
                                if(k>(strmax-1)){
                                    break;
                                }
                            }
                            getchdo;
                            if(k>(strmax-1)){
                                error(37);
                            }
                        }
                        else
                        {
                        	if(ch == '+')
                        	{
                        		getchdo;
		                        if (ch == '=')
		                        {
		                            sym = pluseq;  // +=符号标识为42
		                            getchdo;    // 再读一个, 下次直接检测, 和上面的同步
		                        }
		                        else
		                        {
		                            sym = plus;  // +, 标识为3
		                        }
                        	}
                        	else
                        	{
                        		if(ch=='-')
                        		{
                        			getchdo;
			                        if (ch == '=')
			                        {
			                            sym = minuseq;  // -=符号标识为43
			                            getchdo;    // 再读一个, 下次直接检测, 和上面的同步
			                        }
			                        else
			                        {
			                            sym = minus;  // -, 标识为4
			                        }
                        		}
                        		else    // 剩下的只能是单字符
		                        {
		                            // 如果初始化时有该单字符, 则赋值对应标识, 否则为初始化时的nul标识(0,即unknown)
		                            sym = ssym[ch];     /* 当符号不满足上述条件时，全部按照单字符符号处理 */
		                            //getchdo;
		                            //richard
		                            if (sym != period)  // 只要不是整个程序的最后一个结束符(.),则都得再读一个以供下次使用
		                            {
		                                getchdo;
		                            }
		                            //end richard
		                        }
                        	}
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/*
* 生成虚拟机代码
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, int z , float real)
{
    if (cx >= cxmax)
    {
        printf("Program too long"); /* 程序过长 */
        return -1;
    }
    code[cx].f = x;
    code[cx].l = y;
    code[cx].a = z;
    code[cx].real = real;
//    if ((int)(real*10)%10==0)
//        printf("%d", code[cx].real);
//    else{
//    	printf("代码：%d", code[cx].f);
//		printf("%f--", code[cx].real);
//	} 
        
    cx++;
    return 0;
}


/*
* 测试当前符号是否合法
*
* 在某一部分（如一条语句，一个表达式）将要结束时时我们希望下一个符号属于某集?
* （该部分的后跟符号），test负责这项检测，并且负责当检测不通过时的补救措施，
* 程序在需要检测时指定当前需要的符号集合和补救用的集合（如之前未完成部分的后跟
* 符号），以及检测不通过时的错误号。
*
* s1:   我们需要的符号
* s2:   如果不是我们需要的，则需要一个补救用的集?
* n:    错误号
*/
int test(bool* s1, bool* s2, int n)
{
    if (!inset(sym, s1))
    {
        error(n);
        /* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
        // 就是排除所有不属于FIRST和FOLLOW的符号
        while ((!inset(sym,s1)) && (!inset(sym,s2)))
        {
            getsymdo;
        }
        // *************************这里要加东西，即检测属于FIRST还是FOLLOW
        // 如果是FOLLOW则该句当作结束，getsymdo读取下一个，开始新的分析
        // 如果是FIRST，则不用再读，直接开始新一轮的分析
    }
    return 0;
}

/*
* 编译程序主?
*
* lev:    当前分程序所在层
* tx:     名字表当前尾指针
* fsys:   从main进来的block是：3种声明 + 6种语句的开始符号 + .结束符（是当前模块的FIRST集合）
* param_num: 传参参数个数 
*/
int block(int lev, int tx, bool* fsys, int thisParamNum)
{
    int i;
	
    int dx;                 /* 名字分配到的相对地址 */
    int tx0;                /* 保留初始tx, tx是名字表中最后一个名字的指针, 不是空位的指针 */
    // 此外 cx是虚拟机代码指针，指向的是空位置
    bool nxtlev[symnum];    /* 下级函数的FIRST集合 */

    dx = 3 + thisParamNum;
    tx0 = tx;               /* 记录本层名字的初始位置 */
    // table这个位置不会存放其他名字，就是main或者procedure，先用于存放jmp在code里的位置，等声明分析完了后，才好改；然后再改成inte的位置
    table[tx-thisParamNum].adr = cx;
    // 首先就生成无条件跳转jmp 0 0, 为了跳到主函数虚拟机代码处，后面会改
    gendo(jmp, 0, 0, 0);

    if (lev > levmax)
    {
        error(32);
    }

    do {

        if (sym == constsym)    /* 收到常量声明符号，开始处理常量声明 */
        {
            getsymdo;

            /* the original do...while(sym == ident) is problematic, thanks to calculous */
            /* do { */
            constdeclarationdo(&tx, lev, &dx);  /* dx的值会被constdeclaration改变，使用指针 */
            while (sym == comma)
            {
                getsymdo;
                constdeclarationdo(&tx, lev, &dx);
            }
            if (sym == semicolon)
            {
                getsymdo;
            }
            else
            {
                error(5);   /*漏掉了逗号或者分号*/
            }
            /* } while (sym == ident); */
        }
		
        if (sym == varsym)      /* 收到变量声明符号，开始处理变量声明 */
        {
            getsymdo;

            /* the original do...while(sym == ident) is problematic, thanks to calculous */
            /* do {  */
            int tx_cp = tx+1;	// 用于遍历每个var 
            int dx_cp = dx;		// 用于array里addr的再赋值 
            vardeclarationdo(&tx, lev, &dx);
            while (sym == comma)
            {
                getsymdo;
                vardeclarationdo(&tx, lev, &dx);
            }
            // 判断有无real浮点标志
            if (sym == colon)
			{
				getsymdo;
				if (sym == realsym)
				{
					for(i=tx_cp ; i<=tx ; i++){
						table[i].type = 'f';
					}
					getsymdo;
				}
				else
				{
					error(36);	// 未知变量类型
				}
			}
			// 如果没有指明，则默认是integer
			else
			{
				for(i=tx_cp ; i<=tx ; i++){
					table[i].type = 'i';
				}
			}

            if (sym == lbrakt)  /* 标识符之后是'[', 则识别为数组 */
            {
                getsymdo;
                // 这个函数里找出开辟的空间大小
                arraydeclarationdo(&tx, lev, &dx);
                // 全部修改成array类型, 记录array的size, 调整运行栈里开辟的空间
                for(i=tx_cp ; i<=tx ; i++){
                    table[i].kind = array;
                    table[i].size = arrSize;
                    table[i].adr = dx_cp;
					dx_cp += table[i].size;
                    dx += (table[i].size - 1);
                }
            }

            if (sym == semicolon)
            {
                getsymdo;
            }
            else
            {
                error(5);	// 漏掉了分号
            }
            /* } while (sym == ident);  */
        }
		
        while (sym == procsym) /* 收到过程声明符号，开始处理过程声明 */
        {
            getsymdo;
			
            if (sym == ident)
            {
                enter(procedur, &tx, lev, &dx); /* 记录过程名字 */
                getsymdo;
            }
            else
            {
                error(4);   /* procedure后应为标识符 */
            }
            // 暂存dx
            save_dx = dx;
            // dx变成3, 使得传参变量都从过程中dx=3的位置开始记录
            dx = 3;
            int tx_cp = tx+1;
            if (sym==lparen){
            	param_num = 0;
				do{
            		getsymdo;
            		if(sym==ident){
            			vardeclarationdo(&tx, lev+1, &dx); 
            			param_num++;
            		}
            	}while(sym == comma);

            	if (sym==rparen)
            	{
            		getsymdo;
            	}else{
            		error(55);		// 过程传参语法错误
            	}
            	// 暂时默认为浮点数 
            	for(i=tx_cp ; i<=tx ; i++){
					table[i].type = 'f';
				}
				// 记录过程参数个数到val
				table[tx_cp-1].val = param_num;
            }
            // 赋值回来
            dx = save_dx;
            
            // procedure的 标识符后应该有个 ;
            if (sym == semicolon)
            {
                getsymdo;
            }
            else
            {
                error(5);   /* 漏掉了分号 */
            }

            // fsys: 当前模块的FIRST集合
            memcpy(nxtlev, fsys, sizeof(bool)*symnum);
            // nxtlev下层的FIRST集合，继承了这一层的 3种声明 + 6种语句的开始符号 + .结束符，外加一个；
            nxtlev[semicolon] = true;                             // ***********************这个；不应该存在于FIRST
            if (-1 == block(lev+1, tx, nxtlev, param_num))
            {
                return -1;  /* 递归调用 */
            }
            // tx也回归本层的
			tx = tx_cp-1; 
            // 从下一层出来后，看end后是不是 ; 结尾
            if(sym == semicolon)
            {
                getsymdo;
                // <分程序> ::= [<常量说明>][<变量说明>][<过程说明>]<语句>  注意这里的<语句>只能有一个
                // 这里是说接下来可以是7种<语句>，或者是再定义一个procedure
                memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
                nxtlev[ident] = true;
                // 加上这个的意思是: 还可以继续声明第二个procedure
                nxtlev[procsym] = true;
				nxtlev[constsym] = true;
                // 错误处理用的代码，这里是对<分程序>接下来的<语句>部分进行test
                // fsys是此层的FIRST：3种声明 + 6种语句的开始符号 + .结束符
                bool xia[symnum]; 
                xia[semicolon] = true;
                xia[period] = true;
                testdo(nxtlev, xia, 6);                          // ************************这里的nxtlev没问题，FOLLOW不应该是fsys而是;或.
            }
            else
            {
                error(5);   /* 漏掉了分号 */
            }
        }
        // 这里是在没有<过程说明>部分所直接到达的位置，当然有<过程说明>也会到达
        // statbegsys: 有begin、call、if、while、read、write，占了7种语句里的6种，除了<赋值语句>
        //memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
        // 再加上赋值语句的ident开头
        //nxtlev[ident] = true;
        // declbegsys: 声明开始符号集, 有const、var、procedure
        //testdo(nxtlev, declbegsys, 7);                            // nxtlev合理，***********但这里的后跟符号集FOLLOW 明显不应该是任何<声明>

        // ************如果在2排之上的地方把声明全部过滤了，这里根本就不用循环
    } while (inset(sym, declbegsys));   /* 直到没有声明符号 */
	
    // 这里的table[tx0].adr就是jmp在code里的位置，即修改jmp的a，使之直接跳到程序入口, 即inte代码处
    code[table[tx0].adr].a = cx;    /* 开始生成当前过程代码 */
    // 这里table[tx0] 其实是程序procedure或主程序在名字表中的实例, adr就直接指向inte在code中的位置, 当然此时cx还指向的空，但立刻这个空就会是inte
    table[tx0-thisParamNum].adr = cx;            /* 当前过程代码地址 */
    // size就是dx: 3+声明变量数
    table[tx0-thisParamNum].size = dx;           /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
    // 在栈顶开辟 dx 个存储单元
    gendo(inte, 0, dx, 0);             /* 生成分配内存代码，dx就是size大小 */
    printf("lev=%d, tx0=%d, tx=%d\n",lev, tx0, tx);
    print_nameTable(tx0, tx);
	
    // 语句的FIRST为fsys: 3种声明 + 6种语句的开始符号 + .结束符
    // **********************这个nxtlev是<语句>的FIRST，明显不对，只能是7种语句，如此看来fsys根本都不需要
    memcpy(nxtlev, fsys, sizeof(bool)*symnum);  /* 每个后跟符号集和都包含上层后跟符号集和，以便补救 */
    // 语句的FIRST加上分号或end
    nxtlev[semicolon] = true;       // *******************不应该有什么;和end
    nxtlev[endsym] = true;
    // 这里的statement就是语句处理
    statementdo(nxtlev, &tx, lev);
    // opr 0 0结束被调用过程，返回调用点并退栈
    gendo(opr, 0, 0, 0);                       /* 每个过程出口都要使用的释放数据段指令 */
    memset(nxtlev, 0, sizeof(bool)*symnum); /*分程序没有补救集合 */
    // 这里test的就是有没有period . 符号
    // *************************这里的FIRST也不应该是fsys, 而只能是 . 因为<语句>只有一个, nxtlev是FOLLOW，用于补救，但没法补救
    testdo(fsys, nxtlev, 8);
    return 0;
}

/*
* 在名字表中加入一项
*
* k:      名字种类const,var or procedure
* ptx:    名字表尾指针的指针，为了可以改变名字表尾指针的值
* lev:    名字所在的层次,，以后所有的lev都是这样
* pdx:    dx为当前应分配的变量的相对地址，分配后要增加1
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
    // 这里要++ 代表传进来的tx不是指向空位置
    (*ptx)++;
    strcpy(table[(*ptx)].name, id); /* 全局变量id中已存有当前名字的名字 */
    table[(*ptx)].kind = k;
    switch (k)
    {
    case constant:  /* 常量名字 */
        if (num > amax)
        {
            error(31);  /* 数越界 */
            num = 0;
        }
        if(table[(*ptx)].type=='i'){
            table[(*ptx)].val = num;
        }else if(table[(*ptx)].type=='f'){
            table[(*ptx)].valf = float_;
        }
        
        break;
    case variable:  /* 变量名字 */
        table[(*ptx)].level = lev;
        printf("到这儿了， tx=%d, dx=%d\n",(*ptx),(*pdx));
        table[(*ptx)].adr = (*pdx);
        (*pdx)++;
        break;
    case procedur:  /*　过程名字　*/
        table[(*ptx)].level = lev;
        break;
    }
}

/*
* 查找名字的位置.
* 找到则返回在名字表中的位置,否则返回0.
*
* idt:    要查找的名字
* tx:     当前名字表尾指针
*/
int position(char* idt, int tx)
{
    int i;
    strcpy(table[0].name, idt);
    i = tx;
//    printf("------------------------------------%s,%s\n",(table[i].name),idt);
    
    while (strcmp(table[i].name, idt) != 0)
    {
        i--;
//		printf("------------------------------------%s,%s\n",(table[i].name),idt);
    }
    return i;
}

// 数组声明处理函数
int arraydeclaration(int* ptx, int lev, int* pdx)
{
    int i;
    // 检查是不是const, 中括号内必须是const或number
    if (sym == ident)
    {
        i = position(id, *ptx);
        if (i == 0)
        {
            error(11);  /* 变量未找到 */
        }
        else
        {
            if (table[i].kind == constant){
                arrSize = table[i].val;
            }else{
                error(45);      // 报错: 数组大小必须是常量或立即数
            }
        }
    }
    // 检查是否是integer立即整数
    else if(sym == number)
    {
        arrSize = num;
    }
    else
    {
        error(45);      // 报错: 数组大小必须是常量或立即数
    }

    getsymdo;
    if (sym != rbrakt)
    {
        error(46);      // 数组声明中括号不完整
    }else{
        getsymdo;
    }
    return 0;
}

/*
* 常量声明处理
*/
int constdeclaration(int* ptx, int lev, int* pdx)
{
    if (sym == ident)
    {
        getsymdo;
        if (sym==eql || sym==becomes)
        {
            if (sym == becomes)
            {
                error(1);   /* 把=写成了:= */
            }
            getsymdo;
            if (sym == number || sym == real)
            {
                if(sym==number){
                    table[(*ptx+1)].type = 'i';
                }else if(sym==real){
                    table[(*ptx+1)].type = 'f';
                }
                enter(constant, ptx, lev, pdx);
                getsymdo;
            }
            else
            {
                error(2);   /* 常量说明=后应是数字 */
            }
        }
        else
        {
            error(3);   /* 常量说明标识后应是= */
        }
    }
    else
    {
        error(4);   /* const后应是标识 */
    }
    return 0;
}

/*
* 变量声明处理
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
	// 首先检测是不是标识符
    if (sym == ident)
    {
        enter(variable, ptx, lev, pdx); // 填写名字表, variable表示是变量var
        getsymdo;
    }
    // 如果不是就报错
    else
    {
        error(4);   /* var后应是标识 */
    }
    return 0;
}

/*
* 语句处理
*/
int statement(bool* fsys, int* ptx, int lev)
{
    // k,j用于for循环, numOfFormat记录格式化输出的个数, numOfSegs存放格式化中字符串各分段, perSegIndex是各个分段字符的index
    int i, cx1, cx2,k,j,perSegIndex;  
    struct instruction which;   // 专用于for的step判断
    bool nxtlev[symnum];

    if (sym == ident)   /* 准备按照赋值语句处理 */
    {
        // 寻找这个标识符在名字表中的位置
        i = position(id, *ptx);
        if (i == 0)
        {
            error(11);  /* 变量未找到 */
        }
        // 找到则进入
        else
        {
        	// 说明是数组array
            if(table[i].kind != variable)
            {
                getsymdo;
                // 检查是否是array
                if (sym == lbrakt) /* 索引是括号内的表达式 */
                {
                    getsymdo;
                    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                    nxtlev[rbrakt] = true;
                    expressiondo(nxtlev, ptx, lev);
                    if (sym != rbrakt)
                    {
                        error(48);  // 数组访问缺少右括号
                    }
                    else
                    {
                        getsymdo;
                        // 赋值/+=/-=符号
                        if(sym == becomes || sym == pluseq || sym == minuseq)
                        {
                        	sym_op = sym;
                        	getsymdo;
                        }
                        else{
                        	error(13);  /* 没有检测到赋值/+=/-=符号 */
                        }
                        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                        expressiondo(nxtlev, ptx, lev); /* 处理赋值符号右侧表达式 */

                    	// 如果是+=/-=则多一个操作
                    	if(sym_op == pluseq){
                        	// 取相对当前过程 层差为L 数据偏移量为A+次栈顶的值 的存储单元的值 加上 当前栈顶的值并覆盖栈顶, t不变
                        	gendo(lpl, lev-table[i].level, table[i].adr, 0);
                        }
                        else if(sym_op == minuseq){
                        	// 取相对当前过程 层差为L 数据偏移量为A+次栈顶的值 的存储单元的值 减去 当前栈顶的值并覆盖栈顶, t不变
                        	gendo(lmi, lev-table[i].level, table[i].adr, 0);
                        }
                        // 判断以什么方式存储
                        if(table[i].type=='i'){
                            // 栈顶的值(按整型)存到 相对当前过程层差为L 数据偏移量为A+次栈顶的值 的存储单元, t-2
                            gendo(sta, lev-table[i].level, table[i].adr, 0);   // 按整型存进去，top-2
                        }
                        else if(table[i].type=='f'){
                            // 栈顶的值(按浮点型)存到 相对当前过程层差为L 数据偏移量为A+次栈顶的值 的存储单元, t-2
                            gendo(sta, lev-table[i].level, table[i].adr, 1);   // 按浮点数存进去，top-2
                        }else{
                            printf("wrong!!!\n");
                        }
                    }
                }
                else
                {
                    error(12);  /* 赋值语句格式错误 */
                    i = 0;
                }
            }
            // 普通的variable
            else
            {
                getsymdo;
                if(sym == becomes || sym == pluseq || sym == minuseq)
                {
                	sym_op = sym;
                    getsymdo;
                }
                else
                {
                    error(13);  /* 没有检测到赋值/+=/-=符号 */
                }

                memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                expressiondo(nxtlev, ptx, lev); /* 处理赋值符号右侧表达式 */

                // 如果是+=/-=则需要先计算出结果
                if(sym_op == pluseq || sym_op == minuseq){
                	// 将变量取到栈顶
                	gendo(lod, lev-table[i].level, table[i].adr, 0);
                	if(sym_op == pluseq){
	                	// 次栈顶加上栈顶, 此时栈顶就是计算后的数
	                	gendo(opr, 0, 2, 0);
	                }
	                else if(sym_op == minuseq){
	                	// 次栈顶减去栈顶，但这是我们要的数的相反数
	                	gendo(opr, 0, 3, 0);
	                	// 取反, 此时栈顶就是计算后的数
	                	gendo(opr, 0, 1, 0);
	                }
                }
                
                // 判断以什么方式存储
                if(table[i].type=='i'){
                    // 按整型存入变量
                    gendo(sto, lev-table[i].level, table[i].adr, 0);
                }
                else if(table[i].type=='f'){
                	// 按浮点数存入变量
                    gendo(sto, lev-table[i].level, table[i].adr, 1);
                }else{
                    printf("wrong!!!\n");
                }
            }
        }//if (i == 0)
    }
    else
    {
        if (sym == readsym) /* 准备按照read语句处理 */
        {
            getsymdo;
            if (sym != lparen)
            {
                error(52);  /* 格式错误，应是左括号 */
            }
            else
            {
            	getsymdo;
            	// 如果是str，就是格式化输入
            	// 分别记录read里的ID数和格式化输入的个数
            	ReadNum = 0;
            	ReadFormat = 0;
            	if (sym == str)
                {
                    for(k=0 ; k<strmax ; k++){
                        if(str_[k]=='\0'){ break;}
                        // 预存入 虚拟机代码中
                        code[cx].str[k] = str_[k];
                        // 如果是格式化符号
                        if(str_[k]=='%'){
                            k++;	// 再读一位
                            code[cx].str[k] = str_[k];
                            switch(str_[k]){
                                case 'd':
                                	// do nothing
                                    break;
                                case 'f':
                                	// do nothing
                                    break;
                                default:
                                	error(53);			// 未知格式化类型
                                	break;
                            }
                            ReadFormat++;	// 格式化字符加1
                        }
                    }
                    code[cx].str[k] = 0;
                    gendo(opr, ReadFormat, 19, 0);  // 按要求读入并放到栈顶

                    getsymdo;       // 读出逗号
                    if (sym != comma){
                    	error(5);			// 漏掉了逗号
                    }
                }
                // 继续原来的输入方式
                do {
                    if(sym==comma){
                    	getsymdo;
					}
                    if (sym == ident)
                    {
//                    	printf("id=%s进来了, ptx=%d ",id,*ptx); 
                        i = position(id, *ptx); /* 查找要读的变量 */
//                        printf("i是%d\n",i); 
                        ReadNum++;		// ID数记录+1
                    }
                    else
                    {
                        i=0;
                    }
                    //printf("------------------------------------------------------%d",i); 

                    if (i == 0)
                    {
//                    	printf("sym=%d, ptx=%d ",sym,*ptx); 
//                    	printf("报这个错？？i=%d\n",i); 
                        error(35);  /* read()中应是声明过的变量名 */
                    }
                    // 读入数组
                    else if (table[i].kind != variable)
                    {
                        getsymdo;
                        if (sym == lbrakt) /* 索引是括号内的表达式 */
                        {
                            getsymdo;
                            memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                            nxtlev[rbrakt] = true;
                            expressiondo(nxtlev, ptx, lev);
                            if (sym != rbrakt)
                            {
                                error(48);  // 数组访问缺少右括号
                            }
                            else
                            {
                                // 控制台读入一个%f的数，放到栈顶, 只有当非格式化输入时有效
                                if (ReadFormat==0)
                                	gendo(opr, 0, 16, 0);
                                if(table[i].type=='i'){
                                	// 栈顶的值(按整型)存到 相对当前过程层差为L 数据偏移量为A+次栈顶的值 的存储单元, t-2
                                    gendo(sta, lev-table[i].level, table[i].adr, 0);   // 存进去，top-2
                                }
                                else if(table[i].type=='f'){
                                	// 栈顶的值(按浮点型)存到 相对当前过程层差为L 数据偏移量为A+次栈顶的值 的存储单元, t-2
                                    gendo(sta, lev-table[i].level, table[i].adr, 1);   // 存进去，top-2
                                }else{
                                    printf("wrong!!!\n");
                                }
                                getsymdo;
                            }
                        }
                        else
                        {
                            error(34);  /* read()参数表的标识符不是变量, thanks to amd */
                        }
                    }
                    else    // 当这个标识符是var, 即读入普通variable
                    {
                        // 控制台读入一个%f的数，放到栈顶, 只有当非格式化输入时有效
                        if (ReadFormat==0)
                        	gendo(opr, 0, 16, 0);  /* 生成输入指令，读取值到栈顶 */
                        if(table[i].type=='i'){
                            gendo(sto, lev-table[i].level, table[i].adr, 0);   // 存进去，top-1
                        }
                        else if(table[i].type=='f'){
                            gendo(sto, lev-table[i].level, table[i].adr, 1);   // 存进去，top-1
                        }else{
                            printf("wrong!!!\n");
                        }
                        getsymdo;
                    }
                    
                } while (sym == comma); /* 一条read语句可读多个变量 */
                if(ReadNum!=ReadFormat && ReadFormat!=0){
                	error(51);		// 格式化输入参数错误
                }
                if (ReadNum>readnum){
                	error(54);		// read一次读入不能超过10
                }
            }
            if(sym != rparen)
            {
                error(33);  /* 格式错误，应是右括号 */
                while (!inset(sym, fsys))   /* 出错补救，直到收到上层函数的后跟符号 */
                {
                    getsymdo;
                }
            }
            else
            {
                getsymdo;
            }
        }
        else
        {
            if (sym == writesym)    /* 准备按照write语句处理，与read类似 */
            {
                getsymdo;
                if (sym == lparen)
                {
                    getsymdo;                    
                    if(sym == str){
                        getsymdo;       // 读出逗号，或者直接读出了结尾的）
                        char tempStr[strmax];
                        // 初始化暂存str的数组
                        for(k=0 ; k<strmax ; k++){
                            tempStr[k] = '\0';
                        }
                        // 遍历str中每一个进行分析
                        for(k=0 ; k<strmax ; k++){
                            if(str_[k]=='\0'){ break;}
                            // 如果是格式化符号
                            if(str_[k]=='%'){
                                k++;
                                getsymdo;
                                if(sym!=ident){
                                    error(39);
                                }
                                memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                                nxtlev[rparen] = true;
                                nxtlev[comma] = true;       /* write的后跟符号为) or , */
                                expressiondo(nxtlev, ptx, lev); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
                                switch(str_[k]){
                                    case 'd':
                                        gendo(opr, 0, 18, 0);  // 将栈顶的值输出到控制台，top-1
                                        break;
                                    case 'f':
                                        gendo(opr, 0, 18, 1);  // 将栈顶的值输出到控制台，top-1
                                        break;
                                }
                            }
                            else    // 是其他符号
                            {
                                perSegIndex = 0;
                                do{
                                    tempStr[perSegIndex] = str_[k];
                                    perSegIndex++;
                                    k++;
                                }while(str_[k]!='%' && str_[k]!='\0' && k<strmax);
                                // 直到末尾或者格式化符号则停止
                                // 预存入 虚拟机代码中
                                for(j=0 ; j<perSegIndex ; j++){
                                    code[cx].str[j] = tempStr[j];
                                    tempStr[j] = 0;
                                }
                                gendo(opr, 0, 17, 0);  // 输出字符串的值
                                if(str_[k]=='%'){
                                    k--;
                                }
                            }
                        }
                        if (sym != rparen)
                        {
                            error(33);  /* write()中应为完整表达式 */
                        }
                        else
                        {
                            getsymdo;
                        }
                    }
                    else        // 只是一般的输出id
                    {
                        do {
                            if(sym == comma){
                                getsymdo;
                            }
                            memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                            nxtlev[rparen] = true;
                            nxtlev[comma] = true;       /* write的后跟符号为) or , */
                            expressiondo(nxtlev, ptx, lev); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
                            // 将栈顶的值输出到控制台，top-1
                            gendo(opr, 0, 14, 0);  /* 生成输出指令，输出栈顶的值 */
                        } while (sym == comma);
                        if (sym != rparen)
                        {
                            error(33);  /* write()中应为完整表达式 */
                        }
                        else
                        {
                            getsymdo;
                        }
                    }
                }
                // 输出一个换行
                // gendo(opr, 0, 15, 0);  /* 输出换行 */
            }
            else
            {
                if (sym == callsym) /* 准备按照call语句处理 */
                {
                    getsymdo;
                    if (sym != ident)
                    {
                        error(14);  /* call后应为标识符 */
                    }
                    else
                    {
                        i = position(id, *ptx);
                        if (i == 0)
                        {
                            error(11);  /* 过程未找到 */
                        }
                        else
                        {
                            if (table[i].kind == procedur)
                            {
                            	getsymdo;
								if(sym == lparen){
									// 记录参数个数
									param_num = 0;
									do{
										getsymdo;
										memcpy(nxtlev, fsys, sizeof(bool)*symnum);
										expressiondo(nxtlev,ptx,lev);
										param_num++;
									}while(sym == comma);

									if(sym ==rparen){
										getsymdo;
									}else{
										error(55);		// 过程传参语法错误
									}
								}else{
									error(55);		// 过程传参语法错误
								}
								if (param_num != table[i].val){
									error(56);		// 过程调用参数不一致
								}
								// 将栈顶A个元素往上移3个位置，为过程的链数据腾出位置，t-A
								gendo(mov, 0, param_num, 0);
                                // 用cal L A 调用地址为A的过程，这个在表里查，调用过程与被调用过程层差为L
                                // 这里的table[i].adr就是在code的相对位置, lev肯定大于等于table[i].level
                                gendo(cal, lev-table[i].level, table[i].adr, 0);   /* 生成call指令 */
                            }
                            else
                            {
                                error(15);  /* call后标识符应为过程 */
                            }
                        }
                    }
                }
                else
                {
                    if (sym == ifsym)   /* 准备按照if语句处理 */
                    {
                        getsymdo;
                        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                        nxtlev[thensym] = true;
                        nxtlev[dosym] = true;   /* 后跟符号为then或do */
                        conditiondo(nxtlev, ptx, lev); /* 调用条件处理（逻辑运算）函数 */
                        if (sym == thensym)
                        {
                            getsymdo;
                        }
                        else
                        {
                            error(16);  /* 缺少then */
                        }
                        cx1 = cx;   /* 保存当前指令地址 */
                        // jpc 0 A 条件跳转，若栈顶为1(满足条件) 则不跳转，栈顶为0(不满足条件)，则跳转至A处，这里是0，后面改
                        gendo(jpc, 0, 0, 0);   /* 生成条件跳转指令，跳转地址未知，暂时写0 */
                        statementdo(fsys, ptx, lev);    /* 处理then后的语句 */
                        code[cx1].a = cx;   /* 经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址 */
                    }
                    else
                    {
                        if (sym == beginsym)    /* 准备按照复合语句处理 */
                        {
                            getsymdo;
                            memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                            nxtlev[semicolon] = true;
                            nxtlev[endsym] = true;  /* 后跟符号为分号或end */
                            /* 循环调用语句处理函数，直到下一个符号不是语句开始符号或收到end */
                            statementdo(nxtlev, ptx, lev);

                            while (inset(sym, statbegsys) || sym==semicolon)
                            {
                                if (sym == semicolon)
                                {
                                    getsymdo;
                                }
                                else
                                {
                                    error(10);  /* 缺少分号 */
                                }
                                statementdo(nxtlev, ptx, lev);
                            }
                            if(sym == endsym)
                            {
                                getsymdo;
                            }
                            else
                            {
                                error(17);  /* 缺少end或分号 */
                            }
                        }
                        else
                        {
                            if (sym == whilesym)    /* 准备按照while语句处理 */
                            {
                                loopLevel++;
                                if (loopLevel>loopmax)
                                    error(50);        // 循环超过规定最大层数
                                cx1 = cx;   /* 保存判断条件操作的位置 */
                                getsymdo;
                                memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                                nxtlev[dosym] = true;   /* 后跟符号为do */
                                conditiondo(nxtlev, ptx, lev);  /* 调用条件处理 */
                                cx2 = cx;   /* 保存循环体的结束的下一个位置 */
                                // 这里也是 jpc 条件跳转，若栈顶为0(不满足条件)，则跳到下一句去，这里的A为0，后面也要改
                                gendo(jpc, 0, 0, 0);   /* 生成条件跳转，但跳出循环的地址未知 */
                                if (sym == dosym)
                                {
                                    getsymdo;
                                }
                                else
                                {
                                    error(18);  /* 缺少do */
                                }
                                statementdo(fsys, ptx, lev);    /* 循环体 */
                                // 先生成跳转回去再次判断while条件是否成立的代码
                                gendo(jmp, 0, cx1, 0); /* 回头重新判断条件 */
                                // 这里再改，此时的cx就是jmp 后面的一个虚拟机code，当然，具体是什么在这一句还不知道，还未生成
                                code[cx2].a = cx;   /* 反填跳出循环的地址，与if类似，在这里就是修改上边jpc 0 0 那个A值 */
                                
                                // 检查每一个break
                                while (breakCount)
                                {
                                    // 如果breakList顶层的break的level和当前loopLevel不相等, 说明当前层没有break
                                    if (breakList[breakCount - 1].level == loopLevel)
                                    {
                                        breakCount--;
                                        // 回填这个break的跳转地址
                                        code[breakList[breakCount].cx].a = cx;
                                    }
                                    else
                                        break;
                                }

                                loopLevel--;
                            }
                            else
                            {
                                if (sym == forsym){
                                    loopLevel++;
                                    if (loopLevel>loopmax)
                                        error(50);          // 循环超过规定最大层数
                                    getsymdo;
                                    if(sym == ident)    
                                    {
                                        i = position(id,*ptx);
                                        // 标识符未声明
                                        if(i==0)  error(11);
                                        // 找到标识符
                                        else{
                                            if(table[i].kind!=variable){
                                                error(12);      // 赋值左端应为变量
                                                i=0;
                                            } else{
                                                getsymdo;
                                                if(sym==becomes) {
                                                    getsymdo; 
                                                }else error(13);    // 应为赋值号

                                                expressiondo(nxtlev,ptx,lev); // 计算表达式的值
                                                // 其实这里的判断是多余的
                                                if(i!=0){
                                                    // 直接按整型存储
                                                    gendo(sto, lev-table[i].level, table[i].adr, 0);
                                                }
                                                cx1 = cx;
                                                if (sym == stepsym) 
                                                {
                                                    getsymdo;
                                                    expressiondo(nxtlev,ptx,lev); // 计算表达式(步长的值)，并放到栈顶
                                                    // 记下这个step的code生成，可能是opr(复数取反)，lit(正立即数)、lod(标识符的数)
                                                    which.f = code[cx-1].f;
                                                    which.l = code[cx-1].l;
                                                    which.a = code[cx-1].a;
                                                    // // 将循环变量取到栈顶，此时step的值在次栈顶
                                                    // gendo(lod, lev-table[i].level, table[i].adr, 0);
                                                    // gendo(opr,0,2);  //加步长
                                                    // gendo(sto,lev-table[i].level,table[i].adr);//存数
                                                }else error(42); // 缺step报错

                                                if(sym==untilsym)
                                                {
                                                    getsymdo;
                                                    // 将循环变量取到栈顶，此时step的值在下面，这里把变量取出来是为了判断比较
                                                    gendo(lod, lev-table[i].level, table[i].adr, 0);
                                                    expressiondo(nxtlev,ptx,lev);   // 计算表达式的值，也就是until后的值
                                                    // 这里的判断分两种情况, opr(复数取反)，lit(正立即数)、lod(标识符的数)
                                                    if (which.f==lit || which.f==lod){
                                                        // 如果是用标识符，则只支持正数
                                                        // 若 次栈顶(变量) ≤ 栈顶(until的值)，1存于次栈顶，top-1
                                                        gendo(opr, 0, 13, 0); 
                                                    }else if(which.f==opr){
                                                        // 若 次栈顶(变量) ≥ 栈顶(until的值)，1存于次栈顶，top-1
                                                        gendo(opr, 0, 11, 0);
                                                    }
                                                    cx2 = cx;    // 保存此时的cx(即jpc将要填入的地方)，便于回填jpc
                                                    // 若不满足，则跳转至A
                                                    gendo(jpc, 0, 0, 0);
                                                }else error(43);    // 缺until报错

                                                if(sym==dosym)  
                                                {
                                                    getsymdo;
                                                    statementdo(fsys,ptx,lev);//语句执行
                                                    // 将循环变量取到栈顶，此时step的值在下面，这里取出来是为了加步长
                                                    gendo(lod, lev-table[i].level, table[i].adr, 0);
                                                    gendo(opr, 0, 2, 0);  //加步长
                                                    // 存回去
                                                    gendo(sto, lev-table[i].level, table[i].adr, 0);
                                                    gendo(jmp, 0, cx1, 0);      // 跳回比较处
                                                    code[cx2].a = cx; // 回填jpc判断的跳跃地址
                                                }else error(44); // 缺do报错

                                            }
                                        }

                                    }else
                                        error(41);      // for后应为标识符

                                    // 检查每一个break
                                    while (breakCount)
                                    {
                                        // 如果breakList顶层的break的level和当前loopLevel不相等, 说明当前层没有break
                                        if (breakList[breakCount - 1].level == loopLevel)
                                        {
                                            breakCount--;
                                            // 回填这个break的跳转地址
                                            code[breakList[breakCount].cx].a = cx;
                                        }
                                        else
                                            break;
                                    }

                                    loopLevel--;
                                }
                                else
                                {
                                    if (sym == breaksym){
                                        // 只有当loopLevel为0时, 才会报错，说明不在循环内的地方使用了break语句
                                        if (!loopLevel)
                                            error(49);          // 报错: break不在循环语句内
                                        else
                                        {
                                            getsym();
                                            breakList[breakCount].cx = cx;
                                            breakList[breakCount].level = loopLevel;
                                            breakCount++;
                                            // 先生成jmp，在for里面回填
                                            gen(jmp, 0, 0, 0);
                                        }
                                    }
                                    else
                                    {
                                    	if (sym == exitsym){
                                    		// 直接生成end指令
                                    		gendo(end, 0, 0, 0);
                                    		getsym();
                                    	}else
	                                    {
	                                        memset(nxtlev, 0, sizeof(bool)*symnum); /* 语句结束无补救集合 */
	                                        // ***************************这里也是有问题的，基本全部改
	                                        testdo(fsys, nxtlev, 19);   /* 检测语句结束的正确性 */
	                                    }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/*
* 表达式处理: <表达式> ::= [+|-]<项>{<加减运算符><项>}
*/
int expression(bool* fsys, int* ptx, int lev)
{
    enum symbol addop;  /* 用于保存正负号 */
    bool nxtlev[symnum];

    if(sym==plus || sym==minus) /* 开头的正负号，此时当前表达式被看作一个正的或负的项 */
    {
        addop = sym;    /* 保存开头的正负号 */
        getsymdo;
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev);   /* 处理项 */
        if (addop == minus)
        {
            // 求栈顶元素的相反数，结果值留在栈顶，就是把第一个<项>改变符号
            gendo(opr, 0, 1, 0);
        }
    }
    else    /* 此时表达式被看作项的加减 */
    {
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev);   /* 处理项 */
    }
    while (sym==plus || sym==minus)
    {
        addop = sym;
        getsymdo;
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev);   /* 处理项 */
        if (addop == plus)
        {
            // opr 0 2 次栈顶和栈顶相加，结果放次栈顶，top-1
            gendo(opr, 0, 2, 0);   /* 生成加法指令 */
        }
        else
        {
            // opr 0 3 次栈顶-栈顶，结果放次栈顶，top-1
            gendo(opr, 0, 3, 0);   /* 生成减法指令 */
        }
    }
    return 0;
}

/*
* 项处理： <项> ::= <因子>{<乘除运算符><因子>}
*/
int term(bool* fsys, int* ptx, int lev)
{
    // 用于保存乘除法符号
    enum symbol mulop;
    bool nxtlev[symnum];

    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
    nxtlev[times] = true;
    nxtlev[slash] = true;
    // 处理因子
    factordo(nxtlev, ptx, lev);
    while(sym==times || sym==slash)
    {
        mulop = sym;
        getsymdo;
        factordo(nxtlev, ptx, lev);
        if(mulop == times)
        {
            // 次栈顶×栈顶，结果存次栈顶，top-1
            gendo(opr, 0, 4, 0);   /* 生成乘法指令 */
        }
        else
        {
            // 次栈顶÷栈顶，结果存次栈顶，top-1
            gendo(opr, 0, 5, 0);   /* 生成除法指令 */
        }
    }
    return 0;
}

/*
* 因子处理: <因子> ::= <id>|<integer>|'('<表达式>')'
*/
int factor(bool* fsys, int* ptx, int lev)
{
    int i;
    bool nxtlev[symnum];
    // ********************FIRST没问题，fsys同样要改
    testdo(facbegsys, fsys, 24);    /* 检测因子的开始符号 */
    /* while(inset(sym, facbegsys)) */  /* 循环直到不是因子开始符号 */
    if(inset(sym,facbegsys))
    {
        if(sym == ident)    /* 因子为常量或变量 */
        {
            i = position(id, *ptx); /* 查找名字 */
            if (i == 0)
            {
                error(11);  /* 标识符未声明 */
            }
            else
            {
                switch (table[i].kind)
                {
                case constant:  /* 名字为常量 */
                    // lit 0 A 将A存到栈顶，top+1
                    if (table[i].type=='i'){
                        gendo(lit, 0, table[i].val, 0);    /* 直接把常量的值入栈 */
                    }
                    else if(table[i].type=='f'){
                        gendo(lit, 1, 0, table[i].valf);    /* 直接把常量的值入栈 */
                    }
                    break;
                case variable:  /* 名字为变量 */
                    // 取相对当前过程 层差为L 数据偏移量为A 的存储单元的值到栈顶, top+1
                    gendo(lod, lev-table[i].level, table[i].adr, 0);   /* 找到变量地址并将其值入栈 */
                    break;
                case procedur:  /* 名字为过程 */
                    error(21);  /* 不能为过程 */
                    break;
                case array:
                    getsymdo;
                    if (sym == lbrakt) /* 索引是括号内的表达式 */
                    {
                        getsymdo;
                        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                        nxtlev[rbrakt] = true;
                        expressiondo(nxtlev, ptx, lev);
                        if (sym == rbrakt)
                        {
                        	// 取相对当前过程 层差为L 数据偏移量为A+栈顶的值 的存储单元的值覆盖当前栈顶, t不变
                            gendo(lda, lev-table[i].level, table[i].adr, 0);
                        }
                        else
                        {
                            error(48);  // 数组访问缺少右括号
                        }
                    }
                    else
                    {
                        error(47);  /* 数组访问错误 */
                    }
                    break;
                }
            }
            getsymdo;
        }
        else
        {
            if(sym == number)   /* 因子为数 */
            {
                // num是值，amax是地址上界，很奇怪**************
                if (num > amax)
                {
                    error(31);
                    num = 0;
                }
                // 也就是一个常量，直接放在栈顶
                gendo(lit, 0, num, 0);
                getsymdo;
            }
            // 如果是浮点数 real
			else if(sym == real)
	        {
	        	// 也就是一个常量，直接放在栈顶
                gendo(lit, 1, 0, float_);
                printf("%d %f",num,float_);
                getsymdo;
			}
            else
            {
                if (sym == lparen)  /* 因子为表达式 */
                {
                    getsymdo;
                    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                    nxtlev[rparen] = true;
                    expressiondo(nxtlev, ptx, lev);
                    if (sym == rparen)
                    {
                        getsymdo;
                    }
                    else
                    {
                        error(22);  /* 缺少右括号 */
                    }
                }
                // *****************改
                testdo(fsys, facbegsys, 23);    /* 因子后有非法符号 */
            }
        }
    }
    return 0;
}

/*
* 条件处理: <条件> ::= <表达式><关系运算符><表达式>|odd <表达式>
*/
int condition(bool* fsys, int* ptx, int lev)
{
    enum symbol relop;
    bool nxtlev[symnum];

    if(sym == oddsym)   /* 准备按照odd运算处理 */
    {
        getsymdo;
        expressiondo(fsys, ptx, lev);
        // 栈顶的值，若是奇数则变为1，偶数则变为0
        gendo(opr, 0, 6, 0);   /* 生成odd指令 */
    }
    else
    {
        /* 逻辑表达式处理 */
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[eql] = true;
        nxtlev[neq] = true;
        nxtlev[lss] = true;
        nxtlev[leq] = true;
        nxtlev[gtr] = true;
        nxtlev[geq] = true;
        expressiondo(nxtlev, ptx, lev);
        if (sym!=eql && sym!=neq && sym!=lss && sym!=leq && sym!=gtr && sym!=geq)
        {
            error(20);
        }
        else
        {
            relop = sym;
            getsymdo;
            expressiondo(fsys, ptx, lev);
            // 二元比较指令
            switch (relop)
            {
            case eql:
                gendo(opr, 0, 8, 0);       // 相等让次栈顶为1, top-1
                break;
            case neq:
                gendo(opr, 0, 9, 0);       // 不相等让次栈顶为1, top-1
                break;
            case lss:
                gendo(opr, 0, 10, 0);      // 小于则让次栈顶为1, top-1
                break;
            case geq:
                gendo(opr, 0, 11, 0);      // ≥ 则让次栈顶为1, top-1
                break;
            case gtr:
                gendo(opr, 0, 12, 0);      // 大于则让次栈顶为1, top-1
                break;
            case leq:
                gendo(opr, 0, 13, 0);      // ≤ 则让次栈顶为1, top-1
                break;
            }
        }
    }
    return 0;
}
