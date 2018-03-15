# 更换make版本
在使用make命令自动编译一些项目时, 可能会遇到make版本过高导致不能使用高版本的make编译的问题  
例如本人在用Ubuntu16.04编译android4.4.4源码时就遇到了make版本过高的问题  
这时候需要更换make的版本

> 具体的错误内容和更换过程已经执行完了, 更详细的过程和图如果以后有空再补吧
这里先写个简易版给有一定linux基础或能自行google的人看

### 环境:
    Ubuntu 16.04

## 1. 下载make源码
[不同版本的make源码](http://ftp.gnu.org/gnu/make/)

## 2. 解压make源码
随便哪个目录都行

## 3. 编译make源码
+ 进入刚解压好的make源码目录
+ 编译

  
    ./configuration
    . /build.sh    
> 第二条命令.和/之间有个空格

## 4. 删除已经安装的make
    apt-get remove make

## 5. 将新版本的make拷到系统目录
    cp make /usr/bin/make