#include "dialogbaudrate.h"
#include "ui_dialogbaudrate.h"

DialogBaudrate::DialogBaudrate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBaudrate)
{
    ui->setupUi(this);
    connectSignals();
}

DialogBaudrate::~DialogBaudrate()
{
    delete ui;
}

void DialogBaudrate::connectSignals()
{
    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this,
            SLOT(handleComboboxChoice(QString)));
    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton *)), this,
            SLOT(handleButtonboxClicked(QAbstractButton *)));
}

void DialogBaudrate::handleComboboxChoice(QString value)
{
    ui->spinBox->setValue(value.toInt());
}

void DialogBaudrate::handleButtonboxClicked(QAbstractButton *button)
{
    if ((reinterpret_cast<QPushButton *>(button)
         == ui->buttonBox->button(QDialogButtonBox::Ok))
        || (reinterpret_cast<QPushButton *>(button)
            == ui->buttonBox->button(QDialogButtonBox::Apply))) emit dataReady(
            ui->spinBox->value());
    emit dataReady(0);
}

