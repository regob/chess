///
/// server.cpp
/// A szerver funciókat ellátó függvények implementációja.
///

#include <syslog.h>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>

#include "server.hpp"
#include "auth.hpp"

MessageQueue in_msg, out_msg;
std::vector<pollfd> clients, new_clients;

pthread_mutex_t new_clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t new_clients_empty = PTHREAD_COND_INITIALIZER;

void start_server(){
    struct addrinfo hints;
    struct addrinfo *res, *res_saved;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(NULL, PORT, &hints, &res);
    if(err != 0){
        syslog(LOG_ERR, "getaddrinfo: %s", gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    res_saved = res; //felszabadításhoz kell
    int ssock = -1;
    while(res != NULL && ((ssock = socket(res->ai_family,
                          res->ai_socktype, res->ai_protocol)) < 0)){
        syslog(LOG_WARNING, "socket: %s", strerror(errno));
        res = res->ai_next;
    }

    if(ssock < 0){
        syslog(LOG_ERR, "Cannot open socket.");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
     

    if(bind(ssock, res->ai_addr, res->ai_addrlen) < 0){
        syslog(LOG_ERR, "bind: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(listen(ssock, 10) < 0){
        syslog(LOG_ERR, "listen: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res_saved);
    syslog(LOG_INFO, "Server up.\n");


    pthread_t in_worker, out_worker;
    if(pthread_create(&in_worker, NULL, MessageQueueWorker::work, (void*)&in_msg)){
        syslog(LOG_ERR, "pthread_create: %s", strerror(errno));
    }
    if(pthread_create(&out_worker, NULL, MessageQueueWorker::work, (void*)&out_msg)){
        syslog(LOG_ERR, "pthread_create: %s", strerror(errno));
    }

    pthread_t client_handler;
    
    if(pthread_create(&client_handler, NULL, handle_clients, NULL)){
        syslog(LOG_ERR, "pthread_create: %s", strerror(errno));
    }

    userdb.open_db();

    struct sockaddr_in client_addr;
    int c_sock;
    socklen_t addr_len = sizeof(client_addr);
    
    while((c_sock = accept(ssock, (struct sockaddr*)&client_addr, &addr_len)) >= 0){
        syslog(LOG_INFO, "New client accepted, socket fd: %d", c_sock);
        pthread_mutex_lock(&new_clients_mutex);

        pollfd cpoll;

        memset(&cpoll, 0, sizeof(cpoll));
        cpoll.fd = c_sock;
        cpoll.events |= POLLIN;
        
        if(new_clients.size() == 0) pthread_cond_signal(&new_clients_empty);

        new_clients.push_back(cpoll);
        pthread_mutex_unlock(&new_clients_mutex);
    }

    syslog(LOG_ERR, "accept: %s", strerror(errno));
    exit(EXIT_FAILURE);
}

void* handle_clients(void* data){
    char buf[2048];
    
    while(1) {
        pthread_mutex_lock(&new_clients_mutex);

        if(clients.size() == 0 && new_clients.size() == 0)
            pthread_cond_wait(&new_clients_empty, &new_clients_mutex);

        for(unsigned i = 0; i < new_clients.size(); i++)
            clients.push_back(new_clients[i]);

        new_clients.clear();
        pthread_mutex_unlock(&new_clients_mutex);


        if(poll(&clients[0], clients.size(), POLL_WAIT) < 0){
            syslog(LOG_ERR, "poll: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        for(unsigned i = 0; i < clients.size(); i++){
            if(clients[i].revents & (POLLNVAL | POLLERR)){
                syslog(LOG_INFO, "Client socket error.");
                clients[i].fd *= -1;
            }

            if(clients[i].revents & POLLIN){
                int len = read(clients[i].fd, buf, sizeof(buf));
                
                if(len == 0){
                    in_msg.push(new DisconnectMessage(clients[i].fd));
                    clients[i].fd *= -1;
                } else if(len < 0){
                    syslog(LOG_ERR, "read: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                } else {
                    int start = 0;
                    for(int j = 0; j < len; j++){
                        if(buf[j] == '\n'){
                            Message* msg = parseMessage(buf + start, j - start + 1, clients[i].fd);
                            if(msg != NULL){
                                in_msg.push(msg);
                            }
                            start = j + 1;
                        }
                    }
                }
               
            }
        }

    }
}

