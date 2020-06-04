#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QWidget>
#include <QColor>
#include "field.h"
#include "../board.hpp"

#define NUM_FIELDS 64

// A táblát megjelenítő widget
class BoardWidget : public QWidget
{
    Q_OBJECT
    Board *board = NULL;
    Field *fields[NUM_FIELDS];
    Field *selected_field = NULL; // a jelenleg kijelölt mező a táblán

    qint32 player_side, next_player;
    bool highlightSteps = true;

public:
    enum SIZE {SMALL = 45, MEDIUM = 75, LARGE = 90};
    void setBoardSize(enum SIZE s);
    void setHighlightSteps(bool);

    explicit BoardWidget(QWidget *parent = nullptr);
    ~BoardWidget();

private:
    enum SIZE boardsize;
    QColor fieldColor(int coord);
    void refreshFields();

public slots:
    void resetBoard();
    void playerColor(qint32 color);
    void movePiece(qint32 from, qint32 to);
    void fieldClicked(Field *f);

signals:
    void userMoved(qint32 from, qint32 to);
};

#endif // BOARDWIDGET_H
