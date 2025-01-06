#include "datamodel.h"

QString getStringFromStatus(int status)
{
    if (status == STATUS_EXCEPTION)
    {
        return QString::fromWCharArray(L"异常");
    }
    else if (status == STATUS_GOOD)
    {
        return QString::fromWCharArray(L"良好");
    }
    else
    {
        return QString::fromWCharArray(L"未知");
    }
}

QString getStringFromSwitch(int switchValue)
{
    if (switchValue == SWITCH_CLOSE)
    {
        return QString::fromWCharArray(L"关闭");
    }
    else if (switchValue == SWITCH_OPEN)
    {
        return QString::fromWCharArray(L"开启");
    }
    else
    {
        return QString::fromWCharArray(L"未知");
    }
}
