#include <QPalette>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayout>
#include "boardwidget.h"
#include "field.h"

#include <iostream>


void BoardWidget::setBoardSize(enum SIZE s){
    Field::loadImages(s);
    boardsize = s;
    for(int i = 0; i < NUM_FIELDS; i++)
        fields[i]->setFieldSize(boardsize);
}

void BoardWidget::resetBoard(){
    next_player = WHITE;
    player_side = WHITE;
    selected_field = NULL;

    if(board != NULL) delete board;
    board = new Board();

    for(int i = 0; i < NUM_FIELDS; i++){
        fields[i]->setCoord(i);
        fields[i]->setPiece(board->board[i / 8][i % 8]);
        fields[i]->setBackground(fieldColor(i));
    }
}

BoardWidget::BoardWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *vertical = new QVBoxLayout();
    setLayout(vertical);
    vertical->setSpacing(0);

    QHBoxLayout *horizontal[8];
    for(int i = 0; i < 8; i++){
        horizontal[i] = new QHBoxLayout();
        vertical->addLayout(horizontal[i]);
        horizontal[i]->setSpacing(0);
        horizontal[i]->setSizeConstraint(QLayout::SetFixedSize);
    }

    vertical->setSizeConstraint(QLayout::SetFixedSize);

    for(int i = 0; i < NUM_FIELDS; i++){
        fields[i] = new Field();
        connect(fields[i], SIGNAL(clickedOn(Field*)), this, SLOT(fieldClicked(Field*)));
        horizontal[i / 8]->addWidget(fields[i]);
    }

    setBoardSize(SMALL);
    resetBoard();

}

BoardWidget::~BoardWidget(){
    disconnect(this, NULL, NULL, NULL);
    if(board) delete board;
}

void BoardWidget::setHighlightSteps(bool b){
    highlightSteps = b;
    refreshFields();
}

// kiszámítja egy mező színét
QColor BoardWidget::fieldColor(qint32 coord){
    bool dark_square = ((coord % 8) + (coord/8)) % 2;

    // ha be van kapcsolva a kiemelés, megjelöli a lehetséges lépéseket
    if(highlightSteps && selected_field != NULL){
        qint32 scord = selected_field->getCoord();
        if(board->valid_move(scord / 8, scord % 8, coord / 8, coord % 8)){

            // a mező alapszínétől függően változik a szín
            if(dark_square) return QColor(180, 140, 13);
            return QColor(219, 179, 36);
        }
    }

    // ha utolsó lépés innen, vagy ide történt, más színt kap
    if(board->last_move_from.first*8 + board->last_move_from.second == coord ||
       board->last_move_to.first*8 + board->last_move_to.second == coord)
    {
        return dark_square ? QColor(120, 46, 59) : QColor(207, 120, 156);
    }

    // ha épp kijelölés alatt áll a mező, azt is színezni kell
    if(selected_field != NULL && selected_field->getCoord() == coord)
        return QColor(111, 88, 13);

    // egyéb esetben tábla alapszínét kapja
    return dark_square ? QColor(0, 100, 0) : Qt::white;
}

// a játékos oldalának állítása
void BoardWidget::playerColor(qint32 color){
    if(player_side == color) return;

    // ha változott az oldal, akkor meg kell fordítani a tábla nézetét
    for(int i = 0; i < NUM_FIELDS; i++){

        // ha a játékos oldala fekete, a board koordinátáit visszafele kell számolni, ezzel megfordítva a táblát
        qint32 coord = (color & WHITE) ? i : 63 - i;

        // a mező helye, és tartalma beállításra kerül az új helyének mefelelően
        fields[i]->setCoord(coord);
        fields[i]->setPiece(board->board[coord / 8][coord % 8]);
        fields[i]->setBackground(fieldColor(coord));
    }

    player_side = color;
}

void BoardWidget::refreshFields(){
    for(int i = 0; i < NUM_FIELDS; i++){
        int coord = fields[i]->getCoord();
        fields[i]->setPiece(board->board[coord / 8][coord % 8]);
        fields[i]->setBackground(fieldColor(coord));
    }
}

void BoardWidget::movePiece(qint32 from, qint32 to){

    // megtörténik a lépés
    board->move_piece(from / 8, from % 8, to / 8, to % 8);

    // a teljes pálya frissítésre kerül
    refreshFields();

    // ellenkező játékos kerül sorra
    next_player ^= (WHITE | BLACK);
}

void BoardWidget::fieldClicked(Field *f){
    if(selected_field == NULL){
        qint32 coord = f->getCoord();
        if(!(board->board[coord / 8][coord % 8] & player_side)) return;

        selected_field = f;
        refreshFields();

    } else {
        if(selected_field == f){
            selected_field = NULL;
            refreshFields();
            return;
        }
        int coord = f->getCoord();
        int old_coord = selected_field->getCoord();

        if(board->board[coord / 8][coord % 8] & player_side){
            selected_field = f;
            refreshFields();
            return;
        }

        emit userMoved(old_coord, coord);

        selected_field = NULL;
        refreshFields();
    }
}
