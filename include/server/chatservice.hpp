#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <mutex>
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
//处理事件回调方法类型
using MsgHandler = std::function<void (const TcpConnectionPtr& conn,json &js,Timestamp &recvTime)>;


//聊天服务器业务类
class ChatService
{
public:
    //获取单例对象的方法
    static ChatService* instance();
    void login(const TcpConnectionPtr &conn,json &js,Timestamp &recvTime);
    void reg(const TcpConnectionPtr &conn,json &js,Timestamp &recvTime);
    // 处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //获取处理消息对应的处理器
    MsgHandler getHandler(int msgid);
    //处理一对一的聊天服务
    void oneChat(const TcpConnectionPtr &conn,json &js,Timestamp &recvTime);
    //添加好友业务
    void addFriend(const TcpConnectionPtr &conn,json &js,Timestamp &recvTime);
    //创建群组业务
    void createGroup(const TcpConnectionPtr &conn,json &js,Timestamp &recvTime);
    //加入群组业务
    void addGroup(const TcpConnectionPtr &conn,json &js,Timestamp &recvTime);
    //群组聊天服务
    void chatGroup(const TcpConnectionPtr &conn,json &js,Timestamp &recvTime);
    //处理客户端异常关闭
    void clientCloseException(const TcpConnectionPtr &conn);
    //服务器异常，业务重置方法
    void reset();
private:
    ChatService();

    //存储消息id及其业务的处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap;

    //存储在线用户的通信连接
    unordered_map<int,TcpConnectionPtr> _userConnMap;

    //定义一把锁，保证_userConnMap的线程安全 
    mutex _connMutex;

    //用户数据操作类对象
    UserModel _userModel;

    //离线信息数据操作类对象
    OfflineMessageModel _offlineMessageModel;

    //添加好友信息数据操纵类对象
    FriendModel _friendModel;

    //群聊天操纵类
    GroupModel _groupModel;

    //redis对象
    Redis _redis;
    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);
};

#endif