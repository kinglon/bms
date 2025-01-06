#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mymodbusclient.h"

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

private slots:
    // 每隔一秒触发一次
    void onMainTimer();

    void onRecvData(const QString& context, bool success, const QByteArray& data);

    void onChargeMosButtonClicked();

    void onFangdianMosButtonClicked();

    void onJunhengButtonClicked();

    void onWriteParamButtonClicked();

private:
    Ui::MainWindow *ui;

    MyModbusClient m_modbusClient;
};
#endif // MAINWINDOW_H
