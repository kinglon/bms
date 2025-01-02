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
