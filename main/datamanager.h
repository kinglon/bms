#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QVector>
#include "datamodel.h"

class DataManager
{
protected:
    DataManager();

public:
    static DataManager* getInstance();

    ParamItem* getParamByName(const QString& name);

    void setParamValue(const QString& name, int value);

    void save();

private:
    void load();

public:
    QVector<ParamItem> m_params;
};

#endif // DATAMANAGER_H
