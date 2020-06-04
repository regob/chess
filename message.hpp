#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <queue>
#include <vector>
#include <pthread.h>

// Általános kliens-szerver közötti üzenet
struct Message {
    virtual std::string to_string() = 0;
    virtual void process() = 0;
    virtual ~Message();
};

// Üzenetsor, az üzenetek ideiglenes tárolására
struct MessageQueue {
    std::queue<Message*> queue;
    pthread_cond_t empty;
    pthread_mutex_t mutex;

    MessageQueue();
    ~MessageQueue();
            
    void push(Message *);
    Message* pop();
};

struct MessageQueueWorker {
    static void* work(void *);
};

Message* parseMessage(char *buf, int len, int sockfd);
int string_to_int(std::string s);

///////////// kliens -> szerver üzenetek //////////////////

struct RegisterMessage :public Message {
    std::string name, pass;
    int sockfd;
    
    RegisterMessage(std::string _name, std::string _pass, int _sockfd = -1);
    
    std::string to_string();
    void process();
};

struct LoginMessage :public Message {
    std::string name, pass;
    int sockfd;
    
    LoginMessage(std::string _name, std::string _pass, int _sockfd = -1);
    
    std::string to_string();
    void process();
};


struct StartMessage :public Message {
    int sockfd;
    
    StartMessage(int _sockfd = -1);
    
    std::string to_string();
    void process();
};

struct MoveMessage :public Message {
    int from, to, sockfd;

    MoveMessage(int _from, int _to, int _sockfd = -1);

    std::string to_string();
    void process();
};

/////////// szerver -> kliens üzenetek /////////////

struct OKMessage :public Message {
    void *user;

    OKMessage(void *_user = NULL);
    
    std::string to_string();
    void process();
};

struct ErrorMessage :public Message {
    std::string err;
    void *user;

    ErrorMessage(std::string _err, void *_user = NULL);
    
    std::string to_string();
    void process();
};

struct GameMessage :public Message {
    std::string white, black;
    int time_w, time_b;
    int state;

    GameMessage(std::string _white, std::string _black, int _time_w,
                int _time_b , int _state);
         
    std::string to_string();
    void process();    
};

struct MovedMessage :public Message {
    int from, to; 
    void *user;
    
    MovedMessage(int _from, int _to, void *_user = NULL);
    
    std::string to_string();
    void process();
};

///////////// szerveren belüli üzenet //////////////

struct DisconnectMessage :public Message {
    int sockfd;

    DisconnectMessage(int _sockfd);

    std::string to_string();
    void process();
};
    
#endif
