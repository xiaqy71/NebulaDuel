# Nebula Duel 服务端开发计划

## 1. 项目定位

Nebula Duel 是一个 C++20 游戏服务端学习项目，用于系统性提升网络编程、协议设计、后端状态管理、自动化测试和工程化组织能力。

项目重点是实现一套可测试、可解释、可逐步扩展的游戏服务端核心链路：

```text
TCP 连接
-> 自定义二进制包头
-> Protobuf 消息
-> MessageDispatcher
-> 登录 / 会话 / 匹配 / 房间 / 战斗 / 排行榜
-> Redis / 数据库 / Lua 脚本
-> 单元测试与集成测试
```

外部请求通过 Python 测试脚本、单元测试和后续集成测试模拟，使开发重心集中在服务端网络、协议、状态和业务模块上。

## 2. 技术目标

服务端需要体现以下能力：

- C++20 工程化组织
- Linux epoll / Reactor 网络模型
- 非阻塞 TCP 连接管理
- 输入输出缓冲区
- length + msg_id + seq_id 自定义二进制协议
- TCP 粘包、半包处理
- Protobuf 消息序列化与反序列化
- MessageDispatcher 消息分发
- spdlog 日志
- GoogleTest 单元测试
- Python 协议集成测试脚本
- 后续接入 Redis、SQLite/MySQL、Lua 脚本

## 3. 开发原则

当前阶段聚焦服务端核心能力：

- 网络层先保证连接管理、拆包、回包稳定
- 协议层先保证消息 ID、seq_id、Protobuf 编解码清晰
- 业务层按登录、Session、匹配、房间、战斗逐步推进
- 每个阶段都配套可运行测试
- 公开文档只描述已完成能力和明确规划，避免夸大项目状态

测试脚本作为模拟客户端使用，用于稳定复现协议交互、异常输入和并发场景。

## 4. 当前目录结构

```text
NebulaDuel/
  CMakeLists.txt
  README.md
  docs/
    nebula_duel_development_plan.md
  server/
    CMakeLists.txt
    include/
      net/
        Buffer.h
        EpollPoller.h
        EventLoop.h
        TcpConnection.h
        TcpServer.h
      protocol/
        MessageDispatcher.h
        MessageId.h
        PacketCodec.h
    src/
      main.cpp
      net/
        Buffer.cpp
        EpollPoller.cpp
        EventLoop.cpp
        TcpConnection.cpp
        TcpServer.cpp
      protocol/
        MessageDispatcher.cpp
        PacketCodec.cpp
    proto/
      common.proto
      login.proto
    generated/
      common.pb.h
      common.pb.cc
      login.pb.h
      login.pb.cc
    scripts/
      build.sh
      run_server.sh
      gen_proto.sh
      test_packet_client.py
    tests/
      PacketCodecTest.cpp
```

## 5. 已完成能力

截至 Day 5：

- CMake + Ninja 构建
- spdlog 启动日志
- epoll poller
- EventLoop 事件分发
- TcpServer accept 多连接
- TcpConnection 非阻塞读写
- Buffer 输入输出缓冲
- PacketCodec 编解码
- 支持 length + msg_id + seq_id 包头
- 支持粘包、半包处理
- PacketCodec GoogleTest
- common/login Protobuf
- gen_proto.sh
- MessageId
- MessageDispatcher
- LoginRequest -> LoginResponse 假数据链路
- Python 协议测试脚本

## 6. 协议设计

当前 TCP 包格式：

```text
uint32 length   // msg_id + seq_id + payload 的总长度，不包含 length 自身
uint32 msg_id
uint32 seq_id
bytes  payload  // Protobuf 序列化结果
```

当前消息号：

```text
1001 LoginRequest
1002 LoginResponse
```

当前 Protobuf：

```text
common.proto
login.proto
```

## 7. 测试策略

单元测试：

```bash
ctest --test-dir build --output-on-failure
```

当前覆盖：

- encode 后 decode
- 半包返回 NeedMore 且不消费 Buffer
- 粘包逐个解析
- 非法 length 返回 Error
- 空 payload 合法

集成测试：

```bash
python3 server/scripts/test_packet_client.py
```

当前覆盖：

- LoginRequest / LoginResponse Protobuf 链路
- 粘包登录请求
- 半包登录请求
- 非法包关闭连接

