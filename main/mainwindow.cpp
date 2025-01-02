#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datamodel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initCtrls();

    updateCtrlDatas();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initCtrls()
{
    ui->batteryTypeComboBox->addItem(QString::fromWCharArray(L"三元锂电池"), BATTERY_TYPE_SANYUAN);
    ui->batteryTypeComboBox->addItem(QString::fromWCharArray(L"磷酸铁锂电池"), BATTERY_TYPE_LINSUANTAI);
    ui->batteryTypeComboBox->addItem(QString::fromWCharArray(L"锰酸锂电池"), BATTERY_TYPE_MENGSUAN);
    ui->batteryTypeComboBox->addItem(QString::fromWCharArray(L"其它"), BATTERY_TYPE_OTHER);

    ui->botelvComboBox->addItem(QString::fromWCharArray(L"2400"));
    ui->botelvComboBox->addItem(QString::fromWCharArray(L"9600"));
    ui->botelvComboBox->addItem(QString::fromWCharArray(L"19200"));
    ui->botelvComboBox->addItem(QString::fromWCharArray(L"115200"));
}

void MainWindow::updateCtrlDatas()
{
    //
}
