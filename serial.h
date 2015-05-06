#ifndef ARDUINOSERIAL_H
#define ARDUINOSERIAL_H

#include <QObject>
#include <QSerialPort>

class ArduinoSerial : public QObject
{

Q_OBJECT
public:
    ArduinoSerial();

    void writeChar(char c);
    void writeChars(char* c);
    void SetPortName(QString name);
    bool start();
    void stop();

private slots:
    void readData();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *pSerialPort;
    QString serialPortName;

    bool connectToSerialPort();

};

#endif // ARDUINOSERIAL_H
