# 配置PM2 (Node.js进程管理工具)

PM2是Node.js的进程管理工具, 可以利用它来简化很多Node应用管理的繁琐任务, 比如性能监控, 自动重启, 负载均衡等, 而且使用非常简单.

> npm是Node.js的包管理工具, 如果不知道npm是什么, 也许你不适合此教程

### 环境:
    Node.js
    Express框架
    npm

## 1. 安装PM2
通过npm安装pm2的最新稳定版

    // 安装
    npm install pm2@latest -g

    // 更新
    pm2 update
    

## 2. 启动服务
较老的Express框架通过`app.js`启动服务, 新版本通过`./bin/www`启动服务.

### 2.1 最简单的启动方法 :

    pm2 start ./bin/www
    
### 2.2 通过配置文件启动 : 
首先需要创建一个yml格式的配置文件. 

pm2_configuration.yml : 

    apps:
      - script : ./bin/www
        instances : 4
        exec_mode : cluster
      - script : worker.js
        watch : true
        env : 
            NODE_ENV : development
        env_production:
            NODE_ENV : production

然后通过该配置文件启动 : 

    pm2 start pm2_configuration.yml


## 3. PM2启动时的一些参数

### 3.1 `-n` 给进程起别名
在使用`pm2 list`命令的时候方便查看

    -n --name <name>

    pm2 start ./bin/www -n microblog
    
### 3.2 `-i` 启动多个Node.js进程
Node.js是单线程运行的, 该命令可以充分利用多cpu的机器

    -i --instances <number>
    
    pm2 start ./bin/www -i 0
    
> 其中`num`为要启动的进程数, 如果为0, 则根据机器的cpu数确定启动的进程数

### 3.3 `--watch` 检测应用目录的变化 / 自动重启
检测应用目录的变化, 一旦发生变化, 自动重启服务. 如果要详细说明需要检测或不需要检测的目录, 最好通过配置文件进行设置

    --watch 
    --ignore-watch <folders|files>
    
    pm2 start ./bin/www --watch
    pm2 start ./bin/www --ignore-watch="node_modules"
    
> 如果项目中存在日志文件, 用--watch的时候请务必使用--ignore-watch参数将日志文件忽略, 不然会导致无限重启 (检测到日志文件改动, 重启, 检测到日志文件改动, 重启.......)


