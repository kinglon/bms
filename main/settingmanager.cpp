#include "settingmanager.h"
#include <QFile>
#include "Utility/ImPath.h"
#include "Utility/ImCharset.h"
#include "Utility/LogMacro.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

SettingManager::SettingManager()
{
    load();
}

SettingManager* SettingManager::getInstance()
{
    static SettingManager* pInstance = new SettingManager();
	return pInstance;
}

void SettingManager::load()
{
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"configs.json";    
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.exists())
    {
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        LOG_ERROR(L"failed to open the basic configure file : %s", strConfFilePath.c_str());
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData);
    QJsonObject root = jsonDocument.object();

    m_logLevel = root["log_level"].toInt();
    m_baud = root["baud"].toInt();
    m_passwordMd5 = root["md"].toString();
    if (root.contains("serialPortName"))
    {
        m_serialPortName = root["serialPortName"].toString();
    }
}

void SettingManager::save()
{
    QJsonObject root;
    root["log_level"] = m_logLevel;
    root["baud"] = m_baud;
    root["md"] = m_passwordMd5;
    root["serialPortName"] = m_serialPortName;

    QJsonDocument jsonDocument(root);
    QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Indented);
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"configs.json";
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical("failed to save setting");
        return;
    }
    file.write(jsonData);
    file.close();
}

QString SettingManager::getDefaultPassword()
{
    // 默认：jeric
    QString password;
    password.append((char)('h'+2));
    password.append((char)('g'-2));
    password.append((char)('r'));
    password.append((char)('l'-3));
    password.append((char)('b'+1));
    return password;
}
