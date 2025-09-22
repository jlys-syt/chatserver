#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
using namespace placeholders;
using namespace muduo;
using namespace std;

ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    //用户注销，取消订阅channel(id)
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.update(user);
}

// 处理客户端异常关闭
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);

        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {

            if (it->second == conn)
            {
                // 设置用户id
                user.setId(it->first);

                // 从_userConnMap删除连接信息
                _userConnMap.erase(it);
                break;
            }        

            //异常断开，取消用户订阅channel(id)
            _redis.unsubscribe(user.getId());

            //更新用户的状态信息
            if(user.getId() != -1)
            {
                user.setState("offline");
                _userModel.update(user);
            }
        }
    }
}

// 处理服务器异常中断，重置业务
void ChatService::reset()
{
    _userModel.resetState();
}

// 注册消息以及对应的回调函数
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOG_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this,_1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::chatGroup, this, _1, _2, _3)});

    if(_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }
}

MsgHandler ChatService::getHandler(int msgid)
{
    if (_msgHandlerMap.find(msgid) == _msgHandlerMap.end())
    {

        return [=](const TcpConnectionPtr &conn, json &js, Timestamp &recvTime) -> MsgHandler
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 处理登录的业务服务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp &recvTime)
{
    int id = js["id"];
    string pwd = js["password"];
    User user = _userModel.query(id);

    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登陆，不允许重复登录
            json response;
            response["msgid"] = LOG_MSG_ACK;
            response["errno"] = 1;
            response["errmsg"] = "该账号已经登录!";
            response["id"] = user.getId();
            response["name"] = user.getName();
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            //登录成功后，向redis订阅channel(id)
            _redis.subscribe(id);

            // 登录成功，更新用户状态为online
            user.setState("online");
            _userModel.update(user);

            // 返回当前用户的name和id
            json response;
            response["msgid"] = LOG_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 返回离线信息
            vector<string> vec = _offlineMessageModel.query(user.getId());
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                _offlineMessageModel.remove(user.getId());
            }

            // 将用户的好友信息返回
            vector<User> userVec = _friendModel.query(user.getId());
            if (!userVec.empty())
            {
                vector<string> vec2;
                json js;
                for (User &user : userVec)
                {
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 将用户的群组信息返回
            vector<Group> groupVec = _groupModel.queryGroups(user.getId());
            if (!groupVec.empty())
            {
                vector<string> vec3;
                for (Group &gro : groupVec)
                {
                    json js1;
                    js1["id"] = gro.getId();
                    js1["groupname"] = gro.getName();
                    js1["groupdesc"] = gro.getDesc();
                    vector<string> vec4;
                    for (GroupUser &groupUser : gro.getUsers())
                    {
                        json js2;
                        js2["id"] = groupUser.getId();
                        js2["name"] = groupUser.getName();
                        js2["state"] = groupUser.getState();
                        js2["role"] = groupUser.getState();
                        vec4.push_back(js2.dump());
                    }
                    js1["users"] = vec4;
                    vec3.push_back(js1.dump());
                }
                response["groups"] = vec3;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        // 该用户不存在，登录失败 或 用户存在，但输入密码错误，登陆失败
        json response;
        response["msgid"] = LOG_MSG_ACK;
        response["errmsg"] = "用户不存在或输入密码错误!";
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

// 处理注册的业务服务 name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp &recvTime)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        json response;
        response["msgid"] = "REG_MSG_ACK";
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = "REG_MSG_ACK";
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp &recvTime)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid用户在线，转发消息 服务器提供信息中转
            it->second->send(js.dump());
            return;
        }
    }

    //用户可能连接别的服务器
    User user = _userModel.query(toid);
    if(user.getState() == "online")
    {
        _redis.publish(toid,js.dump());
        return;
    }

    // toid用户不在线，存储离线消息
    _offlineMessageModel.insert(toid, js.dump());
}

// 添加好友
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp &recvTime)
{
    int userId = js["userid"].get<int>();
    int friendId = js["friendid"].get<int>();
    _friendModel.insert(userId, friendId);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp &recvTime)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        // 存储群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp &recvTime)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群聊天业务
void ChatService::chatGroup(const TcpConnectionPtr &conn, json &js, Timestamp &recvTime)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            User user = _userModel.query(id);
            if(user.getState() == "online")
            {
                _redis.publish(id,js.dump());
            }
            else
            {
                // 存储离线群消息
                _offlineMessageModel.insert(id, js.dump());                
            }

        }
    }
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlineMessageModel.insert(userid, msg);
}