## 4. PM2 常用命令 
其他命令详见[备忘录](#%E9%99%84%E5%BD%951--%E5%A4%87%E5%BF%98%E5%BD%95)

    // 查看
    pm2 list               # Display all processes status
    pm2 jlist              # Print process list in raw JSON
    pm2 prettylist         # Print process list in beautified JSON
    
    pm2 show 0             # Show specific process id
    
    // 操作
    pm2 stop 0             # Stop specific process id
    pm2 stop all           # Stop all processes
    
    pm2 restart 0          # Restart specific process id
    pm2 restart all        # Restart all processes
    
    pm2 delete 0           # Will remove process from pm2 list
    pm2 delete all         # Will remove all processes from pm2 list
    
    pm2 reload all         # Will 0s downtime reload (for NETWORKED apps)

## 5. PM2开机自动启动
嗯, 这个很重要, 不然就等着大半夜被叫醒吧.

[官方文档](http://pm2.keymetrics.io/docs/usage/startup/)

在服务配置好并且运行起来后, 输入以下命令 : 

    // 保存当前的进程列表. 在 预期/意外 的服务器重启后, 将自动恢复为保存时的状态
    pm2 save
    
    // 生成开机启动脚本, 可以指定服务器系统, 或忽略让pm2自动检测 (二选一)
    pm2 startup  
    pm2 startup [ubuntu | ubuntu14 | ubuntu12 | centos | centos6 | arch | oracle | amazon | macos | darwin | freebsd | systemd | systemv | upstart | launchd | rcd | openrc]
    
    // 随后需要复制粘贴并运行pm2自动生成的代码 (也许需要root权限), 然而在我配置时这一步它自己搞定了...
    
其他相关命令 :    
    
    // 手动恢复状态
    pm2 resurrect
    
    // 禁用启动脚本, 可以指定服务器系统, 或忽略让pm2自动检测 (二选一)
    pm2 unstartup
    pm2 unstartup [ubuntu | ubuntu14 | ubuntu12 | centos | centos6 | arch | oracle | amazon | macos | darwin | freebsd | systemd | systemv | upstart | launchd | rcd | openrc] 
    
    // 更新启动脚本
    pm2 unstartup [ubuntu | ubuntu14 | ubuntu12 | centos | centos6 | arch | oracle | amazon | macos | darwin | freebsd | systemd | systemv | upstart | launchd | rcd | openrc]
    pm2 startup [ubuntu | ubuntu14 | ubuntu12 | centos | centos6 | arch | oracle | amazon | macos | darwin | freebsd | systemd | systemv | upstart | launchd | rcd | openrc]    
    
> 当更新过Node.js后, pm2的路径可能会发生改变, 需要更新启动脚本


## 附录1 : 备忘录
PM2中一些值得了解的命令 : 
```
# Fork mode
pm2 start app.js --name my-api # Name process

# Cluster mode
pm2 start app.js -i 0        # Will start maximum processes with LB depending on available CPUs
pm2 start app.js -i max      # Same as above, but deprecated.

# Listing

pm2 list               # Display all processes status
pm2 jlist              # Print process list in raw JSON
pm2 prettylist         # Print process list in beautified JSON

pm2 describe 0         # Display all informations about a specific process

pm2 monit              # Monitor all processes

# Logs

pm2 logs [--raw]       # Display all processes logs in streaming
pm2 flush              # Empty all log files
pm2 reloadLogs         # Reload all logs

# Actions

pm2 stop all           # Stop all processes
pm2 restart all        # Restart all processes

pm2 reload all         # Will 0s downtime reload (for NETWORKED apps)

pm2 stop 0             # Stop specific process id
pm2 restart 0          # Restart specific process id

pm2 delete 0           # Will remove process from pm2 list
pm2 delete all         # Will remove all processes from pm2 list

# Misc

pm2 reset <process>    # Reset meta data (restarted time...)
pm2 updatePM2          # Update in memory pm2
pm2 ping               # Ensure pm2 daemon has been launched
pm2 sendSignal SIGUSR2 my-app # Send system signal to script
pm2 start app.js --no-daemon
pm2 start app.js --no-vizion
pm2 start app.js --no-autorestart
```

参数 : 
```
Options:
   -h, --help                           output usage information
   -V, --version                        output the version number
   -v --version                         get version
   -s --silent                          hide all messages
   -m --mini-list                       display a compacted list without formatting
   -f --force                           force actions
   -n --name <name>                     set a <name> for script
   -i --instances <number>              launch [number] instances (for networked app)(load balanced)
   -l --log [path]                      specify entire log file (error and out are both included)
   -o --output <path>                   specify out log file
   -e --error <path>                    specify error log file
   -p --pid <pid>                       specify pid file
   --max-memory-restart <memory>        specify max memory amount used to autorestart (in megaoctets)
   --env <environment_name>             specify environment to get specific env variables (for JSON declaration)
   -x --execute-command                 execute a program using fork system
   -u --user <username>                 define user when generating startup script
   -c --cron <cron_pattern>             restart a running process based on a cron pattern
   -w --write                           write configuration in local folder
   --interpreter <interpreter>          the interpreter pm2 should use for executing app (bash, python...)
   --log-date-format <momentjs format>  add custom prefix timestamp to logs
   --no-daemon                          run pm2 daemon in the foreground if it doesn't exist already
   --merge-logs                         merge logs from different instances but keep error and out separated
   --watch                              watch application folder for changes
   --ignore-watch <folders|files>       folder/files to be ignored watching, chould be a specific name or regex - e.g. --ignore-watch="test node_modules "some scripts""
   --node-args <node_args>              space delimited arguments to pass to node in cluster mode - e.g. --node-args="--debug=7001 --trace-deprecation"
   --no-color                           skip colors
   --no-vizion                          skip vizion features (versioning control)
   --no-autorestart                     do not automatically restart apps
```

PM2支持的脚本 : 

    pm2 start echo.yml
    pm2 start echo.pl --interpreter=perl
    pm2 start echo.coffee
    pm2 start echo.php
    pm2 start echo.py
    pm2 start echo.sh
    pm2 start echo.rb


## 附录2 : 参考
1. [PM2 Quick Start](http://pm2.keymetrics.io/docs/usage/quick-start/) (官网)
1. [PM2实用入门指南](https://segmentfault.com/a/1190000006793571) (SegmentFault)
2. [pm2常用命令](https://www.jianshu.com/p/e709b71f12da) (简书)