## 8. 每日开发计划

每天都按同一个节奏推进：

```text
1. 先补协议或接口定义
2. 再实现服务端模块
3. 最后补单元测试或 Python 集成测试
4. 保证 build + ctest 通过
```

每日基础检查：

```bash
./server/scripts/build.sh
ctest --test-dir build --output-on-failure
python3 -m py_compile server/scripts/test_packet_client.py
```

### Day 1：工程初始化

任务：

- 创建仓库基础结构
- 建立 `server/`、`docs/` 目录
- 配置顶层 `CMakeLists.txt`
- 配置 `server/CMakeLists.txt`
- 引入 C++20 编译选项
- 引入 spdlog
- 编写 `server/src/main.cpp`
- 编写 `server/scripts/build.sh`
- 编写 `server/scripts/run_server.sh`

产出文件：

- `CMakeLists.txt`
- `server/CMakeLists.txt`
- `server/src/main.cpp`
- `server/scripts/build.sh`
- `server/scripts/run_server.sh`

验收标准：

- `./server/scripts/build.sh` 可以编译
- `./build/server/nebula_server` 可以启动
- 服务端启动时打印日志

### Day 2：EpollPoller 与 EventLoop

任务：

- 实现 `EpollPoller`
- 封装 `epoll_create1`
- 封装 `epoll_ctl ADD/MOD/DEL`
- 封装 `epoll_wait`
- 实现 `EventLoop`
- 支持 fd 注册、修改、删除
- 支持事件回调分发
- 支持 `run()` 和 `stop()`

产出文件：

- `server/include/net/EpollPoller.h`
- `server/src/net/EpollPoller.cpp`
- `server/include/net/EventLoop.h`
- `server/src/net/EventLoop.cpp`

测试方式：

- 写一个临时 pipe 或 socket fd 注册到 EventLoop
- 手动触发可读事件
- 确认回调被执行

验收标准：

- epoll fd 正常创建和释放
- fd 可以注册、修改、删除
- EventLoop 能持续轮询事件
- 空事件不会导致崩溃

### Day 3：TcpServer、TcpConnection 与 Buffer

任务：

- 实现 `TcpServer`
- 创建监听 socket
- 设置非阻塞
- 设置 `SO_REUSEADDR`
- 实现 `accept4`
- 实现 `TcpConnection`
- 支持非阻塞 read/write
- 实现连接关闭逻辑
- 实现 `Buffer`
- 支持 append、retrieve、peek
- 支持输入缓冲和输出缓冲

产出文件：

- `server/include/net/TcpServer.h`
- `server/src/net/TcpServer.cpp`
- `server/include/net/TcpConnection.h`
- `server/src/net/TcpConnection.cpp`
- `server/include/net/Buffer.h`
- `server/src/net/Buffer.cpp`

测试方式：

- 用 `nc 127.0.0.1 9000` 连接服务端
- 多开几个连接
- 发送普通字符串
- 主动断开连接

验收标准：

- 多连接可以同时接入
- 客户端断开不崩溃
- 服务端可以回显或记录收到的数据
- 输出缓冲区可以处理短写

### Day 4：PacketCodec 与 TCP 拆包

任务：

- 设计包头格式
- 实现 `Packet` 结构
- 实现 `PacketCodec::encode`
- 实现 `PacketCodec::decode`
- 支持 `length + msg_id + seq_id + payload`
- 处理半包
- 处理粘包
- 处理非法长度
- 将 `TcpConnection::handleRead` 从原始 echo 改为按包解析
- 增加 PacketCodec 单元测试

产出文件：

- `server/include/protocol/PacketCodec.h`
- `server/src/protocol/PacketCodec.cpp`
- `server/tests/PacketCodecTest.cpp`

测试方式：

- `ctest --test-dir build --output-on-failure`
- Python 脚本发送完整包、粘包、半包、非法包

验收标准：

- 单包可以正确解析
- 半包返回 NeedMore 且不消费 Buffer
- 粘包可以连续解析多个 Packet
- 非法 length 会关闭连接
- 单元测试通过

### Day 5：Protobuf 与 MessageDispatcher

任务：

