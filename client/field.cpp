#include <string>
#include <QString>
#include <QPainter>
#include <QIcon>
#include "field.h"
#include "../board.hpp"

#include <iostream>

Field::Field(QWidget *parent) :QWidget(parent) {
    setContentsMargins(0,0,0,0);
    setAutoFillBackground(true);
}


std::map<int, QPixmap> Field::img = std::map<int, QPixmap>();

void Field::setFieldSize(int _size){
    fieldSize = _size;
    setFixedSize(QSize(_size, _size));
}

// betölti az adott méretű ikonokat az erőforrásfájlból
void Field::loadImages(int size){
    int colors[] = {WHITE, BLACK};
    int Fields[] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, PAWN};
    std::string names[] = {"rook", "knight", "bishop", "queen", "king", "pawn"};

    for(unsigned i = 0; i < sizeof(colors)/sizeof(int); i++){
        for(unsigned j = 0; j < sizeof(Fields)/sizeof(int); j++){
            std::string path = std::string(":/resources/") + ((colors[i] & WHITE) ? "w" : "b");
            path += names[j] + ".svg";


            QString _path = QString::fromStdString(path);
            QPixmap pix = QIcon(_path).pixmap(QSize(size,size));

            img[(colors[i]) | Fields[j]] = pix;
        }
    }
}

void Field::setPiece(int p){
    image = &(img[p]);
    repaint();
}

void Field::setBackground(QColor bg){
    QPalette pal = palette();
    pal.setColor(QPalette::Background, bg);
    setPalette(pal);

    repaint();
}

void Field::setCoord(int _coord){
    coord = _coord;
}

qint32 Field::getCoord(){
    return coord;
}


void Field::paintEvent(QPaintEvent *e){
    QPainter painter;
    painter.begin(this);
    painter.drawPixmap(0,0,fieldSize,fieldSize,*image);
    painter.end();
}

void Field::mousePressEvent(QMouseEvent *e){
    emit clickedOn(this);
}
