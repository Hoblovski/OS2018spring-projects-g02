# OS2018spring-projects-g02
项目是清华大学操作系统课的 [课程项目](http://os.cs.tsinghua.edu.cn/oscourse/OS2018spring/projects/g02),
基本目的是在一块非常小的 FPGA 上完成一个非常 RISC 的 CPU, 并且提供 (x86平台上) 的交叉编译器,
在此基础上, 运行一块基于 Robert Elder 的 recc项目的操作系统.

现阶段可以在模拟器上运行操作系统, 硬件运行版本参见 `hack/` 分支.

# 目录结构

```
OS2018spring-projects-g02
├── ...
├── cpu0-emulator       
│     模拟器实现
├── docs
│   ├── cpu.md    
│   │     cpu 的文档
│   ├── dzy.md
│   │     戴臻旸角色说明
│   ├── isa.md
│   │     指令集文档
│   └── llvm.md
│         LLVM 文档
├── LICENSE
│   MIT
├── litecpu
│   CPU 实现, VHDL 代码
├── llvm-3.9.0.src
│   ├── ...
│   ├── lib
│   │   ├── ...
│   │   └── Target
│   │       ├─── ...
│   │       └──── Cpu0
│   │               编译器后端
│   ├── tools
│   │   ├── ...
│   │   └── lld
│   │         链接器实现
│   └── ...
├── log
├── README.md
│     基本说明
├── recc
│     原版的 recc, 我们修改后把内核代码缩减到了 40000 条指令, 之后放弃
│     只是为了留存记录才保存, 现在不使用
├── recc-kernel-llvm
│     现在我们使用的 kernel
├── utils
│     通用工具等
└── 轻量OS在“小脚丫”FPGA开发板上的实现.md
      最终报告
```

# 报告结构
`docs/` 是不同部分分别的报告, 而 `轻量OS在“小脚丫”FPGA开发板上的实现.md` 则是把他们合起来的总的报告.

# 重现结果
1. 确认环境
  * cmake: 3.5.1
  * clang: 3.9.0 or 3.8.0, 注意 clang 更新和更老的版本都不行
  * make: 4.1
  * GNU binutils: 2.26.1

2. 运行脚本 `setup`
