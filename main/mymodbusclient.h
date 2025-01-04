#ifndef MYMODBUSCLIENT_H
#define MYMODBUSCLIENT_H

#include <QObject>
#include <QModbusRtuSerialMaster>
#include <QModbusReply>
#include <QModbusPdu>
#include <QMap>
#include <QString>

class MyModbusClient : public QObject
{
    Q_OBJECT
public:
    explicit MyModbusClient(QObject *parent = nullptr);

    void enableDebug() { m_enableDebug = true; }

    void setBaud(int baud) { m_baud = baud; }

    void setServerAddress(unsigned char address) { m_serverAddress = address; }

    void sendData(const QString& context, QModbusPdu::FunctionCode functionCode, const QByteArray& data);

private:
    void modbusConnect();

signals:
    void recvData(const QString& context, bool success, const QByteArray& data);

private slots:
    void onReadReady();

private:
    QModbusRtuSerialMaster m_modbusDevice;

    int m_baud = 19200;

    unsigned char m_serverAddress = 0xaa;

    QMap<QModbusReply *, QString> m_reply2Context;

    bool m_enableDebug = false;
};

#endif // MYMODBUSCLIENT_H
