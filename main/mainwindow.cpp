#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datamodel.h"
#include "datamanager.h"
#include "uiutil.h"
#include <QTimer>
#include "settingmanager.h"
#include "Utility/LogUtil.h"
#include <QDateTime>
#include <QFileDialog>
#include "publicdef.h"
#include <QCryptographicHash>
#include <QtSerialPort/QSerialPortInfo>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initCtrls();

    updateCtrlDatas();

    m_ctrlDShortcut = new QShortcut(QKeySequence("Ctrl+D"), this);
        connect(m_ctrlDShortcut, &QShortcut::activated, this, &MainWindow::onCtrlDShortcut);

    connect(&m_modbusClient, &MyModbusClient::recvData, this, &MainWindow::onRecvData);

    // 先展示界面再连接
    QTimer::singleShot(100, [this]() {
        if (SettingManager::getInstance()->m_logLevel == (int)ELogLevel::LOG_LEVEL_DEBUG)
        {
            m_modbusClient.enableDebug();
        }
        m_modbusClient.setPortName(SettingManager::getInstance()->m_serialPortName);
        m_modbusClient.setBaud(SettingManager::getInstance()->m_baud);
        m_modbusClient.startConnect();

        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow::onMainTimer);
        timer->start(1000);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initCtrls()
{
    ui->tabWidget->setCurrentIndex(0);

    m_batteryWidget = new BatteryWidget(ui->groupBox);
    m_batteryWidget->setGeometry(ui->batteryVolumnWidget->geometry());
    ui->batteryVolumnWidget->setVisible(false);

    ui->batteryTypeComboBox->addItem(QString::fromWCharArray(L"三元锂电池"), BATTERY_TYPE_SANYUAN);
    ui->batteryTypeComboBox->addItem(QString::fromWCharArray(L"磷酸铁锂电池"), BATTERY_TYPE_LINSUANTAI);
    ui->batteryTypeComboBox->addItem(QString::fromWCharArray(L"锰酸锂电池"), BATTERY_TYPE_MENGSUAN);
    ui->batteryTypeComboBox->addItem(QString::fromWCharArray(L"其它"), BATTERY_TYPE_OTHER);

    ui->botelvComboBox->addItem(QString::fromWCharArray(L"2400"));
    ui->botelvComboBox->addItem(QString::fromWCharArray(L"9600"));
    ui->botelvComboBox->addItem(QString::fromWCharArray(L"19200"));
    ui->botelvComboBox->addItem(QString::fromWCharArray(L"115200"));
    ui->botelvComboBox->setCurrentText(QString::number(SettingManager::getInstance()->m_baud));

    initPortCtrl();

    // 通过参数名称查找控件，并设置值
    for (const auto& paramItem : DataManager::getInstance()->m_params)
    {
        if (!paramItem.m_localSave)
        {
            continue;
        }

        QList<QLabel *> labels = findChildren<QLabel *>(paramItem.m_name+"Label");
        if (labels.size() > 0)
        {
            labels[0]->setText(paramItem.getValueString());
            continue;
        }

        QList<QLineEdit *> edits = findChildren<QLineEdit *>(paramItem.m_name+"Edit");
        if (edits.size() > 0)
        {
            edits[0]->setText(paramItem.getValueString());
            continue;
        }
    }

    // 电池类型
    int value = DataManager::getInstance()->getParamByName(PARAM_NAME_BATTERY_TYPE)->m_value;
    ui->batteryTypeComboBox->setCurrentIndex(value-BATTERY_TYPE_MIN);

    connect(ui->openChargeMosButton, &QPushButton::clicked, [this]() {
        onChargeMosButtonClicked(true);
    });
    connect(ui->closeChargeMosButton, &QPushButton::clicked, [this]() {
        onChargeMosButtonClicked(false);
    });
    connect(ui->openFangdianMosButton, &QPushButton::clicked, [this]() {
        onFangdianMosButtonClicked(true);
    });
    connect(ui->closeFangdianMosButton, &QPushButton::clicked, [this]() {
        onFangdianMosButtonClicked(false);
    });
    connect(ui->openJunhengButton, &QPushButton::clicked, [this]() {
        onJunhengButtonClicked(true);
    });
    connect(ui->closeJunhengButton, &QPushButton::clicked, [this]() {
        onJunhengButtonClicked(false);
    });
    connect(ui->selectAllButton, &QPushButton::clicked, this, &MainWindow::onSelectAllButtonClicked);
    connect(ui->unselectAllButton, &QPushButton::clicked, this, &MainWindow::onUnSelectAllButtonClicked);
    connect(ui->writeParamButton, &QPushButton::clicked, this, &MainWindow::onWriteParamButtonClicked);
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(ui->selectSoftwarePathButton, &QPushButton::clicked, this, &MainWindow::onSelectSoftwarePathButtonClicked);
    connect(ui->readSoftVersionButton, &QPushButton::clicked, this, &MainWindow::onReadSoftwareVersionButtonClicked);
    connect(ui->upgradeButton, &QPushButton::clicked, this, &MainWindow::onUpgradeButtonClicked);
    connect(ui->modifyPasswordButton, &QPushButton::clicked, this, &MainWindow::onModifyPasswordButtonClicked);
    connect(ui->refreshPortButton, &QPushButton::clicked, [this]() {
        initPortCtrl();
    });
}

void MainWindow::initPortCtrl()
{
    QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();
    ui->portComboBox->clear();
    for (const auto& serialPort : serialPorts)
    {
        ui->portComboBox->addItem(serialPort.portName());
    }
    ui->portComboBox->setCurrentText(SettingManager::getInstance()->m_serialPortName);
}

void MainWindow::updateCtrlDatas()
{
    // 通过参数名称查找控件，并设置值
    for (const auto& paramItem : DataManager::getInstance()->m_params)
    {
        if (paramItem.m_localSave)
        {
            // 写入参数初始化时更新就可以
            continue;
        }

        QList<QLabel *> labels = findChildren<QLabel *>(paramItem.m_name+"Label");
        if (labels.size() > 0)
        {
            labels[0]->setText(paramItem.getValueString());
            continue;
        }

        QList<QLineEdit *> edits = findChildren<QLineEdit *>(paramItem.m_name+"Edit");
        if (edits.size() > 0)
        {
            edits[0]->setText(paramItem.getValueString());
            continue;
        }
    }

    // 非数值类的参数重新设置
    int value = DataManager::getInstance()->getParamByName(PARAM_NAME_BATTERY_STATUS)->m_value;
    ui->batteryStatusLabel->setText(getStringFromStatus(value));    

    value = DataManager::getInstance()->getParamByName(PARAM_NAME_JUNHENG_STATUS)->m_value;
    ui->junfengStatusLabel->setText(getStringFromStatus(value));

    value = DataManager::getInstance()->getParamByName(PARAM_NAME_CHARGE_MOS_SWITCH)->m_value;
    ui->chargeMosSwitchLabel->setText(getStringFromSwitch(value));

    value = DataManager::getInstance()->getParamByName(PARAM_NAME_FANGDIAN_MOS_SWITCH)->m_value;
    ui->fangdianMosSwitchLabel->setText(getStringFromSwitch(value));

    // 更新电池电量
    if (m_batteryWidget)
    {
        value = DataManager::getInstance()->getParamByName(PARAM_NAME_SOC)->m_value;
        m_batteryWidget->setBatteryValue(value);
    }

    // 更新保护/警告信息
    updateProtectWarningInfo();

    // 更新温度信息
    updateTemperatureInfo();
}

void MainWindow::updateProtectWarningInfo()
{
    // 保护信息
    bool qianYa = false; // 欠压
    bool guoYa = false; // 过压
    for (int i=1; i<=BATTERY_COUNT; i++)
    {
        int value = DataManager::getInstance()->getParamByName(QString(PARAM_NAME_BATTERY_DIANYA_PREFIX)+QString::number(i))->m_value;
        if (value > (int)(4.2f*FLOAT_SCALE_FACTOR))
        {
            guoYa = true;
        }
        else if (value < (int)(2.75f*FLOAT_SCALE_FACTOR))
        {
            qianYa = true;
        }
    }
    QString protectInfo;
    if (qianYa)
    {
        protectInfo = QString::fromWCharArray(L"单体欠压保护");
    }
    if (guoYa)
    {
        if (!protectInfo.isEmpty())
        {
            protectInfo += "\n";
        }
        protectInfo += QString::fromWCharArray(L"单体过压保护");
    }
    ui->protectInfoLabel->setText(protectInfo);

    // 警告信息
    QString warningInfo;
    int value = DataManager::getInstance()->getParamByName(PARAM_NAME_CHARGE_MOS_STATUS)->m_value;
    if (value == STATUS_EXCEPTION)
    {
        warningInfo = QString::fromWCharArray(L"充电MOS异常");
    }

    value = DataManager::getInstance()->getParamByName(PARAM_NAME_FANGDIAN_MOS_STATUS)->m_value;
    if (value == STATUS_EXCEPTION)
    {
        if (!warningInfo.isEmpty())
        {
            warningInfo += "\n";
        }
        warningInfo += QString::fromWCharArray(L"放电MOS异常");
    }
    ui->warningInfoLabel->setText(warningInfo);
}

void MainWindow::updateTemperatureInfo()
{
    int mosT = DataManager::getInstance()->getParamByName(PARAM_NAME_MOS_TEMPERATURE)->m_value;
    int junhengT = DataManager::getInstance()->getParamByName(PARAM_NAME_JUNHENG_TEMPERATURE)->m_value;
    int t1 = DataManager::getInstance()->getParamByName(PARAM_NAME_T1_TEMPERATURE)->m_value;
    int t2 = DataManager::getInstance()->getParamByName(PARAM_NAME_T2_TEMPERATURE)->m_value;
    ui->mosTempLabel->setText(QString::number(mosT/100.0f, 'f', 2)+QString::fromWCharArray(L"℃"));
    ui->junhengTempLabel->setText(QString::number(junhengT/100.0f, 'f', 2)+QString::fromWCharArray(L"℃"));
    ui->t1TempLabel->setText(QString::number(t1/100.0f, 'f', 2)+QString::fromWCharArray(L"℃"));
    ui->t2TempLabel->setText(QString::number(t2/100.0f, 'f', 2)+QString::fromWCharArray(L"℃"));
}

void MainWindow::onCtrlDShortcut()
{
//    QByteArray data = QByteArray::fromHex("300032000062300000271010680a2803e80fa00fa00fa00fa00fa00fa00fa00fa00fa00fa00fa00fa00fa00fa00fa00fa0");
//    onRecvData(CONTEXT_READ_DIANYA_DIANLIU_DATA, true, data);
}

void MainWindow::onMainTimer()
{
    // 更新与设备的连接信息
    QString connectStatus;
    if (m_modbusClient.isConnected())
    {
        connectStatus = QString::fromWCharArray(L"已连接");
    }
    else
    {
        connectStatus = QString::fromWCharArray(L"未连接");
    }
    QString connectInfo = QString::fromWCharArray(L"连接状态：%1，发送数据包：%2，接收数据包：%3").arg(
                connectStatus, QString::number(m_modbusClient.getSendCount()), QString::number(m_modbusClient.getRecvCount()));
    ui->connectInfoLabel->setText(connectInfo);

    if (m_modbusClient.isConnected())
    {
        // 读取充电MOS、放电MOS、均衡开关的状态
        QByteArray datas;
        datas.append((char)0x00);
        datas.append((char)0x01);
        datas.append((char)0x00);
        datas.append((char)0x03);
        m_modbusClient.sendData(CONTEXT_READ_SWITCH_STATUS, QModbusPdu::ReadCoils, datas);

        // 读取电池、充电MOS、放电MOS、均衡的状态
        datas.clear();
        datas.append((char)0x10);
        datas.append((char)0x01);
        datas.append((char)0x00);
        datas.append((char)0x04);
        m_modbusClient.sendData(CONTEXT_READ_BATTERY_JUNHENG_STATUS, QModbusPdu::ReadDiscreteInputs, datas);

        // 读取电压电流数据
        datas.clear();
        datas.append((char)0x75);
        datas.append((char)0x31);
        datas.append((char)0x00);
        datas.append((char)0x18);
        m_modbusClient.sendData(CONTEXT_READ_DIANYA_DIANLIU_DATA, QModbusPdu::ReadInputRegisters, datas);

        // 更新温度
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - m_lastTemperatureUpdateTime >= 5000)
        {
            datas.clear();
            datas.append((char)0x75);
            datas.append((char)0x47);
            datas.append((char)0x00);
            datas.append((char)0x04);
            m_modbusClient.sendData(CONTEXT_READ_TEMPERATURE_DATA, QModbusPdu::ReadInputRegisters, datas);
            m_lastTemperatureUpdateTime = now;
        }
    }
}

