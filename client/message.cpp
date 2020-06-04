#include <string>
#include <vector>
#include <QString>

#include "../message.hpp"
#include "client.h"

/////// szervertől származó üzenetek feldolgozása /////////

void ErrorMessage::process(){
    emit client->error(QString::fromStdString(err));
    Message *m = client->queue.pop();
    delete m;
}

void OKMessage::process(){
    Message *m = client->queue.pop();
    m->process();
    delete m;
}

// játék státusz üzenet, az időket, és a végeredmény állását küldi
void GameMessage::process(){
    emit client->gameState(QString::fromStdString(white),
                                 QString::fromStdString(black),
                                 time_w, time_b, state);

    if(this->state){ // játék vége, a state ezt jelzi
        client->setStatus(Client::Loggedin);
    } else client->setStatus(Client::InGame);
}

// az ellenfél lépését küldi
void MovedMessage::process(){
    emit client->moved(from, to);
}


////////////// kliens üzenetek process() függvényei /////////////////
// ezek csak akkor hívódnak meg ha OKMessage jött rájuk a szervertől, nem volt hiba.

void RegisterMessage::process(){
    emit client->registered();
}

void LoginMessage::process(){
    client->username = QString::fromStdString(name);
    client->setStatus(Client::Loggedin);
}

void StartMessage::process(){
    client->setStatus(Client::Queued);
}

void MoveMessage::process(){
    emit client->moved(from, to);
}


// ez csak a szerveren belüli üzenet ...
void DisconnectMessage::process(){}


// fogadott bytefolyam üzenetté alakítása.
// A sockfd paraméter csak a szerveren kell a kliens azonosítására, itt használaton kívüli.
Message* parseMessage(char* buf, int len, int sockfd){
    Message *m = NULL;

    std::vector<std::string> parts;
    int start = 0;
    for(int i = 0; i < len; i++){
        if(buf[i] == ' ' || buf[i] == '\n'){
            buf[i] = '\0';
            parts.push_back(std::string(buf + start));
            start = i + 1;
        }
    }

    if(parts.size() == 0) return NULL;

    if(parts[0] == "OK"){
        m = new OKMessage();
    } else if(parts[0] == "ERROR"){
        std::string msg = "";
        for(unsigned i = 1; i < parts.size(); i++){
            msg += parts[i];
            if(i < parts.size() - 1) msg += " ";
        }

        m = new ErrorMessage(msg);
    } else if(parts[0] == "GAME"){
        if(parts.size() != 6) return NULL;
        m = new GameMessage(parts[1], parts[2], string_to_int(parts[3]),
                string_to_int(parts[4]), string_to_int(parts[5]));
    } else if(parts[0] == "MOVED"){
        if(parts.size() != 3) return NULL;
        m = new MovedMessage(string_to_int(parts[1]), string_to_int(parts[2]));
    }

    return m;
}
