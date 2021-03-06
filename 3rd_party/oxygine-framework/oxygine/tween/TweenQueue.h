#pragma once
#include "3rd_party/oxygine-framework/oxygine/oxygine-include.h"
#include "3rd_party/oxygine-framework/oxygine/tween/Tween.h"

namespace oxygine
{
    DECLARE_SMART(TweenQueue, spTweenQueue);
    class TweenQueue : public Tween
    {
    public:

        enum { EVENT_LOOP_BEGIN = sysEventID('T', 'L', 'B'), };
        enum { EVENT_LOOP_END = sysEventID('T', 'L', 'E'), };

        TweenQueue();

        /**short syntax for easy tween queue creation*/
        static spTweenQueue create(spTween t1);

        /**short syntax for easy tween queue creation*/
        static spTweenQueue create(spTween t1, spTween t2);

        /**short syntax for easy tween queue creation*/
        static spTweenQueue create(spTween t1, spTween t2, spTween t3);

        /**short syntax for easy tween queue creation*/
        static spTweenQueue create(spTween t1, spTween t2, spTween t3, spTween t4);

        /**short syntax for easy tween queue creation*/
        static spTweenQueue create(spTween t1, spTween t2, spTween t3, spTween t4, spTween t5);

        /**short syntax for easy tween queue creation*/
        static spTweenQueue create(spTween t1, spTween t2, spTween t3, spTween t4, spTween t5, spTween t6);

        /**short syntax for easy tween queue creation*/
        static spTweenQueue create(spTween t1, spTween t2, spTween t3, spTween t4, spTween t5, spTween t6, spTween t7);

        /*Add tween to Queue, returns added tween*/
        spTween add(spTween t);

        template<class GS>
        spTween add(const GS& gs, timeMS duration, qint32 loops = 1, bool twoSides = false, timeMS delay = timeMS(0), Tween::EASE ease = Tween::ease_linear);

        template<class GS>
        spTween add(const GS& gs, const TweenOptions&);

        void complete(timeMS deltaTime);

    private:
        void _start(Actor& actor);
        void _update(Actor& actor, const UpdateState& us);

    private:
        typedef intrusive_list<Tween> tweens;
        tweens m_tweens;
        spTween m_current;
        qint32 m_loopsDone;
    };


    template<class GS>
    spTween TweenQueue::add(const GS& gs, timeMS duration, qint32 loops, bool twoSides, timeMS delay, Tween::EASE ease)
    {
        return add(createTween(gs, duration, loops, twoSides, delay, ease));
    }

    template<class GS>
    spTween TweenQueue::add(const GS& gs, const TweenOptions& opt)
    {
        return add(createTween2(gs, opt));
    }
}
