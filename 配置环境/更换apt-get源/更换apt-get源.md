# 更换apt-get源
  在使用apt-get命令时, ubuntu会默认使用官方的源, 往往下载的速度非常慢  
  我们可以通过换成一些中间的节点来进行下载, 比如阿里源, 清华源等等, 速度会比直接从官方源下载快很多
  

## 1. 备份源

    cd /etc/apt
    
    cp sources.list sources.list.bak


## 2. 更换源

2.1 和 2.2 选一种即可  

### 2.1 修改源

    vi sources.list
    
    :%s/us.archive/cn.archive/g
    
    :wq

>命令中的符号解释:  
: (冒号), 表示后面是命令  
%(百分号), 表示修改缓冲区中的文件内容, 也就是说如果不保存, 源文件不会改变, 所以之后要用：wq! 命令保存退出  
s(字母s), 表示替换  
/us.archive/cn.archive，表示将文件中的us.archive 替换为cn.archive
/g(撇加字母g), 表示替换所有内容  
这样, 就将apt-get的源更改为国内的源了  

### 2.2 替换源

使用[清华大学Tuna源](https://mirrors.tuna.tsinghua.edu.cn/help/ubuntu/), 选择符合自己版本的源  
取得root权限后(或者在命令前加sudo), 输入

    gedit sources.list

在打开的`sources.list`中将全部文件替换为Tuna源中的内容, 然后保存, 关闭


## 3. 更新源列表
大功告成  
更新源列表  

    apt-get update
