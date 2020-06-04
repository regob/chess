#ifndef Field_H
#define Field_H

#include <QWidget>
#include <QPixmap>
#include <QColor>
#include <map>

// a tábla egy mezője, lehet rajta egy figura, és a háttérszíne állítható
class Field :public QWidget {
    Q_OBJECT

    QPixmap *image = NULL;
    qint32 coord;
    qint32 fieldSize = 0;

    static std::map<qint32, QPixmap> img;

public:

    explicit Field(QWidget *parent = nullptr);
    void setFieldSize(qint32 _size);
    void setBackground(QColor bg);
    void setPiece(qint32 p);
    void setCoord(qint32 _coord);

    qint32 getCoord();

    static void loadImages(qint32 size);

protected:

    virtual void paintEvent(QPaintEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);

signals:
    void clickedOn(Field *f);

};

#endif // Field_H
