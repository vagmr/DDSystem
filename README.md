# DDP文件处理系统

## 项目简介
DDP文件处理系统是一个专门用于处理DDP格式文件的工具集，提供了文件打包和解包功能，并配备了图形用户界面（GUI）以便于操作。该系统支持多种编码格式，包括标准字符和宽字符（wchar）处理。

## 功能特点
- **文件打包功能**：支持将文件打包成DDP格式
  - DDP2标准打包（DDP2_pack）
  - DDP3宽字符打包（DDP3_pack_wchar）

- **文件解包功能**：支持将DDP格式文件解包
  - DDP2标准解包（DDP2_unpack）
  - DDP3宽字符解包（DDP3_unpack_wchar）

- **图形用户界面**：提供直观的操作界面（DDSystemGUI）
  - 可视化文件选择
  - 简单的操作流程
  - 实时处理状态显示

## 系统要求
- Windows操作系统
- Visual Studio 2022或更高版本（用于编译）
- 支持Unicode字符集

## 项目结构
```
├── DDP2_pack/          # DDP2标准打包模块
├── DDP2_unpack/        # DDP2标准解包模块
├── DDP3_pack_wchar/    # DDP3宽字符打包模块
├── DDP3_unpack_wchar/  # DDP3宽字符解包模块
├── DDSystemGUI/        # 图形用户界面模块
│   └── lib/            # 可执行文件库
└── DDSystem.sln        # Visual Studio解决方案文件
```

## 编译说明
1. 使用Visual Studio 2022打开`DDSystem.sln`解决方案文件
2. 选择目标平台和配置（Debug/Release）
3. 执行编译（Build Solution）

## 使用方法
### GUI界面使用
1. 运行DDSystemGUI程序
2. 选择需要处理的文件
3. 选择操作类型（打包/解包）
4. 点击执行按钮开始处理

### 命令行使用
各模块都可以通过命令行方式使用：
- DDP2_pack.exe：标准打包工具
- DDP2_unpack.exe：标准解包工具
- DDP3_pack_wchar.exe：宽字符打包工具
- DDP3_unpack_wchar.exe：宽字符解包工具

## 注意事项
- 确保文件路径不包含不支持的字符
- 处理大文件时请确保有足够的磁盘空间
- 建议定期备份重要数据

## 技术支持
如遇到问题，请检查以下几点：
1. 确认系统环境是否满足要求
2. 检查文件权限设置
3. 查看程序运行日志

## 开发说明
本项目使用C++开发，采用Visual Studio 2022作为开发环境。如需进行二次开发，请确保：
1. 熟悉C++编程语言
2. 了解DDP文件格式
3. 了解Visual Studio开发环境