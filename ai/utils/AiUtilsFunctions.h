#ifndef AIUTILSFUNCTIONS_H
#define AIUTILSFUNCTIONS_H
#include <QPoint>
#include <game/unit.h>

namespace aiutils {

/**
 * @brief quick manhattan distance between 2 points. It shouldn't be here such a function probably
 */
inline static qint32 pointDistance(QPoint p1, QPoint p2) {
    return qAbs(p1.x() - p2.x()) + qAbs(p1.y() - p2.y());
}

inline static qint32 pointDistance(qint32 x1, qint32 y1, qint32 x2, qint32 y2) {
    return qAbs(x1 - x2) + qAbs(y1 - y2);
}

}

#endif // AIUTILSFUNCTIONS_H
