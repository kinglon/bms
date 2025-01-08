#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "uiutil.h"
#include "settingmanager.h"
#include <QCryptographicHash>

LoginWindow::LoginWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    initCtrls();
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::initCtrls()
{
    connect(ui->loginButton, &QPushButton::clicked, [this]() {
        QString password = ui->passwordEdit->text();
        if (password.isEmpty())
        {
            UiUtil::showTip(QString::fromWCharArray(L"请输入密码"));
            return;
        }

        if (SettingManager::getInstance()->m_passwordMd5.isEmpty())
        {
            if (password != SettingManager::getInstance()->getDefaultPassword())
            {
                UiUtil::showTip(QString::fromWCharArray(L"请使用正确的默认密码登录"));
                return;
            }
        }
        else
        {
            QByteArray hashBytes = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);
            QString passwordMd5(hashBytes);
            if (passwordMd5 != SettingManager::getInstance()->m_passwordMd5)
            {
                UiUtil::showTip(QString::fromWCharArray(L"密码错误"));
                return;
            }
        }

        emit loginSuccess();
        close();
    });
}