void MainWindow::onRecvData(const QString& context, bool success, const QByteArray& data)
{
    if (m_upgradeController)
    {
        if (m_upgradeController->recvData(context, success, data))
        {
            return;
        }
    }

    if (context == CONTEXT_READ_SWITCH_STATUS)
    {
        // 读取充电MOS、放电MOS、均衡开关的状态
        if (success && data.length() >= 2)
        {
            bool chargeMosEnable = (data[1] & 0x04);
            bool fangdianMosEnable = (data[1] & 0x02);
            bool junhengEnable = (data[1] & 0x01);
            DataManager::getInstance()->setParamValue(PARAM_NAME_CHARGE_MOS_SWITCH, chargeMosEnable?SWITCH_OPEN:SWITCH_CLOSE);
            DataManager::getInstance()->setParamValue(PARAM_NAME_FANGDIAN_MOS_SWITCH, fangdianMosEnable?SWITCH_OPEN:SWITCH_CLOSE);
            DataManager::getInstance()->setParamValue(PARAM_NAME_JUNHENG_SWITCH, junhengEnable?SWITCH_OPEN:SWITCH_CLOSE);
            updateCtrlDatas();
        }
    }
    else if (context == CONTEXT_WRITE_CHARGE_MOS_SWITCH)
    {
        // 设置充电MOS开关
        if (success && data.length() >= 4)
        {
            bool chargeMosEnable = data[3] != 0x00;
            DataManager::getInstance()->setParamValue(PARAM_NAME_CHARGE_MOS_SWITCH, chargeMosEnable?SWITCH_OPEN:SWITCH_CLOSE);
            updateCtrlDatas();
        }
    }
    else if (context == CONTEXT_WRITE_FANGDIAN_MOS_SWITCH)
    {
        // 设置放电MOS开关
        if (success && data.length() >= 4)
        {
            bool fangdianMosEnable = data[3] != 0x00;
            DataManager::getInstance()->setParamValue(PARAM_NAME_FANGDIAN_MOS_SWITCH, fangdianMosEnable?SWITCH_OPEN:SWITCH_CLOSE);
            updateCtrlDatas();
        }
    }
    else if (context == CONTEXT_WRITE_JUNHENG_SWITCH)
    {
        // 设置均衡开关
        if (success && data.length() >= 4)
        {
            bool junhengEnable = data[3] != 0x00;
            DataManager::getInstance()->setParamValue(PARAM_NAME_JUNHENG_SWITCH, junhengEnable?SWITCH_OPEN:SWITCH_CLOSE);
            updateCtrlDatas();
        }
    }
    else if (context == CONTEXT_READ_BATTERY_JUNHENG_STATUS)
    {
        // 读取电池、均衡、充电MOS、放电MOS的状态
        if (success && data.length() >= 2)
        {
            bool batteryException = (data[1]&0x01) == 0;
            bool junhengException = (data[1]&0x02) == 0;
            bool fangdianMosException = (data[1]&0x04) == 0;
            bool chargeMosException = (data[1]&0x10) == 0;
            DataManager::getInstance()->setParamValue(PARAM_NAME_BATTERY_STATUS, batteryException?STATUS_EXCEPTION:STATUS_GOOD);
            DataManager::getInstance()->setParamValue(PARAM_NAME_JUNHENG_STATUS, junhengException?STATUS_EXCEPTION:STATUS_GOOD);
            DataManager::getInstance()->setParamValue(PARAM_NAME_CHARGE_MOS_STATUS, chargeMosException?STATUS_EXCEPTION:STATUS_GOOD);
            DataManager::getInstance()->setParamValue(PARAM_NAME_FANGDIAN_MOS_STATUS, fangdianMosException?STATUS_EXCEPTION:STATUS_GOOD);
            updateCtrlDatas();
        }
    }
    else if (context == CONTEXT_READ_DIANYA_DIANLIU_DATA)
    {
        // 读取电压电流数据
        if (success && data.length() >= 49)
        {
            int pos = 1;
            int soc = MAKE_INT(data[pos], data[pos+1]);
            pos += 2;
            int totalDianYa = MAKE_INT_4(data[pos], data[pos+1], data[pos+2], data[pos+3]);
            pos += 4;
            int dianLiu = MAKE_INT_4(data[pos], data[pos+1], data[pos+2], data[pos+3]);
            pos += 4;
            int maxDianYa = MAKE_INT(data[pos], data[pos+1]);
            pos += 2;
            int minDianYa = MAKE_INT(data[pos], data[pos+1]);
            pos += 2;
            int totalLoop = MAKE_INT(data[pos], data[pos+1]);
            pos += 2;
            DataManager::getInstance()->setParamValue(PARAM_NAME_SOC, soc);
            DataManager::getInstance()->setParamValue(PARAM_NAME_BATTERY_TOTAL_DIANYA, totalDianYa);
            DataManager::getInstance()->setParamValue(PARAM_NAME_BATTERY_DIANLIU, dianLiu);
            DataManager::getInstance()->setParamValue(PARAM_NAME_MAX_DIANYA, maxDianYa);
            DataManager::getInstance()->setParamValue(PARAM_NAME_MIN_DIANYA, minDianYa);
            DataManager::getInstance()->setParamValue(PARAM_NAME_TOTAL_LOOP, totalLoop);

            for (int i=1; i<=BATTERY_COUNT; i++)
            {
                int batteryDianYa = MAKE_INT(data[pos], data[pos+1]);
                pos += 2;
                DataManager::getInstance()->setParamValue(QString(PARAM_NAME_BATTERY_DIANYA_PREFIX)+QString::number(i),
                                                          batteryDianYa);
            }
            updateCtrlDatas();
        }
    }
    else if (context == CONTEXT_READ_TEMPERATURE_DATA)
    {
        // 读取温度数据
        if (success && data.length() >= 9)
        {
            // 每个温度2个字节，最高位15位为1表示负数，0表示整数
            int pos = 1;
            int sign = (data[pos] & 0x80) ? -1 : 1;
            int mosT = MAKE_INT((data[pos]&0x7f), data[pos+1]) * sign;
            pos += 2;
            sign = (data[pos] & 0x80) ? -1 : 1;
            int junhengT = MAKE_INT((data[pos]&0x7f), data[pos+1]) * sign;
            pos += 2;
            sign = (data[pos] & 0x80) ? -1 : 1;
            int t1 = MAKE_INT((data[pos]&0x7f), data[pos+1]) * sign;
            pos += 2;
            sign = (data[pos] & 0x80) ? -1 : 1;
            int t2 = MAKE_INT((data[pos]&0x7f), data[pos+1]) * sign;
            DataManager::getInstance()->setParamValue(PARAM_NAME_MOS_TEMPERATURE, mosT);
            DataManager::getInstance()->setParamValue(PARAM_NAME_JUNHENG_TEMPERATURE, junhengT);
            DataManager::getInstance()->setParamValue(PARAM_NAME_T1_TEMPERATURE, t1);
            DataManager::getInstance()->setParamValue(PARAM_NAME_T2_TEMPERATURE, t2);
            updateCtrlDatas();
        }
    }
    else if (context == CONTEXT_WRITE_PARAM_DATA)
    {
       if (success)
       {
           if (m_progressDlg)
           {
               m_progressDlg->setSuccess();
           }
       }
    }
    else if (context == CONTEXT_READ_SOFTWARE_VERSION)
    {
        // 读取软件版本信息
        if (success && data.length() >= 5)
        {
            QString hardwareVersion = QString("%1.%2").arg(QString::number((int)data[1]), QString::number((int)data[2]));
            QString softwareVersion = QString("%1.%2").arg(QString::number((int)data[3]), QString::number((int)data[4]));
            ui->softVersionEdit->setText(softwareVersion);
            ui->hardwareVersionEdit->setText(hardwareVersion);
        }
    }
}

