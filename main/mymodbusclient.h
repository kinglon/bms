﻿#ifndef MYMODBUSCLIENT_H
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

    void enableDebug();

    void setPortName(QString portName) { m_portName = portName; }

    void setBaud(int baud) { m_baud = baud; }

    void startConnect();

    void setServerAddress(unsigned char address) { m_serverAddress = address; }

    void sendData(const QString& context, QModbusPdu::FunctionCode functionCode, const QByteArray& data);

    int getSendCount() { return m_sendCount; }

    int getRecvCount() { return m_recvCount; }

    bool isConnected() { return m_modbusDevice.state() == QModbusClient::ConnectedState; }

private:
    void modbusConnect();

signals:    
    void recvData(const QString& context, bool success, const QByteArray& data);

private slots:
    void onReadReady();

private:
    QModbusRtuSerialMaster m_modbusDevice;

    QString m_portName;

    int m_baud = 19200;

    unsigned char m_serverAddress = 0xaa;

    QMap<QModbusReply *, QString> m_reply2Context;

    bool m_enableDebug = false;

    // 统计发送接收数据包个数
    int m_sendCount = 0;
    int m_recvCount = 0;
};

#endif // MYMODBUSCLIENT_H
