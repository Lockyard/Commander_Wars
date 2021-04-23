#include "AdaptaEnums.h"

namespace adaenums {

iMapType getInfluenceMapTypeFromString(QString name, iMapType defaultType) {
    if(name == "NONE")
        return iMapType::NONE;
    if(name == "STD_ATTACK")
        return iMapType::STD_ATTACK;
    if(name == "STD_DANGER")
        return iMapType::STD_DANGER;
    if(name == "STD_VALUE")
        return iMapType::STD_VALUE;
    if(name == "CUSTOM_1")
        return iMapType::CUSTOM_1;

    return defaultType;
}

bool isCustomType(iMapType type) {
    return type >= CUSTOM_1;
}

float getUnitWeightFromIMapType(Unit* pReferenceUnit, iMapType type, Unit* pTargetUnit) {
    switch(type) {
    case STD_ATTACK:
        //todo
    default:
        return 0;
    }
}

}