- 编写 `common.proto`
- 编写 `login.proto`
- 编写 `gen_proto.sh`
- 生成 C++ Protobuf 文件
- CMake 接入 Protobuf
- 定义 `MessageId`
- 实现 `MessageDispatcher`
- `TcpConnection` 解析 Packet 后调用 dispatcher
- 注册 `LoginRequest` handler
- 返回 `LoginResponse` 假数据
- Python 脚本发送 Protobuf 登录请求

产出文件：

- `server/proto/common.proto`
- `server/proto/login.proto`
- `server/generated/*.pb.h`
- `server/generated/*.pb.cc`
- `server/include/protocol/MessageId.h`
- `server/include/protocol/MessageDispatcher.h`
- `server/src/protocol/MessageDispatcher.cpp`
- `server/scripts/test_packet_client.py`

测试方式：

- `ctest --test-dir build --output-on-failure`
- 启动服务端后运行 `python3 server/scripts/test_packet_client.py`

验收标准：

- Protobuf 文件可以生成
- 服务端能解析 `LoginRequest`
- 服务端能返回 `LoginResponse`
- Python 脚本测试登录、粘包、半包、非法包通过

### Day 6：LoginService 模块化

任务：

- 新增 `LoginService`
- 将登录处理逻辑从 `TcpServer` 构造函数移出
- 定义 `LoginResult`
- 定义登录错误码映射
- 增加用户名、密码基础校验
- 空用户名返回错误
- 空密码返回错误
- 合法输入返回测试用户信息
- 为 LoginService 写单元测试

建议目录：

```text
server/include/login/LoginService.h
server/src/login/LoginService.cpp
server/tests/LoginServiceTest.cpp
```

实现重点：

- `TcpServer` 只负责注册 handler
- handler 只负责 Protobuf parse/serialize
- 业务判断放进 `LoginService`

测试方式：

- GoogleTest 直接测试 `LoginService::login`
- Python 脚本测试正常登录和空字段登录

验收标准：

- 登录业务不再写在 `TcpServer` 构造函数里
- `LoginServiceTest` 覆盖正常输入、空用户名、空密码
- Python 登录测试通过

### Day 7：SQLite 基础接入

任务：

- 引入 SQLite 依赖
- 新增 `DbManager`
- 新增数据库初始化脚本
- 创建 `players` 表
- 创建测试玩家数据
- 新增 `PlayerDao`
- 支持按 username 查询玩家
- 支持插入玩家

建议目录：

```text
server/include/db/DbManager.h
server/src/db/DbManager.cpp
server/include/db/PlayerDao.h
server/src/db/PlayerDao.cpp
server/config/schema.sql
server/scripts/init_db.sh
```

建议表结构：

```sql
CREATE TABLE IF NOT EXISTS players (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    rank_score INTEGER NOT NULL DEFAULT 1000,
    win_count INTEGER NOT NULL DEFAULT 0,
    created_at INTEGER NOT NULL
);
```

测试方式：

- 单元测试使用临时 SQLite db
- 测试插入玩家
- 测试查询存在玩家
- 测试查询不存在玩家

验收标准：

- 数据库可以初始化
- `PlayerDao` 能查询玩家
- `LoginService` 可以从数据库读取测试玩家

### Day 8：密码处理与登录校验

任务：

- 增加密码 hash 工具类
- 选择一个简单可解释的 hash 方案用于学习阶段
- 登录时比对密码 hash
- 完善 `LoginResponse` 错误码
- 增加登录失败日志
- Python 脚本增加正确密码和错误密码测试

建议目录：

```text
server/include/security/PasswordHasher.h
server/src/security/PasswordHasher.cpp
server/tests/PasswordHasherTest.cpp
```

实现重点：

- 不在日志中打印明文密码
- DAO 只负责数据访问
- LoginService 负责认证流程

验收标准：

- 正确账号密码登录成功
- 错误密码返回明确错误
- 不存在账号返回明确错误
- 单元测试和 Python 集成测试通过

### Day 9：Session 基础结构

任务：

- 定义 `Session`
- 定义 `SessionState`
- 实现 `SessionManager`
- 支持 connection fd 到 session 的映射
- 登录成功后绑定 player_id
- 连接关闭时清理 session
- 增加在线数量查询
- 增加 SessionManager 单元测试

建议目录：

