# DCCP协议漏洞
1. DCCP协议是面向消息的传输层协议，可最小化数据包报头带来的开销和终端处理的工程量。该协议可建立、维护和拆卸不可靠连接的数据流以及对不可靠数据流进行拥塞控制。
2. 该DCCP双重释放漏洞可允许本地低权限用户修改Linux内核内存，导致拒绝服务（系统崩溃），或者提升权限，获得系统的管理访问权限。
3. 内核编译选项：CONFIG_IP_DCCP=y
4. 在当前DCCP实现中，如果dccp_rcv_state_process中的dccp_v6_conn_request返回“成功” ,dccp_type 为DCCP_PKT_REQUEST的packet的skb会被__kfree_skb强制释放。但是，如果在socket上设置IPV6_RECVPKTINFO，则skb地址会被保存在ireq-> pktopts，然后dccp_v6_conn_request中会增加skb的引用计数，所以skb仍在使用中。然而，它仍然会在dccp_rcv_state_process中被释放。
5. 具体过程
    ```//第一次释放
    kfree(dccp_skb)
    //在与dccp_skb相同的位置分配的另一个对象：
    some_object = kmalloc()
    //第二次释放，实际释放的是some_object对象
    kfree(dccp_skb)
    此时some_object持有一个悬空指针，如此就构造出了一个UAF。攻击者可以控制对象，同时可以通过使用内核堆喷射技术写入任意数据到被覆盖对象。
    如果被覆盖的对象有任何可触发的函数指针，攻击者可以在内核中执行任意代码。
    ```
# 重现
1. 目前通过poc无法重现该攻击，因为poc中的一些offset需要修改,但没有说具体应该如何修改，而且作者也提到了该poc不稳定,原话：
    > The exploit was tested on Ubuntu 16.04 with 4.4.0-62-generic kernel. It
    will most likely crash on anything else, unless you at least update
    the offsets.
    > As I mentioned, the exploit is not very reliable, but I don't want to
    spend any more time on it. The kernel can crash due to a memory
    corruption if we fail to reallocate some objects in time or in the
    correct order.
# 参考
1. [openwall作者分析](http://www.openwall.com/lists/oss-security/2017/02/26/2)
1. [作者的github](https://github.com/xairy/kernel-exploits/tree/master/CVE-2017-6074)
1. [黑吧安全网分析文章](http://www.myhack58.com/Article/html/3/62/2017/83679.htm)
1. [freebug分析文章](http://www.freebuf.com/news/127620.html)