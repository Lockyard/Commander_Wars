#ifndef ADAPTAENUMS_H
#define ADAPTAENUMS_H

#include <QString>
#include "game/unit.h"

namespace adaenums {

/**
 * @brief The iMapType enum encodes if an influence map is of a custom or standard type.
 * Custom type means that the weights assigned to each unit are custom, but the propagation function depends on the custom type.
 * Standard type means that the weights and the function depend on the type. Basically a STD_XXX type defines a heuristic
 * influence map (which can still be weighted)
 */
enum iMapType {
    NONE = 0,
    STD_ATTACK,
    STD_DANGER,
    STD_VALUE,
    CUSTOM_1
};

enum propagationType {
    STEP1, //propagate only where the unit can walk to in 1 turn
    STEP2, //propagate where the unit can walk to in 1 turn at max weight, then in 2 turns at half weight
    ATTACK1, //propagate where the unit can attack to in 1 turn
};

/**
 * @brief get the iMapType from a string, the specified default type if it's not recognized
 * (NONE if the default is not specified)
 */
iMapType getInfluenceMapTypeFromString(QString name, iMapType defaultType = iMapType::NONE);

bool isCustomType(iMapType type);

/**
 * @brief each iMapType represent how a influence map should propagate and weights (this latter only if it's not a standard map)
 * its
 */
float getUnitWeightForUnitFromIMapType(Unit* pReferenceUnit, iMapType type, Unit* pTargetUnit);
}

#endif // ADAPTAENUMS_H
