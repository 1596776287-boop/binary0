# binary0

> it just works

## 项目简介

binary0 是一个基于 C++ 的简单高效的二进制数据库项目，支持结构化数据的持久化存储与管理，适合小型数据管理场景。项目采用 CMake 构建，易于集成和扩展。

## 基本功能

- **二进制文件数据库：** 使用自定义二进制格式保存记录，包含头部和变长载荷，支持基础校验。
- **记录管理：**
  - 追加写入新纪录（`add`）
  - 顺序列出所有记录（`list`），可选显示已删除项
  - 按 ID 查找记录（`get`）
  - 按 ID 逻辑删除记录（`del`，非物理删除）
- **命令行交互：**
  - 支持命令行界面（CLI），提供 `add`、`get`、`list`、`del`、`help`、`quit` 等命令
- **加密/解密支持：**
  - 支持数据库文件的简单加密（`.bin` <-> `.enc` 文件互转）
- **自定义文件格式：**
  - 记录头包含 id、时间戳、删除标志、载荷长度、载荷 CRC
  - 载荷为两个变长字符串（name, note），支持 utf-8 和任意字节内容
- **示例命令：**
  ```bash
  add <id> <name> <note...>  # 添加新记录
  get <id>                   # 查询指定记录
  list [all]                 # 列出全部（含已删除）
  del <id>                   # 逻辑删除记录
  help                       # 查看命令帮助
  quit/exit                  # 退出程序
  ```

## 安装与使用

### 构建

1. 克隆本仓库：
   ```bash
   git clone https://github.com/1596776287-boop/binary0.git
   cd binary0
   ```

2. 构建项目：
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

### 运行

- 使用默认数据库文件 `records.bin`：
  ```bash
  ./binary
  ```

- 指定数据库文件路径：
  ```bash
  ./binary mydata.bin
  ```

- 加密/解密数据库文件：
  ```bash
  ./encrypt records.bin      # 生成加密文件 records.enc
  ./binary records.enc      # 自动解密为 records.bin 并打开
  ```

## 依赖项

- C++11 或更高版本
- CMake 3.10 或更高版本

## 贡献方式

欢迎提交 Issue 或 Pull Request 改进本项目。

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](./LICENSE)。

## 联系方式

如有问题或建议，请通过 GitHub Issue 联系。
