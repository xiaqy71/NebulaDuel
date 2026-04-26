# Nebula Duel 工业级开发文档

## 1. 项目目标

### 项目定位

Nebula Duel 是一个 2D 房间制 1v1 回合对战游戏。

完整玩家流程：

```text
启动客户端
-> 登录 / 注册
-> 进入大厅
-> 点击匹配
-> 匹配成功
-> 进入房间
-> 双方选择技能
-> 服务器结算回合
-> 客户端播放表现
-> 战斗结束
-> 结算胜负
-> 更新排行榜
```

### 技术目标

服务端必须体现游戏服务器核心能力：

- C++20 工程化项目
- Linux epoll / Reactor 网络模型
- Protobuf 协议
- Lua 5.4 / sol2 脚本系统
- Redis 缓存、匹配队列与排行榜
- SQLite 或 MySQL 持久化
- spdlog 日志
- GoogleTest 单元测试
- Bash 自动化构建、启动、测试
- Git 管理代码和 Lua 脚本版本

客户端必须体现完整游戏流程：

- Godot 4
- 2D UI
- 登录界面
- 大厅界面
- 匹配界面
- 战斗界面
- 排行榜界面
- TCP 或 WebSocket 通信
- Protobuf 或 JSON 协议解析

### 最终目标

完成一个可以演示的全栈游戏 Demo：

- 两个客户端可以登录
- 可以匹配到彼此
- 可以进入同一个房间
- 可以选择技能
- 服务端权威结算
- 客户端显示 HP、能量、回合、技能结果
- 战斗结束写数据库
- 排行榜写 Redis
- 可以查看排行榜

## 2. 游戏玩法设计

### 2.1 基础规则

游戏类型：

```text
1v1 回合制对战
```

每名玩家拥有：

| 属性 | 说明 |
|---|---|
| HP | 生命值，归零失败 |
| ATK | 基础攻击 |
| DEF | 基础防御 |
| Energy | 能量，用于释放技能 |
| MaxEnergy | 最大能量 |
| WinCount | 胜场 |
| RankScore | 排行榜积分 |

初始属性建议：

```text
HP: 100
ATK: 10
DEF: 5
Energy: 3
MaxEnergy: 10
```

每回合流程：

```text
1. 服务器广播回合开始
2. 双方客户端选择技能
3. 客户端发送技能选择
4. 服务器等待双方输入或超时
5. 服务器调用 Lua 计算技能效果
6. 服务器更新战斗状态
7. 服务器广播回合结果
8. 客户端播放表现
9. 判断是否结束
```

回合选择时间：

```text
15 秒
```

如果玩家超时：

```text
默认使用普通攻击
```

### 2.2 技能设计

第一版只做 4 个技能，保证可玩。

| 技能 ID | 名称 | 消耗能量 | 效果 |
|---|---|---:|---|
| 1001 | 普通攻击 | 0 | 造成 ATK - DEF 修正伤害 |
| 1002 | 重击 | 2 | 造成较高伤害 |
| 1003 | 防御姿态 | 1 | 本回合减伤 |
| 1004 | 能量聚焦 | 0 | 恢复能量 |

客户端表现必须包含：

- HP 条变化
- 能量变化
- 技能文字提示
- 伤害数字
- 回合结果日志
- 胜负结算弹窗

## 3. 服务端架构

### 3.1 总体架构

```text
Client Godot
    |
 TCP/WebSocket
    |
Gateway / Net Layer
    |
SessionManager
    |
MessageDispatcher
    |
+-------------------------+
| Login / Lobby / Match   |
| Room / Battle / Rank    |
+-------------------------+
    |
+-------------------------+
| Redis / DB / Lua / Log  |
+-------------------------+
```

### 服务端核心原则

- 客户端只提交输入
- 服务端维护真实状态
- 所有战斗结果由服务端计算
- 客户端不可直接修改 HP、胜负、排名
- 关键逻辑可测试
- 战斗状态可序列化、可日志追踪

### 3.2 推荐目录结构

```text
NebulaDuel/
  server/
    CMakeLists.txt
    cmake/
    proto/
      common.proto
      login.proto
      lobby.proto
      match.proto
      battle.proto
      rank.proto
    scripts/
      build.sh
      run_server.sh
      run_tests.sh
      gen_proto.sh
      start_redis.sh
      init_db.sh
    config/
      server.yaml
      db.sql
      redis.conf
    lua/
      skills/
        skill_1001.lua
        skill_1002.lua
        skill_1003.lua
        skill_1004.lua
      battle/
        battle_rules.lua
    src/
      main.cpp
      core/
        Application.h
        Application.cpp
      net/
        EpollPoller.h
        EventLoop.h
        TcpServer.h
        TcpConnection.h
        Buffer.h
      protocol/
        Packet.h
        PacketCodec.h
        MessageDispatcher.h
      session/
        Session.h
        SessionManager.h
      login/
        LoginService.h
      lobby/
        LobbyService.h
      match/
        MatchService.h
      room/
        Room.h
        RoomManager.h
      battle/
        BattleService.h
        BattleRoom.h
        BattleContext.h
      script/
        ScriptEngine.h
        ScriptVersionManager.h
      cache/
        CacheManager.h
        RedisClient.h
      db/
        DbManager.h
        PlayerDao.h
        BattleRecordDao.h
      timer/
        TimerManager.h
      logger/
        Logger.h
      admin/
        AdminService.h
    tests/
      test_packet_codec.cpp
      test_battle.cpp
      test_match.cpp
      test_lua_skill.cpp
  client/
    NebulaDuelGodot/
      project.godot
      scenes/
        LoginScene.tscn
        LobbyScene.tscn
        MatchScene.tscn
        BattleScene.tscn
        RankScene.tscn
      scripts/
        NetworkClient.gd
        MessageDispatcher.gd
        UIManager.gd
        LoginScene.gd
        LobbyScene.gd
        MatchScene.gd
        BattleScene.gd
        RankScene.gd
        BattleView.gd
      assets/
        ui/
        effects/
  docs/
    architecture.md
    protocol.md
    db.md
    redis.md
    lua_skill.md
```

