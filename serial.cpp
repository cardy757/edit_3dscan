#include "serial.h"
#include <QThread>

ArduinoSerial::ArduinoSerial()
{
    pSerialPort = NULL;
}

bool ArduinoSerial::connectToSerialPort()
{
    pSerialPort->setPortName(serialPortName);
    pSerialPort->setBaudRate(QSerialPort::Baud9600);
    pSerialPort->setDataBits(QSerialPort::Data8);
    pSerialPort->setParity(QSerialPort::NoParity);
    pSerialPort->setStopBits(QSerialPort::OneStop);
    pSerialPort->setFlowControl(QSerialPort::NoFlowControl);
    if (pSerialPort->open(QIODevice::ReadWrite))
    {
    }
    else
    {
        return false;
    }
    return true;
}

void ArduinoSerial::writeChar(char c)
{
    Q_ASSERT(pSerialPort->isOpen());
    if (pSerialPort->isOpen())
    {
        pSerialPort->write(&c, 1);
        pSerialPort->waitForBytesWritten(1000);
    }
}

void ArduinoSerial::SetPortName(QString name)
{
#ifdef WIN32
    serialPortName = name;
#else
    serialPortName = "/dev/cu." + name;
#endif
}

bool ArduinoSerial::start()
{
    Q_ASSERT(pSerialPort == NULL);
    if (pSerialPort == NULL)
    {
        pSerialPort = new QSerialPort();
        if (connectToSerialPort())
        {
            connect(pSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this,
                    SLOT(handleError(QSerialPort::SerialPortError)));
            connect(pSerialPort, SIGNAL(readyRead()), this, SLOT(readData()));
            return true;
        }
        else
        {
            stop();
        }
    }
    return false;
}

void ArduinoSerial::stop()
{
    Q_ASSERT(pSerialPort != NULL);
    if (pSerialPort != NULL)
    {
        pSerialPort->close();
        delete pSerialPort;
        pSerialPort = NULL;
    }
}

void ArduinoSerial::readData()
{
    QByteArray data = pSerialPort->readAll();
}

void ArduinoSerial::handleError(QSerialPort::SerialPortError error)
{

}
