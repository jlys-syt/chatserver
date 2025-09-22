#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

//User表的数据操纵类
class UserModel
{
public:
    //User表的添加数据方法
    bool insert(User &user);

    //根据id查询User表的信息
    User query(int id);

    //更新用户的信息
    bool update(User &user);

    //重置用户的状态信息
    void resetState();
};

#endif