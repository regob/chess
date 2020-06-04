#include "gamewindow.h"
#include "ui_gamewindow.h"
#include "ui_dialog.h"
#include <QInputDialog>
#include <QSizePolicy>

#include "../message.hpp"
#include <iostream>
#include <QVBoxLayout>
#include <QList>
#include <QHBoxLayout>
#include "field.h"

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GameWindow)
{
    ui->setupUi(this);
    connect(ui->gameWidget, SIGNAL(statusBarMessage(QString)),
            this, SLOT(statusBarMessage(QString)));
    setWindowTitle("Chess client");

}

GameWindow::~GameWindow()
{
    delete ui;
}

void GameWindow::resizeBoard(enum BoardWidget::SIZE s){
    BoardWidget *bw = ui->gameWidget->findChild<BoardWidget*>();
    bw->setBoardSize(s);
}

void GameWindow::paintEvent(QPaintEvent *){
    // ez kell, hogy rögtön átméreteződjön a boarddal,
    // és ne is lehessen átméretezni az ablakot kézzel
    setFixedSize(minimumSize());
}


void GameWindow::on_actionLarge_triggered()
{
    ui->gameWidget->setFontSize(GameWidget::LARGE);
    resizeBoard(BoardWidget::LARGE);
}

void GameWindow::on_actionMedium_triggered()
{
    ui->gameWidget->setFontSize(GameWidget::MEDIUM);
    resizeBoard(BoardWidget::MEDIUM);
}

void GameWindow::on_actionSmall_triggered()
{
    ui->gameWidget->setFontSize(GameWidget::SMALL);
    resizeBoard(BoardWidget::SMALL);
}



void GameWindow::statusBarMessage(QString msg){
    ui->statusbar->showMessage(msg, 10000);
}

void GameWindow::on_actionConnect_triggered()
{
    QInputDialog dlg;
    dlg.setLabelText("Connect to server");
    dlg.setTextValue("127.0.0.1");
    if(dlg.exec() == QDialog::Accepted)
    {
        emit ui->gameWidget->connectTo(dlg.textValue());
    }

}

void GameWindow::on_actionRegister_triggered()
{
    QDialog qd;
    Ui::UserDialog dlg;
    dlg.setupUi(&qd);
    if(qd.exec() == QDialog::Accepted){
        Message *m = new RegisterMessage(dlg.text_username->toPlainText().toStdString(),
                                         dlg.text_password->toPlainText().toStdString());
        emit ui->gameWidget->sendMessage(m);
    }
}



void GameWindow::on_actionLogin_triggered()
{
    QDialog qd;
    Ui::UserDialog dlg;
    dlg.setupUi(&qd);
    qd.setWindowTitle("Login");
    if(qd.exec() == QDialog::Accepted){
        Message *m = new LoginMessage(dlg.text_username->toPlainText().toStdString(),
                                      dlg.text_password->toPlainText().toStdString());
        emit ui->gameWidget->sendMessage(m);
    }
}


void GameWindow::on_actionNew_game_triggered()
{
    Message *m = new StartMessage();
    emit ui->gameWidget->sendMessage(m);
}


void GameWindow::on_actionHighlight_moves_toggled(bool highlight){
    BoardWidget *bw = ui->gameWidget->findChild<BoardWidget*>();
    bw->setHighlightSteps(highlight);
}
