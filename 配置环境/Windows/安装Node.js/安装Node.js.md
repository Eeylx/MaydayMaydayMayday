# 安装Node.js
Windows下安装Node.js
>顺手提供一个[Node.js中文社区](https://cnodejs.org/)

### 环境:
    Windwos10 64位

## 1. 从官网下载安装包并安装
[Node.js官网](https://nodejs.org/en/) 会自动检测操作系统, 并提供LTS和Latest版

[Node.js官网下载页](https://nodejs.org/en/download/) 可以自己选择操作系统和版本, 看着眼晕的用上面那个吧

[Node.js中文网](http://nodejs.cn/) 不推荐


## 2. 检查是否安装成功
在cmd中输入`node -v`, 如果出现版本号则说明Node.js安装成功

>打开cmd的方法 : 按下win+R, 输入cmd后点击确定

## 3. 安装Node.js的包管理工具npm
一般来说npm会跟着node.js一起安装好, 在cmd中输入`npm -v`, 如果出现版本号则说明npm安装成功

## 4. 使用npm
语法 :

    npm install                         // 安装package.json文件中所记录的所有包
    npm install [<pkg>...]              // 本地安装
    npm install -g [<pkg>...]           // 全局安装
    npm install [<pkg>...] --save       // 安装并将信息写入package.json文件的dependencies字段中
    npm install [<pkg>...] --save-dev   // 安装并将信息写入package.json文件的devDependencies字段中

    npm uninstall [-g] [<pkg>...]       // 卸载
    npm update [-g] [<pkg>...]          // 更新

例子 :

    npm install express -g          // 安装node.js的express框架
    npm install async --save        // 安装项目所需的async
    npm install grunt --save-dev    // 安装项目开发时所需的grunt
    
> + 本地安装 : 将安装包放在`./node_modules`目录下, 也就是执行npm时所在的目录, 一般来说安装项目所需的包时会使用本地安装, 直接安装在项目目录中, 可以在代码中通过`require()`来引入本地安装的包
> + 全局安装 : 将安装包放在` /usr/local`下(Linux), 或者你的node安装目录中(Windows), 通过这种方式安装的包可以在命令行中直接使用

## 5. 更新Node.js

    npm install -g n
    n lastest
