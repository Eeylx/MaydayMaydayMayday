# How to add directory to system path in Linux



## 什么是PATH

在Linux中, PATH环境变量会存储可执行文件的存放路径. PATH环境变量的值是一个包含一系列路径名的字符串, 每个路径名由`:`(冒号)分割. 

例如, 经典系统上默认的PATH变量可能如下所示:

```bash
/usr/local/bin:/usr/bin:/bin:/usr/local/games:/usr/games
```



当你在终端输入命令(例如`cat`)时, shell会按顺序在每个目录中查找名为cat的可执行文件, 并运行它所找到的第一个.

要查看PATH变量, 可以使用`echo`命令. 与shell中的其他变量一样, 当引用变量时, 需要在变量名前加上一个美元符号:

```bash
echo $PATH
```



## 为当前终端设置PATH变量

你可以像设置任何其他shell变量一样设置PATH变量的值, 形式为`NAME=VALUE`, 例如:

```bash
PATH=/my/first/path:my/second/path
```



上述命令的问题在于它将完全覆盖PATH变量的值, 导致之前存在的路径丢失. 如果想要添加旧值以外的新值, 可以在设置变量时引入PATH变量. 同时, 由于shell会按照顺序依次搜索PATH中的路径, 所以你可能希望将你的路径加在某些路径之前, 使其先于系统路径. 以下是两种示例:

```bash
PATH=$PATH:/my/new/path
PATH=/my/new/path:$PATH
```



## 使用`export`命令将PATH变量传递给子进程

这种类型的PATH定义可以为你当前的终端设置环境变量, 但是并不能影响任何新运行的程序, 这是因为shell允许通过手动声明来决定将那些环境变量传递给其他程序和进程. 可以使用`export`命令完成此操作:

```bash
export PATH
```

执行上述命令后, 在注销之前, 运行的任何程序都将使用当前的PATH环境变量.

如果你愿意, 可以将这两个命令合成一行, 只需要在他们之间加一个分号:

```bash
PATH=$PATH:/my/new/path:/my/other/new/path;export PATH
```

> 如果你的任何路径中存在空格, 则应该将路变量定义括在引号中, 以确保正确, 例如:
>
> ```bash
> PATH="$PATH:/putting/spaces in pathnames:/makes/life very/inconvenient";export PATH
> ```



## 为每个新的shell终端设置PATH变量

到目前为止, 我们使用的方法只能为当前的shell终端设置环境变量, 但是当你注销或关闭终端时, 所做的更改将被舍弃. 如果你希望能在每次登陆或启动新的终端时都能将PATH设置为某个值的话, 可以将其添加到bash的启动脚本中. 每次启动交互式shell终端时, bash会按顺序读取以下文件(如果存在), 并执行其中的命令:

```bash
/etc/profile
~/.bash_profile
~/.bash_login
~/.profile
```

第一个文件`/etc/profile`是系统上所有用户的默认启动脚本, 其他三个都在每个用户的用户目录中, 可以使用这几个文件中的任意一个, 但最好知道它们会按照此顺序运行.

您可以编辑这些文件并手动更改包含`PATH=definitions`的任何行, 不过最好不要随意改动.

如果你只想添加自己的路径, 则可以保持其他`PATH=`的行不变, 只需要将类似下面的代码添加到文件末尾即可:

```bash
PATH="$PATH:/new/path";export PATH
```

如果将其添加到用户目录中`.bash_profile`文件的末尾, 则每次用户启动新的shell会话时它都会生效. 如果将其添加到`/etc/profile `中, 它将对系统上的每个用户生效.





