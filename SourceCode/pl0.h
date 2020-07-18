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

typedef enum {
    false,
    true
} bool;


#define norw 19     /* 关键字个数 */
#define txmax 100   /* 名字表容量 */
#define nmax 14     /* number的最大位数 */
#define al 10       /* 符号的最大长度 */
#define amax 2047   /* 地址上界*/
#define levmax 3    /* 最大允许过程嵌套声明层数 [0,  levmax]*/
#define cxmax 500   /* 最多的虚拟机代码数 */
#define strmax 100  // 字符串最大长度, 只能为偶数
#define loopmax 10  // 循环最多层数
#define readnum 10  // read一次最多读入的数

/* 符号 */
// 定义枚举类型，第一个枚举成员的默认值为整型的 0，后续枚举成员的值在前一个成员上加 1，以此类推
enum symbol {
	/*不能识别、名字(标识符ident)、整数数字number*/
    nul,/*0unknown*/ident,/*1*/     number,/*2*/    

	/*11单字符(不包括>和<) + 5双字符(>,>=,<,<=,:=) + odd保留字*/
    plus,/*3*/      minus,/*4*/
    times,/*5*/     slash,/*6 /*/   oddsym,/*7*/    eql,/*8*/       neq,/*9*/
    lss,/*10 <*/    leq,/*11 <=*/   gtr,/*12 >*/    geq,/*13 >=*/   lparen,/*14 (*/
    rparen,/*15 )*/ comma,/*16*/    semicolon,/*17*/period,/*18*/   becomes,/*19 赋值:=*/
    
	/*12保留字*/
    beginsym,/*20*/ endsym,/*21*/   ifsym,/*22*/    thensym,/*23*/  whilesym,/*24*/
    writesym,/*25*/ readsym,/*26*/  dosym,/*27*/    callsym,/*28*/  constsym,/*29*/
    varsym,/*30*/   procsym,/*31*/	

	/*增加的: 1个浮点数，1个单字符冒号，一个浮点声明保留字, 1个字符串, 三个for循环保留字, 2个中括号单字符, break保留字,
            +=/-+两个双字符, 一个exit语句保留字*/
    real,/*32*/     colon,/*: 33*/  realsym,/*34*/  str,/*35*/      forsym,/*36*/
    stepsym,/*37*/  untilsym,/*38*/ lbrakt,/*[39*/  rbrakt,/*]40*/  breaksym,/*41*/
    pluseq,/*42+=*/ minuseq,/*43-=*/exitsym,/*44*/
};
// 符号共45个:分为19保留字、14单字符(不包括>和<)、7双字符(>,>=,<,<=,:=,+=,-=)以及另外5个: 不能识别、名字(标识符ident)、数字、浮点数、字符串
#define symnum 45   

/* 名字表中的类型 */
enum object {
    constant,
    variable,
    procedur,
    array       //add
};

/* 虚拟机代码 */
enum fct {
    lit,     opr,     lod,
    sto,     cal,     inte,
    jmp,     jpc,     nan,
    lda,     sta,     lpl,
    lmi,     end,     mov,
};
#define fctnum 15

/* 虚拟机代码结构 */
struct instruction
{
    enum fct f; /* 虚拟机代码指令 */
    int l;      /* 引用层与声明层的层次差 */
    int a;      /* 根据f的不同而不同 */
    float real;     // 仅当涉及到浮点数变量时使用
    char str[strmax];   // 仅涉及字符串、格式化read时使用
};

struct BREAKLIST
{
    int cx;         // 存放当前break生成的jmp在code中的位置，便于回写
    int level;      // 存放当前break所在循环层数
};

