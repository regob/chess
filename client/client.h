#ifndef CLIENT_H
#define CLIENT_H
#include <QObject>
#include <QtNetwork>
#include <QString>
#include <QStringList>

#include "../message.hpp"

#define SERVER_PORT 25000

// A szerverrel kommunikál, nyilvántartja a kapcsolat állapotát, és
// fogadja, küldi az üzeneteket
class Client :public QObject
{
    Q_OBJECT
public:
    Client();
    ~Client();
    enum status {Disconnected, Connected, Loggedin, Queued, InGame};
    enum status getStatus();
    QString getUsername();

private:
    enum status clientStatus = Disconnected;
    QTcpSocket *sock = NULL;
    MessageQueue queue;
    QString username = "";

    void setStatus(enum status s);

public slots:
    void connectTo(QString server);
    void sendMessage(Message *m);
    void disconnected();
    void readyRead();

signals:
    void gameState(QString white, QString black, qint32 time_w, qint32 time_b, qint32 stat);
    void error(QString err);
    void moved(qint32 from, qint32 to);
    void statusChanged(enum Client::status stat);
    void registered();

public:
     // a feldolgozó függvényeknek módosítaniuk kell a privát adattagokat is,
     // ezért friendnek adom meg őket
     friend void ErrorMessage::process();
     friend void OKMessage::process();
     friend void GameMessage::process();
     friend void MoveMessage::process();
     friend void StartMessage::process();
     friend void RegisterMessage::process();
     friend void LoginMessage::process();
};



extern Client *client;


#endif // CLIENT_H
