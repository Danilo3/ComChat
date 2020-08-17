#ifndef DIALOGBAUDRATE_H
#define DIALOGBAUDRATE_H

#include <QAbstractButton>
#include <QDialog>

namespace Ui {
class DialogBaudrate;
}

class DialogBaudrate : public QDialog
{
    Q_OBJECT

public:

    explicit DialogBaudrate(QWidget *parent = nullptr);
    ~DialogBaudrate();

    void connectSignals();

private:

    Ui::DialogBaudrate *ui;

signals:

    void dataReady(qint32 data);

private slots:

    void handleComboboxChoice(QString value);
    void handleButtonboxClicked(QAbstractButton *button);
};

#endif // DIALOGBAUDRATE_H
