#include <string>
#include <stdlib.h>
#include <iostream>

#include "client.h"

Client *client;

Client::Client()
{}

Client::~Client(){
    if(sock != NULL) delete sock;
}

enum Client::status Client::getStatus(){
    return clientStatus;
}

void Client::setStatus(enum status s){
    enum status st = clientStatus;
    clientStatus = s;
    if(st != s){
        emit statusChanged(s); // minden státusz változásnál jelzést küld
    }
}

// csatlakozik egy szerverhez
void Client::connectTo(QString server){
    int sep_cnt = server.count(':');
    if (sep_cnt > 1){
        emit error("Invalid server address.");
        return;
    }
    
    uint16_t port = SERVER_PORT;
    if (sep_cnt == 1) {
        QStringList parts = server.split(':');
        server = parts[0];
        std::string port_str = parts[1].toStdString();
        int port_candidate = atoi(port_str.c_str());
        if (port_candidate <= 0 or port_candidate >= (1 << 16)){
            emit error("Invalid port.");
            return;
        }
        port = port_candidate;
    }

    // ha volt eddigi kapcsolat, az megszűnik
    setStatus(Disconnected);
    if(sock) delete sock;

    sock = new QTcpSocket();

    connect(sock, SIGNAL(disconnected()), this, SLOT(disconnected()));

    sock->connectToHost(server, port);

    // ha nem sikerül csatlakozni, hibakezelés
    if(!sock->waitForConnected(3000)){
        delete sock;
        sock = NULL;
        emit error("Connection to server unsuccessful.");
        return;
    }

    setStatus(Connected);
    connect(sock, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void Client::disconnected(){
    if(sock){
        disconnect(sock, NULL, this, NULL);
        sock->deleteLater();
        sock = NULL;
    }

    setStatus(Disconnected);
}

// adat érkezett a socketbe
void Client::readyRead(){
    char buf[512];
    Message *m;

    // mivel minden üzenetet '\n' zár, addig olvas, amíg van új sor
    while(sock->canReadLine()){
        qint64 len = sock->readLine(buf, sizeof(buf));
        if(len > 0){

            // ha sikeres volt az olvasás, létrehozzuk az üzenetet
            m = parseMessage(buf, len, -1);
            if(m == NULL) continue;

            // ha debug módban van, kiíratjuk a kapott üzenetet
            #ifdef QT_DEBUG
            std::cerr << "uzenet fogadva: " << m->to_string();
            #endif

            m->process();
            delete m;
        } else return;
    }
}

// üzenet elküldése a szervernek
void Client::sendMessage(Message *m){

    if(clientStatus == Disconnected){
        emit error("Not connected to server.");
        delete m;
        return;
    }

    //az üzenet bekerül az elküldöttek közé
    queue.push(m);

    std::string msg = m->to_string();

    // ha debug módban van, kiíratjuk a küldött üzenetet
    #ifdef QT_DEBUG
    std::cerr << "uzenet elkuldve: " << msg;
    #endif

    sock->write(msg.c_str(), msg.size());
}

QString Client::getUsername(){
    return username;
}
