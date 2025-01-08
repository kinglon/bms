#ifndef CRC32UTIL_H
#define CRC32UTIL_H

#include <QString>

class Crc32Util
{
public:
    Crc32Util();

public:
    static bool calcFileCRC(const QString& fileName, quint32& crc32);
};

#endif // CRC32UTIL_H
