#ifndef AIUTILSFUNCTIONS_H
#define AIUTILSFUNCTIONS_H
#include <QPoint>
#include <game/unit.h>
#include "coreengine/console.h"
#include "game/unit.h"

namespace aiutils {
static const float UnitHpRatioMul = 1 / Unit::MAX_UNIT_HP;

inline static float unitHpRatio(float unitHp) {
    return unitHp * UnitHpRatioMul;
}

/**
 * @brief quick manhattan distance between 2 points. It shouldn't be here such a function probably
 */
inline static qint32 pointDistance(QPoint p1, QPoint p2) {
    return qAbs(p1.x() - p2.x()) + qAbs(p1.y() - p2.y());
}

inline static qint32 pointDistance(qint32 x1, qint32 y1, qint32 x2, qint32 y2) {
    return qAbs(x1 - x2) + qAbs(y1 - y2);
}


class MyTimer
{
private:
    // Type aliases to make accessing nested type easier
    using clock_t = std::chrono::high_resolution_clock;
    using second_t = std::chrono::duration<double, std::ratio<1>>;

    std::chrono::time_point<clock_t> m_beg;

public:
    MyTimer();

    void reset();

    double elapsed() const;
};

//this is just for manually flushing the console since doesn't seem to have a method for it
inline void flushConsole(Console::eLogLevels ll) {
    for(qint32 i=0; i<20; i++) {
        Console::print("--flushing log--", ll);
    }
}

}

#endif // AIUTILSFUNCTIONS_H
