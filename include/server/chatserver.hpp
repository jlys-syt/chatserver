#ifndef CHATSERVER_H
#define CHATSERVER_H
#include <iostream>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <string>
using namespace muduo;
using namespace muduo::net;
using namespace std;
using namespace placeholders;

class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);
    
    void start();
private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp recvTime);

    TcpServer _server;//组合的muduo库
    EventLoop *_loop;
};

#endif