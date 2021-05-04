#include "AdaptaEnums.h"

namespace adaenums {

iMapType getInfluenceMapTypeFromString(QString name, iMapType defaultType) {
    if(name == "NONE")
        return iMapType::NONE;
    if(name == "STD_ATTACK")
        return iMapType::STD_ATTACK;
    if(name == "STD_DAMAGE")
        return iMapType::STD_DAMAGE;
    if(name == "STD_VALUE")
        return iMapType::STD_VALUE;
    if(name == "STD_MAPDEFENSE")
        return iMapType::STD_MAPDEFENSE;
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

QString iMapTypeToQString(iMapType type) {
    switch(type) {
    case NONE:
        return "NONE";
    case STD_ATTACK:
        return "STD_ATTACK";
    case STD_DAMAGE:
        return "STD_DAMAGE";
    case STD_VALUE:
        return "STD_VALUE";
    case STD_MAPDEFENSE:
        return "STD_MAPDEFENSE";
    case CUSTOM_1:
        return "CUSTOM_1";
    default:
        return "UNK_MAP_TYPE";
    }
}

}
