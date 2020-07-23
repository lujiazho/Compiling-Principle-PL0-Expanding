# Compiling-Principle-PL-0-Expanding
编译原理大作业  
1、PL/0语言扩展（包括I/O、浮点、for循环、数组、复合运算、过程传参、注释、乱序声明、exit等）  
基本功能扩展
  I/O 功能的扩展：格式化输入、格式化输出。
  数据结构的扩展：增加浮点数、一维数组。
  计算功能的扩展：增加对浮点数运算、一维数组运算（关系运算、加、减、乘、除）。
  控制逻辑的扩展：增加for循环，以及适用于for循环的break语句。
  个人想法的扩展：允许常量/变量/过程的无序声明，增加行注释和块注释，过程允许传值参，增加<exit语句>退出程序，增加复合运算(+=、-=)。
2、基于editplus的IDE配置  
3、包含详细的注释  
4、程序分为编译器（pl0c_ansi.c）和解释器（pl0i_ansi.c）以及pl0.h文件，前二编译后可直接配置在editplus  
5、语法高亮和自动补全相关文件也可直接配置到editplus  
6、测试文件

# 系统体系结构
![image](https://github.com/leaving-voider/Compiling-Principle-PL0-Expanding/blob/master/ScreenShot/architecture.png)

# 系统运行截图
![image](https://github.com/leaving-voider/Compiling-Principle-PL0-Expanding/blob/master/ScreenShot/systemDisplay.png)

# EBNF 语法描述（含扩充：黑色部分是原有版本，红色部分是添加部分）
![image](https://github.com/leaving-voider/Compiling-Principle-PL0-Expanding/blob/master/ScreenShot/EBNF.png)

# 类 P-code 虚拟机指令系统（含扩充:黑色部分是原有版本，红色部分是添加部分）
![image](https://github.com/leaving-voider/Compiling-Principle-PL0-Expanding/blob/master/ScreenShot/P-code.png)

# 基于 EditPlus 的 PL/0 语言集成开发环境配置
### 1、在 EditPlus 中分别点击 Tools -> Preferences -> User tools，然后添加 PL/0 编译器和 PL/0 解释器，添加完成如下图所示
![image](https://github.com/leaving-voider/Compiling-Principle-PL0-Expanding/blob/master/ScreenShot/%E5%9B%BE%E7%89%871.png)

### 2、将EditPlus配置文件，导入EditPlus文件夹下，并在EditPlus的Settings & syntax中进行配置；其中，acp表示auto complete自动补全，stx表示syntax语法高亮；配置完成如下图所示
![image](https://github.com/leaving-voider/Compiling-Principle-PL0-Expanding/blob/master/ScreenShot/%E5%9B%BE%E7%89%872.png)

### 3、可根据需要修改语法高亮的颜色，如下图
![image](https://github.com/leaving-voider/Compiling-Principle-PL0-Expanding/blob/master/ScreenShot/%E5%9B%BE%E7%89%873.png)

### 4、当然，还可以修改acp配置文件，添加自己想要的补全方式，以下图为例子
![image](https://github.com/leaving-voider/Compiling-Principle-PL0-Expanding/blob/master/ScreenShot/%E5%9B%BE%E7%89%874.png)
当我们输入for并接着按下空格时，就会直接出现下面3段语句，并把指针指在^!所在位置处。
