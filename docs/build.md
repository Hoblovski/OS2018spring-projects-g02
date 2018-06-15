# 构建指南

## 环境要求

推荐使用 Linux 环境。 要求 

* cmake：3.5.1
* clang：3.9.0
* make：4.1
* GNU binutils：2.26.1

视机器配置， 构建过程可能花费二十分钟到一小时不等。

以下小节都假设从项目根目录下开始.

## 一键构建运行
~~~ sh
$ ./setup
~~~

如果已经构建完编译器, 不希望浪费时间构建编译器
~~~ sh
$ ./setup_nocompiler
~~~

运行后, 左边绿字是串口输出, 右边蓝字是调试 log.

目前的用户进程支持两个命令:
* `q`: 退出用户进程
* `a xxxxxxxx`: 读取 `xxxxxxxx` 地址的内容, `xxxxxxxx` 是小写的 16 进制地址.
  - 如果 `a 00000000`: 应该出现 pagefault, 机器挂机
  - 如果 `a bffffff0`: 应该正常范围, 因为访问的是用户栈的内容
  - 如果 `a cc000000`: 应该触发保护错误, 机器挂机

## 编译器构建
~~~ sh
$ cd llvm-3.9.0.src
$ mkdir build && cd build
$ cmake -DCMAKE_CXX_COMPILER=clang++-3.9 -DCMAKE_C_COMPILER=clang-3.9 -DCMAKE_BUILD_TYPE=Debug -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -G "Unix Makefiles" ..
$ make llc llvm-mc lld      # 机器性能允许则可以 -jX 并行构建
$ ls bin/
ld.lld  llc  lld  lld-link  llvm-lit  llvm-mc  llvm-tblgen
~~~

## 构建操作系统
~~~ sh
$ cd recc-kernel-llvm
$ make
$ ls build/a.img
build/a.img
~~~

## 构建模拟器
~~~ sh
$ cd cpu0-emulator
$ mkdir build && cd build
$ cmake ..
$ make
$ ls cpu0emu
cpu0emu
~~~

## 在模拟器上运行操作系统
完成以上构建之后， 可以开始在模拟器上运行操作系统了。
~~~ sh
$ cp recc-kernel-llvm/build/a.img cpu0-emulator/build/
$ cd cpu0-emulator/build
$ ../split './cpu0emu a.img 0 2'
~~~

## 构建硬件设计
为了构建硬件, 需要先切换到 hack 分支.
1. 使用Quartus Prime Lite 打开`OS2018spring-projects-g02/litecpu/litecpu.qpf` ，并执行编译综合。
2. 使用USB Blaster（需要安装驱动）将计算机与“小脚丫”STEP-CYC10硬件开发板链接，打开Quartus中的Programmer，将`OS2018spring-projects-g02/litecpu/output_files/litecpu.sof` 下载到硬件开发板中。

## 在硬件上运行操作系统
1. 使用USB线将计算机与STEP-CYC10硬件开发板的串口链接（需要安装串口驱动），向左拨动硬件开发板中的五向开关重置CPU运行。
2. 在PC上运行`OS2018spring-projects-g02/utils/ser.py` ，输入 `file a.img` ，其中系统镜像`a.img` 需要拷贝到当前文件夹下。
3. 输入`start` 即可启动操作系统，此时应该在串口输出端（python程序输出）看到与模拟器输出类似的结果。
4. 之后与模拟器类似使用即可，`ser.py` 会将指令通过串口传输给硬件开发板，再将串口输出结果显示在屏幕上。