void MainWindow::onChargeMosButtonClicked(bool open)
{
    if (!m_modbusClient.isConnected())
    {
        UiUtil::showTip(QString::fromWCharArray(L"未连接"));
        return;
    }

    // 设置充电MOS开关的状态
    QByteArray datas;
    datas.append((char)0x00);
    datas.append((char)0x01);
    datas.append((char)0x00);
    if (open)
    {
        datas.append((char)0x01);
    }
    else
    {
        datas.append((char)0x00);
    }
    m_modbusClient.sendData(CONTEXT_WRITE_CHARGE_MOS_SWITCH, QModbusPdu::WriteSingleCoil, datas);
}

void MainWindow::onFangdianMosButtonClicked(bool open)
{
    if (!m_modbusClient.isConnected())
    {
        UiUtil::showTip(QString::fromWCharArray(L"未连接"));
        return;
    }

    // 设置放电MOS开关的状态
    QByteArray datas;
    datas.append((char)0x00);
    datas.append((char)0x02);
    datas.append((char)0x00);
    if (open)
    {
        datas.append((char)0x01);
    }
    else
    {
        datas.append((char)0x00);
    }
    m_modbusClient.sendData(CONTEXT_WRITE_FANGDIAN_MOS_SWITCH, QModbusPdu::WriteSingleCoil, datas);
}

