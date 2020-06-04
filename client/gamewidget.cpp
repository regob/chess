#include "gamewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <iostream>

// felépíti a GUI-t, és létrehozza a klienst
GameWidget::GameWidget(QWidget *parent) : QWidget(parent)
{
    boardWidget = new BoardWidget();
    white = new QLabel();
    black = new QLabel();
    time_b = new QLabel();
    time_w = new QLabel();
    status_label = new QLabel();
    user_name = new QLabel();
    white_text = new QLabel();
    black_text = new QLabel();

    white_text->setText("white");
    black_text->setText("black");
    status_label->setText("Disconnected");

    // az elején elrejtjük a feliratokat
    white_text->hide();
    black_text->hide();

    title_font = QFont("roboto");
    title_font.setBold(true);
    normal_font = QFont("roboto");

    setFontSize(SMALL);

    // a fő layout, ebbe kerül bele a többi
    QVBoxLayout *vert = new QVBoxLayout();
    setLayout(vert);
    vert->setSizeConstraint(QLayout::SetFixedSize);

    // színfeliratokat, és státuszt megjelenítő sor
    QHBoxLayout *zeroth_row = new QHBoxLayout();
    zeroth_row->addWidget(white_text);
    zeroth_row->addStretch();
    zeroth_row->addWidget(status_label);
    zeroth_row->addStretch();
    zeroth_row->addWidget(black_text);
    vert->addLayout(zeroth_row);

    // a játékosok hátralévő idejét, és középen a játékos nevét megjelenítő sor
    QHBoxLayout *first_row = new QHBoxLayout();
    first_row->addWidget(time_w);
    first_row->addStretch();
    first_row->addWidget(user_name);
    first_row->addStretch();
    first_row->addWidget(time_b);
    vert->addLayout(first_row);

    // a két játékos neveit tartalmazó sor
    QHBoxLayout *second_row = new QHBoxLayout();
    second_row->addWidget(white);
    second_row->addStretch();
    second_row->addWidget(black);
    vert->addLayout(second_row);

    // a játéktáblát tartalmazó layout, ez van alul
    QHBoxLayout *boardlayout = new QHBoxLayout();
    boardlayout->addWidget(boardWidget);
    vert->addLayout(boardlayout);

    client = new Client();

    // bekötjük a kliens, és a GameWidget signaljait oda-vissza, mert a kliens nem ismeri ezt az objektumot
    connect(client, SIGNAL(gameState(QString, QString, qint32, qint32, qint32)),
            this, SLOT(gameState(QString, QString, qint32, qint32, qint32)));
    connect(client, SIGNAL(error(QString)), this, SLOT(error(QString)));
    connect(client, SIGNAL(moved(qint32, qint32)), this, SLOT(moved(qint32, qint32)));
    connect(client, SIGNAL(registered()), this, SLOT(registered()));
    connect(client, SIGNAL(statusChanged(enum Client::status)), this, SLOT(statusChanged(enum Client::status)));
    connect(this, SIGNAL(connectTo(QString)), client, SLOT(connectTo(QString)));
    connect(this, SIGNAL(sendMessage(Message*)), client, SLOT(sendMessage(Message*)));


    connect(this, SIGNAL(newGame()), boardWidget, SLOT(resetBoard()));
    connect(this, SIGNAL(playerColor(qint32)), boardWidget, SLOT(playerColor(qint32)));
    connect(this, SIGNAL(movePiece(qint32, qint32)), boardWidget, SLOT(movePiece(qint32, qint32)));
    connect(boardWidget, SIGNAL(userMoved(qint32, qint32)), this, SLOT(userMoved(qint32, qint32)));
}

GameWidget::~GameWidget(){
    disconnect(this, NULL, NULL, NULL); // a signalokat le kell kapcsolni
    delete client;
}