```text
server/include/session/Session.h
server/include/session/SessionManager.h
server/src/session/SessionManager.cpp
server/tests/SessionManagerTest.cpp
```

状态建议：

```cpp
enum class SessionState {
    Connected,
    Authed,
    Disconnected
};
```

验收标准：

- 新连接可以创建 session
- 登录成功后 session 绑定 player_id
- 断开连接后 session 被清理
- 未登录连接不能进入后续业务 handler

### Day 10：登录链路收尾

任务：

- 整合 `LoginService`
- 整合 `PlayerDao`
- 整合 `SessionManager`
- 登录成功后写 session
- 登录失败返回错误码
- 增加登录日志表或日志文件
- 整理登录相关 proto 字段
- 扩展 Python 登录测试

建议增加：

```text
Login success
Login invalid username
Login invalid password
Login duplicated connection
```

验收标准：

- 登录链路从 TCP 到 DB 到 Session 完整跑通
- 所有登录错误都有响应
- `ctest` 通过
- Python 登录集成测试通过

### Day 11：CI 接入

任务：

- 新增 `.github/workflows/ci.yml`
- 使用 GitHub Actions 的 `ubuntu-24.04`
- 安装构建依赖
- 在 CI 中重新生成 Protobuf C++ 文件
- 运行 CMake configure
- 运行 build
- 运行 ctest
- 运行 Python 语法检查

CI 步骤：

```text
checkout
apt install build-essential cmake ninja-build protobuf-compiler libprotobuf-dev libabsl-dev libspdlog-dev libgtest-dev python3
./server/scripts/gen_proto.sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
python3 -m py_compile server/scripts/test_packet_client.py
```

验收标准：

- GitHub Actions 能自动运行
- push 后 CI 通过
- README 中 CI badge 可选

### Day 12：协议错误处理统一

任务：

- 统一错误码定义
- 扩展 `common.proto`
- 定义通用 `ErrorCode`
- LoginResponse 使用统一错误码
- MessageDispatcher 处理未知 msg_id
- PacketCodec 处理超大包日志
- Python 脚本增加未知 msg_id 测试

验收标准：

- 未知 msg_id 不会导致崩溃
- 非法 Protobuf payload 不会导致崩溃
- 错误响应格式统一

### Day 13：Heartbeat 心跳协议

任务：

- 新增 `HeartbeatRequest`
- 新增 `HeartbeatResponse`
- 分配 msg_id
- 注册 heartbeat handler
- Session 记录 `last_active_time`
- Python 脚本增加 heartbeat 测试

建议消息号：

```text
1101 HeartbeatRequest
1102 HeartbeatResponse
```

验收标准：

- 登录前也可以心跳
- 心跳响应 seq_id 与请求一致
- session last_active_time 更新

### Day 14：定时器基础

任务：

- 实现 `TimerManager`
- 支持一次性定时任务
- 支持周期性定时任务
- EventLoop 每轮检查到期 timer
- 用 timer 扫描超时 session
- 增加 TimerManager 单元测试

建议目录：

```text
server/include/timer/TimerManager.h
server/src/timer/TimerManager.cpp
server/tests/TimerManagerTest.cpp
```

验收标准：

- 定时器能按时间触发
- session 超时后可标记或清理
- 定时器测试通过

### Day 15：Redis 基础接入

任务：

- 选择 Redis C++ 客户端
- 封装 `RedisClient`
- 支持 get/set/del
- 支持 expire
- 支持 list 或 zset 的基础操作
- 编写 Redis 连接配置
- 增加 Redis 可用性检查脚本

建议目录：

```text
server/include/cache/RedisClient.h
server/src/cache/RedisClient.cpp
server/config/server.yaml
```

验收标准：

- 服务端能连接本地 Redis
- 能写入 token 测试 key
- 能设置过期时间
- Redis 不可用时有清晰日志

### Day 16：Token 与在线状态

任务：

- 登录成功生成 token
- Redis 保存 token
- Redis 保存在线状态
- Session 保存 token
- 心跳刷新在线状态
- 断开连接清理在线状态
- Python 脚本验证登录返回 token

Redis key 建议：

```text
token:{token} -> player_id
online:{player_id} -> connection/session info
```

验收标准：

- 登录成功返回 token
- Redis 可以查到 token
- 断开后在线状态被清理或过期

