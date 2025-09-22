#ifndef PUBLIC_H
#define PUBLIC_H

/* server 和client的功能方法 */
enum EnMsgType
{
    LOG_MSG = 1,//登录信息
    LOG_MSG_ACK,//登录响应信息
    LOGINOUT_MSG, //登录退出
    REG_MSG,//注册信息
    REG_MSG_ACK,//注册响应信息
    ONE_CHAT_MSG,//点对点聊天信息
    ADD_FRIEND_MSG,//添加好友信息

    CREATE_GROUP_MSG,//创建群组消息
    ADD_GROUP_MSG,//加入群组消息
    GROUP_CHAT_MSG,//群聊天消息
    
};

#endif