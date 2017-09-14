
# 1. 备份源

    cd /etc/apt
    
    cp sources.list sources.list.bak


# 2. 更新源

2.1和2.2选一种即可

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

使用[清华大学Tuna源](https://mirrors.tuna.tsinghua.edu.cn/help/ubuntu/)

取得root权限后(或者在命令前加sudo)输入

    gedit sources.list

在打开的`sources.list`中将全部文件替换为Tuna源中的内容, 然后保存, 关闭


## 3.更新源列表
大功告成  
更新源列表  

    apt-get update
