#ifndef GROUPUSER_H
#define GROUPUSER_H

#include <vector>
#include <string>
#include "user.hpp"
using namespace std;

//User表的ORM类
class GroupUser : public User
{
public:
    void setRole(string role){this->role = role;}
    string getRole(){return this->role;}
private:
    string role;
};

#endif