### Day 17：防重复登录

任务：

- 检查 `online:{player_id}`
- 同账号重复登录时选择策略
- 策略一：拒绝新登录
- 策略二：踢掉旧连接
- 当前建议先实现拒绝新登录
- 增加重复登录测试

验收标准：

- 同一账号不能产生两个有效 session
- 重复登录返回明确错误码
- 原连接状态保持正常

### Day 18：MatchService 内存队列

任务：

- 新增 match proto
- 定义 StartMatchRequest/Response
- 定义 MatchSuccessNotify
- 实现 `MatchService`
- 使用内存队列匹配两个玩家
- 登录后才允许进入匹配
- Python 脚本模拟两个连接登录并匹配

建议目录：

```text
server/proto/match.proto
server/include/match/MatchService.h
server/src/match/MatchService.cpp
server/tests/MatchServiceTest.cpp
```

验收标准：

- 单个玩家进入等待队列
- 第二个玩家进入后匹配成功
- 两个连接都收到 MatchSuccessNotify

### Day 19：取消匹配与重复匹配保护

任务：

- 定义 CancelMatchRequest/Response
- MatchService 支持取消匹配
- 防止同一 player_id 重复入队
- 断线时从匹配队列移除
- 增加 MatchService 单元测试

验收标准：

- 玩家可以取消匹配
- 重复点击匹配不会重复入队
- 断开连接不会残留匹配状态

### Day 20：Redis 匹配队列

任务：

- 将内存匹配队列迁移到 Redis
- 使用 list/set 存储匹配队列和玩家状态
- 支持重复匹配保护
- 支持取消匹配
- 保留 MatchService 接口不变

Key 建议：

```text
match:queue
match:player:{player_id}
```

验收标准：

- Redis 中能看到匹配队列
- 匹配成功后 key 被清理
- 服务重启后不会误匹配脏数据

### Day 21：RoomManager

任务：

- 新增 room proto
- 定义 RoomCreatedNotify
- 实现 `Room`
- 实现 `RoomManager`
- 匹配成功后创建房间
- 房间绑定两个 player_id
- 房间状态从 Created 进入 WaitingReady

建议目录：

```text
server/proto/room.proto
server/include/room/Room.h
server/include/room/RoomManager.h
server/src/room/RoomManager.cpp
```

验收标准：

- 匹配成功生成 room_id
- 两个玩家属于同一个 room
- 可以通过 player_id 查询 room_id

### Day 22：房间状态机

任务：

- 定义 RoomState
- 支持 WaitingReady
- 支持 Fighting
- 支持 Finished
- 支持 Destroyed
- 增加状态流转校验
- 非法状态流转返回错误

验收标准：

- 房间状态不能乱跳
- 玩家断线时房间状态可被标记
- RoomManagerTest 覆盖状态流转

### Day 23：Battle proto 与 BattleState

任务：

- 新增 battle proto
- 定义 BattleStartNotify
- 定义 BattlePlayerState
- 定义 SelectSkillRequest/Response
- 实现 `BattleState`
- 初始化双方 HP、ATK、DEF、Energy
- 房间进入 Fighting 后创建 BattleState

验收标准：

- 两个玩家匹配后能收到 BattleStartNotify
- BattleState 中双方初始状态正确
- 单元测试覆盖初始化

### Day 24：技能选择收集

任务：

- 实现 SelectSkill handler
- BattleState 记录玩家本回合选择
- 双方都选择后进入结算准备
- 重复选择返回错误
- 非房间玩家请求返回错误
- 未登录请求返回错误

验收标准：

- 玩家只能选择一次
- 双方选择后触发回合结算入口
- Python 脚本能模拟两人选择技能

### Day 25：回合结算基础

任务：

- 实现普通攻击
- 实现基础伤害公式
- 更新 HP
- 更新 Energy
- 生成 RoundResultNotify
- 判断 HP 是否归零
- 回合数递增

基础公式：

```text
damage = max(1, attacker.atk - defender.def)
```

验收标准：

- 一回合后双方状态变化正确
- 双方连接收到相同 RoundResultNotify
- 单元测试覆盖伤害计算

### Day 26：Timer 驱动回合超时

任务：

