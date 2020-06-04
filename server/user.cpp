///
///  user.cpp
///  Az User osztály implementációja
///

#include <syslog.h>
#include <unistd.h>

#include "user.hpp"
#include "../message.hpp"

std::map<int, User*> sockfd_users;
pthread_mutex_t sockfd_users_mutex = PTHREAD_MUTEX_INITIALIZER;

std::map<std::string, User*> name_users;
pthread_mutex_t name_users_mutex = PTHREAD_MUTEX_INITIALIZER;

User::User(std::string _name, int _sockfd)
    :name(_name), sockfd(_sockfd){
    pthread_mutex_init(&mutex, NULL);
}

User::~User(){
    pthread_mutex_destroy(&mutex);
}

// üzenet küldése az User-nek
void User::send(Message *m){
    if(sockfd <= 0) return; 
    std::string s = m->to_string();
    
    pthread_mutex_lock(&mutex);
    ssize_t len = write(sockfd, s.c_str(), s.size());
    pthread_mutex_unlock(&mutex);

    // nem sikerült a teljes üzenet elküldése, hiba
    if((size_t) len != s.size())
        syslog(LOG_ERR, "User::send: cannot send message.");
}
    
    
                    

    
    
    
