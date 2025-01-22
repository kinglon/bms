#ifndef UPGRADECONTROLLER_H
#define UPGRADECONTROLLER_H

#include <QObject>
#include "publicdef.h"
#include "myprogressdialog.h"
#include "mymodbusclient.h"

class UpgradeController : public QObject
{
    Q_OBJECT
public:
    explicit UpgradeController(QObject *parent = nullptr);

    void run(QWidget* parent);

    // 处理返回true, 未处理返回false
    bool recvData(const QString& context, bool success, const QByteArray& data);

signals:
    void runFinish();

private:
    void doStartUpgrade();

    void doSendData();

    void doSendDataFinish();

    void doCancelUpgrade();

    void splitFileData();

public:
    MyModbusClient* m_modbusClient = nullptr;

    // 软件版本号
    unsigned char m_mainVersion = 0x00;
    unsigned char m_minorVersion = 0x00;

    // 软件包路径
    QString m_softFilePath;

    MyProgressDialog* m_progressDlg = nullptr;

    QTimer* m_progressTimer = nullptr;

    QVector<QByteArray> m_fileDatas;

    // 下一个发送的数据分片索引
    int m_nextSendIndex = 0;

    int m_retryCount = 0;
};

#endif // UPGRADECONTROLLER_H
