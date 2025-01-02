#include "datamanager.h"
#include <QFile>
#include "Utility/ImPath.h"
#include "Utility/ImCharset.h"
#include "Utility/LogMacro.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

DataManager::DataManager()
{
    m_params.append(ParamItem(PARAM_NAME_BATTERY_STATUS,
                              PARAM_VALUE_TYPE_INT,
                              STATUS_UNKNOWN, // value
                              -100, // min value
                              100, // max value
                              "",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_BATTERY_CHUANSHU,
                              PARAM_VALUE_TYPE_INT,
                              PARAM_VALUE_NOT_SETTING, // value
                              7, // min value
                              16, // max value
                              "",  // unit
                              true)); // local save

    load();
}

DataManager* DataManager::getInstance()
{
    static DataManager* instance = new DataManager();
    return instance;
}

ParamItem* DataManager::getParamByName(const QString& name)
{
    for (int i=0; i<m_params.size(); i++)
    {
        if (m_params[i].m_name == name)
        {
            return &m_params[i];
        }
    }

    return nullptr;
}

void DataManager::setParamValue(const QString& name, int value)
{
    for (auto& param : m_params)
    {
        if (param.m_name == name)
        {
            param.m_value = value;
            break;
        }
    }
}

void DataManager::load()
{
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"params.json";
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.exists())
    {
        return;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        LOG_ERROR(L"failed to open the param configure file : %s", strConfFilePath.c_str());
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData);
    QJsonObject root = jsonDocument.object();

    for (auto& param : m_params)
    {
        if (root.contains(param.m_name))
        {
            param.m_value = root[param.m_name].toInt();
        }
    }
}

void DataManager::save()
{
    QJsonObject root;
    for (const auto& param : m_params)
    {
        if (param.m_localSave)
        {
            root[param.m_name] = param.m_value;
        }
    }

    QJsonDocument jsonDocument(root);
    QByteArray jsonData = jsonDocument.toJson(QJsonDocument::Indented);
    std::wstring strConfFilePath = CImPath::GetConfPath() + L"params.json";
    QFile file(QString::fromStdWString(strConfFilePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical("failed to save param");
        return;
    }
    file.write(jsonData);
    file.close();
}
