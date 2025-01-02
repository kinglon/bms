#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datamodel.h"
#include "datamanager.h"
#include "uiutil.h"

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
