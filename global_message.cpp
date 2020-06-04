#include <stdlib.h>
#include <vector>
#include <string>
#include <syslog.h>
#include <stdio.h>
#include <stdexcept>


#include "message.hpp"

Message::~Message(){}

MessageQueue::MessageQueue(){
    pthread_cond_init(&empty, NULL);
    pthread_mutex_init(&mutex, NULL);
}

MessageQueue::~MessageQueue(){
    pthread_cond_destroy(&empty);
    pthread_mutex_destroy(&mutex);
}

void MessageQueue::push(Message *m){
    pthread_mutex_lock(&mutex);
    if(queue.empty()) pthread_cond_signal(&empty);
    queue.push(m);
    pthread_mutex_unlock(&mutex);
}

Message* MessageQueue::pop(){
    pthread_mutex_lock(&mutex);
    if(queue.empty()) pthread_cond_wait(&empty, &mutex);

    Message* m = queue.front();
    queue.pop();
    
    pthread_mutex_unlock(&mutex);
    return m;
}

void* MessageQueueWorker::work(void *data){
    MessageQueue *q = (MessageQueue*) data;
    while(1){
        Message *m = q->pop();
        m->process();
        delete m;
    }
}

///////////////// to_string() függvények //////////////////////

std::string RegisterMessage::to_string(){
    std::string s("REGISTER " + name + " " + pass + "\n");
    return s;
}

std::string LoginMessage::to_string(){
    std::string s = "LOGIN " + name + " " + pass + "\n";
    return s;
}

std::string StartMessage::to_string(){
    std::string s = "START\n";
    return s;
}

std::string MoveMessage::to_string(){
    std::string s = "MOVE " + std::to_string(from) + " " + std::to_string(to) + "\n";
    return s;
}

std::string OKMessage::to_string(){
    return std::string("OK\n");
}

std::string ErrorMessage::to_string(){
    std::string s = "ERROR " + err + "\n";
    return s;
}

std::string GameMessage::to_string(){
    std::string s = "GAME " + white + " " + black + " " + std::to_string(time_w) + " " +
        std::to_string(time_b) + " " + std::to_string(state) + "\n";
    return s;
}

std::string MovedMessage::to_string(){
    std::string s = "MOVED " + std::to_string(from) + " " + std::to_string(to) + "\n";
    return s;
}

std::string DisconnectMessage::to_string(){
    std::string s = "DISCONNECTED " + std::to_string(sockfd) + "\n";
    return s;
}

///////////////// konstruktorok //////////////////////////////

RegisterMessage::RegisterMessage(std::string _name, std::string _pass, int _sockfd)
    :name(_name), pass(_pass), sockfd(_sockfd) {}

LoginMessage::LoginMessage(std::string _name, std::string _pass, int _sockfd)
    :name(_name), pass(_pass), sockfd(_sockfd) {}

StartMessage::StartMessage(int _sockfd)
    :sockfd(_sockfd) {}

MoveMessage::MoveMessage(int _from, int _to, int _sockfd)
    :from(_from), to(_to), sockfd(_sockfd) {}

OKMessage::OKMessage(void *_user)
    :user(_user) {}

ErrorMessage::ErrorMessage(std::string _err, void *_user)
    :err(_err), user(_user) {}

GameMessage::GameMessage(std::string _white, std::string _black, int _time_w,
            int _time_b , int _state)
    :white(_white), black(_black), time_w(_time_w), time_b(_time_b), state(_state) {}
         

MovedMessage::MovedMessage(int _from, int _to, void *_user)
    :from(_from), to(_to), user(_user) {}
   
DisconnectMessage::DisconnectMessage(int _sockfd)
    :sockfd(_sockfd) {}

/////////////////////////////////////////////////////////////

// stringet integerré alakít
int string_to_int(std::string s){
    int res;
    try {
        res = std::stoi(s);
    } catch(std::invalid_argument const &e){
        return -1;
    } catch(std::out_of_range const &e){
        return -1;
    }

    return res;
}