void MainWindow::onJunhengButtonClicked(bool open)
{
    if (!m_modbusClient.isConnected())
    {
        UiUtil::showTip(QString::fromWCharArray(L"未连接"));
        return;
    }

    // 设置均衡开关的状态
    QByteArray datas;
    datas.append((char)0x00);
    datas.append((char)0x03);
    datas.append((char)0x00);
    if (open)
    {
        datas.append((char)0x01);
    }
    else
    {
        datas.append((char)0x00);
    }
    m_modbusClient.sendData(CONTEXT_WRITE_JUNHENG_SWITCH, QModbusPdu::WriteSingleCoil, datas);
}

void MainWindow::onSelectAllButtonClicked()
{
    for (const auto& param : DataManager::getInstance()->m_params)
    {
        if (!param.m_localSave)
        {
            continue;
        }

        QList<QCheckBox *> checkBoxes = findChildren<QCheckBox *>(param.m_name+"CheckBox");
        if (checkBoxes.size() > 0)
        {
            checkBoxes[0]->setChecked(true);
        }
    }
}

void MainWindow::onUnSelectAllButtonClicked()
{
    for (const auto& param : DataManager::getInstance()->m_params)
    {
        if (!param.m_localSave)
        {
            continue;
        }

        QList<QCheckBox *> checkBoxes = findChildren<QCheckBox *>(param.m_name+"CheckBox");
        if (checkBoxes.size() > 0)
        {
            checkBoxes[0]->setChecked(false);
        }
    }
}

