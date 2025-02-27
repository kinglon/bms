﻿#pragma once

#include <QString>
#include <QVector>

class SettingManager
{
protected:
    SettingManager();

public:
    static SettingManager* getInstance();

    void save();

    QString getDefaultPassword();

private:
    void load();

public:
    int m_logLevel = 2;  // info

    QString m_serialPortName;

    int m_baud = 19200;

    QString m_passwordMd5;
};
