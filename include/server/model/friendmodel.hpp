#ifndef FRIENDModel_H
#define FRIENDModel_H

#include "user.hpp"
#include <vector>
using namespace std;

class FriendModel
{
public:
    //添加好友方法
    void insert(int userId,int friendId);

    //返回用户查询列表
    vector<User> query(int userId);
};

#endif