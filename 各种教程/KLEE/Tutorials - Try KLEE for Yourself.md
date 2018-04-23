# [Try KLEE for Yourself](http://klee.github.io/releases/docs/v1.4.0/tutorials/)

### 环境: 
	Ubuntu 14.04 64-bit
	KLEE v1.4.0 (LLVM 3.4)

> 文章大标题那个参考网址 ~~是个假网址~~ 有部分错误, 有些命令请参考[此网址](https://klee.github.io/tutorials/testing-function/)  
> 请确保 `clang`, `klee`, `ktest-tools` 等命令都可以使用, 并已链接到系统目录下, 不然敲命令时需要带上命令所在目录  
> 关于如何构建KLEE请参考 [Building KLEE with LLVM 3.4](https://github.com/Eeylx/MaydayMaydayMayday/blob/master/%E5%90%84%E7%A7%8D%E6%95%99%E7%A8%8B/Building%20KLEE%20with%20LLVM%203.4/Building%20KLEE%20with%20LLVM%203.4.md)  


## 目录:
[01. First Tutorial](#01-firsttutorial-http-klee-github-io-releases-docs-v1-4-0-tutorials-testing-function-)  
[02. Second Tutorial]  
[03. Solving a maze with KLEE]  
[04. Keygenning with KLEE and Hex-Rays]  
[05. Keygenning With KLEE]  
[06. Testing Coreutils](#06-testing-coreutils-http-klee-github-io-releases-docs-v1-4-0-tutorials-testing-coreutils-)  
[附录1: klee常用函数](#-1-klee-)  
[附录2: klee常用命令](#-2-klee-)  
[附录3: linux编译器相关常用命令](#-3-linux-)  


## 01. [FirstTutorial](http://klee.github.io/releases/docs/v1.4.0/tutorials/testing-function/)
测试一个简单的程序.(使用klee的简单的整个流程)

示例在 `klee/examples` 目录下, 本例子使用 `klee/examples/get_sign`目录下的 `get_sign.c`
```
/*
 * First KLEE tutorial: testing a small function
 */

#include <klee/klee.h>

int get_sign(int x) {
  if (x == 0)
     return 0;
  
  if (x < 0)
     return -1;
  else 
     return 1;
} 

int main() {
  int a;
  klee_make_symbolic(&a, sizeof(a), "a");
  return get_sign(a);
} 
```


#### Marking input as symbolic
为了使用KLEE测试这个函数, 我们需要将变量标记为符号, 使用 `klee_make_symbolic()` 函数, 该函数接收有三个参数: 
  + 看作符号的变量地址
  + 变量大小
  + 变量名称(任意)

具体例子如上述示例代码的main函数中所示


#### Compiling to LLVM bitcode
	clang -I ../../include -emit-llvm -c -g get_sign.c

  + `-I` : 使编译器可以找到klee/klee.h  
  + `-c` : 只将代码编译到目标文件, 而不是可执行文件
  + `-g` : additional source-level debug information to be stored in the object file, KLEE use to determine source line number information
  + 不使用任何优化标志, 代码可以稍后进行优化, klee提供了 `--optimize`命令赖在内部运行优化器 

> 以上参数可以使用 `clang --help` 命令查看说明

该命令会生成 `get_sign.bc` 文件

> 如果使用LLVM2.9构建KLEE, 请使用 `llvm-gcc` 替换 `clang`  
> `llvm-gcc` 会生成 .o 文件(例如 `get_sign.o` ), 所以后续步骤也需要相应的修改命令


#### Running KLEE
	klee get_sign.bc

可以看到以下输出: 

	KLEE: output directory is "/home/eeyore/work/klee/examples/get_sign/klee-out-0"
	KLEE: Using STP solver backend
		
	KLEE: done: total instructions = 31
	KLEE: done: completed paths = 3
	KLEE: done: generated tests = 3

  + KLEE在程序中探索了三条路径(=0, <0, >0), 并为每条探索路径生成了一个测试用例  
  + KLEE执行后的输出是一个包含KLEE生成的测试用例的目录(klee-out-0)
  + KLEE将输出的目录命名为 `klee-out-N` , 其中N是最新的可用数字, 并且生成一个名为 `klee-last` 的符号链接到这个目录

如果你想了解由KLEE生成的文件概况, 请点击[这里](http://klee.github.io/releases/docs/v1.4.0/docs/files/), 本教程只关注由KLEE生成的实际测试文件


#### KLEE-generated test cases
KLEE生成的测试用例被写入扩展名为 `.ktest` 的文件中, 这些是二进制文件, 可以用 `ktest-tool` 工具读取

    ktest-tool --write-ints klee-last/test000001.ktest

可以看到以下输出: 

    ktest file : 'klee-last/test000001.ktest'
    args       : ['get_sign.bc']
    num objects: 1
    object    0: name: 'a'
    object    0: size: 4
    object    0: data: 0

每个测试文件中, KLEE都会记录以下参数
  + 被调用的程序 (get_sign.bc)
  + 该路径上的符号对象数量 (1个)
  + 符号对象的名字 (a)
  + 符号对象的大小 (4)
  + 数据值 (0)

可以用同样的方法查看 `test000002.ktest` 和 `test000003.ktest` 文件, 可以看到三个测试用例中符号对象a的数据值分别为 0, 16843009, -2147483648, 正好覆盖了三条路径  

我们可以在实际程序上运行这些测试用例


#### Replaying a test case
虽然可以手动运行由KLEE生成的测试用例(或者在现有测试框架的帮助下), 但是KLEE提供了一个方便的重放库, 可以将.ktest文件中对应的变量值赋给函数中由klee_make_symbolic函数指定的符号

要使用它, 只需将程序与libkleeRuntest库连接起来, 并将环境变量 `KTEST_FILE` 设置为要使用的测试用例的名称

    // 该版本为 KLEE v1.4.0 官方版本, 但不能正常使用
    export LD_LIBRARY_PATH=/path-to-your-klee-build-dir/Release+Debug+Asserts/lib/:$LD_LIBRARY_PATH
    gcc -L /home/eeyore/work/klee-build/Release+Debug+Asserts/lib/ get_sign.c -lkleeRuntest

    // 该版本为官网另外一份教程的版本, 可以正常使用
    // 另使用该命令时需要修改 get_sign.c 文件中 include 的路径使其可以找到 klee.h 文件
    // klee.h 文件在 your-path-to-klee-dir/include/klee目录下
    export LD_LIBRARY_PATH=/path-to-your-klee-build-dir/lib/:$LD_LIBRARY_PATH
    gcc -L /home/eeyore/work/klee-build/lib/ get_sign.c -lkleeRuntest

执行完善上述命令后, 即可选择测试用例: 

    KTEST_FILE=klee-last/test000001.ktest ./a.out 
    echo $?
    0                                               // 输出结果
    KTEST_FILE=klee-last/test000002.ktest ./a.out 
    echo $?
    1                                               // 输出结果
    KTEST_FILE=klee-last/test000003.ktest ./a.out 
    echo $?
    255                                             // 输出结果
 
就像预期的那样, 程序在运行三个测试用例的时候分别返回了0, 1 和 -1(255)


## 02. [SecondTutorial](http://klee.github.io/releases/docs/v1.4.0/tutorials/testing-regex/)
测试一个简单的正则表达式库.(如何使用klee_assume())


## 03. [Solving a maze with KLEE](https://feliam.wordpress.com/2010/10/07/the-symbolic-maze/)
一个解释符号执行如何产生一些很有意思的程序输入的例子. 这个例子展示了如何使用 klee 在一个迷宫游戏里找到所有的解决方案


## 04. [Keygenning with KLEE and Hex-Rays](https://gitlab.com/Manouchehri/Matryoshka-Stage-2/blob/master/stage2.md)
一个初级的解释如何使用符号执行来解决一个很小的二进制代码的例子(没看懂)


## 05. [Keygenning With KLEE](https://doar-e.github.io/blog/2015/08/18/keygenning-with-klee/)
使用KLEE解决规模更大的二进制文件的深入指导(反编译一款软件, 用klee生成符合条件的序列号)


## 06. [Testing Coreutils](http://klee.github.io/releases/docs/v1.4.0/tutorials/testing-coreutils/)
详细介绍如何使用KLEE测试GNU Coreutils

### Step 1 : Build coreutils with gcov
首先去下个 [coreutils](http://www.gnu.org/software/coreutils/coreutils.html), 推荐下载 [6.11版本(官网教程版本)](http://ftp.gnu.org/gnu/coreutils/).

在使用 LLVM 构建之前, 先构建一个带有 gcov 支持的 coreutils, 稍后将用它来获取 KLEE 生成的测试用例的覆盖信息. 解压并进入刚才下载的 coreutils 目录(coreutils-6.11), 然后执行以下命令 :

    coreutils-6.11$ mkdir obj-gcov
    coreutils-6.11$ cd obj-gcov
    obj-gcov$ ../configure --disable-nls CFLAGS="-g -fprofile-arcs -ftest-coverage"
    ... verify that configure worked ...
    obj-gcov$ make
    obj-gcov$ make -C src arch hostname
    ... verify that make worked ...

使用 `--disable-nls` 参数是因为在C代码库中有许多额外的初始化但是我们并不想测试它们. 即使这些不是 KLEE 将要运行的可执行文件, 但我们希望使用相同的编译器标志, 以便在未安装的二进制文件上运行时, KLEE 生成的测试用例最有可能正常工作

你现在应该在 `objc-gcov/src` 目录下有一组 coreutils. 例如 : 

    obj-gcov$ cd src
    src$ ls -l ls echo cat
    -rwxrwxr-x 1 klee klee 150632 Nov 21 21:58 cat
    -rwxrwxr-x 1 klee klee 135984 Nov 21 21:58 echo
    -rwxrwxr-x 1 klee klee 390552 Nov 21 21:58 ls
    
    src$ ./cat --version
    cat (GNU coreutils) 6.11
    Copyright (C) 2008 Free Software Foundation, Inc.
    License GPLv3+: GNU GPL version 3 or later
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.

    Written by Torbjorn Granlund and Richard M. Stallman.

另外, 这些可执行文件应该用 gcov 支持来构建, 所以在运行时会把 `.gcda` 写入当前目录. 该文件包含有关在程序运行时确切执行哪些代码的信息. 有关更多信息请参阅[Gcov文档](http://gcc.gnu.org/onlinedocs/gcc/Gcov.html). 我们可以使用 gcov 工具本身来生成覆盖率信息的可读形式. 例如 : 

    src$ rm -f *.gcda # Get rid of any stale gcov files
    src$ ./echo**
    
    src$ ls -l echo.gcda
    -rw-rw-r-- 1 klee klee 896 Nov 21 22:00 echo.gcda

    src$ gcov echo
    File '../../src/echo.c'
    Lines executed:24.27% of 103
    Creating 'echo.c.gcov'
    
    File '../../src/system.h'
    Lines executed:0.00% of 3
    Creating 'system.h.gcov'

默认情况下, gcov将显示程序中执行的行数(.h文件中的代码会被编辑到echo.c中)

### Step 2 : Install WLLVM
使用klee测试实际程序的一个难点是实际程序必须被编译成llvm bitcode文件而不是二进制文件. 对于CoreUtils, 我们使用[whole-program-llvm(WLLVM)](https://github.com/travitch/whole-program-llvm), 它提供了工具可以将c/c++源代码构建成完整程序的llvm bitcode文件.

wllvm提供了4个python可执行文件:
+ wllvm
  - c编译器
+ wllvm++
  - c++编译器
+ extract-bc
  - 用于从构建好的程序(目标文件, 可执行文件, 库或归档文件)中提取bitcode
+ wllvm-sanity-checker
  - 用于检测配置的疏漏
  
本教程中使用的wllvm版本是`1.0.17`, 安装wllvm : 

    pip install --upgrade wllvm

要想成功执行wllvm, 必须将环境变量`LLVM_COMPILER=clang`设置为底层llvm编译器(dragonegg或者clang), 在本教程中使用clang : 

    export LLVM_COMPILER=clang

要使环境变量持久化, 请将上述命令添加到shell脚本中(例如.bashrc等)

### Step 3 : Build Coreutils with LLVM
我们将在一个单独的目录中构建Coreutils, 以便我们可以轻松访问可执行版本和llvm版本 : 

    coreutils-6.11$ mkdir obj-llvm
    coreutils-6.11$ cd obj-llvm
    obj-llvm$ CC=wllvm ../configure --disable-nls CFLAGS="-g"
    ... verify that configure worked ...
    obj-llvm$ CC=wllvm make
    obj-llvm$ CC=wllvm make -C src arch hostname
    ... verify that make worked ...

这里我们做了两个更改, 首先我们不想在要用klee测试的二进制文件中添加gcov工具, 所以我们没有使用`-fprofile-arcs -ftest-coverage`参数, 其次在运行make时, 我们设置CC变量指向wllvm

如果一切顺利, 你现在应该已经有了Coreutils的可执行文件, 例如 : 

    obj-llvm$ cd src
    src$ ls -l ls echo cat
    -rwxrwxr-x 1 klee klee 105448 Nov 21 12:03 cat
    -rwxrwxr-x 1 klee klee  95424 Nov 21 12:03 echo
    -rwxrwxr-x 1 klee klee 289624 Nov 21 12:03 ls
    src$ ./cat --version
    cat (GNU coreutils) 6.11
    Copyright (C) 2008 Free Software Foundation, Inc.
    License GPLv3+: GNU GPL version 3 or later
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.

    Written by Torbjorn Granlund and Richard M. Stallman.

你可能会注意到我们获得的是可执行文件而不是LLVM的bitcode文件, 这是因为WLLVM分两步工作. WLLVM首先调用标准编译器, 然后对每个obj文件调用bitcode编译器生成LLVM的bitcode文件. WLLVM stores the location of the generated bitcode files in a dedicated section of the object file. When object files are linked together, the locations are concatenated to save the locations of all constituent files. 当构建完成后, 可以使用WLLVM的实用工具extract-bc读取专用部分的内容, 并将所有bitcode链接到单个的完整bitcode文件中. 

要获得所有Coreutils的LLVM bitcode版本，我们可以在所有可执行文件上调用extract-bc : 

    // ln -s /usr/bin/llvm-link-3.4 /usr/bin/llvm-link
    src$ find . -executable -type f | xargs -I '{}' extract-bc '{}'
    src$ ls -l ls.bc
    -rw-rw-r-- 1 klee klee 543052 Nov 21 12:03 ls.bc

### Step 4 : Using KLEE as an interpreter
下面的例子是如何使用KLEE运行与之前相同的cat命令. 
> 这一步你需要使用uclibc和POSIX参数, 如果之前跳过了相关步骤, 现在需要补上T^T.  
> 链接: [(Optional) Build uclibc and the POSIX environment model](https://github.com/Eeylx/MaydayMaydayMayday/blob/master/%E5%90%84%E7%A7%8D%E6%95%99%E7%A8%8B/KLEE/Building%20KLEE%20with%20LLVM%203.4.md#4-optional-build-uclibc-and-the-posix-environment-model)

    src$ klee --libc=uclibc --posix-runtime ./cat.bc --version
    KLEE: NOTE: Using klee-uclibc : /usr/local/lib/klee/runtime/klee-uclibc.bca
    KLEE: NOTE: Using model: /usr/local/lib/klee/runtime/libkleeRuntimePOSIX.bca
    KLEE: output directory is "/home/klee/coreutils-6.11/obj-llvm/src/./klee-out-0"
    Using STP solver backend
    KLEE: WARNING ONCE: function "vasnprintf" has inline asm
    KLEE: WARNING: undefined reference to function: __ctype_b_loc
    KLEE: WARNING: undefined reference to function: klee_posix_prefer_cex
    KLEE: WARNING: executable has module level assembly (ignoring)
    KLEE: WARNING ONCE: calling external: syscall(16, 0, 21505, 42637408)
    KLEE: WARNING ONCE: calling __user_main with extra arguments.
    KLEE: WARNING ONCE: calling external: getpagesize()
    KLEE: WARNING ONCE: calling external: vprintf(43649760, 51466656)
    cat (GNU coreutils) 6.11

    License GPLv3+: GNU GPL version 3 or later
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.

    Written by Torbjorn Granlund and Richard M. Stallman.
    Copyright (C) 2008 Free Software Foundation, Inc.
    KLEE: WARNING ONCE: calling close_stdout with extra arguments.

    KLEE: done: total instructions = 28988
    KLEE: done: completed paths = 1
    KLEE: done: generated tests = 1

{...} 省略了对上面例子的解释, 大致意思是:   
命令的格式是 klee命令 + klee的参数 + 要执行的命令的.bc文件 + 命令的参数  
`--libc=uClibc`模拟正常的C库, `--posix--runtime`模拟操作系统级别的POSIX库  
有一些KLEE输出的附加信息, 这些警告大部分都是无害的, 可以无视掉

一般来说，对于相同的警告, KLEE只会警告一次. 警告也会记录到KLEE输出目录中的warnings.txt文件中.

### Step 5 : Introducing symbolic data to an application
上面是使用KLEE正常的解释程序, 下面的例子展示KLEE如何将部分输入符号化, 更详尽的探索程序.

以echo命令为例, 当使用uclibc和POSIX运行时, KLEE会将程序的`main()`函数替换成`klee_init_env()`函数. 这个函数改变了应用程序的命令处理行为, 特别是支持符号参数的构造. 例如, 传递`--help`参数时会产生如下输出: 

    src$ klee --libc=uclibc --posix-runtime ./echo.bc --help
    ...
    usage: (klee_init_env) [options] [program arguments]
      -sym-arg <N>              - Replace by a symbolic argument with length N
      -sym-args <MIN> <MAX> <N> - Replace by at least MIN arguments and at most
                                  MAX arguments, each with maximum length N
      -sym-files <NUM> <N>      - Make NUM symbolic files ('A', 'B', 'C', etc.),
                                  each with size N
      -sym-stdin <N>            - Make stdin symbolic with size N.
      -sym-stdout               - Make stdout symbolic.
      -max-fail <N>             - Allow up to N injected failures
      -fd-fail                  - Shortcut for '-max-fail 1'
    ...

下面的例子使用长度为3个字符的符号参数运行echo: 
 
    src$ klee --libc=uclibc --posix-runtime ./echo.bc --sym-arg 3
    KLEE: NOTE: Using klee-uclibc : /usr/local/lib/klee/runtime/klee-uclibc.bca
    KLEE: NOTE: Using model: /usr/local/lib/klee/runtime/libkleeRuntimePOSIX.bca
    KLEE: output directory is "/home/klee/coreutils-6.11/obj-llvm/src/./klee-out-1"
    Using STP solver backend
    KLEE: WARNING ONCE: function "vasnprintf" has inline asm
    KLEE: WARNING: undefined reference to function: __ctype_b_loc
    KLEE: WARNING: undefined reference to function: klee_posix_prefer_cex
    KLEE: WARNING: executable has module level assembly (ignoring)
    KLEE: WARNING ONCE: calling external: syscall(16, 0, 21505, 39407520)
    KLEE: WARNING ONCE: calling __user_main with extra arguments.
    ..
    KLEE: WARNING: calling close_stdout with extra arguments.
    ...
    KLEE: WARNING ONCE: calling external: printf(42797984, 41639952)
    ..
    KLEE: WARNING ONCE: calling external: vprintf(41640400, 52740448)
    ..
    Echo the STRING(s) to standard output.                                      // --help

        -n             do not output the trailing newline
        -e             enable interpretation of backslash escapes
        -E             disable interpretation of backslash escapes (default)
            --help     display this help and exit
            --version  output version information and exit
    Usage: ./echo.bc [OPTION]... [STRING]...
    echo (GNU coreutils) 6.11                                                       // --version
    Copyright (C) 2008 Free Software Foundation, Inc.                               // --version
    If -e is in effect, the following sequences are recognized:

      \0NNN   the character whose ASCII code is NNN (octal)                     // --help
      \\     backslash
      \a     alert (BEL)
      \b     backspace

    License GPLv3+: GNU GPL version 3 or later                                      // --version
    This is free software: you are free to change and redistribute it.              // --version
    There is NO WARRANTY, to the extent permitted by law.                           // --version

      \c     suppress trailing newline                                          // --help
      \f     form feed
      \n     new line
      \r     carriage return
      \t     horizontal tab
      \v     vertical tab

    NOTE: your shell may have its own version of echo, which usually supersedes
    the version described here.  Please refer to your shell's documentation
    for details about the options it supports.

    Report bugs to <bug-coreutils@gnu.org>.
    Written by FIXME unknown.                                                       // --version

    KLEE: done: total instructions = 64546
    KLEE: done: completed paths = 25
    KLEE: done: generated tests = 25

可以看到KLEE探索了25条路径, 所有路径的输出混合到了一起. 除了显示各种字符串外, echo的`--version`和`--help`参数也被探索到了.

KLEE生成的测试用例在`klee-out-n`文件夹中, `n`的值会随运行KLEE的次数而递增, 当然也可以通过`klee-last`目录直接进入到最新一次执行所生成的测试用例中.
我们可以使用klee-stats工具来获得KLEE内部统计的简短摘要: 

    src$ klee-stats klee-last
    ------------------------------------------------------------------------
    |  Path   |  Instrs|  Time(s)|  ICov(%)|  BCov(%)|  ICount|  TSolver(%)|
    ------------------------------------------------------------------------
    |klee-last|   64546|     0.15|    22.07|    14.14|   19943|       62.97|
    ------------------------------------------------------------------------

ICov是被覆盖到的LLVM指令的百分比, BCov是被覆盖到的分支的百分比. 百分比如此之低的原因是这些数字是通过统计bitcode文件中所有指令或分支来计算的, 其中包括一堆执行不到的库代码. 可以通过增加`--optimize`选项来解决该问题, 这会导致KLEE在执行bitcode文件之前先对其进行优化(删除死代码等).

    src$ klee --optimize --libc=uclibc --posix-runtime ./echo.bc --sym-arg 3
    ...
    KLEE: done: total instructions = 33991
    KLEE: done: completed paths = 25
    KLEE: done: generated tests = 25
    src$ klee-stats klee-last
    ------------------------------------------------------------------------
    |  Path   |  Instrs|  Time(s)|  ICov(%)|  BCov(%)|  ICount|  TSolver(%)|
    ------------------------------------------------------------------------
    |klee-last|   33991|     0.13|    30.16|    21.91|    8339|       80.66|
    ------------------------------------------------------------------------

可以看到这次指令覆盖率提高了6%, KLEE执行的更快, 执行的指令也更少. 但是优化并不完美, 剩下未被覆盖的大部分仍然是库函数. 我们可以使用KCachegrind显示KLEE的运行结果(在echo中查找未被覆盖的代码)来验证这一点.  

> 吐槽: 部分情况下使用`--optimize`参数会导致错误, 请去除该参数再试试.  

### Step 6 : Visualizing KLEE’s progress with KCachegrind
[KCachegrind](http://kcachegrind.sourceforge.net/)是一个出色的可视化工具, 如果没有安装过, 可以通过对应平台的软件安装工具直接安装(apt-get, yum等).

    src$ kcachegrind klee-last/run.istats

{...}省略对KCachegrind的介绍, 可以用该工具看到具体那些行被覆盖, 那些行未被覆盖

> 并没有什么卵用

### Step 7 : Replaying KLEE generated test cases
 让我们看看KLEE生成的测试用例, 如果我们查看`klee-last`目录, 应该可以看到25个`.ktest`文件(之前执行echo.bc所生成的).

    src$ ls klee-last
    assembly.ll	  test000004.ktest  test000012.ktest  test000020.ktest
    info		  test000005.ktest  test000013.ktest  test000021.ktest
    messages.txt	  test000006.ktest  test000014.ktest  test000022.ktest
    run.istats	  test000007.ktest  test000015.ktest  test000023.ktest
    run.stats	  test000008.ktest  test000016.ktest  test000024.ktest
    test000001.ktest  test000009.ktest  test000017.ktest  test000025.ktest
    test000002.ktest  test000010.ktest  test000018.ktest  warnings.txt
    test000003.ktest  test000011.ktest  test000019.ktest

这些文件包含了符号数据的实际值, 用来重现KLEE所探索的路径(获取覆盖率或重现错误). 
他们还包含POSIX运行时生成的附加元数据, 以便在运行时跟踪这些值对应于什么以及当时的版本.
我们可以使用ktest-tool查看某一个测试用例的内容: 

    $ ktest-tool klee-last/test000001.ktest
    ktest file : 'klee-last/test000001.ktest'
    args       : ['./echo.bc', '--sym-arg', '3']
    num objects: 2
    object    0: name: 'arg0'
    object    0: size: 4
    object    0: data: '\x00\x00\x00\x00'
    object    1: name: 'model_version'
    object    1: size: 4
    object    1: data: '\x01\x00\x00\x00'

该测试用例表明`\x00\x00\x00\x00`作为第一个参数传递给了echo. 但是`.ktest`文件一般来说都不是用来看的.
对于POSIX runtime, 我们提供了一个工具klee-replay, 它可以用来读取`.ktest`文件并调用应用程序, 自动向程序传递必要的数据以呈现该测试用例中KLEE所探索的路径.

为了查看他的工作原理, 请返回到我们构建的本地可执行文件目录: 

    obj-llvm/src$ cd ../../obj-gcov/src
    src$ ls -l echo
    -rwxrwxr-x 1 klee klee 135984 Nov 21 21:58 echo

要使用klee-replay, 我们只需要告诉他要运行的可执行文件和要使用的的`.ktest`文件 . 参数和输入文件等都将从`.ktest`文件中的数据读取.

    src$ klee-replay ./echo ../../obj-llvm/src/klee-last/test000001.ktest
    klee-replay: TEST CASE: ../../obj-llvm/src/klee-last/test000001.ktest
    klee-replay: ARGS: "./echo" ""

    klee-replay: EXIT STATUS: NORMAL (0 seconds)

上面例子中第一行显示正在运行的测试用例, 第二行显示执行的可执行文件以及传递的参数(与`.ktest`文件中相匹配). 最后一行是程序的退出状态和运行时间.

我们还可以使用klee-replay工具一个接一个的运行测试用例, 并将从gcov得到的覆盖率与从klee-ststs中得到的覆盖率进行比较.

    src$ rm -f *.gcda # Get rid of any stale gcov files
    src$ klee-replay ./echo ../../obj-llvm/src/klee-last/*.ktest
    klee-replay: TEST CASE: ../../obj-llvm/src/klee-last/test000001.ktest
    klee-replay: ARGS: "./echo" "@@@"
    @@@
    klee-replay: EXIT STATUS: NORMAL (0 seconds)
    ...
    klee-replay: TEST CASE: ../../obj-llvm/src/klee-last/test000022.ktest
    klee-replay: ARGS: "./echo" "--v"
    echo (GNU coreutils) 6.11
    Copyright (C) 2008 Free Software Foundation, Inc.
    ...

    src$ gcov echo
    File '../../src/echo.c'
    Lines executed:52.43% of 103
    Creating 'echo.c.gcov'

    File '../../src/system.h'
    Lines executed:100.00% of 3
    Creating 'system.h.gcov'

gcov得到的覆盖率明显高于klee-stats得到的覆盖率, 这是因为gcov只考虑单个文件, 而klee-stats考虑整个应用程序.
和kcachegrind一样, 我们可以检查gcov生成的覆盖率文件, 明确了解哪些行被覆盖到了以及哪些行没被覆盖到. 以下是输出的一个片段: 

        -:  194:
       23:  195:just_echo:
        -:  196:
       23:  197:  if (do_v9)
        -:  198:    {
       10:  199:      while (argc > 0)
        -:  200:	{
    #####:  201:	  char const *s = argv[0];
        -:  202:	  unsigned char c;
        -:  203:
    #####:  204:	  while ((c = *s++))
        -:  205:	    {
    #####:  206:	      if (c == '\\' && *s)
        -:  207:		{
    #####:  208:		  switch (c = *s++)
        -:  209:		    {
    #####:  210:		    case 'a': c = '\a'; break;
    #####:  211:		    case 'b': c = '\b'; break;
    #####:  212:		    case 'c': exit (EXIT_SUCCESS);
    #####:  213:		    case 'f': c = '\f'; break;
    #####:  214:		    case 'n': c = '\n'; break;

最左边的列是每行的执行次数, `-`表示改行没有可执行的代码, `####`表示改行从未被覆盖.
正如你所看到的, 这里未被覆盖的行与kcachegrind中报告的完全一致.

之前的测试因为没有提供足够的参数, 所以覆盖率并不理想. 提供两个参数差不多可以覆盖到整个echo的代码.
我们可以使用POSIX runtime的`--sym-args`选项来传递多个参数.
切换回`obj-llvm/src`目录后, 执行以下步骤:

    src$ klee --only-output-states-covering-new --optimize --libc=uclibc --posix-runtime ./echo.bc --sym-args 0 2 4
    ...
    KLEE: done: total instructions = 7611521
    KLEE: done: completed paths = 10179
    KLEE: done: generated tests = 57

`--sym-args`参数..., Emmmmm懒得解释了直接粘用法吧

    -sym-args <MIN> <MAX> <N>  - Replace by at least MIN arguments and at most MAX arguments, each with maximum length N

我们还在KLEE命令中添加了`--only-output-states-covering-new`参数. 默认情况下, KLEE会为每个探索到的路径生成一个测试用例.
当程序很庞大时, 很多测试用例最终会执行相同的路径, 计算(或重新执行)这些路径是非常浪费时间的.
使用该参数告诉KLEE只针对覆盖代码中新指令(或遇到错误)的路径生成测试用例.
上面例子的最后两行显示, 虽然KLEE探索了10179条路径, 但只需要生成57个测试用例.

我们可以回到`obj-gcov/src`目录运行新得到的测试用例, 并查看覆盖率: 

    src$ rm -f *.gcda # Get rid of any stale gcov files
    src$ klee-replay ./echo ../../obj-llvm/src/klee-last/*.ktest
    klee-replay: TEST CASE: ../../obj-llvm/src/klee-last/test000001.ktest
    klee-replay: ARGS: "./echo"
    ...
    ...

    src$ gcov echo
    File '../../src/echo.c'
    Lines executed:97.09% of 103
    Creating 'echo.c.gcov'

    File '../../src/system.h'
    Lines executed:100.00% of 3
    Creating 'system.h.gcov'

### Step 8 : Using zcov to analyze coverage
如果想要可视化的覆盖率结果，需要安装zcov工具

### Extra : 相关问题及参考
+ [OSDI'08 Coreutils Experiments](http://klee.github.io/docs/coreutils-experiments/)
  - 使用KLEE测试CoreUtils的版本, 环境, 测试的CoreUtils指令, 测试时执行的命令等
  - 测试时使用的选项 `klee -xxx -xxx ...`, 以及目前推荐的更新后选项
  - 如何生成 `test.env` 和 `/tmp/sandbox`
  - 测试结果不佳的指令及针对性的测试命令
  
    
    $ klee --simplify-sym-indices --write-cvcs --write-cov --output-module \  
    \--max-memory=1000 --disable-inlining --optimize --use-forked-solver \  
    \--use-cex-cache --with-libc --with-file-model=release \  
    \--allow-external-sym-calls --only-output-states-covering-new \  
    \--exclude-libc-cov --exclude-cov-file=./../lib/functions.txt \  
    \--environ=test.env --run-in=/tmp/sandbox --output-dir=paste-data-1h \  
    \--max-sym-array-size=4096 --max-instruction-time=10. --max-time=3600. \  
    \--watchdog --max-memory-inhibit=false --max-static-fork-pct=1 \  
    \--max-static-solve-pct=1 --max-static-cpfork-pct=1 --switch-type=internal \  
    \--randomize-fork --use-random-path --use-interleaved-covnew-NURS \  
    \--use-batching-search --batch-instructions 10000 --init-env \  
    ./paste.bc --sym-args 0 1 10 --sym-args 0 2 2 --sym-files 1 8 --sym-stdout


## 07. [Using symbolic environment](http://klee.github.io/tutorials/using-symbolic/)
指导在使用klee进行测试时如何使用 程序的命令行参数 和 符号文件 等符号环境



## 附录1: klee常用函数

+ 将变量标记为符号
  - `klee_make_symbolic( var, sizeof(var), "var" )`
  - 该函数接收三个参数
    * 看作符号的变量地址
    * 变量大小
    * 变量名称(任意)


+ 只在该表达式为真的路径上探索
  - `klee_assume( arr[size - 1] == '\0' )`
  - 相当于把整个程序包在 if( arr[size - 1] == '\0' ) 的条件判断中
  - 如果该表达式永远不可能为真(即表达式可能是错的), 则klee报告一个错误
  - 尽可能使用简单的表达式, 并使用 `&` 和 `|` ,而不是 `&&` 和 `||`


+ 强制一个条件成立(断言)
  - `klee_assert()`
  - 可以用该命令标记想要的结果, 例如在成功时的代码后添加`klee_assert(0)` (klee会将其标记为一个错误)



## 附录2: klee常用命令
[klee --help全部内容](https://pastebin.com/tDPGNn9D)

+ 使用 klee 对 hello.bc 开始符号执行并生成测试用例
  - `klee hello.bc`


+ 使用 uClibc c 库和 POSIX
  - `klee --libc=uclibc --posix-runtime ./cat.bc --version`


+ 用3个字符的符号当做输入参数
  - `klee --libc=uclibc --posix-runtime ./echo.bc --sym-arg 3`


+ 用 ktest-tool 工具读取 klee 生成的测试用例
  - `ktest-tool --write-ints klee-last/test000001.ktest`


+ 使用 klee 官方自带的重放工具重放测试用例
  - `export LD_LIBRARY_PATH=/path-to-your-klee-build-dir/lib/:$LD_LIBRARY_PATH` 添加环境变量
  - `gcc -L /home/eeyore/work/klee-build/lib/ get_sign.c -lkleeRuntest` 选择要重放的程序
  - `KTEST_FILE=klee-last/test000001.ktest ./a.out` 选择要重放的测试用例 
  - `echo $?` 查看运行结果


+ 


## 附录3: linux编译器相关常用命令

+ 将源代码编译为可执行文件
  - `gcc hello.c -o hello.exe`
    * `-o` : 指定生成的输出文件


+ 将源代码编译为 LLVM IR 字节码
  - `clang -emit-llvm -c -g get_sign.c`
  - `clang -I ../../include -emit-llvm -c -g get_sign.c`
    * `-I` : 使编译器可以找到klee/klee.h  
    * `-c` : 只将代码编译到目标文件, 而不是可执行文件
    * `-g` : 在目标文件中存储额外的源代码级的调试信息
    * `-emit-llvm` : 获得LLVM IR. 对应的 `-emit-obj` 是获得.o目标文件
  - `llvm-gcc -c -emit-llvm maze.c -o maze.bc`


## NOTE

    export LLVM_COMPILER=clang

要使环境变量持久化, 请将上述命令添加到shell脚本中(例如.bashrc等)