# 1.0说明
采用socket编程，实现二人对话，初步设想是客户端和服务端对话。不搞多个客户端对话了。

目前实现功能如下：

首先客户端给服务端发个默认的消息，然后由服务端开始回消息。两个人一人一句对话，可以由任何一个人退出。

存在问题：
1. 一个人不能发很多句话。
2. 如果没到输入的时候，不能退出。
3. 协议还没设计，甚至不知道怎么设计。
4. OS还一点没看呢。要G了

# 2.0说明
这个给分标准就是鼓励你卷呢，因此先更一波2.0。采用多线程，可以实现每个人哐哐发送和接收消息。然后搞了点带颜色的输出，只能说太卷了。

存在问题：
1. 还是两人聊天，还是TMD客户端和服务端对话，没实现服务端转发。
2. OS和COMPILER又NM留新的作业了，真要G了
3. 这计网这个还没写完又留新的了。。。大无语

# 3.0说明
很好，在交之前一通改，弄了个群发功能出来。也就这样了，就交了。累死我了。

存在问题：
1. 需要指定多少个人聊天，只有指定人数都进来了才能开始聊天。
2. OS的LAB2还一点没看呢，NM编译又留了。。。
3. 退出功能是一个人退出了就都退出了，挺离谱的，反正功能改不了就改协议，能过就行。。。

3.0即为最终提交版本，千万不要试图运行，也不要试图复现，更不要试图在我的程序的基础上修改以期望得到正确答案。
关于具体怎么写这个作业，可以在我的博客上看到一些过程和参考资料。[这里放一个我的博客的链接](https://tephrocactushc.github.io/post/16eec244.html)。


