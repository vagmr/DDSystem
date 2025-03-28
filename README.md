# DDPSystem
  
  一个专门用于处理DDP格式文件的工具集，提供了文件打包和解包功能，并配备了图形用户以便于操作

 功能
- DDP2文件打包和解包
- DDP3文件打包和解包

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
- DDP2_pack.exe：DDP2打包工具
- DDP2_unpack.exe：DDP2解包工具
- DDP3_pack_wchar.exe：DDP3打包工具
- DDP3_unpack_wchar.exe：DDP3解包工具