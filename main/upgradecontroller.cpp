#include "upgradecontroller.h"
#include <QTimer>
#include "crc32util.h"

// 最大重试次数
#define MAX_RETRY_COUNT 2

UpgradeController::UpgradeController(QObject *parent)
    : QObject{parent}
{

}

void UpgradeController::run(QWidget* parent)
{
    // 显示一个进度条，发送数据
    m_progressDlg = new MyProgressDialog(QString(), QString(), 0, 150, parent);
    m_progressDlg->setAttribute(Qt::WA_DeleteOnClose);
    m_progressDlg->setWindowTitle(QString::fromWCharArray(L"升级"));
    m_progressDlg->setLabelText(QString::fromWCharArray(L""));
    m_progressDlg->setValue(0);
    m_progressDlg->setCanClose();
    m_progressDlg->show();

    m_progressTimer = new QTimer(m_progressDlg);
    connect(m_progressTimer, &QTimer::timeout, [this]() {
        if (m_progressDlg == nullptr)
        {
            return;
        }

        int value = m_progressDlg->value();
        if (value >= m_progressDlg->maximum() - 2)
        {
            m_progressTimer->stop();
            // do nothing
        }
        else
        {
            m_progressDlg->setValue(value+1);
        }
    });
    m_progressTimer->start();

    connect(m_progressDlg, &MyProgressDialog::finished, [this](int result) {
        m_progressDlg = nullptr;
        m_progressTimer = nullptr;
        if (result != QDialog::Accepted)
        {
            doCancelUpgrade();
        }
        else
        {
            emit runFinish();
        }
    });

    doStartUpgrade();
}

bool UpgradeController::recvData(const QString& context, bool success, const QByteArray& data)
{
    if (context == CONTEXT_START_UPGRADE)
    {
        if (success)
        {
            splitFileData();
            if (m_fileDatas.size() > 0)
            {
                m_nextSendIndex = 0;
                m_retryCount = 0;
                doSendData();
            }
        }
        else
        {
            if (m_progressTimer)
            {
                m_progressTimer->stop();
            }

            if (m_progressDlg)
            {
                m_progressDlg->setLabelText(QString::fromWCharArray(L"下发启动升级命令失败"));
            }
        }

        return true;
    }
    else if (context == CONTEXT_SEND_UPGRADE_DATA)
    {
        if (success && data.length() >= 4)
        {
            if (data[3] == 0x01)
            {
                // 接收成功，发送下一个数据包
                m_nextSendIndex++;
                m_retryCount = 0;
                if (m_nextSendIndex >= m_fileDatas.size())
                {
                    doSendDataFinish();
                }
                else
                {
                    doSendData();
                }
            }
            else
            {
                // 接收失败，重试
                if (m_retryCount >= MAX_RETRY_COUNT)
                {
                    m_progressDlg->setLabelText(QString::fromWCharArray(L"发送数据失败"));
                }
                else
                {
                    m_retryCount++;
                    doSendData();
                }
            }
        }

        return true;
    }
    else if (context == CONTEXT_SEND_UPGRADE_DATA_FINISH)
    {
        if (success)
        {
            QTimer* timer = new QTimer();
            connect(timer, &QTimer::timeout, [this]() {
                doGetProgress();
            });
            timer->start(1000);
        }

        return true;
    }
    else if (context == CONTEXT_GET_UPGRADE_PROGRESS)
    {
        if (success && data.length() >= 4)
        {
            int progress = (data[2]<<8) + data[3];
            if (progress < 100)
            {
                m_progressDlg->setLabelText(QString::fromWCharArray(L"正在升级 ") + QString::number(progress) + "%");
            }
            else
            {
                m_progressDlg->accept();
                m_progressDlg->setLabelText(QString::fromWCharArray(L"升级成功"));
                m_progressTimer->stop();
            }
        }

        return true;
    }

    return false;
}

void UpgradeController::splitFileData()
{
    QFile file(m_softFilePath);
    bool result = file.open(QFile::ReadOnly);
    if (!result)
    {
        qCritical("failed to open file to split");
        return;
    }

    while(!file.atEnd())
    {
        QByteArray data = file.read(240);
        m_fileDatas.append(data);
    }

    file.close();
}

void UpgradeController::doStartUpgrade()
{
    if (m_progressDlg)
    {
        m_progressDlg->setLabelText(QString::fromWCharArray(L"正在下发启动升级命令"));
    }

    // 计算CRC32
    quint32 crc32 = 0;
    if (!Crc32Util::calcFileCRC(m_softFilePath, crc32))
    {
        return;
    }

    QByteArray datas;
    datas.append((char)0x9c);
    datas.append((char)0x57);
    datas.append((char)m_mainVersion);
    datas.append((char)m_minorVersion);
    datas.append((char)((crc32>>24)&0xff));
    datas.append((char)((crc32>>16)&0xff));
    datas.append((char)((crc32>>8)&0xff));
    datas.append((char)(crc32&0xff));
    m_modbusClient->sendData(CONTEXT_START_UPGRADE, QModbusPdu::WriteMultipleRegisters, datas);
}

void UpgradeController::doSendData()
{
    m_progressDlg->setLabelText(QString::fromWCharArray(L"发送数据 %1/%2").arg(
                                    QString::number(m_nextSendIndex+1, m_fileDatas.length())));

    QByteArray datas;
    datas.append((char)0xa0);
    datas.append((char)0x28);

    // 帧序号
    datas.append((char)(((m_nextSendIndex+1)>>8)&0xff));
    datas.append((char)((m_nextSendIndex+1)&0xff));

    // 长度
    datas.append((char)m_fileDatas[m_nextSendIndex].length());

    // 数据
    datas.append(m_fileDatas[m_nextSendIndex]);

    m_modbusClient->sendData(CONTEXT_SEND_UPGRADE_DATA, QModbusPdu::WriteFileRecord, datas);
}

void UpgradeController::doSendDataFinish()
{
    m_progressDlg->setLabelText(QString::fromWCharArray(L"发送数据完成"));

    QByteArray datas;
    datas.append((char)0x9c);
    datas.append((char)0x5a);
    datas.append((char)0x00);
    datas.append((char)0x01);
    m_modbusClient->sendData(CONTEXT_SEND_UPGRADE_DATA_FINISH, QModbusPdu::WriteSingleRegister, datas);
}

void UpgradeController::doGetProgress()
{
    QByteArray datas;
    datas.append((char)0x75);
    datas.append((char)0x4d);
    datas.append((char)0x00);
    datas.append((char)0x01);
    m_modbusClient->sendData(CONTEXT_GET_UPGRADE_PROGRESS, QModbusPdu::ReadInputRegisters, datas);
}

void UpgradeController::doCancelUpgrade()
{
    QByteArray datas;
    datas.append((char)0x9c);
    datas.append((char)0x5b);
    datas.append((char)0x00);
    datas.append((char)0x01);
    m_modbusClient->sendData(CONTEXT_CANCEL_UPGRADE, QModbusPdu::WriteSingleRegister, datas);

    emit runFinish();
}
