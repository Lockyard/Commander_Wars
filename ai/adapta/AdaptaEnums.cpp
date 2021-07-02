#include "AdaptaEnums.h"
#include "coreengine/console.h"
#include "game/gamemap.h"
#include "multiinfluencenetworkmodule.h"
#include "normalbehavioralmodule.h"
#include "normalbuildingmodule.h"

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
    if(name == "ONCE_MAPDEFENSE")
        return iMapType::ONCE_MAPDEFENSE;
    if(name == "CUSTOM_1")
        return iMapType::CUSTOM_1;
    if(name == "CUSTOM_ALLIES")
        return iMapType::CUSTOM_ALLIES;
    if(name == "CUSTOM_ENEMIES")
        return iMapType::CUSTOM_ENEMIES;

    return defaultType;
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
    case ONCE_MAPDEFENSE:
        return "ONCE_MAPDEFENSE";
    case CUSTOM_1:
        return "CUSTOM_1";
    case CUSTOM_ALLIES:
        return "CUSTOM_ALLIES";
    case CUSTOM_ENEMIES:
        return "CUSTOM_ENEMIES";
    default:
        return "UNKNOWN_MAP_TYPE";
    }
}

evalType getEvalTypeFromString(QString eTypeString, evalType defaultType) {
    if(eTypeString == "VICTORY_COUNT_ONLY")
        return evalType::VICTORY_COUNT_ONLY;
    if(eTypeString == "PLAYER_VALUE_PREDEPLOYED")
        return evalType::PLAYER_VALUE_PREDEPLOYED;
    return defaultType;
}

spAdaptaModule generateAdaptaModuleFromString(QString moduleID, AdaptaAI* pAdaptaAI) {
    if(moduleID == MultiInfluenceNetworkModule::MODULE_ID) {
        return spAdaptaModule(new MultiInfluenceNetworkModule(pAdaptaAI));
    } else if (moduleID == NormalBehavioralModule::MODULE_ID){
        return spAdaptaModule(new NormalBehavioralModule(pAdaptaAI));
    } else {
        Console::print("Trying to generate an AdaptaModule from ID, but ID '" + moduleID + "' does not match with any module ID!", Console::eWARNING);
        return spAdaptaModule(nullptr);
    }
}


spBuildingModule generateBuildingModuleFromString(QString moduleID, AdaptaAI* pAdaptaAI) {
    if(moduleID == NormalBuildingModule::MODULE_ID) {
        return spBuildingModule(new NormalBuildingModule(pAdaptaAI));
    } else {
        Console::print("Trying to generate a BuildingModule from ID, but ID '" + moduleID + "' does not match with any module ID!", Console::eWARNING);
        return spBuildingModule(nullptr);
    }
}


float calculatePartialFitnessFromType(evalType eType, qint32 playerIndex) {
    switch(eType) {
    case VICTORY_COUNT_ONLY:
        return calculateFitnessVictoryCountOnlyPartial(playerIndex);
    case PLAYER_VALUE_PREDEPLOYED:
        return calculateFitnessPlayerValuePredeployedPartial(playerIndex);
    default:
        return calculateFitnessVictoryCountOnlyPartial(playerIndex);
    }
}


/**
 * @brief return the final fitness of player index based on the type of evaluation
 */
float calculateFinalFitnessFromType(evalType eType, std::vector<float> partialFitnesses) {
    switch(eType) {
    case VICTORY_COUNT_ONLY:
    case PLAYER_VALUE_PREDEPLOYED:
        return calculateFinalFitnessAccumulate(partialFitnesses);
    default:
        return calculateFinalFitnessAccumulate(partialFitnesses);
    }
}

/**
 * @brief -1 if lost, +1 if won
 */
