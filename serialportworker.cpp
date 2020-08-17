#include "serialportworker.h"

SerialPortWorker::SerialPortWorker(QSerialPort *serialPort, QObject *parent) :
    QObject(parent),
    m_serialPort(serialPort),
    m_standardOutput(stdout)
{
    connect(m_serialPort, &QSerialPort::readyRead,
            this, &SerialPortWorker::handleReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred,
            this, &SerialPortWorker::handleError);
    connect(m_serialPort, &QSerialPort::bytesWritten,
            this, &SerialPortWorker::handleBytesWritten);
}

SerialPortWorker::~SerialPortWorker()
{
    if (m_serialPort) {
        if (m_serialPort->isOpen()) m_serialPort->close();
        delete m_serialPort;
    }
}

void SerialPortWorker::handleBytesWritten(qint64 bytes)
{
    m_bytesWritten += bytes;

    if (m_bytesWritten == m_writeData.size()) {
        m_bytesWritten = 0;
        m_standardOutput << QObject::tr(
            "Data successfully sent to port %1")
            .arg(m_serialPort->portName())
                         << endl;
    }
}

void SerialPortWorker::handleReadyRead()
{
    m_readData = m_serialPort->readAll();

    if (m_readData.isEmpty()) {
        m_standardOutput << QObject::tr(
            "No data was currently available "
            "for reading from port %1")
            .arg(m_serialPort->portName())
                         << endl;
    } else {
        m_standardOutput << QObject::tr(
            "Data successfully received from port %1")
            .arg(m_serialPort->portName())
                         << endl;
        char crc = crc8(m_readData);
        if (crc == 0) {
            m_readData.remove(m_readData.length() - 1, 1);
            m_readData.remove(0, 1);
            m_readData = bitsToBytes(bitsDeStuffing(
                                         bytesToBits(
                                             m_readData)));
            emit dataReady();
        } else {
            QMessageBox::critical(nullptr, "Sending error.",
                                  "CRC has detected an error. CRC result - "
                                  + QString::number(crc));
        }
    }
}

void SerialPortWorker::handleError(QSerialPort::SerialPortError error)
{
    QString errorTextTemplate;

    switch (error) {
    case QSerialPort::ReadError:
        errorTextTemplate = "An I/O error occurred while reading "
                            "the data from port %1, error: %2";
        break;
    case QSerialPort::WriteError:
        errorTextTemplate = "An I/O error occurred while writing"
                            " the data to port %1, error: %2";
        break;
    default:
        break;
    }

    QMessageBox::critical(nullptr, "Warning!",
                          QObject::tr(errorTextTemplate.toUtf8())
                          .arg(m_serialPort->portName())
                          .arg(m_serialPort->errorString()));

    QCoreApplication::exit(1);
}

void SerialPortWorker::handleCrashBit(int state)
{
    if (state == Qt::Checked) crashBitFlag = true;
    if (state == Qt::Unchecked) crashBitFlag = false;
}

void SerialPortWorker::write(const QByteArray &writeData)
{
    m_writeData = writeData;

    char crc = crc8(m_writeData);
    QByteArray testData(m_writeData + crc);
    qDebug() << "CRC: " << QString::number(crc);
    qDebug() << "Data with crc: " << testData;
    qDebug() << "Result: " << QString::number(crc8(testData));

    QBitArray bitData = bytesToBits(m_writeData);

#ifdef QT_DEBUG

    std::cout << "===============Source===============" << std::endl
              << "Source data:" << std::endl;
    log_bit_array(bitData);
    std::cout << "Source Byte data: " << m_writeData.constData() << std::endl
              << "Combination count: " << combinationCount(bitData) << std::endl
              << "====================================" << std::endl
              <<std::endl;
#endif // ifdef QT_DEBUG

    QBitArray bitDataStuff(bitsStuffing(bitData));

#ifdef QT_DEBUG

    std::cout << "===============Result===============" << std::endl
              << "Result data:" << std::endl;
    log_bit_array(bitDataStuff);
    std::cout << "Result Bytes data: "
              << bitsToBytes(bitDataStuff).constData() << std::endl
              << "Combination count: " << combinationCount(bitDataStuff)
              << std::endl
              << "====================================" << std::endl
              <<std::endl;
#endif // ifdef QT_DEBUG

    m_writeData = bitsToBytes(bitDataStuff);
    m_writeData.prepend('~');
    m_writeData.append(crc8(m_writeData));

    if (crashBitFlag) {
        char crashedByte = m_writeData[2] & 0xEF;
        m_writeData[2] = crashedByte;
    }

    const qint64 bytesWritten = m_serialPort->write(m_writeData.constData(),
                                                    m_writeData.size());

    if (bytesWritten == -1) {
        m_standardOutput << QObject::tr(
            "Failed to write the data to port %1, error: %2")
            .arg(m_serialPort->portName())
            .arg(m_serialPort->errorString())
                         << endl;
    } else if (bytesWritten != m_writeData.size()) {
        m_standardOutput << QObject::tr(
            "Failed to write Rall the data to port %1, error: %2")
            .arg(m_serialPort->portName())
            .arg(m_serialPort->errorString())
                         << endl;
    }
}

