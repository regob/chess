///
///  auth.hpp
///  A felhasználói adatok tárolását, kezelését végzi
///

#ifndef AUTH_HPP
#define AUTH_HPP

#include <pthread.h>
#include <string>
#include <vector>
#include "user.hpp"

// az user adatokat tároló fájl neve
#define DB_PATH "users.db"

// egy felhasználó adatait tároló bejegyzés
struct DBEntry {
    std::string name, passwd;

    DBEntry(std::string _name, std::string _passwd);
    ~DBEntry();
};

// Az "adatbázis" osztály, itt csak egy fájlt kezel, nem csatlakozik adatbázishoz
struct UserDB {
    pthread_rwlock_t rwlock;

    UserDB();
    ~UserDB();

    void open_db();
    std::vector<DBEntry>* read_db();
    
    User* auth_user(std::string name, std::string passwd);
    int new_user(std::string name, std::string passwd);
};

extern UserDB userdb;

#endif