### 3.3 服务端模块设计

#### Net 网络层

职责：

- epoll 事件循环
- 监听客户端连接
- 处理 TCP 读写
- 管理连接生命周期
- 提供消息回调给 Protocol 层

核心类：

```cpp
EventLoop
EpollPoller
TcpServer
TcpConnection
Buffer
```

第一版建议：

- 单线程 Reactor
- 后续可扩展为主从 Reactor
- 每个连接维护输入/输出缓冲区
- 粘包拆包交给 PacketCodec

#### Protocol 协议层

职责：

- 处理包头
- Protobuf 编解码
- 消息 ID 分发
- 请求与响应对应

包结构建议：

```text
uint32 packet_len
uint16 msg_id
uint32 seq_id
bytes protobuf_body
```

| 字段 | 说明 |
|---|---|
| packet_len | 整包长度，不含自身或含自身需统一 |
| msg_id | 协议号 |
| seq_id | 请求序号 |
| protobuf_body | Protobuf 序列化内容 |

#### Session 会话层

职责：

- 绑定连接和玩家
- 记录登录状态
- 管理 token
- 断线处理
- 防重复登录

Session 状态：

```cpp
enum class SessionState {
    Connected,
    Authed,
    InLobby,
    Matching,
    InRoom,
    Disconnected
};
```

#### Login 登录系统

职责：

- 注册账号
- 登录校验
- 生成 token
- Redis 保存 session
- MySQL/SQLite 写登录日志

登录成功后：

```text
1. 校验账号密码
2. 生成 token
3. Redis 写入 token
4. Redis 写在线状态
5. 创建 Session
6. 返回玩家基础数据
```

#### Lobby 大厅系统

职责：

- 玩家进入大厅
- 获取基础信息
- 获取排行榜摘要
- 处理进入匹配

#### Match 匹配系统

职责：

- 玩家加入匹配队列
- 取消匹配
- 匹配成功创建房间

第一版匹配规则：

```text
队列中有两个玩家即可匹配成功
```

后续可扩展：

```text
按 RankScore 范围匹配
等待时间越长，匹配范围越大
```

#### Room 房间系统

职责：

- 创建房间
- 管理房间玩家
- 房间状态流转
- 进入战斗

房间状态：

```cpp
enum class RoomState {
    Created,
    WaitingReady,
    Fighting,
    Finished,
    Destroyed
};
```

#### Battle 战斗系统

职责：

- 维护权威战斗状态
- 接收玩家技能选择
- 处理回合计时
- 调用 Lua 技能逻辑
- 广播回合结果
- 判断胜负
- 写战斗记录
- 更新排行榜

核心状态：

```cpp
struct BattlePlayer {
    uint64_t playerId;
    int hp;
    int atk;
    int def;
    int energy;
    int maxEnergy;
    int selectedSkillId;
    bool hasSelected;
};

struct BattleRoom {
    uint64_t roomId;
    int round;
    BattlePlayer playerA;
    BattlePlayer playerB;
    BattleState state;
};
```

#### ScriptEngine Lua 脚本系统

职责：

- 加载 Lua 5.4 脚本
- 用 sol2 暴露 C++ 战斗上下文
- 调用技能函数
- 捕获脚本错误
- 支持重载脚本

技能脚本输入：

```text
caster
target
battle_context
```

技能脚本输出：

```lua
{
  damage = 12,
  heal = 0,
  energy_delta = -2,
  shield = 0,
  text = "Heavy Strike"
}
```

#### ScriptVersionManager

职责：

- 记录当前 Lua 脚本版本
- 使用 Git commit hash 标记脚本版本
- 战斗记录写入脚本版本
- 支持管理命令 reload

最低实现：

```text
启动时读取 lua/ 当前 git commit hash
战斗记录 battle_records.script_version 写入 hash
```

#### CacheManager Redis 模块

职责：

- token/session
- 在线状态
- 匹配队列
- 排行榜
- 登录锁
- 玩家热数据

建议 C++ Redis 客户端：

```text
redis-plus-plus
```

#### DbManager 数据库模块

个人项目建议优先用 SQLite：

- 环境简单
- 方便演示
- 30-45 天更稳

如果想贴近公司环境，可以 MySQL。

推荐策略：

```text
开发期 SQLite
文档中保留 MySQL 兼容设计
```

#### TimerManager 定时器模块

职责：

- 回合超时
- 匹配超时
- session 过期扫描
- 房间销毁延迟

第一版推荐：

```text
EventLoop 内部最小堆定时器
```

#### Logger 日志模块

使用 spdlog。

日志类型：

```text
server.log
login.log
battle.log
error.log
```

关键日志：

- 登录成功/失败
- 匹配成功
- 房间创建
- 技能选择
- 回合结算
- 战斗结束
- Redis/DB 错误
- Lua 错误

#### Admin 管理接口

