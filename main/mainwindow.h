#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mymodbusclient.h"
#include "batterywidget.h"
#include "myprogressdialog.h"
#include "upgradecontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initCtrls();

    void updateCtrlDatas();

    // 更新保护/警告信息
    void updateProtectWarningInfo();

    void updateTemperatureInfo();

private slots:
    // 每隔一秒触发一次
    void onMainTimer();

    void onRecvData(const QString& context, bool success, const QByteArray& data);

    void onChargeMosButtonClicked();

    void onFangdianMosButtonClicked();

    void onJunhengButtonClicked();

    void onSelectAllButtonClicked();

    void onUnSelectAllButtonClicked();

    void onWriteParamButtonClicked();

    void onConnectButtonClicked();

    void onSelectSoftwarePathButtonClicked();

    void onReadSoftwareVersionButtonClicked();

    void onUpgradeButtonClicked();

    void onModifyPasswordButtonClicked();

private:
    Ui::MainWindow *ui;

    MyModbusClient m_modbusClient;

    BatteryWidget* m_batteryWidget = nullptr;

    // 上一次温度更新的时间
    qint64 m_lastTemperatureUpdateTime = 0;

    MyProgressDialog* m_progressDlg = nullptr;

    UpgradeController* m_upgradeController = nullptr;
};
#endif // MAINWINDOW_H