- 每回合开始注册超时定时器
- 超时时未选择玩家默认普通攻击
- 已选择玩家不受影响
- 回合结束后取消或忽略旧 timer
- 增加超时测试

验收标准：

- 玩家不发送技能也能推进回合
- 超时后使用默认技能
- 不会重复结算同一回合

### Day 27：Lua 脚本接入

任务：

- 引入 Lua 5.4
- 引入 sol2 或等价绑定方式
- 实现 `ScriptEngine`
- 加载 `lua/skills/skill_1001.lua`
- C++ 调用 Lua 计算普通攻击
- 捕获 Lua 异常

建议目录：

```text
server/include/script/ScriptEngine.h
server/src/script/ScriptEngine.cpp
server/lua/skills/skill_1001.lua
```

验收标准：

- 普通攻击伤害由 Lua 返回
- Lua 报错不会导致服务端崩溃
- Lua 错误有日志

### Day 28：四个技能脚本

任务：

- 实现普通攻击
- 实现重击
- 实现防御姿态
- 实现能量聚焦
- C++ 校验能量是否足够
- Lua 返回技能结果结构
- 单元测试覆盖四个技能

技能建议：

```text
1001 普通攻击：0 能量
1002 重击：2 能量
1003 防御姿态：1 能量
1004 能量聚焦：0 能量，恢复能量
```

验收标准：

- 四个技能都能通过脚本执行
- 能量不足时拒绝释放高消耗技能
- 战斗回合结果包含技能文本

### Day 29：战斗结束与结算

任务：

- 判断胜负
- 发送 BattleEndNotify
- 房间状态置为 Finished
- Session 回到大厅/空闲状态
- 更新玩家临时战斗统计
- 增加完整战斗集成测试

验收标准：

- HP 归零后战斗停止
- 双方收到相同胜负结果
- Finished 房间不再接受技能请求

### Day 30：战斗记录入库

任务：

- 设计 battle_records 表
- 设计 battle_round_records 表
- 保存胜者、败者、回合数
- 保存每回合技能与伤害
- 保存脚本版本字段
- 增加 DAO 和测试

验收标准：

- 战斗结束后数据库有记录
- 每回合结果可以查询
- 记录中包含 battle_id 和 round_index

### Day 31：排行榜 Redis

任务：

- 使用 Redis zset 保存积分
- 战斗结束后更新积分
- 定义 RankListRequest/Response
- 查询排行榜前 N 名
- Python 脚本测试排行榜

Key 建议：

```text
rank:score
```

验收标准：

- 胜者加分
- 败者扣分或保持
- RankListResponse 返回有序列表

### Day 32：断线处理

任务：

- 匹配中断线处理
- 房间中断线处理
- 战斗中断线处理
- Session 清理
- Redis 在线状态清理
- 房间延迟销毁或判负策略

验收标准：

- 匹配中断线不会残留队列
- 战斗中断线不会卡死房间
- 日志能说明断线处理结果

### Day 33：配置系统

任务：

- 新增 server config
- 配置监听端口
- 配置 DB 路径
- 配置 Redis 地址
- 配置日志等级
- 实现 ConfigManager

建议文件：

```text
server/config/server.yaml
```

验收标准：

- 修改配置即可切换端口
- 测试环境可以使用独立 DB
- 配置加载失败有明确日志

### Day 34：日志整理

任务：

- 区分 server/login/match/battle/db/cache 日志
- 统一日志格式
- 对关键业务增加日志
- 对异常输入增加 warning
- 对系统错误增加 error

验收标准：

- 登录、匹配、战斗主流程日志清晰
- 排查一个请求可以通过 seq_id 串起来
- 日志不输出密码等敏感字段

### Day 35：协议文档

任务：

- 新增 `docs/protocol.md`
- 说明包头格式
- 说明 msg_id 规划
- 说明 seq_id 用途
- 说明错误码
- 说明当前 proto 文件

验收标准：

- 只看文档就能写一个测试客户端
- Python 脚本实现和文档一致

### Day 36：架构文档

任务：

- 新增 `docs/architecture.md`
- 画出网络层、协议层、业务层、存储层关系
- 说明 TcpConnection 生命周期
- 说明 MessageDispatcher 扩展方式
- 说明 Session 和 Room 的关系

验收标准：

