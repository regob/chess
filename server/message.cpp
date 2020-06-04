///
///  message.cpp
///  Az üzenet interfész szerver oldali implementációja. Ez csak a feldolgozó függvényeket implementálja,
///  az üzenetek általános kezelése a közös global_message.cpp része.
///

#include <stdlib.h>
#include <vector>
#include <syslog.h>
#include <pthread.h>
#include <unistd.h>

#include "server.hpp"
#include "../message.hpp"
#include "auth.hpp"
#include "user.hpp"
#include "game.hpp"


// a regisztrációs üzenet feldolgozása, itt még nincs a felhasználó autentikálva,
// ezért a válaszokat direktben a socketbe kell írni, nem lehet az user-nek címezni
// TODO: a közvetlen socketen válaszolásnál nincs kezelve a konkurrencia, több üzenetfeldolgozó
// szál esetén versenyhelyzet van a socketben
void RegisterMessage::process(){
    if(name.length() == 0 || name.length() > MAX_NAME_LENGTH || pass.length() > MAX_PASSWD_LENGTH){
        ErrorMessage err("Invalid name or password length.");
        std::string msg = err.to_string();
        
        if(sockfd > 0) write(sockfd, msg.c_str(), msg.size());
        return;
    }

    // létrejön az új felhasználó az adatbázisban
    if(userdb.new_user(name, pass) < 0){
        ErrorMessage err("Username is already in use.");
        std::string msg = err.to_string();
        if(sockfd > 0) write(sockfd, msg.c_str(), msg.size());
    } else {
        OKMessage ok;
        std::string msg = ok.to_string();
        if(sockfd > 0) write(sockfd, msg.c_str(), msg.size());
    }
}    

// A felhasználó bejelentkező üzenetének kezelése
void LoginMessage::process(){
    if(name.length() == 0 || name.length() > MAX_NAME_LENGTH || pass.length() > MAX_PASSWD_LENGTH){
        ErrorMessage err("Invalid name or pwd length.");
        std::string msg = err.to_string();
        if(sockfd > 0) write(sockfd, msg.c_str(), msg.size());
        return;
    }

    // az adatbázisban kikeressük, ha nem létezik, akkor hiba
    User *user = userdb.auth_user(name, pass);
    if(user == NULL){
        ErrorMessage err("Invalid name or pwd.");
        std::string msg = err.to_string();
        if(sockfd > 0) write(sockfd, msg.c_str(), msg.size());
        return;
    }

    pthread_mutex_lock(&sockfd_users_mutex);

    // ha már volt az userhez socket rendelve, azt ki kell törölni
    if(user->sockfd > 0) sockfd_users.erase(user->sockfd);
    user->sockfd = sockfd;

    // ha ugyanezzel a sockettel más felhasználó van bejelentkezve, tőle ki kell venni
    User *other_user = sockfd_users[sockfd];
    if(other_user != NULL) other_user->sockfd = -1;

    // beállítjuk ehhez a sockethez, az új felhasználót
    sockfd_users[sockfd] = user;

    pthread_mutex_unlock(&sockfd_users_mutex);

    out_msg.push(new OKMessage((void*)user));
    syslog(LOG_INFO, "User logged in: %s", user->name.c_str());
}    

// új játék kezdeményező üzenet
void StartMessage::process(){
    pthread_mutex_lock(&sockfd_users_mutex);
    User *u = sockfd_users[sockfd];    
    pthread_mutex_unlock(&sockfd_users_mutex);

    // ha nincs bejelentkezve, a socketen visszaküldünk egy errort 
    if(u == NULL){
        ErrorMessage err("User hasn't logged in yet.");
        std::string msg = err.to_string();
        if(sockfd) write(sockfd, msg.c_str(), msg.size());
        return;
    }
    
    pthread_mutex_lock(&game_mutex);
    Game *g = user_game[u];

    // ha már létezik játék vagy már indított egyet, hibát adunk vissza
    
    if(g != NULL){
        pthread_mutex_unlock(&game_mutex);
        out_msg.push(new ErrorMessage("You are already in a game.", (void*) u));
        return;
    }
    
    if(u == queued_user){
        pthread_mutex_unlock(&game_mutex);
        out_msg.push(new ErrorMessage("You are already queued.", (void*) u));
        return;
    }

    // nincs még senki várólistán, ezért felkerül oda
    if(queued_user == NULL){
        queued_user = u;
        pthread_mutex_unlock(&game_mutex);
        syslog(LOG_INFO, "StartMessage: User %s queued.", u->name.c_str());
        out_msg.push(new OKMessage((void*) u));
        return;
    }

    // itt már a játék létrejön u és queued_user között
    out_msg.push(new OKMessage((void*) u));

    g = new Game(u, queued_user, GAME_LENGTH_ms);
    user_game[u] = g;
    user_game[queued_user] = g;

    syslog(LOG_INFO, "Game starting: %s %s\n",u->name.c_str(), queued_user->name.c_str());
    queued_user = NULL;    

    pthread_mutex_unlock(&game_mutex);
    
    g->start();
}

