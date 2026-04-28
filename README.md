# Nebula Duel

Nebula Duel 是一个 C++20 游戏服务端学习项目，用于系统性训练网络编程、二进制协议、Protobuf 消息分发、自动化测试和工程化组织能力。

项目以服务端为核心，通过 Python 集成测试脚本模拟外部请求，便于持续验证网络协议和业务链路。

## 当前能力

目前已完成的基础能力：

- CMake + Ninja C++20 构建
- 基于 epoll 的事件循环
- 非阻塞 TCP 服务端
- 连接输入/输出缓冲区
- 自定义包头：`length + msg_id + seq_id`
- TCP 粘包、半包处理
- Protobuf 接入
- 初版 `MessageDispatcher`
- 登录请求/响应原型
- PacketCodec 的 GoogleTest 单元测试
- Python 协议集成测试脚本，用于模拟外部客户端请求

## 目录结构

```text
.
├── CMakeLists.txt
├── docs/
│   └── nebula_duel_development_plan.md
└── server/
    ├── CMakeLists.txt
    ├── generated/
    ├── include/
    │   ├── net/
    │   └── protocol/
    ├── proto/
    ├── scripts/
    ├── src/
    │   ├── net/
    │   └── protocol/
    └── tests/
```

## 协议格式

每个 TCP 包使用固定 12 字节包头：

```text
uint32 length
uint32 msg_id
uint32 seq_id
bytes  payload
```

`length` 表示 `msg_id + seq_id + payload` 的总长度，不包含 `length` 字段本身。

当前消息号：

```text
1001 LoginRequest
1002 LoginResponse
```

`payload` 使用 Protobuf 序列化。

## 构建

依赖：

- C++20 编译器
- CMake
- Ninja
- spdlog
- Protobuf
- Abseil
- GoogleTest

构建：

```bash
./server/scripts/build.sh
```

启动服务端：

```bash
./build/server/nebula_server
```

## 测试

运行 C++ 单元测试：

```bash
ctest --test-dir build --output-on-failure
```

启动服务端后，运行 Python 协议集成测试：

```bash
python3 server/scripts/test_packet_client.py
```

Python 测试脚本覆盖：

- LoginRequest / LoginResponse Protobuf 链路
- 粘包请求
- 半包请求
- 非法包关闭连接

## 重新生成 Protobuf

```bash
./server/scripts/gen_proto.sh
```

生成后的 C++ 文件提交在 `server/generated/` 下，保证 fresh checkout 后可以直接构建。

## GitHub CI

仓库已提供 GitHub Actions 配置：

```text
.github/workflows/ci.yml
```

CI 会在 `master` / `main` 分支的 push 和 pull request 上自动执行：

```text
1. 安装 C++ 构建依赖
2. 重新生成 Protobuf C++ 文件
3. 配置 CMake
4. 编译服务端和测试目标
5. 运行 ctest
6. 检查 Python 测试脚本语法
```

当前阶段优先建设 CI，用于保证每次提交后的构建和测试稳定性。部署流水线可以在后续引入 Docker 镜像或演示环境时再扩展。

## 演进方向

近期计划继续补齐服务端核心模块：

- 抽出 `LoginService`
- 增加 Session 管理
- 接入 SQLite 账号查询
- 接入 Redis Token 和在线状态
- 增加匹配与房间管理
- 实现服务端权威回合制战斗逻辑
- 增加 Lua 技能脚本
- 扩展单元测试和集成测试

## 项目收获

这个项目主要用于沉淀 C++ 服务端开发中的关键能力：

- 理解 Linux 网络 IO、非阻塞 socket 和 Reactor 模型
- 掌握 TCP 流式协议下的粘包、半包处理
- 练习 Protobuf 协议设计和消息路由
- 建立可测试的服务端模块边界
- 使用 CMake、GoogleTest、脚本和 CI 提升工程化质量