第一版可以做 TCP 内部命令或命令行工具。

最低命令：

```text
reload_lua
room_count
online_count
kick player_id
```

## 4. 客户端架构

### 4.1 Godot 总体结构

```text
NetworkClient
    |
MessageDispatcher
    |
UIManager
    |
Scenes
```

### 客户端原则

- 不计算最终伤害
- 不决定胜负
- 不相信本地状态
- 所有战斗状态以服务器广播为准
- 本地只做 UI、动画、输入、提示

### 4.2 客户端模块

#### NetworkClient

职责：

- 连接服务器
- 发送协议
- 接收协议
- 断线重连提示
- 心跳

Godot 可选：

```text
StreamPeerTCP
WebSocketPeer
```

建议第一版：

```text
TCP + JSON
```

如果想强化简历：

```text
TCP + Protobuf
```

实际建议：

```text
服务端 Protobuf 为主
客户端可以先 JSON，后续再 Protobuf
```

#### LoginScene

功能：

- 输入账号
- 输入密码
- 注册按钮
- 登录按钮
- 错误提示
- 登录成功进入 LobbyScene

#### LobbyScene

功能：

- 显示玩家名
- 显示胜场/积分
- 开始匹配按钮
- 排行榜按钮
- 退出登录按钮

#### MatchScene

功能：

- 显示匹配中
- 显示等待时间
- 取消匹配按钮
- 匹配成功跳转战斗

#### BattleScene

功能：

- 显示双方 HP
- 显示双方能量
- 显示当前回合
- 显示技能按钮
- 显示倒计时
- 显示战斗日志
- 显示结算弹窗

#### RankScene

功能：

- 请求排行榜
- 显示前 20 名
- 显示自己排名
- 返回大厅

## 5. 通信协议设计

### 5.1 协议号规划

```text
1000 - 1999: 系统协议
2000 - 2999: 登录协议
3000 - 3999: 大厅协议
4000 - 4999: 匹配协议
5000 - 5999: 房间协议
6000 - 6999: 战斗协议
7000 - 7999: 排行榜协议
9000 - 9999: 管理协议
```

### 5.2 核心协议

| MsgId | 名称 | 方向 |
|---:|---|---|
| 1001 | HeartbeatReq | C -> S |
| 1002 | HeartbeatResp | S -> C |
| 2001 | RegisterReq | C -> S |
| 2002 | RegisterResp | S -> C |
| 2003 | LoginReq | C -> S |
| 2004 | LoginResp | S -> C |
| 3001 | EnterLobbyReq | C -> S |
| 3002 | EnterLobbyResp | S -> C |
| 4001 | StartMatchReq | C -> S |
| 4002 | StartMatchResp | S -> C |
| 4003 | CancelMatchReq | C -> S |
| 4004 | MatchSuccessNotify | S -> C |
| 5001 | EnterRoomNotify | S -> C |
| 6001 | BattleStartNotify | S -> C |
| 6002 | SelectSkillReq | C -> S |
| 6003 | SelectSkillResp | S -> C |
| 6004 | RoundResultNotify | S -> C |
| 6005 | BattleEndNotify | S -> C |
| 7001 | RankListReq | C -> S |
| 7002 | RankListResp | S -> C |

## 6. Protobuf 消息设计

### common.proto

```proto
syntax = "proto3";

package nebula;

message ErrorInfo {
  int32 code = 1;
  string message = 2;
}

message PlayerBrief {
  uint64 player_id = 1;
  string name = 2;
  int32 rank_score = 3;
  int32 win_count = 4;
}
```

### login.proto

```proto
syntax = "proto3";

package nebula;

import "common.proto";

message RegisterReq {
  string username = 1;
  string password = 2;
}

message RegisterResp {
  bool success = 1;
  ErrorInfo error = 2;
}

message LoginReq {
  string username = 1;
  string password = 2;
}

message LoginResp {
  bool success = 1;
  ErrorInfo error = 2;
  string token = 3;
  PlayerBrief player = 4;
}
```

### match.proto

```proto
syntax = "proto3";

package nebula;

message StartMatchReq {
}

message StartMatchResp {
  bool success = 1;
  string message = 2;
}

message CancelMatchReq {
}

message MatchSuccessNotify {
  uint64 room_id = 1;
  uint64 opponent_id = 2;
  string opponent_name = 3;
}
```

### battle.proto

```proto
syntax = "proto3";

package nebula;

message BattlePlayerState {
  uint64 player_id = 1;
  string name = 2;
  int32 hp = 3;
  int32 energy = 4;
  int32 atk = 5;
  int32 def = 6;
}

message BattleStartNotify {
  uint64 room_id = 1;
  int32 round = 2;
  BattlePlayerState self = 3;
  BattlePlayerState opponent = 4;
}

message SelectSkillReq {
  uint64 room_id = 1;
  int32 skill_id = 2;
}

message SelectSkillResp {
  bool success = 1;
  string message = 2;
}

message SkillResult {
  uint64 caster_id = 1;
  uint64 target_id = 2;
  int32 skill_id = 3;
  int32 damage = 4;
  int32 heal = 5;
  int32 energy_delta = 6;
  string text = 7;
}

message RoundResultNotify {
  uint64 room_id = 1;
  int32 round = 2;
  repeated SkillResult results = 3;
  BattlePlayerState player_a = 4;
  BattlePlayerState player_b = 5;
}

message BattleEndNotify {
  uint64 room_id = 1;
  uint64 winner_id = 2;
  uint64 loser_id = 3;
  int32 rank_score_delta = 4;
}
```

