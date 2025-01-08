#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private:
    void initCtrls();

    ;

signals:
    void loginSuccess();

private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
