### Lab 2:The TCP Receiver

这一部分的[document](https://cs144.github.io/assignments/check2.pdf)很明确的阐述了前文所实现的两个模块与整个传输层的关系：

1. ByteStream就是传输层的TCP协议直接面向应用层的接口，应用层的应用（或协议）直接从该ByteStream中读取可靠的字节流。
2. Reassembler是ByteStream的前一环节，它接受接受一系列子字符串，然后按照顺序排序好后交给ByteStream。

而本实验的目标是实现TCP Receiver， 则是Reassembler的再前一环节，它从与对等的发送者（具体来讲这里的发送者，应当是TCP/IP参考模型中的对等层的概念）那里接收信息，**将这些信息传给Reassembler，并向对等的发送者发送确认信息和窗口信息。**

Part 1: warpping_integers

首先明确三个index的概念，分别是：

1. seqno: TCP 
2. absolute seqno: 这个是考虑了 SYN 和 FIN的seqno。
3. stream index: 这个最好理解，就是前文所提到的Reassembler中的index。

我们可以用一个简单的"cat"的字符串整理出下面的表格来理解这三个index的关系：
| element         | syn          | c            | a            | t            | fin          |
|-----------------|--------------|--------------|--------------|--------------|--------------|
| seqno           | 2^32         | −2           | 2^32         | −1           | 0            |
| absolute seqno  | 0            | 1            | 2            | 3            | 4            |
| stream index    |              | 0            | 1            | 2            |              |
