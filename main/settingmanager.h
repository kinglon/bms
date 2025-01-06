#pragma once

#include <QString>
#include <QVector>

class SettingManager
{
protected:
    SettingManager();

public:
    static SettingManager* getInstance();

    void save();    

private:
    void load();

public:
    int m_logLevel = 2;  // info

    int m_baud = 19200;
};
