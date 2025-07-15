# ServerClientChat
本项目是 `CMake` 构建管理，采用C++17标准，依赖 `Muduo` 网络库、`MySQL++` 数据访问库、`fmt` 格式化库、`Redis++` 客户端库，运行在Linux环境下的网络聊天服务程序

### 服务器端的项目依赖

#### **C++ 第三方库依赖：**

| 依赖库                        | 功能描述                                          |
| -------------------------- | --------------------------------------------- |
| `mysqlpp`                  | MySQL++ 库，C++ 封装的 MySQL 客户端库，用于数据库访问          |
| `muduo_base` + `muduo_net` | 高性能 C++ 网络库，适用于多线程 TCP 网络编程                   |
| `fmt`                      | 格式化输出库，C++20 中被采纳为 `std::format` |
| `hiredis`                  | 官方 Redis C 客户端库，用于和 Redis 通信                  |
| `redis++`                  | Redis C++ 客户端库，封装在 hiredis 上，提供更现代 C++ 接口     |
|`nlohammn Json`|JSON序列化和反序列化库，最好用的Json库|
|`magic_enum`|一个轻量级的 C++17/C++20 库，它提供了 枚举类型的反射能力，无需编译，头文件only|

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

muduo 在该路径下进行编译并链接

---


### 客户端的项目依赖
由于已提取共享的数据类型与方法定义，因此项目依赖和服务端相同，具体可参考Client端的CMakeLists.txt
## **额外说明：**

* 编译目标输出路径为 `${PROJECT_SOURCE_DIR}/bin`
* Debug 模式下会生成 `*_d` 可执行文件（加了 DEBUG\_POSTFIX）