FILE* virCode;   /* 输出虚拟机代码 */
char ch;            /* 获取字符的缓冲区，getch 使用 */
// 定义枚举变量，可用已有的类型进行赋值, 存放当前符号的标识
enum symbol sym;    /* 当前的符号 */
enum symbol sym_op;    /* +=/-=里暂存符号 */
// 存放标识符
char id[al+1];      /* 当前ident, 多出的一个字节用于存放0 */
int num;            /* 当前number */
float float_;       // 当前real
char str_[strmax];  // 存放当前字符串
char format_[strmax/2];     // 存放当前字符串中格式化符号
float tempfloat[readnum];   // 临时存放解释器中read scanf进来的数据
char tempstr[strmax] = "";  // 在解释其中暂存字符串
int index_temp = 0;         // 专用于给 tempstr赋值
// char str_segs[strmax][strmax];      // 存放格式化中各分段的其他直接输出的字符
int arrSize = 0;    // 用于存放当前array的size
int loopLevel = 0;  // 用于记录当前处于for或while的循环第几层, 0就代表没进入
struct BREAKLIST breakList[loopmax];        // 保存循环中break的信息
int breakCount = 0; // 记录在进入当前循环的第一层之后, 已经累计有多少个break了
int ReadNum = 0;    // 记录格式化读入里面有多少个标识符
int ReadFormat = 0; // 记录格式化输入里的格式化符号个数
int save_dx;        // 过程传参部分使用的辅助变量, 存放之前的dx值
int param_num = 0;   // 记录传参参数个数
int cc, ll;          /* getch使用的计数器，cc表示当前字符(ch)的位置 */
int cx;             /* 虚拟机代码指针, 取值范围[0, cxmax-1]*/
char line[81];      /* 读取行缓冲区 */
// 存放读取的一个词
char a[al+1];       /* 临时符号, 多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
// norw: 保留字的个数为13，al: 符号的最大长度为10
char word[norw][al];        /* 保留字 */
// 定义枚举变量，可用已有的类型进行赋值
enum symbol wsym[norw];     /* 保留字对应的符号值 */
enum symbol ssym[256];      /* 单字符的符号值 */
// 类P_code语言指令，8个
char mnemonic[fctnum][5];   /* 虚拟机代码指令名称 */
bool declbegsys[symnum];    /* 表示声明开始的符号集合 */
bool statbegsys[symnum];    /* 表示语句开始的符号集合 */
bool facbegsys[symnum];     /* 表示因子开始的符号集合 */

/* 名字表结构 */
struct tablestruct
{
    char name[al];      /* 名字 */
    enum object kind;   /* 类型：const, var, array or procedure */
    int val;            /* 数值，const使用记录值，procedure记录参数个数 */
    float valf;         // 浮点，仅const使用
    int level;          /* 所处层，仅const不使用 */
    int adr;            /* 地址，仅const不使用，因为常量的值val可直接存在虚拟机代码的A里面 */
    int size;           /* 需要分配的数据区空间, procedure和array使用 */
	// 增加type, i表示整型, f表示浮点
	char type;
};

struct tablestruct table[txmax]; /* 名字表 */

FILE* Pl0_SourceCode; // 存储所读取文件的 文件指针, 该指针指向该文件
// 计算已发现的错误个数
int err; /* 错误计数器 */

/* 当函数中会发生fatal error时，返回-1告知调用它的函数，最终退出程序 */
// 宏定义函数，编译时，识别define是以回车组为结束的
#define getsymdo                      if(-1 == getsym()) return -1
#define getchdo                       if(-1 == getch()) return -1
#define testdo(a, b, c)               if(-1 == test(a, b, c)) return -1
#define gendo(a, b, c, d)             if(-1 == gen(a, b, c, d)) return -1
#define expressiondo(a, b, c)         if(-1 == expression(a, b, c)) return -1
#define factordo(a, b, c)             if(-1 == factor(a, b, c)) return -1
#define termdo(a, b, c)               if(-1 == term(a, b, c)) return -1
#define conditiondo(a, b, c)          if(-1 == condition(a, b, c)) return -1
#define statementdo(a, b, c)          if(-1 == statement(a, b, c)) return -1
#define arraydeclarationdo(a, b, c)   if(-1 == arraydeclaration(a, b, c)) return -1
#define constdeclarationdo(a, b, c)   if(-1 == constdeclaration(a, b, c)) return -1
#define vardeclarationdo(a, b, c)     if(-1 == vardeclaration(a, b, c)) return -1

/*
    error: 编码对应错误
    30: 数字过长
    9: 程序结尾不是 .(period), 即没有正常结束
*/
void error(int n);
int getsym();
int getch();
void init();
int gen(enum fct x, int y, int z, float real);
int test(bool* s1, bool* s2, int n);
int inset(int e, bool* s);
int addset(bool* sr, bool* s1, bool* s2, int n);
int subset(bool* sr, bool* s1, bool* s2, int n);
int mulset(bool* sr, bool* s1, bool* s2, int n);
int block(int lev, int tx, bool* fsys, int thisParamNum);
void interpret();
int factor(bool* fsys, int* ptx, int lev);
int term(bool* fsys, int* ptx, int lev);
int condition(bool* fsys, int* ptx, int lev);
int expression(bool* fsys, int* ptx, int lev);
int statement(bool* fsys, int* ptx, int lev);
void listcode(int cx0);
int arraydeclaration(int* ptx, int lev, int* pdx);
int vardeclaration(int* ptx, int lev, int* pdx);
int constdeclaration(int* ptx, int lev, int* pdx);
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int lev, int* pdx);
int base(int l, float* s, int b);
void StoreToFile();
void print_nameTable(int tx0, int tx);
