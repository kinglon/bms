#include "datamanager.h"
#include <QFile>
#include "Utility/ImPath.h"
#include "Utility/ImCharset.h"
#include "Utility/LogMacro.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#define MAX_INT_VALUE 10000000


DataManager::DataManager()
{
    m_params.append(ParamItem(PARAM_NAME_CHARGE_MOS_SWITCH,
                              PARAM_VALUE_TYPE_INT,
                              SWITCH_UNKNOWN, // value
                              -100, // min value
                              100, // max value
                              "",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_FANGDIAN_MOS_SWITCH,
                              PARAM_VALUE_TYPE_INT,
                              SWITCH_UNKNOWN, // value
                              -100, // min value
                              100, // max value
                              "",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_JUNHENG_SWITCH,
                              PARAM_VALUE_TYPE_INT,
                              SWITCH_UNKNOWN, // value
                              -100, // min value
                              100, // max value
                              "",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_BATTERY_STATUS,
                              PARAM_VALUE_TYPE_INT,
                              STATUS_UNKNOWN, // value
                              -100, // min value
                              100, // max value
                              "",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_JUNHENG_STATUS,
                              PARAM_VALUE_TYPE_INT,
                              STATUS_UNKNOWN, // value
                              -100, // min value
                              100, // max value
                              "",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_CHARGE_MOS_STATUS,
                              PARAM_VALUE_TYPE_INT,
                              STATUS_UNKNOWN, // value
                              -100, // min value
                              100, // max value
                              "",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_FANGDIAN_MOS_STATUS,
                              PARAM_VALUE_TYPE_INT,
                              STATUS_UNKNOWN, // value
                              -100, // min value
                              100, // max value
                              "",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_SOC,
                              PARAM_VALUE_TYPE_INT,
                              20, // value
                              0, // min value
                              100, // max value
                              "%",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_BATTERY_TOTAL_DIANYA,
                              PARAM_VALUE_TYPE_FLOAT,
                              60000, // value
                              0, // min value
                              10000000, // max value
                              "V",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_BATTERY_DIANLIU,
                              PARAM_VALUE_TYPE_FLOAT,
                              5000, // value
                              0, // min value
                              10000000, // max value
                              "A",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_MAX_DIANYA,
                              PARAM_VALUE_TYPE_FLOAT,
                              3780, // value
                              0, // min value
                              10000000, // max value
                              "V",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_MIN_DIANYA,
                              PARAM_VALUE_TYPE_FLOAT,
                              3570, // value
                              0, // min value
                              10000000, // max value
                              "V",  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_TOTAL_LOOP,
                              PARAM_VALUE_TYPE_FLOAT,
                              50000, // value
                              0, // min value
                              10000000, // max value
                              "AH",  // unit
                              false)); // local save

    for (int i=1; i<=BATTERY_COUNT; i++)
    {
        m_params.append(ParamItem(QString(PARAM_NAME_BATTERY_DIANYA_PREFIX)+QString::number(i),
                                  PARAM_VALUE_TYPE_FLOAT,
                                  3700, // value
                                  0, // min value
                                  10000000, // max value
                                  "V",  // unit
                                  false)); // local save
    }

    m_params.append(ParamItem(PARAM_NAME_MOS_TEMPERATURE,
                              PARAM_VALUE_TYPE_INT,
                              0, // value
                              -10000000, // min value
                              10000000, // max value
                              QString::fromWCharArray(L"℃"),  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_JUNHENG_TEMPERATURE,
                              PARAM_VALUE_TYPE_INT,
                              0, // value
                              -10000000, // min value
                              10000000, // max value
                              QString::fromWCharArray(L"℃"),  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_T1_TEMPERATURE,
                              PARAM_VALUE_TYPE_INT,
                              0, // value
                              -10000000, // min value
                              10000000, // max value
                              QString::fromWCharArray(L"℃"),  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_T2_TEMPERATURE,
                              PARAM_VALUE_TYPE_INT,
                              0, // value
                              -10000000, // min value
                              10000000, // max value
                              QString::fromWCharArray(L"℃"),  // unit
                              false)); // local save

    m_params.append(ParamItem(PARAM_NAME_MOS_TEMPERATURE2,
                              PARAM_VALUE_TYPE_INT,
                              90, // value
                              40, // min value
                              150, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_JUNHENG_TEMPERATURE2,
                              PARAM_VALUE_TYPE_INT,
                              90, // value
                              40, // min value
                              150, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_T1_TEMPERATURE2,
                              PARAM_VALUE_TYPE_INT,
                              90, // value
                              40, // min value
                              150, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_T2_TEMPERATURE2,
                              PARAM_VALUE_TYPE_INT,
                              90, // value
                              40, // min value
                              150, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_BATTERY_TYPE,
                              PARAM_VALUE_TYPE_INT,
                              BATTERY_TYPE_SANYUAN, // value
                              BATTERY_TYPE_MIN, // min value
                              BATTERY_TYPE_MAX, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_BATTERY_CHUANSHU,
                              PARAM_VALUE_TYPE_INT,
                              PARAM_VALUE_NOT_SETTING, // value
                              7, // min value
                              16, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_DANTI_GUOYA,
                              PARAM_VALUE_TYPE_INT,
                              4200, // value
                              0, // min value
                              65535, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_ZONG_GUOYA,
                              PARAM_VALUE_TYPE_INT,
                              64000, // value
                              0, // min value
                              MAX_INT_VALUE, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_DANTI_QIANYA,
                              PARAM_VALUE_TYPE_INT,
                              2700, // value
                              0, // min value
                              65535, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_ZONG_QIANYA,
                              PARAM_VALUE_TYPE_INT,
                              48000, // value
                              0, // min value
                              MAX_INT_VALUE, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_CHARGE_GUOLIU_PROTECT,
                              PARAM_VALUE_TYPE_INT,
                              30000, // value
                              0, // min value
                              MAX_INT_VALUE, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_FANGDIAN_GUOLIU_PROTECT,
                              PARAM_VALUE_TYPE_INT,
                              100000, // value
                              0, // min value
                              MAX_INT_VALUE, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_DUANLU_PROTECT,
                              PARAM_VALUE_TYPE_INT,
                              200000, // value
                              0, // min value
                              MAX_INT_VALUE, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_FANGDIAN_GUOLIU_WARNING,
                              PARAM_VALUE_TYPE_INT,
                              80000, // value
                              0, // min value
                              MAX_INT_VALUE, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_TWO_FANGDIAN_GUOLIU_PROTECT,
                              PARAM_VALUE_TYPE_INT,
                              150000, // value
                              0, // min value
                              MAX_INT_VALUE, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_JUNHENG_JIXIAN_DIANYA,
                              PARAM_VALUE_TYPE_INT,
                              4, // value
                              0, // min value
                              65535, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_JUNHENG_QIDONG_DIANYA,
                              PARAM_VALUE_TYPE_INT,
                              2, // value
                              0, // min value
                              65535, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_JUNHENG_QIDONG_YACHA,
                              PARAM_VALUE_TYPE_INT,
                              50, // value
                              0, // min value
                              65535, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_JUNHENG_JUESHU_YACHA,
                              PARAM_VALUE_TYPE_INT,
                              10, // value
                              0, // min value
                              65535, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_CHONGDIAN_GUOLIU_BAOHU_YANSHI,
                              PARAM_VALUE_TYPE_INT,
                              10, // value
                              0, // min value
                              65535, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_FANGDIAN_GUOLIU_BAOHU_YANSHI,
                              PARAM_VALUE_TYPE_INT,
                              10, // value
                              0, // min value
                              65535, // max value
                              "",  // unit
                              true)); // local save

    m_params.append(ParamItem(PARAM_NAME_DUANLU_BAOHU_YANSHI,
                              PARAM_VALUE_TYPE_INT,
                              100, // value
                              0, // min value
                              65535, // max value
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
