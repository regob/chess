///
///  auth.cpp
///  A felhaszálók adatait kezelő adatbázis osztály implementációja.
///

#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "user.hpp"
#include "auth.hpp"

// a globális adatbázis példány
UserDB userdb;

UserDB::UserDB(){
    if(pthread_rwlock_init(&rwlock, NULL)){
        syslog(LOG_ERR, "pthread_rwlock_init: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

UserDB::~UserDB(){
    if(pthread_rwlock_destroy(&rwlock)){
        syslog(LOG_ERR, "pthread_rwlock_destroy: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }        
}

// létrehozza az adatbázis fájlt, ha még nem létezik
void UserDB::open_db(){
    int fd = open(DB_PATH, O_CREAT, 0660);
    if(fd < 0){
        syslog(LOG_ERR, "open_db: %s", strerror(errno));
    }
    close(fd);
}

// kiolvassa a db fájlból az összes bejegyzést
std::vector<DBEntry>* UserDB::read_db(){
    std::vector<DBEntry> *db = new std::vector<DBEntry>();

    pthread_rwlock_rdlock(&rwlock);
    
    FILE *f = fopen(DB_PATH, "r+");
    if(f == NULL){
        syslog(LOG_ERR, "fopen: %s", strerror(errno));
    }
        
    char buf_name[MAX_NAME_LENGTH + 1], buf_passwd[PASSWD_LENGTH + 1];
    
    while(fscanf(f, "%s %s", buf_name, buf_passwd) != EOF){
        db->push_back(DBEntry(buf_name, buf_passwd));
    }

    fclose(f);
    if(pthread_rwlock_unlock(&rwlock))
        syslog(LOG_ERR, "UserDB::read_db: rwlock_unlock: %s", strerror(errno));

    return db;
}
    
// egy felhasználót hitelesít az adatai alapján
User* UserDB::auth_user(std::string name, std::string passwd){
    std::vector<DBEntry> *db = read_db();

    // alapértelmezésben NULL-t ad vissza, ilyenkor sikertelen a hitelesítés
    User *user = NULL;
    
    for(unsigned i = 0; i < db->size(); i++){

        // ha egyezik a név, és a jelszó, sikeres a bejelentkezés
        if((*db)[i].name == name && (*db)[i].passwd == passwd){
            pthread_mutex_lock(&name_users_mutex);

            // megnézzük be van-e már jelentkezve, ha igen akkor
            // az eddigi User objektumot adjuk vissza
            user = name_users[name];

            // ha még nem volt bejelentkezve létre kell hozni egy új struktúrát
            if(user == NULL){
                user = new User(name);
                name_users[name] = user;
            }
            
            pthread_mutex_unlock(&name_users_mutex);
            break;
        }
    }
    
    delete db;
    return user;
}

// regisztrál egy új felhasználót az adatbázisba
int UserDB::new_user(std::string name, std::string passwd){
    std::vector<DBEntry> *db = read_db();

    // megnézzük már foglalt-e a név
    for(unsigned i = 0; i < db->size(); i++){
        if((*db)[i].name == name){
            delete db;
            return -1;
        }
    }

    delete db;
    
   
    pthread_rwlock_wrlock(&rwlock);
    FILE *f = fopen(DB_PATH, "a");

    // itt már nem foglalt a név, létrejön az új felhasználó
    fprintf(f, "%s %s\n", name.c_str(), passwd.c_str());
    fclose(f);
    pthread_rwlock_unlock(&rwlock);

    return 0;
}


DBEntry::DBEntry(std::string _name, std::string _passwd)
    :name(_name), passwd(_passwd) {}

DBEntry::~DBEntry(){}
    