float calculateFitnessVictoryCountOnlyPartial(qint32 playerIndex) {
    GameMap* pMap = GameMap::getInstance();
    qint32 winnerTeam = pMap->getWinnerTeam();
    //evaluate
    if (winnerTeam >= 0 && playerIndex <= pMap->getPlayerCount()) {
        if (pMap->getPlayer(playerIndex)->getTeam() == winnerTeam){
            return 1;
        } else {
            return -1;
        }
    } else {
        if(winnerTeam >=0)
            Console::print("The trained player has an index greater than the number of players! Couldn't evaluate player!", Console::eWARNING);
        else {
            Console::print("Winner team is < 0! Can't evaluate trained player!", Console::eWARNING);
        }
    }
    return -1;
}


float calculateFitnessPlayerValuePredeployedPartial(qint32 playerIndex) {
    GameMap* pMap = GameMap::getInstance();
    //todo remove this! Is to check how the error happens
    if(pMap == nullptr)
        return 0.0f;
    qint32 winnerTeam = pMap->getWinnerTeam();
    //evaluate if winner team and player index are valid
    if (winnerTeam >= 0 && playerIndex <= pMap->getPlayerCount()) {
        DayToDayRecord* pDayRecordBegin = pMap->getGameRecorder()->getDayRecord(0);
        DayToDayRecord* pDayRecordEnd = pMap->getGameRecorder()->getDayRecord(pMap->getCurrentDay()-1);
        //if there's no record, return a safe value based just on victory
        if(pDayRecordBegin == nullptr || pDayRecordEnd == nullptr) {
            Console::print("Evaluation of AI for pre-deployed player value failed, no DayToDayRecord found to evaluate! Returning a value based just on victory", Console::eWARNING);
            return calculateFitnessVictoryCountOnlyPartial(playerIndex)*.2f; //return +/- 0.2 to be safe
        }

        //if the trainee won
        if(pMap->getPlayer(playerIndex)->getTeam() == winnerTeam) {
            //to calculate strength remove income since we only want army strength
            float playerStrengthStart = pDayRecordBegin->getPlayerRecord(playerIndex)->getPlayerStrength() - pDayRecordBegin->getPlayerRecord(playerIndex)->getIncome();
            float playerStrengthFinal = pDayRecordEnd->getPlayerRecord(playerIndex)->getPlayerStrength() - pDayRecordEnd->getPlayerRecord(playerIndex)->getIncome();
            if(playerStrengthStart == 0) {
                Console::print("Trainee has a strength of 0 at start of game! Can't evaluate properly! Evaluating with safe value!", Console::eWARNING);
                return calculateFitnessVictoryCountOnlyPartial(playerIndex)*.2f; //return +/- 0.2 to be safe
            } else {
                return qMin(playerStrengthFinal/playerStrengthStart, 1.0f);
            }
        }
        //if an opponent won
        else {
            for (qint32 i = 0; i < pMap->getPlayerCount(); i++)
            {
                if(pMap->getPlayer(i)->getTeam() == winnerTeam) {
                    float playerStrengthStart = pDayRecordBegin->getPlayerRecord(i)->getPlayerStrength() - pDayRecordBegin->getPlayerRecord(i)->getIncome();
                    float playerStrengthFinal = pDayRecordEnd->getPlayerRecord(i)->getPlayerStrength() - pDayRecordEnd->getPlayerRecord(i)->getIncome();
                    if(playerStrengthStart == 0) {
                        Console::print("Player #" + QString::number(i) + " has a strength of 0 at start of game! Can't evaluate properly! Evaluating with safe value!", Console::eWARNING);
                        return calculateFitnessVictoryCountOnlyPartial(playerIndex)*.2f; //return +/- 0.2 to be safe
                    } else {
                        return qMax(-playerStrengthFinal/playerStrengthStart, -1.0f);
                    }
                }
            }
        }
    } else {
        if(winnerTeam >=0)
            Console::print("The trained player has an index greater than the number of players! Couldn't evaluate player!", Console::eWARNING);
        else {
            Console::print("Winner team is < 0! Can't evaluate trained player!", Console::eWARNING);
        }
    }
    return -1;
}



float calculateFinalFitnessAccumulate(std::vector<float> partialFitnesses) {
    return std::accumulate(partialFitnesses.begin(), partialFitnesses.end(), 0.0f);
}

}