- 面试时能按文档讲清整体架构
- 每个模块职责清晰

### Day 37：集成测试整理

任务：

- 重构 Python 测试脚本
- 支持指定 case
- 支持并发连接数
- 支持请求数
- 输出通过率和耗时
- 增加登录、匹配、战斗基础流程测试

验收标准：

- 一条命令可以跑核心集成测试
- 失败时能看到具体 case 和原因

### Day 38：基础压测

任务：

- Python 脚本增加 benchmark 模式
- 模拟 N 个连接
- 每个连接发送 M 个请求
- 统计总耗时、平均延迟、失败数
- 记录测试环境

验收标准：

- 可以得到基础吞吐数据
- 服务端不会因为普通压测崩溃
- README 只记录测试方法，不夸大性能结果

### Day 39：边界与异常测试

任务：

- 超大包测试
- 非法 length 测试
- 非法 Protobuf payload 测试
- 未知 msg_id 测试
- 重复登录测试
- 重复匹配测试
- 战斗非法技能测试

验收标准：

- 异常输入不会导致崩溃
- 连接关闭或错误响应符合预期
- 核心异常都有测试覆盖

### Day 40：代码整理

任务：

- 清理 include 顺序
- 清理命名不一致
- 清理无用函数
- 保持模块边界
- 避免业务代码堆在 TcpServer
- 整理 CMake target

验收标准：

- build 无业务代码警告
- 目录结构清晰
- 新增模块能快速找到位置

### Day 41：CI 完善

任务：

- CI 增加 proto 同步检查
- CI 增加格式检查可选
- CI 缓存依赖可选
- README 增加 CI 状态说明

验收标准：

- PR 或 push 自动跑完整构建测试
- proto 改动未生成代码时 CI 能发现

### Day 42：README 完善

任务：

- 更新当前能力
- 更新构建步骤
- 更新测试步骤
- 更新模块说明
- 更新后续计划
- 保持描述准确、专业、不过度包装

验收标准：

- 新读者 5 分钟能理解项目
- 新环境按 README 可以构建测试

### Day 43：面试问答整理

任务：

- 整理网络层问题
- 整理协议层问题
- 整理 Session 问题
- 整理匹配/房间问题
- 整理战斗系统问题
- 整理 Redis/DB 问题

建议输出：

```text
docs/interview_notes.md
```

验收标准：

- 每个问题都能结合项目代码回答
- 不背概念，能说出自己实现中的取舍

### Day 44：项目复盘

任务：

- 整理项目从 Day 1 到当前的演进
- 记录踩坑：epoll、生命周期、Protobuf、CMake、Redis、Lua
- 记录性能和稳定性改进点
- 记录后续可扩展方向

建议输出：

```text
docs/project_review.md
```

验收标准：

- 能讲清为什么这样设计
- 能讲清还有哪些不足和下一步计划

### Day 45：阶段版本整理

任务：

- 跑完整 build
- 跑完整 ctest
- 跑完整 Python 集成测试
- 更新 README 和 docs
- 打 tag
- 准备简历项目描述

建议 tag：

```bash
git tag server-v0.1
```

验收标准：

- 仓库结构整洁
- 文档和代码状态一致
- 一条命令能构建
- 测试全部通过
- 项目描述准确、专业、可解释

## 9. 项目表达

这个项目适合表达为一个持续演进的 C++ 游戏服务端训练项目，重点突出：

- 从零实现 C++20 游戏服务端基础网络框架
- 使用 epoll/Reactor 管理 TCP 连接
- 设计自定义二进制包头并处理粘包半包
- 接入 Protobuf 并实现消息分发
- 使用 GoogleTest 和 Python 脚本验证协议链路

后续会继续围绕 Session、数据库、Redis、匹配、房间、战斗和 Lua 脚本扩展服务端业务闭环。

## 10. 面试准备方向

重点准备：

- 为什么 TCP 需要拆包
- length 字段为什么不包含自身
- seq_id 的用途
- Protobuf 和 JSON 的取舍
- Reactor 模型的职责划分
- Buffer 如何避免频繁移动内存
- 非阻塞 socket 如何处理 EAGAIN
- 连接断开时如何管理生命周期
- MessageDispatcher 如何扩展
- 单元测试和集成测试分别覆盖什么
