#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QLabel>
#include "boardwidget.h"
#include "../message.hpp"
#include "../board.hpp"
#include "client.h"

// a teljes GUI-t tartalmazza az ablak keretét leszámítva.
// kezeli a játék állapotát, és ennek megfelelően módosítja a felületet
class GameWidget : public QWidget
{
    Q_OBJECT
    BoardWidget *boardWidget;
    QLabel *white, *black;
    QLabel *time_w, *time_b;
    QLabel *status_label;
    QLabel *user_name;
    QLabel *white_text, *black_text;

    QFont title_font, normal_font;

    qint32 player_color = -1, next_player = -1;

public:
    explicit GameWidget(QWidget *parent = nullptr);
    ~GameWidget();

    enum FONT_SIZE {SMALL = 10, MEDIUM = 14, LARGE = 16};
    void setFontSize(enum FONT_SIZE);

public slots:
    void gameState(QString w, QString b, qint32 time_w, qint32 time_b, qint32 status);
    void error(QString err);
    void moved(qint32 from, qint32 to);
    void statusChanged(enum Client::status s);
    void registered();

    void userMoved(qint32 from, qint32 to);

signals:
    void sendMessage(Message *m);
    void connectTo(QString addr);
    void statusBarMessage(QString msg);

    void newGame();
    void playerColor(qint32 color);
    void movePiece(qint32 from, qint32 to);
};

#endif // GAMEWIDGET_H
