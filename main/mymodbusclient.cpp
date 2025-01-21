#include "mymodbusclient.h"
#include <QVariant>
#include <QTimer>
#include <QLoggingCategory>

MyModbusClient::MyModbusClient(QObject *parent)
    : QObject{parent}
{
    connect(&m_modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        qCritical("modbus error: %s", m_modbusDevice.errorString().toStdString().c_str());

        // 设备断电重启，导致无法正常通信，断开连接重连
        if (m_modbusDevice.errorString().indexOf("Cannot open serial device due to permissions") >= 0)
        {
            if (m_modbusDevice.state() == QModbusDevice::ConnectedState)
            {
                qInfo("disconnect device for reconnecting");
                m_modbusDevice.disconnectDevice();
            }
        }
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

        // 连接之前，如果有正在进行的请求，先结束掉
        if (m_currentReply)
        {
            emit recvData(m_currentContext, false, QByteArray());
            m_currentReply->deleteLater();
            m_currentReply = nullptr;
            m_currentContext = "";
        }

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

    for (auto it=m_requestToSend.begin(); it!=m_requestToSend.end(); it++)
    {
        if (it->m_context == context)
        {
            qDebug("the context(%s) is sending data", context.toStdString().c_str());
            return;
        }
    }

    if (m_modbusDevice.state() != QModbusDevice::ConnectedState)
    {
        qCritical("failed to send data, the modbus is not connected");
        emit recvData(context, false, QByteArray());
        return;
    }

    RequestItem requestItem;
    requestItem.m_context = context;
    requestItem.m_functionCode = functionCode;
    requestItem.m_data = data;
    m_requestToSend.push_back(requestItem);

    sendDataInternal();
}

void MyModbusClient::sendDataInternal()
{
    if (m_modbusDevice.state() != QModbusDevice::ConnectedState)
    {
        return;
    }

    if (m_requestToSend.empty())
    {
        return;
    }

    // 有正在发送还未响应
    if (m_currentReply)
    {
        return;
    }

    if (m_requestToSend.size() >= 30)
    {
        qInfo("the size of request queue is %d", m_requestToSend.size());
    }

    RequestItem requestItem = m_requestToSend.front();
    m_requestToSend.pop_front();

    if (m_enableDebug)
    {
        qDebug("send data, function code: %d, data: %s", (int)requestItem.m_functionCode, requestItem.m_data.toHex().toStdString().c_str());
    }

    QModbusRequest request(requestItem.m_functionCode, requestItem.m_data);
    if (auto *reply = m_modbusDevice.sendRawRequest(request, m_serverAddress))
    {
        m_sendCount++;

        if (!reply->isFinished())
        {
            m_currentContext = requestItem.m_context;
            m_currentReply = reply;
            connect(reply, &QModbusReply::finished, this, &MyModbusClient::onReadReady);
            connect(reply, &QModbusReply::errorOccurred, this, &MyModbusClient::onReadError);
        }
        else
        {
            delete reply; // broadcast replies return immediately
            emit recvData(requestItem.m_context, true, QByteArray());
        }
    }
    else
    {
        qCritical("failed to send data, %s", m_modbusDevice.errorString().toStdString().c_str());
        emit recvData(requestItem.m_context, false, QByteArray());
    }
}

void MyModbusClient::readCompletely()
{
    if (m_currentReply == nullptr)
    {
        qCritical("current reply is empty");
        return;
    }

    if (m_currentReply->error() == QModbusDevice::NoError)
    {
        m_recvCount++;

        QModbusResponse response = m_currentReply->rawResult();
        if (m_enableDebug)
        {
            qDebug("recv data, function code: %d, data: %s", (int)response.functionCode(), response.data().toHex().toStdString().c_str());
        }
        emit recvData(m_currentContext, true, response.data());
    }
    else if (m_currentReply->error() == QModbusDevice::ProtocolError)
    {
        qCritical("Read response error: %s (Mobus exception: %d)",
                                    m_currentReply->errorString().toStdString().c_str(),
                                    m_currentReply->rawResult().exceptionCode());
        emit recvData(m_currentContext, false, QByteArray());
    }
    else
    {
        qCritical("Read response error: %s (code: %d)",
                                    m_currentReply->errorString().toStdString().c_str(),
                                    m_currentReply->error());
        emit recvData(m_currentContext, false, QByteArray());
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    m_currentContext = "";

    sendDataInternal();
}

void MyModbusClient::onReadReady()
{
    readCompletely();
}

void MyModbusClient::onReadError(QModbusDevice::Error)
{
    readCompletely();
}