### rank.proto

```proto
syntax = "proto3";

package nebula;

import "common.proto";

message RankListReq {
  int32 offset = 1;
  int32 limit = 2;
}

message RankListResp {
  repeated PlayerBrief players = 1;
  int32 self_rank = 2;
}
```

## 7. Redis Key 设计

### 7.1 登录与 Session

```text
token:{token} -> player_id
session:{player_id} -> token
online:{player_id} -> 1
login_lock:{username} -> 1
```

TTL：

| Key | TTL |
|---|---:|
| token:{token} | 24h |
| session:{player_id} | 24h |
| online:{player_id} | 连接期间 |
| login_lock:{username} | 5s |

### 7.2 匹配队列

```text
match:queue -> list/player_id
match:player:{player_id} -> timestamp
```

操作：

```text
LPUSH match:queue player_id
RPOP match:queue
DEL match:player:{player_id}
```

第一版也可以只在 C++ 内存做匹配，Redis 做展示用途。为了贴合项目要求，建议 Redis 队列参与匹配。

### 7.3 排行榜

```text
rank:score -> zset
```

结构：

```text
ZADD rank:score score player_id
ZREVRANGE rank:score 0 19 WITHSCORES
ZREVRANK rank:score player_id
```

### 7.4 玩家热数据

```text
player:hot:{player_id} -> hash
```

字段：

```text
name
rank_score
win_count
last_login_time
```

## 8. 数据库表设计

以 SQLite/MySQL 兼容设计为例。

### players

```sql
CREATE TABLE players (
  player_id INTEGER PRIMARY KEY AUTOINCREMENT,
  username VARCHAR(64) NOT NULL UNIQUE,
  password_hash VARCHAR(128) NOT NULL,
  nickname VARCHAR(64) NOT NULL,
  rank_score INTEGER NOT NULL DEFAULT 1000,
  win_count INTEGER NOT NULL DEFAULT 0,
  lose_count INTEGER NOT NULL DEFAULT 0,
  created_at DATETIME NOT NULL,
  updated_at DATETIME NOT NULL
);
```

### player_profiles

```sql
CREATE TABLE player_profiles (
  player_id INTEGER PRIMARY KEY,
  level INTEGER NOT NULL DEFAULT 1,
  exp INTEGER NOT NULL DEFAULT 0,
  avatar_id INTEGER NOT NULL DEFAULT 0,
  last_login_at DATETIME
);
```

### battle_records

```sql
CREATE TABLE battle_records (
  battle_id INTEGER PRIMARY KEY AUTOINCREMENT,
  room_id INTEGER NOT NULL,
  player_a_id INTEGER NOT NULL,
  player_b_id INTEGER NOT NULL,
  winner_id INTEGER NOT NULL,
  loser_id INTEGER NOT NULL,
  rounds INTEGER NOT NULL,
  script_version VARCHAR(64) NOT NULL,
  created_at DATETIME NOT NULL
);
```

### battle_round_records

```sql
CREATE TABLE battle_round_records (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  battle_id INTEGER NOT NULL,
  round INTEGER NOT NULL,
  player_a_skill INTEGER NOT NULL,
  player_b_skill INTEGER NOT NULL,
  player_a_hp INTEGER NOT NULL,
  player_b_hp INTEGER NOT NULL,
  detail_json TEXT NOT NULL
);
```

### skill_versions

```sql
CREATE TABLE skill_versions (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  version_hash VARCHAR(64) NOT NULL,
  description TEXT,
  created_at DATETIME NOT NULL
);
```

### login_logs

```sql
CREATE TABLE login_logs (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  player_id INTEGER,
  username VARCHAR(64) NOT NULL,
  ip VARCHAR(64),
  success INTEGER NOT NULL,
  reason VARCHAR(128),
  created_at DATETIME NOT NULL
);
```

## 9. Lua 脚本设计

### 9.1 技能脚本规范

每个技能一个 Lua 文件：

```text
lua/skills/skill_1001.lua
lua/skills/skill_1002.lua
lua/skills/skill_1003.lua
lua/skills/skill_1004.lua
```

统一接口：

```lua
function cast(caster, target, context)
  return {
    damage = 0,
    heal = 0,
    energy_delta = 0,
    shield = 0,
    text = ""
  }
end
```

### 9.2 普通攻击

```lua
function cast(caster, target, context)
  local damage = math.max(1, caster.atk - math.floor(target.def * 0.5))

  return {
    damage = damage,
    heal = 0,
    energy_delta = 1,
    shield = 0,
    text = "Normal Attack"
  }
end
```

### 9.3 重击

```lua
function cast(caster, target, context)
  if caster.energy < 2 then
    return {
      damage = 0,
      heal = 0,
      energy_delta = 0,
      shield = 0,
      text = "Not enough energy"
    }
  end

  local damage = math.max(1, caster.atk * 2 - target.def)

  return {
    damage = damage,
    heal = 0,
    energy_delta = -2,
    shield = 0,
    text = "Heavy Strike"
  }
end
```

### 9.4 防御姿态

```lua
function cast(caster, target, context)
  return {
    damage = 0,
    heal = 0,
    energy_delta = -1,
    shield = 10,
    text = "Guard"
  }
end
```

### 9.5 能量聚焦

```lua
function cast(caster, target, context)
  return {
    damage = 0,
    heal = 0,
    energy_delta = 3,
    shield = 0,
    text = "Focus"
  }
end
```

