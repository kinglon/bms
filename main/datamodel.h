#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QString>

// 电池类型
#define BATTERY_TYPE_SANYUAN  1  // 三元锂电池
#define BATTERY_TYPE_LINSUANTAI  2  // 磷酸铁锂电池
#define BATTERY_TYPE_MENGSUAN  3  // 锰酸锂电池
#define BATTERY_TYPE_OTHER  4  // 其它

// 参数值类型
#define PARAM_VALUE_TYPE_INT  1
#define PARAM_VALUE_TYPE_FLOAT  2

// 参数没有值
#define PARAM_VALUE_NOT_SETTING  -100

class ParamItem
{
public:
    // 参数名字
    QString m_name;

    // 值类型，如果是浮点数表示放大1000倍
    int m_valueType = PARAM_VALUE_TYPE_INT;

    // 值
    int m_value = PARAM_VALUE_NOT_SETTING;

    // 最小值和最大值
    int m_minValue = 0;
    int m_maxValue = 10000;

    // 单位，有设置尾部追加显示单位
    QString m_unit;

    // 标志是否要保存到配置文件
    bool m_localSave = false;

public:
    ParamItem(const QString& name, int valueType, int value, int minValue, int maxValue, const QString& unit, bool localSave)
    {
        this->m_name = name;
        this->m_valueType = valueType;
        this->m_value = value;
        this->m_minValue = minValue;
        this->m_maxValue = maxValue;
        this->m_unit = unit;
        this->m_localSave = localSave;
    }

    QString getValueString() const
    {
        if (m_value == PARAM_VALUE_NOT_SETTING)
        {
            return "";
        }

        if (m_valueType == PARAM_VALUE_TYPE_FLOAT)
        {
            float value = m_value / 1000.0f;
            return QString::number(value, 'f', 2) + m_unit;
        }
        else
        {
            return QString::number(m_value) + m_unit;
        }
    }

    QString getValueRangeString() const
    {
        if (m_valueType == PARAM_VALUE_TYPE_FLOAT)
        {
            return QString::fromWCharArray(L"[%1, %2]").arg(
                        QString::number(m_minValue, 'f', 2),
                        QString::number(m_maxValue, 'f', 2));
        }
        else
        {
            return QString::fromWCharArray(L"[%1, %2]").arg(
                        QString::number(m_minValue),
                        QString::number(m_maxValue));
        }
    }

    bool isValueOk(QString value) const
    {
        bool ok = false;
        int valueInt = 0;
        if (m_valueType == PARAM_VALUE_TYPE_FLOAT)
        {
            valueInt = (int)(value.toFloat(&ok) * 1000);
        }
        else
        {
            valueInt = value.toInt(&ok);
        }

        if (!ok)
        {
            return false;
        }

        if (valueInt < m_minValue || valueInt > m_maxValue)
        {
            return false;
        }

        return true;
    }
};

// 状态
#define STATUS_EXCEPTION     0  // 异常
#define STATUS_GOOD     1  // 良好
#define STATUS_UNKNOWN     2  // 未知

QString getStringFromStatus(int status);

// 参数名字
#define PARAM_NAME_BATTERY_STATUS   "batteryStatus"     // 电池状态
#define PARAM_NAME_BATTERY_CHUANSHU   "batteryChuanshu"     // 电池串数

#endif // DATAMODEL_H