void MainWindow::onWriteParamButtonClicked()
{
    if (!m_modbusClient.isConnected())
    {
        UiUtil::showTip(QString::fromWCharArray(L"未连接"));
        return;
    }

    // 获取勾选需要设置的参数
    QVector<QString> writeParamNames;
    QVector<QString> writeParamTitles;
    for (const auto& param : DataManager::getInstance()->m_params)
    {
        if (!param.m_localSave)
        {
            continue;
        }

        QList<QCheckBox *> checkBoxes = findChildren<QCheckBox *>(param.m_name+"CheckBox");
        if (checkBoxes.size() > 0)
        {
            if (checkBoxes[0]->isChecked())
            {
                writeParamNames.append(param.m_name);
                writeParamTitles.append(checkBoxes[0]->text());
                continue;
            }
        }
    }

    if (writeParamNames.empty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"至少选择一个参数"));
        return;
    }

    // 校验参数
    QVector<QString> writeParamValues;
    for (int i=0; i<writeParamNames.size(); i++)
    {
        QList<QLineEdit *> edits = findChildren<QLineEdit *>(writeParamNames[i]+"Edit");
        if (edits.size() > 0)
        {
            ParamItem* param = DataManager::getInstance()->getParamByName(writeParamNames[i]);
            if (param)
            {
                if (!param->isValueOk(edits[0]->text()))
                {
                    QString tip = writeParamTitles[i] + ", ";
                    tip += QString::fromWCharArray(L"取值范围") + param->getValueRangeString();
                    UiUtil::showTip(tip);
                    return;
                }

                writeParamValues.append(edits[0]->text());
                continue;
            }
        }
        else
        {
            QList<QComboBox *> comboBoxes = findChildren<QComboBox *>(writeParamNames[i]+"ComboBox");
            if (comboBoxes.size() > 0)
            {
                int batteryType = comboBoxes[0]->currentData().toInt();
                writeParamValues.append(QString::number(batteryType));
                continue;
            }
        }

        UiUtil::showTip(QString::fromWCharArray(L"未知错误"));
        return;
    }

    // 保存参数    
    for (int i=0; i<writeParamNames.size(); i++)
    {
        ParamItem* param = DataManager::getInstance()->getParamByName(writeParamNames[i]);
        if (param)
        {
            if (param->m_valueType == PARAM_VALUE_TYPE_FLOAT)
            {
                bool ok = false;
                float value = writeParamValues[i].toFloat(&ok);
                if (ok)
                {
                    param->m_value = (int)(value * FLOAT_SCALE_FACTOR);
                }
            }
            else
            {
                bool ok = false;
                int value = writeParamValues[i].toInt(&ok);
                if (ok)
                {
                    param->m_value = value;
                }
            }
        }
    }
    DataManager::getInstance()->save();

    // 显示一个进度条，发送数据
    if (m_progressDlg)
    {
        return;
    }
    m_progressDlg = new MyProgressDialog(QString(), QString(), 0, 5, this);
    m_progressDlg->setAutoReset(false);
    m_progressDlg->setAttribute(Qt::WA_DeleteOnClose);
    m_progressDlg->setWindowTitle(QString::fromWCharArray(L"提示"));
    m_progressDlg->setLabelText(QString::fromWCharArray(L"正在写入"));
    m_progressDlg->setValue(0);
    m_progressDlg->show();

    QTimer* progressTimer = new QTimer(m_progressDlg);
    connect(progressTimer, &QTimer::timeout, [this]() {
        if (m_progressDlg == nullptr)
        {
            return;
        }

        int value = m_progressDlg->value();
        if (m_progressDlg->isSuccess())
        {
            // 至少显示2秒再关闭
            if (value >= 1)
            {
                m_progressDlg->setCanClose();
                m_progressDlg->close();
                m_progressDlg = nullptr;

                UiUtil::showTip(QString::fromWCharArray(L"写入完成"));
                return;
            }
        }
        else
        {
            if (value >= m_progressDlg->maximum())
            {

                m_progressDlg->setCanClose();
                m_progressDlg->close();
                m_progressDlg = nullptr;

                UiUtil::showTip(QString::fromWCharArray(L"写入超时"));
                return;
            }
        }
        m_progressDlg->setValue(value+1);
    });
    progressTimer->start(1000);

    // 发送写入命令
    static QString needWriteParamNames[] = {
        PARAM_NAME_BATTERY_TYPE, PARAM_NAME_BATTERY_CHUANSHU,PARAM_NAME_DANTI_GUOYA,PARAM_NAME_ZONG_GUOYA,
        PARAM_NAME_DANTI_QIANYA, PARAM_NAME_ZONG_QIANYA, PARAM_NAME_CHARGE_GUOLIU_PROTECT, PARAM_NAME_FANGDIAN_GUOLIU_PROTECT,
        PARAM_NAME_DUANLU_PROTECT, PARAM_NAME_FANGDIAN_GUOLIU_WARNING, PARAM_NAME_TWO_FANGDIAN_GUOLIU_PROTECT,
        PARAM_NAME_MOS_TEMPERATURE2, PARAM_NAME_JUNHENG_TEMPERATURE2, PARAM_NAME_T1_TEMPERATURE2, PARAM_NAME_T2_TEMPERATURE2,
        PARAM_NAME_JUNHENG_JIXIAN_DIANYA, PARAM_NAME_JUNHENG_QIDONG_DIANYA, PARAM_NAME_JUNHENG_QIDONG_YACHA, PARAM_NAME_JUNHENG_JUESHU_YACHA,
        PARAM_NAME_CHONGDIAN_GUOLIU_BAOHU_YANSHI, PARAM_NAME_FANGDIAN_GUOLIU_BAOHU_YANSHI, PARAM_NAME_DUANLU_BAOHU_YANSHI};

    QByteArray datas;
    datas.append((char)0x9c);
    datas.append((char)0x41);
    datas.append((char)0x00);
    datas.append((char)0x16);
    datas.append((char)0x2c);
    for (int i=0; i<sizeof(needWriteParamNames)/sizeof(needWriteParamNames[0]); i++)
    {
        bool found = false;
        for (int j=0; j<writeParamNames.size(); j++)
        {
            if (needWriteParamNames[i] == writeParamNames[j])
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            datas.append((char)0x00);
            datas.append((char)0x00);
        }
        else
        {
            int value = DataManager::getInstance()->getParamByName(needWriteParamNames[i])->m_value;
            datas.append((char)((value>>8)&0xff));
            datas.append((char)(value&0xff));
        }
    }
    m_modbusClient.sendData(CONTEXT_WRITE_PARAM_DATA, QModbusPdu::WriteMultipleRegisters, datas);
}


