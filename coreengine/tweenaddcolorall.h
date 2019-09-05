#ifndef TWEENADDCOLORALL_H
#define TWEENADDCOLORALL_H

#include "oxygine-framework.h"


class TweenAddColorAll
{
public:
    typedef oxygine::VStyleActor type;
    TweenAddColorAll(oxygine::Color addColor);

    void update(oxygine::VStyleActor& actor, float p, const oxygine::UpdateState& us);

    void init(oxygine::VStyleActor& actor){}
    void done(oxygine::VStyleActor& actor){}
protected:
    oxygine::Color m_addColor;
};

#endif // TWEENADDCOLORALL_H