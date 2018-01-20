### meltdown和spectre比较

漏洞主页：[meltdown](https://meltdownattack.com/) [spectre](https://spectreattack.com/)

该漏洞主要用来读取内存中的数据，暂时未发现该漏洞能被用来修改内容的情况



相同点：

1. meltdown和spectre都是利用cpu的缺陷来读取cache中的数据
2. 二者都利用了cpu乱序执行的特点

不同点：

* meltdown

  1. 破坏的是用户态和内核态的隔离

  2. 利用的是cpu的指令乱序执行

  3. 影响几乎全部Intel、小部分arm，amd未受影响

     为什么amd不受影响，Linux内核相关的patch直接排除了AMD，[kernel  commit]( https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=694d99d40972f12e59a3696effee8a376b79d7c8)

     ```
     diff --git a/arch/x86/kernel/cpu/common.c b/arch/x86/kernel/cpu/common.c

     index f2a94df..b1be494 100644

     --- a/arch/x86/kernel/cpu/common.c

     +++ b/arch/x86/kernel/cpu/common.c

     @@ -899,8 +899,8 @@ static void __init early_identify_cpu(struct cpuinfo_x86 *c)

      	setup_force_cpu_cap(X86_FEATURE_ALWAYS);

     - /* Assume for now that ALL x86 CPUs are insecure */
       -setup_force_cpu_bug(X86_BUG_CPU_INSECURE);
       +if (c->x86vendor != X86VENDOR_AMD)
       - setup_force_cpu_bug(X86_BUG_CPU_INSECURE);
        fpu__init_system(c);
     ```

     ​

* spectre漏洞破坏的是用户态不同进程之间的隔离性

  1. 破坏的是用户态程序之间的隔离性
  2. 利用cpu的分支预测的功能，诱导获取非法地址内容
  3. 影响intel、arm、amd

### 修复方法

* meltdown
  1. 采用KPTI(kernel page table isolution)来消除影响，对性能有一定的影响
  2. [不一样解读meltdown攻击成功原因](https://herculesinchina.wordpress.com/2018/01/04/%E4%B8%8D%E4%B8%80%E6%A0%B7%E8%A7%A3%E8%AF%BBmeltdown%E6%94%BB%E5%87%BB%E6%88%90%E5%8A%9F%E5%8E%9F%E5%9B%A0/)
* spectre
  1. 未知

### 原理分析

1. [首发：Meltdown漏洞分析与实践](https://mp.weixin.qq.com/s/zlspXeDGlAEzVsq2h6gg8w) ，写的非常好
2. ​

### POC

1. [宋宝华： 用代码切身实践体会meltdown漏洞](https://mp.weixin.qq.com/s/lJJU3LCepJgNq5AxyFFM8Q)，该文章结合了实际例子
2. [turbo](https://github.com/turbo/KPTI-PoC-Collection)
3. [Eugnis](https://github.com/Eugnis/spectre-attack)
4. [feruxmax](https://github.com/feruxmax/meltdown)
5. [gkaindl](https://github.com/gkaindl/meltdown-poc)
6. [IAIK](https://github.com/IAIK/meltdown) 这篇POC中包含的例子很多

### 相关链接

1. [知乎,如何看待 2018 年 1 月 2 日爆出的 Intel CPU 设计漏洞](https://www.zhihu.com/question/265012502?utm_medium=social&utm_source=wechat_session&from=groupmessage&isappinstalled=0)
2. [meltdown论文](https://meltdownattack.com/meltdown.pdf)
3. [spectre论文](https://spectreattack.com/spectre.pdf)
4. [Negative Result: Reading Kernel Memory From User Mode](https://cyber.wtf/2017/07/28/negative-result-reading-kernel-memory-from-user-mode/)
5. ['Kernel memory leaking' Intel processor design flaw forces Linux, Windows redesign](https://www.theregister.co.uk/AMP/2018/01/02/intel_cpu_design_flaw/?from=timeline&isappinstalled=0)
6. [近几年曝出的芯片漏洞究竟会不会影响到我们？](https://www.wukong.com/question/6507030792781168910/)
7. [Building a More Secure World with the RISC-V ISA](https://riscv.org/2018/01/more-secure-world-risc-v-isa/) RISC-V不受影响
8. [Project Zero](https://googleprojectzero.blogspot.com/2018/01/reading-privileged-memory-with-side.html) 
9. [bbs1](http://bbs.ngacn.cc/read.php?tid=13191205&rand=243) [bbs2](http://bbs.ngacn.cc/read.php?tid=13191205&rand=603) 专业角度说说这次的CPU漏洞，从原理到答疑，需要注册，暂未打开阅读
10. [安天对meltdown和spectre的报告](https://mp.weixin.qq.com/s/2FvvFUT8taRPv6GOHzNW-g)
11. [上海市网信办发布的内容](https://mp.weixin.qq.com/s/__izj5YB_yt2ZZu9rrsGjw)，包括各大厂商的补丁链接
12. [英特尔CPU漏洞门】AI云计算遭打击，谷歌、微软、AWS首当其冲](https://mp.weixin.qq.com/s/fBzkwxyowv39qr0dql5-tA) ，列出了对各大厂商的影响

## 其它

这个描述比较清楚，但没有给出完整的PoC代码。

https://mp.weixin.qq.com/s/2FvvFUT8taRPv6GOHzNW-g
处理器A级漏洞Meltdown(熔毁)和Spectre(幽灵)分析报告

这个文档比较直观地介绍了漏洞的工作原理和PoC的效果；

看了spectre的文章，对于Sec 5.2的例子有几个问题想请教一下：（1）其步骤1是用一些简单的指针操作获取victim的sleep入口地址和跳转地址，既然我们无法正常获取victim进程的内存信息，那如何得到其函数的地址？（2）在步骤6，怎么保证在执行sleep之前恰好指定edi的值 （victim的代码不受攻击者控制，只能是在一些需要输入的地方输入数据）（3）攻击者最后如何使victim执行用于边信道攻击的代码（遍历数组，判断哪个page访问时间较短）。   理解的不对的地方还请指出。
