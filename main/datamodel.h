#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QString>

// 电池类型
#define BATTERY_TYPE_MIN 1
#define BATTERY_TYPE_SANYUAN  1  // 三元锂电池
#define BATTERY_TYPE_LINSUANTAI  2  // 磷酸铁锂电池
#define BATTERY_TYPE_MENGSUAN  3  // 锰酸锂电池
#define BATTERY_TYPE_OTHER  4  // 其它
#define BATTERY_TYPE_MAX 4

// 参数值类型
#define PARAM_VALUE_TYPE_INT  1
#define PARAM_VALUE_TYPE_FLOAT  2

// 参数没有值
#define PARAM_VALUE_NOT_SETTING  -100

// 浮点数放大倍数
#define FLOAT_SCALE_FACTOR 1000

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
            // 展示给用户，还原为真实数据
            float value = m_value / FLOAT_SCALE_FACTOR;
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
                        QString::number(m_minValue/FLOAT_SCALE_FACTOR, 'f', 3),
                        QString::number(m_maxValue/FLOAT_SCALE_FACTOR, 'f', 3));
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
            valueInt = (int)(value.toFloat(&ok) * FLOAT_SCALE_FACTOR);
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
#define PARAM_NAME_CHARGE_MOS_STATUS   "chargeMosStatus"     // 充电MOS状态
#define PARAM_NAME_FANGDIAN_MOS_STATUS   "fangdianMosStatus"     // 放电MOS状态

#define PARAM_NAME_SOC   "soc"     // 电池电量

#define PARAM_NAME_BATTERY_TOTAL_DIANYA  "batteryTotalDianya"  // 电池总电压
#define PARAM_NAME_BATTERY_DIANLIU  "batteryDianliu"  // 电池电流
#define PARAM_NAME_MAX_DIANYA  "maxDianya"  // 最高电压
#define PARAM_NAME_MIN_DIANYA  "minDianya"  // 最低电压
#define PARAM_NAME_TOTAL_LOOP  "totalLoop"  // 总循环

#define PARAM_NAME_BATTERY_DIANYA_PREFIX  "batteryDianya"  // 电池电压前缀，总共16节，尾部从1到16
#define BATTERY_COUNT 16  // 16节电池

// 温度值都是实际值放大100倍
#define PARAM_NAME_MOS_TEMPERATURE  "mosTemp"  // MOS温度
#define PARAM_NAME_JUNHENG_TEMPERATURE  "junhengTemp"  // 均衡温度
#define PARAM_NAME_T1_TEMPERATURE  "t1Temp"  // T1温度
#define PARAM_NAME_T2_TEMPERATURE  "t2Temp"  // T2温度

#define PARAM_NAME_MOS_TEMPERATURE2  "mosTemp2"  // MOS温度，设置使用
#define PARAM_NAME_JUNHENG_TEMPERATURE2  "junhengTemp2"  // 均衡温度，设置使用
#define PARAM_NAME_T1_TEMPERATURE2  "t1Temp2"  // T1温度，设置使用
#define PARAM_NAME_T2_TEMPERATURE2  "t2Temp2"  // T2温度，设置使用

#define PARAM_NAME_BATTERY_TYPE   "batteryType"     // 电池类型
#define PARAM_NAME_BATTERY_CHUANSHU   "batteryChuanshu"     // 电池串数

#define PARAM_NAME_DANTI_GUOYA   "dantiGuoya"     // 单体过压
#define PARAM_NAME_ZONG_GUOYA   "zongGuoya"     // 总过压
#define PARAM_NAME_DANTI_QIANYA   "dantiQianya"     // 单体欠压
#define PARAM_NAME_ZONG_QIANYA  "zongQianya"     // 总欠压

#define PARAM_NAME_CHARGE_GUOLIU_PROTECT   "chargeGuoliuProtect"     // 充电过流保护
#define PARAM_NAME_FANGDIAN_GUOLIU_PROTECT   "fangdianGuoliuProtect"     // 放电过流保护
#define PARAM_NAME_DUANLU_PROTECT   "duanluProtect"     // 短路保护
#define PARAM_NAME_FANGDIAN_GUOLIU_WARNING  "fangdianGuoliuWarning"     // 放电过流报警
#define PARAM_NAME_TWO_FANGDIAN_GUOLIU_PROTECT   "twoFangdianGuoliuProtect"     // 二级放电过流保护

#define PARAM_NAME_JUNHENG_JIXIAN_DIANYA   "junhengJixianDianya"     // 均衡极限电压
#define PARAM_NAME_JUNHENG_QIDONG_DIANYA   "junhengQidongDianya"     // 均衡启动电压
#define PARAM_NAME_JUNHENG_QIDONG_YACHA   "junhengQidongYacha"     // 均衡启动压差
#define PARAM_NAME_JUNHENG_JUESHU_YACHA   "junhengJieSuYacha"     // 均衡结束压差

#define PARAM_NAME_CHONGDIAN_GUOLIU_BAOHU_YANSHI   "chongdianGuoliuBaohuYansi"     // 充电过流保护延时
#define PARAM_NAME_FANGDIAN_GUOLIU_BAOHU_YANSHI   "fangdianGuoliuBaohuYansi"     // 放电过流保护延时
#define PARAM_NAME_DUANLU_BAOHU_YANSHI   "duanluBaohuYansi"     // 短路保护延时

#endif // DATAMODEL_H
