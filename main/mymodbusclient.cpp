#include "mymodbusclient.h"
#include <QVariant>
#include <QTimer>
#include <QLoggingCategory>

MyModbusClient::MyModbusClient(QObject *parent)
    : QObject{parent}
{
    connect(&m_modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        qCritical("modbus error: %s", m_modbusDevice.errorString().toStdString().c_str());

    });

    connect(&m_modbusDevice, &QModbusClient::stateChanged, [](int state) {
        if (state == QModbusDevice::ConnectedState)
        {
            qInfo("modbus connected");
        }
        else if (state == QModbusDevice::UnconnectedState)
        {
            qInfo("modbus disconnected");
        }
    });    
}

void MyModbusClient::enableDebug()
{
    m_enableDebug = true;
    QLoggingCategory::setFilterRules(QStringLiteral("qt.modbus* = true"));

}

void MyModbusClient::startConnect()
{
    static bool first = true;
    if (!first)
    {
        return;
    }
    first = false;

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() {
        modbusConnect();
    });
    timer->start(3000);
    modbusConnect();
}

void MyModbusClient::modbusConnect()
{
    if (m_modbusDevice.state() != QModbusDevice::ConnectedState && !m_portName.isEmpty())
    {        
        qInfo("begin to connect");
        m_modbusDevice.setConnectionParameter(QModbusDevice::SerialPortNameParameter, m_portName);
        m_modbusDevice.setConnectionParameter(QModbusDevice::SerialParityParameter, 0);
        m_modbusDevice.setConnectionParameter(QModbusDevice::SerialBaudRateParameter, m_baud);
        m_modbusDevice.setConnectionParameter(QModbusDevice::SerialDataBitsParameter, 8);
        m_modbusDevice.setConnectionParameter(QModbusDevice::SerialStopBitsParameter, 1);
        m_modbusDevice.setTimeout(1000);
        m_modbusDevice.setNumberOfRetries(3);
        if (!m_modbusDevice.connectDevice())
        {
            qCritical("failed to connect, error: %s", m_modbusDevice.errorString().toStdString().c_str());
        }
    }
}

void MyModbusClient::sendData(const QString& context, QModbusPdu::FunctionCode functionCode, const QByteArray& data)
{
    if (context.isEmpty())
    {
        qCritical("context can not empty");
        return;
    }

    for (auto it=m_reply2Context.begin(); it!=m_reply2Context.end(); it++)
    {
        if (it.value() == context)
        {
            qCritical("the context(%s) is sending data", context.toStdString().c_str());
            return;
        }
    }

    if (m_modbusDevice.state() != QModbusDevice::ConnectedState)
    {
        qCritical("failed to send data, the modbus is not connected");
        emit recvData(context, false, QByteArray());
        return;
    }

    if (m_enableDebug)
    {
        qDebug("send data, function code: %d, data: %s", (int)functionCode, data.toHex().toStdString().c_str());
    }

    QModbusRequest request(functionCode, data);
    if (auto *reply = m_modbusDevice.sendRawRequest(request, m_serverAddress))
    {
        m_sendCount++;

        if (!reply->isFinished())
        {
            m_reply2Context[reply] = context;
            connect(reply, &QModbusReply::finished, this, &MyModbusClient::onReadReady);
        }
        else
        {
            delete reply; // broadcast replies return immediately
            emit recvData(context, true, QByteArray());
        }
    }
    else
    {
        qCritical("failed to send data, %s", m_modbusDevice.errorString().toStdString().c_str());
        emit recvData(context, false, QByteArray());
    }
}

void MyModbusClient::onReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
    {
        return;
    }

    QString context;
    auto it = m_reply2Context.find(reply);
    if (it == m_reply2Context.end())
    {
        reply->deleteLater();
        qCritical("failed to find context");
        return;
    }
    context = it.value();
    m_reply2Context.erase(it);

    if (reply->error() == QModbusDevice::NoError)
    {
        m_recvCount++;

        QModbusResponse response = reply->rawResult();
        if (m_enableDebug)
        {
            qDebug("recv data, function code: %d, data: %s", (int)response.functionCode(), response.data().toHex().toStdString().c_str());
        }
        emit recvData(context, true, response.data());
    }
    else if (reply->error() == QModbusDevice::ProtocolError)
    {
        qCritical("Read response error: %s (Mobus exception: %d)",
                                    reply->errorString().toStdString().c_str(),
                                    reply->rawResult().exceptionCode());
        emit recvData(context, false, QByteArray());
    }
    else
    {
        qCritical("Read response error: %s (code: %d)",
                                    reply->errorString().toStdString().c_str(),
                                    reply->error());
        emit recvData(context, false, QByteArray());
    }

    reply->deleteLater();
}
