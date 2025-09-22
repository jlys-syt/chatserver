#include "offlinemessagemodel.hpp"
#include "db.h"
using namespace std;

// 保存用户离线消息
void OfflineMessageModel::insert(int userId,string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d,'%s')", userId, msg.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 删除用户离线消息
void OfflineMessageModel::remove(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d", userId);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查看用户的离线消息
vector<string> OfflineMessageModel::query(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userId);

    vector<string> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES * res = mysql.query(sql);
        if(res != nullptr)
        {
            //把用户所有的离线信息查询保存在vec中返回
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
