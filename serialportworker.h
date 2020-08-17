#ifndef SERIALPORTWORKER_H
#define SERIALPORTWORKER_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QCoreApplication>
#include <QTextStream>
#include <QMessageBox>
#include <QBitArray>
#include <QDataStream>
#include <QDebug>

#include <iostream>

class SerialPortWorker : public QObject
{
    Q_OBJECT

public:

    explicit SerialPortWorker(QSerialPort *serialPort, QObject *parent = nullptr);
    ~SerialPortWorker();

    void write(const QByteArray &writeData);
    bool changeBaudrate(const qint32 &value);

    QByteArray m_readData;
    bool crashBitFlag = false;

signals:

    void dataReady();

public slots:

    void handleCrashBit(int state);

private slots:

    void handleBytesWritten(qint64 bytes);
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:

    QSerialPort *m_serialPort = nullptr;
    QByteArray m_writeData;
    qint64 m_bytesWritten = 0;
    QTextStream m_standardOutput;

    QByteArray bitsToBytes(const QBitArray &bits);
    QBitArray  bytesToBits(const QByteArray &bytes);
    int        combinationCount(const QBitArray &bits);
    QBitArray  bitsStuffing(const QBitArray &bits);
    QBitArray  bitsDeStuffing(const QBitArray &bits);
    char crc8(const QByteArray &bytes);

#ifdef QT_DEBUG
    void       log_bit_array(QBitArray bits);
#endif // ifdef QT_DEBUG
};

#endif // SERIALPORTWORKER_H
