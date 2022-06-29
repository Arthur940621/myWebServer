# 计算机网络与套接字编程

阅读本项目前，可以先进行[计算机网络与套接字编程](https://github.com/Arthur940621/computerNetworking)的学习。

# `WebServer`

## `C++` 实现的 `WEB` 服务器

利用正则与状态机解析 `HTTP` 请求报文，实现处理静态资源的请求

利用 `IO` 复用技术 `Epoll`、线程池实现多线程的 `Reactor` 高并发模型

利用标准库容器封装 `char`，实现自动增长的缓冲区

基于小根堆实现的定时器，关闭超时的非活动连接

利用单例模式与阻塞队列实现异步的日志系统，记录服务器运行状态

实现了数据库连接池，减少数据库连接建立与关闭的开销，同时实现了用户注册登录功能

加入了 `Json` 解析，支持对本地配置进行读取

# 项目启动

```
// 建立 testdb 库
create database testdb;

// 创建 user 表
USE testdb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, password) VALUES('name', 'password');
```

```
make
./bin/server
```

# 致谢

[@markparticle](https://github.com/markparticle/WebServer)

参考了大佬的代码，重写了数据库连接池和线程池，加入了 `Json` 解析模块，加入了对 `http` 的 `url` 编解码，以及修复了一些我认为有错误的 `bug`。

**ps: 由于实现过[红黑树](https://github.com/Arthur940621/myRBTree)，本来想把定时器改成基于红黑树的，但是不知道哪里出了问题 ORZ。**