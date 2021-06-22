#ifndef BEHAVIORALMODULE_H
#define BEHAVIORALMODULE_H

#include "adaptamodule.h"
#include <QObject>

/**
 * @brief BehavioralModule is a module which implements just a behavioral AI. It will be basically a reimplementation of the
 * existing normal AI by Robosturm, maybe with some changes (credits to him in any case).
 * This module should be used so that the Adapta AI has at least an action to do with each unit, if some units are not supported
 * by any other module
 */
class BehavioralModule : public AdaptaModule
{
    Q_OBJECT
public:
    BehavioralModule();
};

#endif // BEHAVIORALMODULE_H
