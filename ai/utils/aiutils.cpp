#include "aiutils.h"
#include "game/gamemap.h"
#include <QtMath>

namespace aiutils {



//timer class
MyTimer::MyTimer() : m_beg(clock_t::now()){
}

void MyTimer::reset()
{
    m_beg = clock_t::now();
}

double MyTimer::elapsed() const
{
    return std::chrono::duration_cast<second_t>(clock_t::now() - m_beg).count();
}

}
