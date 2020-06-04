#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include "boardwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class GameWindow; }
QT_END_NAMESPACE

// A teljes alkalmazás ablaka
class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    GameWindow(QWidget *parent = nullptr);
    ~GameWindow();

    virtual void paintEvent(QPaintEvent *);

private slots:
    void on_actionLarge_triggered();
    void on_actionMedium_triggered();
    void on_actionSmall_triggered();
    void on_actionNew_game_triggered();
    void statusBarMessage(QString msg);
    void on_actionConnect_triggered();
    void on_actionRegister_triggered();
    void on_actionLogin_triggered();

    void resizeBoard(enum BoardWidget::SIZE);

    void on_actionHighlight_moves_toggled(bool arg1);

private:
    Ui::GameWindow *ui;
};
#endif // GAMEWINDOW_H