// a játékos lépő üzenete
void MoveMessage::process(){
    pthread_mutex_lock(&sockfd_users_mutex);
    User *user = sockfd_users[sockfd];
    pthread_mutex_unlock(&sockfd_users_mutex);

    if(user == NULL) return;

    pthread_mutex_lock(&game_mutex);
    Game *game = user_game[user];
    pthread_mutex_unlock(&game_mutex);

    if(game == NULL){
        out_msg.push(new ErrorMessage("no game running.", (void*) user));
        return;
    }

    game->move(user, from, to);
}

// ez azt jelzi, hogy egy socketen megszűnt a kapcsolat
void DisconnectMessage::process(){
    pthread_mutex_lock(&sockfd_users_mutex);
    User* u = sockfd_users[sockfd];
    sockfd_users.erase(sockfd);
    pthread_mutex_unlock(&sockfd_users_mutex);

    // nem volt bejelentkezve senki a socketen
    if(u == NULL){
        syslog(LOG_INFO, "client disconnected from socket %d", sockfd);
        return;
    }

    // ha a felhasználó játékra várt, kivesszük.
    // ha játékban van, az megy tovább, még visszacsatlakozhat
    pthread_mutex_lock(&game_mutex);    
    u->sockfd = -1;
    if(queued_user == u) queued_user = NULL;
    pthread_mutex_unlock(&game_mutex);

    syslog(LOG_INFO, "User disconnected: %s", u->name.c_str());
}


//// szerver -> kliens üzenetek. Ezeket csak továbbítani kell a címzetthez

void ErrorMessage::process(){
    User *u = (User*) user;
    
    if(u != NULL)
        u->send(this);
}

void OKMessage::process(){
    User *u = (User*) user;

    if(u != NULL)
        u->send(this);
}

void GameMessage::process() {
    pthread_mutex_lock(&name_users_mutex);
    User *w = name_users[white];
    User *b = name_users[black];
    pthread_mutex_unlock(&name_users_mutex);

    if(b != NULL){
        b->send(this);
    }

    if(w != NULL){
        w->send(this);
    }
}


void MovedMessage::process(){
    User *u = (User*) user;

    if(u != NULL){
        u->send(this);
    }
}

    
Message* parseMessage(char *buf, int len, int sockfd) {
    std::vector<std::string> parts;
    int start = 0;
    for(int i = 0; i < len; i++){
        if(buf[i] == ' ' || buf[i] == '\n'){
            buf[i] = '\0';
            parts.push_back(std::string(buf + start));
            start = i + 1;
        }
    }

    if(parts.size() == 0) goto err;

    Message *m;

    if(parts[0] == "REGISTER"){
        if(parts.size() != 3) goto err;
        m = new RegisterMessage(parts[1], parts[2], sockfd);
    } else if(parts[0] == "LOGIN"){
        if(parts.size() != 3) goto err;
        m = new LoginMessage(parts[1], parts[2], sockfd);
    } else if(parts[0] == "START"){
        if(parts.size() > 1) goto err;
        m = new StartMessage(sockfd);
    } else if(parts[0] == "MOVE"){
        if(parts.size() != 3) goto err;
        
        int a = string_to_int(parts[1]);
        int b = string_to_int(parts[2]);
        if(a < 0 || b < 0 || a > 63 || b > 63) goto err;
        
        m = new MoveMessage(a, b, sockfd);
    } else goto err; 
    
    return m;
    
err:
    syslog(LOG_ERR, "parseMessage: Invalid message format.");
    return NULL;
}