void MainWindow::onConnectButtonClicked()
{
    QString serialPortName = ui->portComboBox->currentText();
    if (serialPortName.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请选择端口"));
        return;
    }

    int baud = ui->botelvComboBox->currentText().toInt();

    SettingManager::getInstance()->m_serialPortName = serialPortName;
    SettingManager::getInstance()->m_baud = baud;
    SettingManager::getInstance()->save();
    m_modbusClient.setPortName(serialPortName);
    m_modbusClient.setBaud(baud);
}

void MainWindow::onSelectSoftwarePathButtonClicked()
{
    QFileDialog fileDialog;
    fileDialog.setWindowTitle(QString::fromWCharArray(L"选择文件"));
    fileDialog.setNameFilters(QStringList() << "bin files (*.bin)");
    if (fileDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QStringList selectedFiles = fileDialog.selectedFiles();
    ui->softwarePathEdit->setText(selectedFiles[0]);
}

void MainWindow::onReadSoftwareVersionButtonClicked()
{
    if (!m_modbusClient.isConnected())
    {
        UiUtil::showTip(QString::fromWCharArray(L"未连接"));
        return;
    }

    QByteArray datas;
    datas.append((char)0x75);
    datas.append((char)0x4b);
    datas.append((char)0x00);
    datas.append((char)0x02);
    m_modbusClient.sendData(CONTEXT_READ_SOFTWARE_VERSION, QModbusPdu::ReadInputRegisters, datas);
}

void MainWindow::onUpgradeButtonClicked()
{
    QString softFilePath = ui->softwarePathEdit->text();
    if (softFilePath.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请选择软件路径"));
        return;
    }

    QFile file(softFilePath);
    if (!file.exists())
    {
        UiUtil::showTip(QString::fromWCharArray(L"指定的软件路径不存在"));
        return;
    }

    QString softVersion = ui->softVersionEdit->text();
    if (softVersion.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请先读取软件版本"));
        return;
    }

    // 解析新软件版本
    int index = softFilePath.indexOf("V", Qt::CaseInsensitive);
    if (index < 0)
    {
        UiUtil::showTip(QString::fromWCharArray(L"无法从软件路径获取版本号"));
        return;
    }
    QStringList parts = softFilePath.mid(index+1).split('.');
    if (parts.size() != 3)
    {
        UiUtil::showTip(QString::fromWCharArray(L"无法从软件路径获取版本号"));
        return;
    }

    bool ok = false;
    int newMainVersion = parts[0].toInt(&ok);
    if (!ok)
    {
        UiUtil::showTip(QString::fromWCharArray(L"无法从软件路径获取版本号"));
        return;
    }

    int newMinorVersion = parts[1].toInt(&ok);
    if (!ok)
    {
        UiUtil::showTip(QString::fromWCharArray(L"无法从软件路径获取版本号"));
        return;
    }

    // 解析旧软件版本
    parts = softVersion.split('.');
    if (parts.size() != 2)
    {
        UiUtil::showTip(QString::fromWCharArray(L"软件版本错误"));
        return;
    }

    int oldMainVersion = parts[0].toInt(&ok);
    if (!ok)
    {
        UiUtil::showTip(QString::fromWCharArray(L"软件版本错误"));
        return;
    }

    int oldMinorVersion = parts[1].toInt(&ok);
    if (!ok)
    {
        UiUtil::showTip(QString::fromWCharArray(L"软件版本错误"));
        return;
    }

    // 比较版本
    if (oldMainVersion > newMainVersion ||
            (oldMainVersion == newMainVersion && oldMinorVersion >= newMinorVersion))
    {
        UiUtil::showTip(QString::fromWCharArray(L"无法升级，版本不够高"));
        return;
    }

    // 开始升级
    m_upgradeController = new UpgradeController(this);
    connect(m_upgradeController, &UpgradeController::runFinish, [this]() {
        m_upgradeController->deleteLater();
        m_upgradeController = nullptr;
    });

    m_upgradeController->m_modbusClient = &m_modbusClient;
    m_upgradeController->m_mainVersion = newMainVersion;
    m_upgradeController->m_minorVersion = newMinorVersion;
    m_upgradeController->m_softFilePath = softFilePath;
    m_upgradeController->run(this);
}

void MainWindow::onModifyPasswordButtonClicked()
{
    QString oldPassword = ui->oldPasswordEdit->text();
    QString newPassword = ui->newPasswordEdit->text();
    QString newPassword2 = ui->newPasswordEdit2->text();
    if (oldPassword.isEmpty() || newPassword.isEmpty() || newPassword2.isEmpty())
    {
        UiUtil::showTip(QString::fromWCharArray(L"请输入密码"));
        return;
    }

    if (SettingManager::getInstance()->m_passwordMd5.isEmpty())
    {
        if (oldPassword != SettingManager::getInstance()->getDefaultPassword())
        {
            UiUtil::showTip(QString::fromWCharArray(L"旧密码不是默认密码"));
            return;
        }
    }
    else
    {
        QByteArray hashBytes = QCryptographicHash::hash(oldPassword.toUtf8(), QCryptographicHash::Md5);
        QString passwordMd5(hashBytes);
        if (passwordMd5 != SettingManager::getInstance()->m_passwordMd5)
        {
            UiUtil::showTip(QString::fromWCharArray(L"旧密码错误"));
            return;
        }
    }

    if (newPassword != newPassword2)
    {
        UiUtil::showTip(QString::fromWCharArray(L"新密码不一致"));
        return;
    }

    QByteArray hashBytes = QCryptographicHash::hash(newPassword.toUtf8(), QCryptographicHash::Md5);
    SettingManager::getInstance()->m_passwordMd5 = hashBytes;
    SettingManager::getInstance()->save();
    UiUtil::showTip(QString::fromWCharArray(L"修改密码成功"));
}
