#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>
using namespace std;

// 提供离线消息表的操作接口方法
class OfflineMessageModel
{
public:
    // 保存用户离线消息
    void insert(int userId, string msg);

    // 删除用户离线消息
    void remove(int userId);

    //查看用户的离线消息
    vector<string> query(int userId);
};

#endif