// beállítja a betűméretet
void GameWidget::setFontSize(enum FONT_SIZE s){
    normal_font.setPointSize(s);
    title_font.setPointSize(s);

    white_text->setFont(title_font);
    black_text->setFont(title_font);
    status_label->setFont(title_font);

    white->setFont(normal_font);
    black->setFont(normal_font);
    time_b->setFont(normal_font);
    time_w->setFont(normal_font);
    user_name->setFont(normal_font);

}


// a játék által ms-ban megadott időt olvasható formátumba váltja
static QString timeToString(qint32 time){
    QString minutes = QString::number(time/60/1000);
    QString secs = QString::number((time/1000) % 60);
    if((time/1000) % 60 < 10) secs = "0" + secs;
    return minutes + ":" + secs;
}

// játékstátusz frissítés
void GameWidget::gameState(QString w, QString b, qint32 timew, qint32 timeb, qint32 status){

    if(player_color < 0){
        if(w == client->getUsername()){
            player_color = WHITE;
            status_label->setText("Your turn");
        }
        else {
            player_color = BLACK;
            status_label->setText("Opponent's turn.");
        }
        next_player = WHITE;
        emit newGame();
        emit playerColor((qint32)player_color);
    }

    white->setText(w);
    black->setText(b);

    // biztonsági ellenőrzés, bár elvileg nem jöhet negatív idő a szervertől
    if(timew < 0) timew = 0;
    if(timeb < 0) timeb = 0;

    // beállítjuk az időt mutató címkéket
    time_w->setText(timeToString(timew));
    time_b->setText(timeToString(timeb));

    // ha vége lett, frissítjük a státuszt az eredményre
    if(status & WHITE_WON){
        status_label->setText("White won!");
    } else if(status & BLACK_WON){
        status_label->setText("Black won!");
    } else if(status & TIE){
        status_label->setText("Tie!");
    }
}

// hiba esetén a státusz bar jelzi ezt
void GameWidget::error(QString err){
    emit statusBarMessage(err);
}

// lépés történt a játékban
void GameWidget::moved(qint32 from, qint32 to){
    // a pályán elmozgatjuk a bábut
    boardWidget->movePiece(from, to);

    // a soron lévő játékos ellentettjére vált
    next_player ^= (BLACK | WHITE);

    // a feliratot változtatni kell, hogy ki jön
    if(next_player & player_color) status_label->setText("Your turn");
    else status_label->setText("Opponent's turn");
}


// a kliens státusza megváltozott, a státusz baron jelezzük
void GameWidget::statusChanged(enum Client::status s){
    QString msg;

    if(s == Client::Loggedin){
        user_name->setText(client->getUsername());
        // ha még nem volt játék, a player_color < 0.
        if(player_color < 0){
            msg = "You logged in to the server.";
            status_label->setText("Logged in");
        } else {
            // ha már volt, akkor most lett vége, frissítjük az állapotot
            next_player = -1;
            player_color = -1;
            msg = "Game has ended.";
        }
    }
    else if(s == Client::Queued){
         msg = "You are queued.";
         status_label->setText("Queued");
    }
    else if(s == Client::InGame){
         msg = "Game started.";
         black_text->show();
         white_text->show();
    }
    else if(s == Client::Disconnected){
        status_label->setText("Disconnected");
        user_name->setText("");
        msg = "Disconnected from server.";

        // elrejtjük a szín feliratokat
        black_text->hide();
        white_text->hide();
    }
    else {
        status_label->setText("Connected");
        msg = "Connected to server";
    }

    emit statusBarMessage(msg);
}

// a regisztráció sikeres volt
void GameWidget::registered(){
    emit statusBarMessage("Registration successful.");
}

// a felhasználó lépést kattintott
void GameWidget::userMoved(qint32 from, qint32 to){

    // ha megy a játék, és soron van a játékos, akkor elküldjük
    if(next_player > 0 && next_player == player_color){
        Message *m = new MoveMessage(from, to);
        emit sendMessage(m);
    } else {
        if(next_player > 0) emit statusBarMessage("Not your turn.");
        else emit statusBarMessage("No game running.");
    }
}
