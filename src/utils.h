#ifndef UTILS_H
#define UTILS_H

#include <QString>

inline QString posToStr(int row, int col)
{
    return QString("%1%2").arg(QChar('a'+col)).arg(8-row);
}

#endif // UTILS_H
