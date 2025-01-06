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

private:
    float m_scaleFactor = 1000.0f;  // 放大因子

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
            // 展示给用户，还原为真实数据
            float value = m_value / m_scaleFactor;
            return QString::number(value, 'f', 3) + m_unit;
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
            // 展示给用户，还原为真实数据
            return QString::fromWCharArray(L"[%1, %2]").arg(
                        QString::number(m_minValue/m_scaleFactor, 'f', 3),
                        QString::number(m_maxValue/m_scaleFactor, 'f', 3));
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
            // 用户输入是真实数据，需要放大
            valueInt = (int)(value.toFloat(&ok) * m_scaleFactor);
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
#define STATUS_UNKNOWN     10  // 未知

QString getStringFromStatus(int status);

// 开关
#define SWITCH_CLOSE    0
#define SWITCH_OPEN     1
#define SWITCH_UNKNOWN  10  // 未知

QString getStringFromSwitch(int switchValue);

// 参数名字
#define PARAM_NAME_CHARGE_MOS_SWITCH   "chargeMosSwitch"     // 充电MOS开关
#define PARAM_NAME_FANGDIAN_MOS_SWITCH   "fangdianMosSwitch"     // 放电MOS开关
#define PARAM_NAME_JUNHENG_SWITCH   "junhengSwitch"     // 均衡开关

#define PARAM_NAME_BATTERY_STATUS   "batteryStatus"     // 电池状态
#define PARAM_NAME_JUNHENG_STATUS   "junfengStatus"     // 均衡状态

#define PARAM_NAME_SOC   "soc"     // 电池电量

#define PARAM_NAME_BATTERY_TOTAL_DIANYA  "battery_total_dianya"  // 电池总电压
#define PARAM_NAME_BATTERY_DIANLIU  "battery_dianliu"  // 电池电流
#define PARAM_NAME_MAX_DIANYA  "max_dianya"  // 最高电压
#define PARAM_NAME_MIN_DIANYA  "max_dianya"  // 最低电压
#define PARAM_NAME_TOTAL_LOOP  "total_loop"  // 总循环

#define PARAM_NAME_BATTERY_DIANYA_PREFIX  "battery_dianya"  // 电池电压前缀，总共16节，尾部从1到16
#define BATTERY_COUNT 16  // 16节电池

#define PARAM_NAME_BATTERY_CHUANSHU   "batteryChuanshu"     // 电池串数

#endif // DATAMODEL_H
