#include "friendmodel.hpp"
#include "db.h"
using namespace std;

//记录好友信息
void FriendModel::insert(int userId,int friendId)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d,%d)", userId,friendId);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }    
}

//返回查询的好友列表
vector<User> FriendModel::query(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid = %d",userId);

    MySQL mysql;
    vector<User> vec;
    User user;
    if (mysql.connect())
    {
        MYSQL_RES * res =  mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
        return vec;
    } 
}

