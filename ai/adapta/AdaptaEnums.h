#ifndef ADAPTAENUMS_H
#define ADAPTAENUMS_H

#include <QString>
#include "game/unit.h"
#include "adaptamodule.h"
#include "buildingmodule.h"
#include "ai/adaptaai.h"

namespace adaenums {

/**
 * @brief The iMapType enum encodes if an influence map is of a custom or standard type.
 * Custom type means that the weights assigned to each unit are custom, but the propagation function depends on the custom type.
 * Standard type means that the weights and the function depend on the type. Basically a STD_XXX type defines a heuristic
 * influence map (which can still be weighted)
 * ONCE_XXX types are types which are ok if just evaluated once and don't need to be reevaluated each turn
 */
enum iMapType {
    NONE = 0,
    STD_ATTACK, //the influence of attack all units have
    STD_DAMAGE, //the influence of potential damage done to all enemy units in field
    STD_VALUE, //the general influence based on units' values

    //ONCE_MAPDEFENSE MUST BE THE FIRST ONCE TYPE!
    ONCE_MAPDEFENSE, //the influence given by each single tile's terrain/building (local to each tile, not propagated)

    //CUSTOM 1 MUST BE THE FIRST CUSTOM TYPE
    CUSTOM_1, //custom 1 uses attack type propagation, but gives custom weights.
    CUSTOM_ALLIES, //custom allies propagates positive weights only for allies
    CUSTOM_ENEMIES, //custom enemies propagates negative weights only for enemies
};


enum propagationType {
    STEP1, //propagate only where the unit can walk to in 1 turn
    STEP2, //propagate where the unit can walk to in 1 turn at max weight, then in 2 turns at half weight
    STEP3, //same in 3 turns
    ATTACK1, //propagate where the unit can attack to in 1 turn
    ATTACK2, //same in 2 turns, on tiles attackable on 2nd turn do half influence
    ATTACK3, //etc, on tiles reachable in 3 turns do 1/4th of influence
    DEFENSE_GENERAL, //
};

/**
 * @brief The evaluationType enum indicates how the training manager will evaluate the performances of an AI, returning a
 * fitness value. This eval type is used to calculate both the fitness of only 1 match and the final fitness based on all
 * partial fitnesses calculated after each match
 */
enum evalType {
    VICTORY_COUNT_ONLY,
    PLAYER_VALUE_PREDEPLOYED,
};

inline bool isCustomType(iMapType type) {
    return type >= CUSTOM_1;
}

inline bool isOnceType(iMapType type) {
    return type < CUSTOM_1 && type >= ONCE_MAPDEFENSE;
}

inline bool isStdType(iMapType type) {
    return type < ONCE_MAPDEFENSE;
}

/**
 * @brief get the iMapType from a string, the specified default type if it's not recognized
 * (NONE if the default is not specified)
 */
iMapType getInfluenceMapTypeFromString(QString name, iMapType defaultType = iMapType::NONE);

/**
 * @brief each iMapType represent how a influence map should propagate and weights (this latter only if it's not a standard map)
 * its
 */
float getUnitWeightForUnitFromIMapType(Unit* pReferenceUnit, iMapType type, Unit* pTargetUnit);

/**
 * @brief get the QString name of a iMapType
 */
QString iMapTypeToQString(iMapType type);

/**
 * @brief get eval type based on string, if it's equal to an evalType's name. Eventually set a default evalType to return
 * (VICTORY_COUNT_ONLY is default if not specified)
 */
evalType getEvalTypeFromString(QString eTypeString, evalType defaultType = evalType::VICTORY_COUNT_ONLY);

/**
 * @brief generates and return a new AdaptaModule for the specified adaptaAI (required not null), based on the passed moduleID.
 * A module ID should be specified by the module class itself.
 * If the moduleID is not recognized among the valid ones, returns a sp with nullptr
 */
spAdaptaModule generateAdaptaModuleFromString(QString moduleID, AdaptaAI* pAdaptaAI);


/**
 * @brief generates and return a new BuildingModule for the specified adaptaAI (required not null), based on the passed moduleID.
 * A module ID should be specified by the module class itself.
 * If the moduleID is not recognized among the valid ones, returns a sp with nullptr
 */
spBuildingModule generateBuildingModuleFromString(QString moduleID, AdaptaAI* pAdaptaAI);


/**
 * @brief get the fitness of this match for player #playerIndex
 */
float calculatePartialFitnessFromType(evalType eType, qint32 playerIndex);


/**
 * @brief return the final fitness of player index based on the type of evaluation
 */
float calculateFinalFitnessFromType(evalType eType, std::vector<float> partialFitnesses);


float calculateFitnessVictoryCountOnlyPartial(qint32 playerIndex);
float calculateFitnessPlayerValuePredeployedPartial(qint32 playerIndex);

float calculateFinalFitnessAccumulate(std::vector<float> partialFitnesses);
}




#endif // ADAPTAENUMS_H
