# ServerClientChat
本项目是一个基于 C++ 17 编写的网络服务端，依赖 `Muduo` 网络库、`MySQL++` 数据库访问库、`fmt` 格式化库、`Redis++` Redis 客户端库，并通过 `CMake` 构建管理，适配 Linux 环境下的 MySQL/Redis 服务对接与网络通信功能。

## 服务器端的项目依赖

#### **C++ 第三方库依赖：**

| 依赖库                        | 功能描述                                          |
| -------------------------- | --------------------------------------------- |
| `mysqlpp`                  | MySQL++ 库，C++ 封装的 MySQL 客户端库，用于数据库访问          |
| `muduo_base` + `muduo_net` | 高性能 C++ 网络库，适用于多线程 TCP 网络编程                   |
| `fmt`                      | 格式化输出库，C++20 中被采纳为 `std::format`，优于 `printf` |
| `hiredis`                  | 官方 Redis C 客户端库，用于和 Redis 通信                  |
| `redis++`                  | Redis C++ 客户端库，封装在 hiredis 上，提供更现代 C++ 接口     |
|`nlohammn Json`|JSON序列化和反序列化库，最好用的Json库|

---

#### **项目运行环境配置**

| 路径                              | 含义或来源                         |
| ------------------------------- | ----------------------------- |
| `${PROJECT_SOURCE_DIR}/include` | 项目自定义头文件                      |
| `/usr/include/mysql++`          | 系统安装的 MySQL++ 库头文件            |
| `/usr/include/mysql`            | MySQL 客户端基础头文件（libmysql）      |
| `/home/g/muduo/`                | 注意Ubuntu上只能通过源码编译Muduo库 |

---

####  **库文件路径：**

```cmake
target_link_directories(server PRIVATE /home/g/build/release-cpp11/lib)
```

muduo 在这个路径下编译并链接

---

#### **额外说明：**

* 编译目标输出路径为 `${PROJECT_SOURCE_DIR}/bin`
* Debug 模式下会生成 `server_d` 可执行文件（加了 DEBUG\_POSTFIX）
## 客户端的项目依赖
由于已提取公共部分，含有共享的数据类型与方法定义，故和服务端相同，具体可参考Client端的CMakeLists.txt
