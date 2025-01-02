#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QString>

// 电池类型
#define BATTERY_TYPE_SANYUAN  1  // 三元锂电池
#define BATTERY_TYPE_LINSUANTAI  2  // 磷酸铁锂电池
#define BATTERY_TYPE_MENGSUAN  3  // 锰酸锂电池
#define BATTERY_TYPE_OTHER  4  // 其它

class ParamItem
{
public:
    // 参数名字
    QString m_name;

    // 值
    int m_value = 0;

    // 单位，有设置尾部追加显示单位
    QString m_unit;

    // 标志是否要保存到配置文件
    bool m_localSave = false;
};

#endif // DATAMODEL_H