## 10. Git 分支策略

### 分支模型

```text
main
develop
feature/server-net
feature/server-login
feature/server-battle
feature/client-login
feature/client-battle
release/demo-v1
```

### 规则

| 分支 | 作用 |
|---|---|
| main | 稳定可演示版本 |
| develop | 日常集成 |
| feature/* | 功能开发 |
| release/* | 阶段版本 |
| hotfix/* | 紧急修复 |

### 提交规范

```text
feat: add battle room state machine
fix: handle client disconnect in match queue
test: add lua skill unit tests
docs: update protocol document
build: add protobuf generation script
```

Lua 脚本每次修改必须提交，例如：

```text
feat(lua): adjust heavy strike damage
```

战斗记录保存当前 commit hash。

## 11. Bash 自动化脚本规划

### scripts/build.sh

职责：

```text
创建 build 目录
运行 cmake -G Ninja
运行 ninja
```

### scripts/gen_proto.sh

职责：

```text
根据 proto 文件生成 C++ 代码
可选生成 Godot 可用协议文件
```

### scripts/run_server.sh

职责：

```text
启动服务端
加载 config/server.yaml
输出日志路径
```

### scripts/run_tests.sh

职责：

```text
编译并运行 GoogleTest
输出测试结果
```

### scripts/init_db.sh

职责：

```text
创建 SQLite 数据库
执行 db.sql
插入测试账号
```

### scripts/start_redis.sh

职责：

```text
本地启动 Redis
或检测 Redis 是否可连接
```

### scripts/dev_all.sh

职责：

```text
生成协议
构建服务端
初始化数据库
启动 Redis
启动服务器
```

## 12. 30-45 天开发计划

建议做 6 个阶段。

| 阶段 | 时间 | 目标 |
|---|---:|---|
| 阶段 1 | Day 1-5 | 工程骨架、网络、协议 |
| 阶段 2 | Day 6-10 | 登录、大厅、数据库、Redis |
| 阶段 3 | Day 11-16 | 匹配、房间、客户端流程 |
| 阶段 4 | Day 17-25 | 战斗系统、Lua 技能、回合制 |
| 阶段 5 | Day 26-33 | 排行榜、记录、断线、测试 |
| 阶段 6 | Day 34-45 | 打磨、Demo、文档、简历、面试准备 |

## 13. 每天 5 小时以上任务拆解

下面按 45 天规划。若压缩到 30 天，可以把后 15 天的打磨、测试、文档合并到每天晚上。

### Day 1：项目初始化

任务：

- 创建 Git 仓库
- 建立 server/client/docs 目录
- 配置 CMake + Ninja
- 引入 spdlog
- 写 main.cpp 启动日志
- Godot 创建空项目

学习目标：

- CMake 基础结构
- C++20 编译配置
- Godot 项目结构

验收标准：

- `build.sh` 可以编译 server
- server 启动打印日志
- Godot 项目可运行空场景

### Day 2：网络层 EventLoop

任务：

- 实现 EpollPoller
- 实现 EventLoop
- 支持 fd 注册、删除、事件分发
- 写简单 echo server
- Godot 创建 LoginScene UI 雏形

学习目标：

- epoll LT/ET 差异
- Reactor 模型

验收标准：

- telnet/nc 能连接 server
- 服务端能回显消息
- 客户端有登录界面

### Day 3：TcpServer 与 TcpConnection

任务：

- 封装 TcpServer
- 封装 TcpConnection
- 实现 Buffer
- 处理断开连接
- NetworkClient.gd 支持连接服务器

学习目标：

- 非阻塞 socket
- 输入输出缓冲区设计

验收标准：

- 多客户端可连接
- 断开连接不崩溃
- Godot 能连接服务端

### Day 4：协议包头与拆包

任务：

- 实现 PacketCodec
- 支持 length + msg_id + seq_id
- 解决粘包半包
- 写 GoogleTest
- Godot 可以发送测试包

学习目标：

- TCP 流式协议
- 二进制协议设计

验收标准：

- 单元测试通过
- 服务端能解析客户端测试消息

### Day 5：Protobuf 接入

任务：

- 编写 common/login proto
- gen_proto.sh
- C++ 接入 protobuf
- MessageDispatcher 初版
- 客户端建立 JSON 或 Protobuf 协议适配层

学习目标：

- Protobuf 编译与序列化
- 协议 ID 分发

验收标准：

- LoginReq/LoginResp 可以序列化
- 服务端收到登录请求并返回假数据

### Day 6：数据库初始化

任务：

- SQLite/MySQL 表结构
- DbManager
- PlayerDao
- init_db.sh
- LoginScene 接入真实登录按钮

学习目标：

- DAO 分层
- 账号表设计

验收标准：

- 可以创建账号表
- 可以查询玩家
- 登录按钮能收到服务端响应

### Day 7：注册系统

任务：

- RegisterReq/RegisterResp
- 密码 hash
- 注册写数据库
- 登录日志表
- LoginScene 加注册按钮

学习目标：

- 账号安全基础
- 错误码设计

验收标准：

- 新账号可注册
- 重复账号返回错误
- 客户端显示错误提示

### Day 8：登录系统

任务：

- LoginService
- token 生成
- Session 绑定 player_id
- Redis token/session
- 登录成功进入 LobbyScene

学习目标：

- Session 管理
- Redis 基础操作

验收标准：

- 正确账号可以登录
- 错误账号不能登录
- Redis 能看到 token

### Day 9：大厅系统

任务：

- EnterLobbyReq/Resp
- 玩家基础信息
- LobbyService
- 在线状态 Redis
- LobbyScene 显示昵称、积分、胜场

学习目标：

- 游戏大厅状态管理

验收标准：

- 登录后进入大厅
- 大厅显示真实玩家信息

### Day 10：心跳与断线

任务：

- HeartbeatReq/Resp
- Session 超时检测
- 断线清理在线状态
- NetworkClient 心跳
- 断线提示

学习目标：

- 连接保活
- 异常断线处理

验收标准：

- 客户端定期心跳
- 断线后 Redis online 被清理

### Day 11：匹配系统初版

任务：

- StartMatchReq/Resp
- MatchService 内存队列
- 两人匹配成功
- MatchScene
- 点击匹配进入匹配界面

学习目标：

- 匹配队列设计
- 玩家状态机

验收标准：

- 两个客户端点击匹配后匹配成功
- 客户端收到 MatchSuccessNotify

### Day 12：Redis 匹配队列

任务：

- match:queue
- match:player:{id}
- 取消匹配
- 重复匹配保护
- 取消匹配按钮

学习目标：

- Redis list/set 使用
- 分布式匹配基础

验收标准：

- 玩家进入 Redis 匹配队列
- 取消匹配生效
- 重复点击不会重复入队

### Day 13：房间系统

任务：

- RoomManager
- Room 创建与销毁
- EnterRoomNotify
- 房间状态机
- 匹配成功后切到 BattleScene

学习目标：

- 房间制游戏架构

验收标准：

- 匹配成功创建 room_id
- 双方进入同一个房间

### Day 14：战斗房间初始化

任务：

- BattleRoom
- BattlePlayerState
- BattleStartNotify
- 初始化 HP/ATK/DEF/Energy
- BattleScene 显示双方状态

学习目标：

- 战斗状态建模

验收标准：

- 进入战斗后双方 HP/能量显示正确

### Day 15：技能选择协议

任务：

- SelectSkillReq/Resp
- 记录玩家选择
- 双方都选择后进入结算
- BattleScene 增加 4 个技能按钮

学习目标：

- 输入收集与状态同步

验收标准：

- 玩家点击技能后按钮锁定
- 服务端收到双方技能选择

### Day 16：回合定时器

任务：

- TimerManager
- 回合 15 秒超时
- 超时默认普通攻击
- 客户端显示倒计时

学习目标：

- 游戏定时器设计

验收标准：

- 不选择技能时自动普通攻击
- 客户端倒计时正常

### Day 17：Lua 接入

任务：

- 引入 Lua 5.4 / sol2
- ScriptEngine
- 加载 skill_1001.lua
- C++ 调 Lua
- 战斗日志显示技能名

学习目标：

- C++ 与 Lua 交互

验收标准：

- 普通攻击由 Lua 返回伤害
- Lua 错误不会导致服务端崩溃

### Day 18：完成 4 个技能

任务：

- skill_1001 普通攻击
- skill_1002 重击
- skill_1003 防御
- skill_1004 聚能
- 技能能量校验
- 技能按钮显示消耗

学习目标：

- 脚本化战斗逻辑

验收标准：

- 4 个技能都可用
- 能量不足不能释放高消耗技能

### Day 19：回合结算

任务：

- RoundResultNotify
- 伤害、护盾、能量更新
- 回合数递增
- HP 条变化
- 能量条变化
- 战斗日志追加

学习目标：

- 服务端权威同步

验收标准：

- 双方客户端看到一致结果

### Day 20：胜负判断

任务：

- HP <= 0 判断胜负
- BattleEndNotify
- 房间状态 Finished
- 结算弹窗
- 返回大厅按钮

学习目标：

- 战斗生命周期

验收标准：

- 一方 HP 归零后战斗结束
- 双方看到胜负结果

### Day 21：战斗记录入库

任务：

- battle_records
- battle_round_records
- 保存每回合结果
- 保存 script_version
- 结算界面显示回合数

学习目标：

- 战斗回放数据结构

验收标准：

- 战斗结束后 DB 有记录

### Day 22：排行榜 Redis

任务：

- 胜负后更新 rank:score
- RankListReq/Resp
- 查询前 20
- RankScene 初版

学习目标：

- Redis zset 排行榜

验收标准：

- 胜者加分
- 排行榜可显示

### Day 23：大厅接入排行榜入口

任务：

- 大厅请求玩家最新积分
- 排行榜返回自己的名次
- LobbyScene -> RankScene -> LobbyScene

学习目标：

- UI 页面流转

验收标准：

- 玩家可从大厅查看排行榜并返回

### Day 24：战斗表现打磨

任务：

- 战斗结果增加 text
- 技能结果结构完善
- 日志更清晰
- 伤害数字
- 技能提示
- 简单闪烁/颜色反馈

学习目标：

- 客户端表现层与数据层分离

验收标准：

- 战斗看起来像游戏，不像接口测试

### Day 25：断线处理

任务：

- 战斗中断线
- 大厅断线
- 匹配中断线
- Session 清理
- 断线弹窗
- 返回登录

学习目标：

- 异常状态处理

验收标准：

- 玩家断线不会卡死房间
- 匹配队列不会残留脏数据

### Day 26：防重复登录

任务：

- login_lock
- 同账号重复登录踢旧连接或拒绝新连接
- session:{player_id}
- 重复登录提示

学习目标：

- 登录互斥设计

验收标准：

- 同账号不能产生两个有效在线状态

### Day 27：配置系统

任务：

- server.yaml
- 端口、DB、Redis、日志路径配置
- ConfigManager
- 客户端服务器地址配置

学习目标：

- 配置化服务

验收标准：

- 修改配置即可切换端口/数据库路径

### Day 28：Admin 命令

任务：

- reload_lua
- online_count
- room_count
- 简单 admin 控制台
- 客户端 UI 修正

学习目标：

- 运营管理接口

验收标准：

- 不重启服务端可 reload Lua

### Day 29：Lua 版本管理

任务：

- ScriptVersionManager
- 获取 git commit hash
- 写入 skill_versions
- 战斗记录绑定版本

学习目标：

- 热更新与版本追踪思想

验收标准：

- 每场战斗记录有 script_version

### Day 30：第一版完整 Demo

任务：

- 从注册到排行榜全流程联调
- 修复阻塞 bug
- 打 tag：demo-v0.1
- 全流程可跑

学习目标：

- 集成测试

验收标准：

- 两个客户端可以完整打一局
- 排行榜更新
- 数据库有记录

### Day 31-35：测试与稳定性

任务：

- GoogleTest 覆盖 PacketCodec
- BattleService 测试
- MatchService 测试
- Lua 技能测试
- 异常输入测试
- 修 UI 状态错乱
- 防止重复点击
- 网络错误提示

验收标准：

- 核心测试通过
- 异常操作不会崩溃

### Day 36-40：体验打磨

任务：

- 日志分类
- 错误码统一
- 脚本补注释
- 战斗数值微调
- README 完善
- 架构图完善
- BattleScene 美化
- RankScene 美化
- 加简单音效/动画可选

验收标准：

- 项目可以给别人拉下来跑
- Demo 观感完整

### Day 41-45：展示与面试准备

任务：

- 录制演示视频
- 写项目文档
- 整理技术难点
- 准备简历描述
- 准备面试问答
- 打 tag：demo-v1.0

验收标准：

- 5 分钟内可以讲清项目
- 10 分钟内可以演示完整流程
- GitHub 仓库整洁
- README 有启动步骤

## 14. 每天学习目标总结

每天至少覆盖三类能力：

```text
1. 服务端工程能力
2. 客户端交互能力
3. 游戏业务闭环能力
```

重点学习路线：

| 阶段 | 学习重点 |
|---|---|
| Day 1-5 | CMake、epoll、Reactor、协议拆包 |
| Day 6-10 | 登录、Session、Redis、DB |
| Day 11-16 | 匹配、房间、状态机、定时器 |
| Day 17-25 | Lua、战斗结算、服务端权威 |
| Day 26-35 | 稳定性、断线、测试、配置 |
| Day 36-45 | 项目展示、简历、面试表达 |

## 15. 每天验收标准原则

每天必须满足：

```text
服务端有新增能力
客户端有可见变化
至少一个可运行检查
代码提交一次
```

每日提交格式：

```text
Day 12:
- server: add redis match queue
- client: add cancel match button
- test: verify duplicated match request
```

不允许连续多天只写服务端。客户端可以简陋，但每天都要更接近真实游戏流程。

## 16. 最终可展示 Demo 标准

### 必须演示

```text
1. 启动 Redis
2. 初始化数据库
3. 启动 C++ 服务端
4. 启动两个 Godot 客户端
5. 注册/登录两个账号
6. 进入大厅
7. 双方点击匹配
8. 匹配成功进入战斗
9. 双方选择技能
10. 多回合战斗
11. 一方胜利
12. 写入数据库
13. 更新 Redis 排行榜
14. 客户端查看排行榜
```

### 演示时可以展示的命令

```bash
./scripts/build.sh
./scripts/init_db.sh
./scripts/start_redis.sh
./scripts/run_server.sh
./scripts/run_tests.sh
```

### Demo 合格标准

- 不是纯控制台
- 不是纯接口测试
- 有 Godot 客户端
- 有完整 UI 流程
- 有真实网络通信
- 有服务端权威战斗
- 有 Lua 技能
- 有 Redis 排行榜
- 有数据库战斗记录
- 有测试和文档

## 17. 简历写法

### 项目名称

Nebula Duel：C++20 全栈 2D 回合制对战游戏服务器

### 简历描述

使用 C++20 从零实现一套 2D 房间制回合对战游戏服务器，基于 Linux epoll/Reactor 网络模型处理客户端连接，采用 Protobuf 自定义二进制协议完成登录、匹配、房间、战斗、排行榜等核心流程。服务端使用 Lua 5.4 + sol2 实现技能脚本化，保证战斗由服务端权威判定；使用 Redis 管理 Token、在线状态、匹配队列与排行榜，使用 SQLite/MySQL 持久化玩家数据、战斗记录和登录日志。客户端使用 Godot 4 实现登录、大厅、匹配、战斗和排行榜界面，完成可实际游玩的 1v1 回合制 Demo。

### 技术亮点

```text
- 基于 epoll + Reactor 实现非阻塞 TCP 网络层，支持粘包半包处理和消息分发
- 设计 Protobuf 协议体系，覆盖登录、匹配、房间、战斗、排行榜完整链路
- 实现服务端权威战斗系统，客户端只负责输入和展示，避免客户端作弊
- 接入 Lua 5.4/sol2 实现技能逻辑脚本化，并通过 Git commit hash 记录脚本版本
- 使用 Redis 实现 Token 缓存、在线状态、匹配队列、防重复登录与排行榜
- 使用 SQLite/MySQL 持久化玩家账号、战斗记录、技能版本和登录日志
- 使用 GoogleTest 对协议编解码、匹配逻辑、战斗结算和 Lua 技能进行测试
- 使用 Bash 脚本完成协议生成、构建、测试、数据库初始化和服务启动自动化
```

### 面向游戏服务端岗位的强表达

项目重点不是单点功能，而是完整复现了游戏服务器的核心业务链路：连接管理、会话状态、匹配、房间、战斗状态机、脚本化技能、缓存、持久化、排行榜、断线处理和客户端同步。

## 18. 面试问题清单

### 网络层

1. 为什么游戏服务器常用 Reactor 模型？
2. epoll 的 LT 和 ET 有什么区别？
3. 如何处理 TCP 粘包和半包？
4. 为什么 socket 要设置非阻塞？
5. PacketCodec 如何保证包完整性？
6. 如果客户端发恶意超大包怎么办？
7. 单线程 Reactor 和多线程 Reactor 有什么区别？
8. 如何处理客户端突然断线？
9. 如何设计心跳？
10. 如何避免 send buffer 堆积？

### 协议层

1. 为什么使用 Protobuf？
2. 协议号如何规划？
3. seq_id 有什么用？
4. 如何兼容旧版本客户端？
5. Protobuf 字段删除有什么风险？
6. 如何处理未知 msg_id？
7. 错误码如何设计？
8. 二进制协议和 JSON 协议有什么区别？

### 登录与 Session

1. 登录 token 如何生成？
2. token 放 Redis 的好处是什么？
3. 如何防止重复登录？
4. Session 和 Connection 的区别是什么？
5. 玩家断线后是否立即下线？
6. 登录日志有什么用？
7. 密码为什么不能明文存储？

### 匹配与房间

1. 匹配队列用内存还是 Redis？
2. Redis 匹配队列有什么一致性问题？
3. 如何避免一个玩家重复进入多个房间？
4. 房间状态机如何设计？
5. 房间什么时候销毁？
6. 匹配成功后有玩家断线怎么办？
7. 如何扩展到按段位匹配？

### 战斗系统

1. 为什么战斗必须由服务端权威判定？
2. 客户端能否预测伤害？
3. 回合制战斗状态如何设计？
4. 双方同时选择技能如何结算？
5. 玩家超时不操作怎么办？
6. 如何保证双方客户端看到一致结果？
7. 如何记录战斗回放？
8. 战斗中 Lua 报错怎么办？
9. 如何防止客户端发送非法技能？
10. 如何扩展更多技能？

### Lua 脚本

1. 为什么技能逻辑适合脚本化？
2. sol2 的作用是什么？
3. C++ 如何向 Lua 暴露对象？
4. Lua 脚本热更新有哪些风险？
5. 如何记录脚本版本？
6. 如果热更新后旧战斗还在进行怎么办？
7. Lua 执行错误如何隔离？
8. 如何限制 Lua 脚本权限？

### Redis

1. Redis 在项目中承担了哪些职责？
2. 为什么排行榜适合用 zset？
3. Redis token 过期如何处理？
4. Redis 宕机怎么办？
5. Redis 和数据库的数据如何同步？
6. 玩家热数据为什么放 Redis？
7. 防重复登录锁如何设计？

### 数据库

1. 玩家表如何设计？
2. 战斗记录为什么要分 battle_records 和 round_records？
3. 为什么登录日志单独建表？
4. 数据库写失败怎么办？
5. 战斗结束时先写 DB 还是先回客户端？
6. SQLite 和 MySQL 的区别？
7. 如何做索引？

### 客户端

1. Godot 客户端如何组织场景？
2. 客户端为什么不计算最终战斗结果？
3. 网络消息如何分发到 UI？
4. 战斗界面如何避免重复点击？
5. 断线后客户端如何处理？
6. 如何表现回合结果？
7. 客户端收到服务器状态和本地显示不一致怎么办？

### 工程化

1. CMake 项目如何组织？
2. 为什么使用 Ninja？
3. Bash 脚本解决了什么问题？
4. GoogleTest 测试了哪些核心逻辑？
5. spdlog 如何分类日志？
6. Git 分支如何管理？
7. 如何让别人快速运行项目？
8. 这个项目还能如何扩展？

## 推荐最小可行版本

为了 30-45 天稳妥完成，建议第一版坚持这个范围：

```text
账号：注册 + 登录
大厅：显示玩家信息
匹配：两人匹配
房间：自动进入
战斗：4 个技能，回合制
结算：胜负 + 加减分
排行：Redis 前 20
数据库：玩家 + 战斗记录 + 登录日志
客户端：5 个界面完整流转
```

不要一开始就做：

```text
复杂动画
复杂技能树
好友系统
聊天系统
多房间观战
断线重连完整恢复
分布式网关
复杂 ECS 战斗框架
```

这些可以放到 v2。

## 最终项目一句话

Nebula Duel 是一个使用 C++20 + epoll/Reactor + Protobuf + Lua + Redis + SQLite/MySQL + Godot 4 实现的完整 1v1 回合制对战游戏项目，覆盖登录、大厅、匹配、房间、战斗、结算、排行榜和工程化自动化流程。

这个项目做完后，不只是“我会 C++ 网络编程”，而是能表达成：

> 我完整实现过一个可玩的游戏服务器业务闭环，并且理解客户端、服务端、缓存、数据库、脚本、协议和工程化之间如何协作。