bool SerialPortWorker::changeBaudrate(const qint32 &value)
{
    if (m_serialPort) return m_serialPort->setBaudRate(value);
    return false;
}

QByteArray SerialPortWorker::bitsToBytes(const QBitArray &bits)
{
    QByteArray bytes;

    int bytesSize = bits.count() / 8;

    bytesSize += (bits.count() % 8 ? 1 : 0);
    bytes.resize(bytesSize);
    bytes.fill(0);

    // Convert from QBitArray to QByteArray
    for (int b = 0; b < bits.count(); ++b)
        bytes[b / 8] = (
            bytes.at(b / 8)
            |((bits[b] ? 1 : 0) << (7 - (b % 8)))
            );
    return bytes;
}

QBitArray SerialPortWorker::bytesToBits(const QByteArray &bytes)
{
    QBitArray bits;

    bits.resize(bytes.count() * 8);
    bits.fill(false);

    // Convert from QByteArray to QBitArray
    for (int i = 0; i < bytes.count(); ++i) {
        for (int b = 0; b < 8; b++)
            bits.setBit(i * 8 + b, bytes.at(i) & (1 << (7 - b)));
    }
    return bits;
}

int SerialPortWorker::combinationCount(const QBitArray &bits)
{
    int combination_count = 0;
    int one_count = 0;

    for (int i = 0; i < bits.size(); ++i) {
        bits.testBit(i) ? ++one_count : one_count = 0;

        if (one_count == 5) {
            one_count = 0;
            ++combination_count;
        }
    }

    return combination_count;
}

QBitArray SerialPortWorker::bitsStuffing(const QBitArray &bits)
{
    int combination_count = combinationCount(bits);
    QBitArray bitsStuff(bits.count() + combination_count,
                        false);
    int one_count = 0;

    for (int i = 0, j = 0; i < bits.size(); ++i, ++j) {
        bits.testBit(i) ? ++one_count : one_count = 0;

        bitsStuff.setBit(j,
                         bits.testBit(i));

        if (one_count == 5) {
            one_count = 0;
            ++j;
        }
    }

    return bitsStuff;
}

QBitArray SerialPortWorker::bitsDeStuffing(const QBitArray &bits)
{
    QBitArray bitsDeStuff(bits.count(),
                          false);

    int one_count = 0;

    for (int i = 0, j = 0; i < bits.size(); ++i, ++j) {
        bits.testBit(i) ? ++one_count : one_count = 0;

        bitsDeStuff.setBit(j,
                           bits.testBit(i));

        if ((one_count == 5)
            && (i + 1 < bits.count())
            && !bits.testBit(i + 1)) {
            one_count = 0;
            ++i;
        }
    }

    return bitsDeStuff;
}

char SerialPortWorker::crc8(const QByteArray &bytes)
{
    char crc = 0xFF;

    for (char ch
         : bytes) {
        crc ^= ch;

        for (int i = 0; i < 8; ++i)
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }

    return crc;
}

#ifdef QT_DEBUG
void SerialPortWorker::log_bit_array(QBitArray bits)
{
    for (int i = 0; i < bits.size(); ++i) {
        std::cout << (bits.testBit(i) ? '1' : '0');

        if ((i + 1) % 8 == 0) std::cout << ' ';
    }
    std::cout << std::endl;
}

#endif // ifdef QT_DEBUG
