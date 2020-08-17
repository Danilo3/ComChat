#ifndef MAINCHATWINDOW_H
#define MAINCHATWINDOW_H

#include <QMainWindow>
#include <QDialogButtonBox>
#include <QDebug>

#include "serialportworker.h"
#include "dialogbaudrate.h"

#include <iostream>

namespace Ui {
class MainChatWindow;
}

class MainChatWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainChatWindow(QWidget *parent = nullptr);
    ~MainChatWindow();

    void initMain();
    void initSerialPort();
    void connectSignals();

private:

    Ui::MainChatWindow *ui;
    SerialPortWorker *m_serialPortWorker = nullptr;

    void createActions();

private slots:

    void handlerSendButton();
    void handleDataReady();
    void handleMenuBaudRate();
    void handlerDialogDataReady(qint32 data);
};

#endif // MAINCHATWINDOW_H
