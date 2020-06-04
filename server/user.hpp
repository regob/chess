///
///  user.hpp
///  A bejelentkezett felhasználót tároló User osztályt tartalmazza.
///  Definiál globális adatstruktúrákat is, amiben tárolhatók a bejelentkezett
///  felhasználók név, és socket szerint.
///

#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <map>
#include <vector>
#include <pthread.h>
#include "../message.hpp"

#define MAX_NAME_LENGTH 15
#define PASSWD_LENGTH 15 
#define MAX_PASSWD_LENGTH 15

// memória struktúra az aktív user-ek tárolására
struct User {
    std::string name;
    int sockfd;

    pthread_mutex_t mutex;    
    User(std::string _name, int _sockfd = -1);
    ~User();

    void send(Message *);
};

// {sockfd, User*} párok tárolására
extern std::map<int, User*> sockfd_users;
extern pthread_mutex_t sockfd_users_mutex;

// {user_név, User*} párok tárolására
extern std::map<std::string, User*> name_users;
extern pthread_mutex_t name_users_mutex;


#endif
