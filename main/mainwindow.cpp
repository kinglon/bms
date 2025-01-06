#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datamodel.h"
#include "datamanager.h"
#include "uiutil.h"
#include <QTimer>
#include "settingmanager.h"
#include "Utility/LogUtil.h"


#define MAKE_INT(byte1, byte2) ((int)((byte1<<8) + byte2))
#define MAKE_INT_4(byte1, byte2, byte3, byte4) ((int)((byte1<<24) + (byte2<<16) + (byte3<<8) + byte4))

// context定义
#define CONTEXT_READ_SWITCH_STATUS  "read_switch_status"  // 读取充电MOS、放电MOS、均衡开关的状态
#define CONTEXT_WRITE_CHARGE_MOS_SWITCH  "write_charge_mos_switch"  // 设置充电MOS开关的状态
#define CONTEXT_WRITE_FANGDIAN_MOS_SWITCH  "write_fangdian_mos_switch"  // 设置放电MOS开关的状态
#define CONTEXT_WRITE_JUNHENG_SWITCH  "write_junheng_switch"  // 设置均衡开关的状态
#define CONTEXT_READ_BATTERY_JUNHENG_STATUS  "read_battery_junheng_status"  // 读取电池和均衡的状态
#define CONTEXT_READ_DIANYA_DIANLIU_DATA  "read_dianya_dianliu_data"  // 读取电压电流数据

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initCtrls();

    updateCtrlDatas();

    connect(&m_modbusClient, &MyModbusClient::recvData, this, &MainWindow::onRecvData);

    // 先展示界面再连接
    QTimer::singleShot(100, [this]() {
        if (SettingManager::getInstance()->m_logLevel == (int)ELogLevel::LOG_LEVEL_DEBUG)
        {
            m_modbusClient.enableDebug();
        }
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

    connect(ui->chargeMosButton, &QPushButton::clicked, this, &MainWindow::onChargeMosButtonClicked);
    connect(ui->fangdianMosButton, &QPushButton::clicked, this, &MainWindow::onFangdianMosButtonClicked);
    connect(ui->junhengButton, &QPushButton::clicked, this, &MainWindow::onJunhengButtonClicked);
    connect(ui->writeParamButton, &QPushButton::clicked, this, &MainWindow::onWriteParamButtonClicked);
}

void MainWindow::updateCtrlDatas()
{
    // 通过参数名称查找控件，并设置值
    for (const auto& paramItem : DataManager::getInstance()->m_params)
    {
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

    value = DataManager::getInstance()->getParamByName(PARAM_NAME_CHARGE_MOS_SWITCH)->m_value;
    ui->chargeMosSwitchLabel->setText(getStringFromSwitch(value));
    if (value == SWITCH_OPEN)
    {
        ui->chargeMosButton->setText(QString::fromWCharArray(L"关闭充电MOS"));
    }
    else
    {
        ui->chargeMosButton->setText(QString::fromWCharArray(L"打开充电MOS"));
    }

    value = DataManager::getInstance()->getParamByName(PARAM_NAME_FANGDIAN_MOS_SWITCH)->m_value;
    ui->fangdianMosSwitchLabel->setText(getStringFromSwitch(value));
    if (value == SWITCH_OPEN)
    {
        ui->fangdianMosButton->setText(QString::fromWCharArray(L"关闭放电MOS"));
    }
    else
    {
        ui->fangdianMosButton->setText(QString::fromWCharArray(L"打开放电MOS"));
    }

    value = DataManager::getInstance()->getParamByName(PARAM_NAME_JUNHENG_STATUS)->m_value;
    ui->junfengStatusLabel->setText(getStringFromStatus(value));

    value = DataManager::getInstance()->getParamByName(PARAM_NAME_JUNHENG_SWITCH)->m_value;
    if (value == SWITCH_OPEN)
    {
        ui->junhengButton->setText(QString::fromWCharArray(L"关闭均衡"));
    }
    else
    {
        ui->junhengButton->setText(QString::fromWCharArray(L"打开均衡"));
    }

    // 更新电池电量
    if (m_batteryWidget)
    {
        value = DataManager::getInstance()->getParamByName(PARAM_NAME_SOC)->m_value;
        m_batteryWidget->setBatteryValue(value);
    }

    // 保护信息
    bool qianYa = false; // 欠压
    bool guoYa = false; // 过压
    for (int i=1; i<=BATTERY_COUNT; i++)
    {
        value = DataManager::getInstance()->getParamByName(QString(PARAM_NAME_BATTERY_DIANYA_PREFIX)+QString::number(i))->m_value;
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
}

void MainWindow::onMainTimer()
{
    if (m_modbusClient.isConnected())
    {
        // 读取充电MOS、放电MOS、均衡开关的状态
        QByteArray datas;
        datas.append((char)0x00);
        datas.append((char)0x01);
        datas.append((char)0x00);
        datas.append((char)0x03);
        m_modbusClient.sendData(CONTEXT_READ_SWITCH_STATUS, QModbusPdu::ReadCoils, datas);

        // 读取电池和均衡的状态
        datas.clear();
        datas.append((char)0x10);
        datas.append((char)0x01);
        datas.append((char)0x00);
        datas.append((char)0x02);
        m_modbusClient.sendData(CONTEXT_READ_BATTERY_JUNHENG_STATUS, QModbusPdu::ReadDiscreteInputs, datas);

        // 读取电压电流数据
        datas.clear();
        datas.append((char)0x75);
        datas.append((char)0x31);
        datas.append((char)0x00);
        datas.append((char)0x18);
        m_modbusClient.sendData(CONTEXT_READ_DIANYA_DIANLIU_DATA, QModbusPdu::ReadInputRegisters, datas);

    }
}

void MainWindow::onRecvData(const QString& context, bool success, const QByteArray& data)
{
    if (context == CONTEXT_READ_SWITCH_STATUS)
    {
        // 读取充电MOS、放电MOS、均衡开关的状态
        if (success && data.length() >= 8)
        {
            bool chargeMosEnable = data[3] != 0x00;
            bool fangdianMosEnable = data[5] != 0x00;
            bool junhengEnable = data[7] != 0x00;
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
        // 读取电池和均衡的状态
        if (success && data.length() >= 6)
        {
            bool batteryException = data[3] == 0x00;
            bool junhengException = data[5] == 0x00;
            DataManager::getInstance()->setParamValue(PARAM_NAME_BATTERY_STATUS, batteryException?STATUS_EXCEPTION:STATUS_GOOD);
            DataManager::getInstance()->setParamValue(PARAM_NAME_JUNHENG_STATUS, junhengException?STATUS_EXCEPTION:STATUS_GOOD);
            updateCtrlDatas();
        }
    }
    else if (context == CONTEXT_READ_DIANYA_DIANLIU_DATA)
    {
        // 读取电压电流数据
        if (success && data.length() >= 48)
        {
            int pos = 2;
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
}

void MainWindow::onChargeMosButtonClicked()
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
    if (ui->chargeMosButton->text().indexOf(QString::fromWCharArray(L"打开")) >= 0)
    {
        datas.append((char)0x01);
    }
    else
    {
        datas.append((char)0x00);
    }
    m_modbusClient.sendData(CONTEXT_WRITE_CHARGE_MOS_SWITCH, QModbusPdu::WriteSingleCoil, datas);
}

void MainWindow::onFangdianMosButtonClicked()
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
    if (ui->fangdianMosButton->text().indexOf(QString::fromWCharArray(L"打开")) >= 0)
    {
        datas.append((char)0x01);
    }
    else
    {
        datas.append((char)0x00);
    }
    m_modbusClient.sendData(CONTEXT_WRITE_FANGDIAN_MOS_SWITCH, QModbusPdu::WriteSingleCoil, datas);
}

void MainWindow::onJunhengButtonClicked()
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
    if (ui->junhengButton->text().indexOf(QString::fromWCharArray(L"打开")) >= 0)
    {
        datas.append((char)0x01);
    }
    else
    {
        datas.append((char)0x00);
    }
    m_modbusClient.sendData(CONTEXT_WRITE_JUNHENG_SWITCH, QModbusPdu::WriteSingleCoil, datas);
}

void MainWindow::onWriteParamButtonClicked()
{
    // 获取勾选需要设置的参数
    QVector<QString> writeParamNames;
    QVector<QString> writeParamTitles;
    for (const auto& param : DataManager::getInstance()->m_params)
    {
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

    // 校验参数
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
                continue;
            }
        }

        UiUtil::showTip(QString::fromWCharArray(L"未知错误"));
        return;
    }

    // 保存参数
}
