#include "normalbehavioralmodule.h"
#include <QSettings>

//CoreAI cpp includes
#include "ai/targetedunitpathfindingsystem.h"
#include "game/gameanimation/gameanimationfactory.h"
#include "game/gamemap.h"
#include "game/unit.h"
#include "game/gameaction.h"
#include "game/unitpathfindingsystem.h"
#include "game/player.h"
#include "game/co.h"
#include "menue/gamemenue.h"
#include "coreengine/mainapp.h"
#include "coreengine/console.h"
#include "coreengine/globalutils.h"
#include "resource_management/cospritemanager.h"
#include "resource_management/unitspritemanager.h"
#include "resource_management/buildingspritemanager.h"
#include <qfile.h>
//NormalAI cpp includes (not already in CoreAi cpp includes)
#include "coreengine/qmlvector.h"
#include "game/building.h"
#include "resource_management/weaponmanager.h"

//own static stuff
const QString NormalBehavioralModule::MODULE_ID = "NORMAL_BEHAVIORAL";

//coreAI static stuff
const QString NormalBehavioralModule::ACTION_WAIT = "ACTION_WAIT";
const QString NormalBehavioralModule::ACTION_HOELLIUM_WAIT = "ACTION_HOELLIUM_WAIT";
const QString NormalBehavioralModule::ACTION_SUPPORTSINGLE = "ACTION_SUPPORTSINGLE";
const QString NormalBehavioralModule::ACTION_SUPPORTSINGLE_REPAIR = "ACTION_SUPPORTSINGLE_REPAIR";
const QString NormalBehavioralModule::ACTION_SUPPORTSINGLE_FREEREPAIR = "ACTION_SUPPORTSINGLE_FREEREPAIR";
const QString NormalBehavioralModule::ACTION_SUPPORTALL = "ACTION_SUPPORTALL";
const QString NormalBehavioralModule::ACTION_SUPPORTALL_RATION = "ACTION_SUPPORTALL_RATION";
const QString NormalBehavioralModule::ACTION_UNSTEALTH = "ACTION_UNSTEALTH";
const QString NormalBehavioralModule::ACTION_STEALTH = "ACTION_STEALTH";
const QString NormalBehavioralModule::ACTION_BUILD_UNITS = "ACTION_BUILD_UNITS";
const QString NormalBehavioralModule::ACTION_CAPTURE = "ACTION_CAPTURE";
const QString NormalBehavioralModule::ACTION_MISSILE = "ACTION_MISSILE";
const QString NormalBehavioralModule::ACTION_PLACE = "ACTION_PLACE";
const QString NormalBehavioralModule::ACTION_FIRE = "ACTION_FIRE";
const QString NormalBehavioralModule::ACTION_UNLOAD = "ACTION_UNLOAD";
const QString NormalBehavioralModule::ACTION_LOAD = "ACTION_LOAD";
const QString NormalBehavioralModule::ACTION_NEXT_PLAYER = "ACTION_NEXT_PLAYER";
const QString NormalBehavioralModule::ACTION_SWAP_COS = "ACTION_SWAP_COS";
const QString NormalBehavioralModule::ACTION_ACTIVATE_TAGPOWER = "ACTION_ACTIVATE_TAGPOWER";
const QString NormalBehavioralModule::ACTION_ACTIVATE_POWER_CO_0 = "ACTION_ACTIVATE_POWER_CO_0";
const QString NormalBehavioralModule::ACTION_ACTIVATE_POWER_CO_1 = "ACTION_ACTIVATE_POWER_CO_1";
const QString NormalBehavioralModule::ACTION_ACTIVATE_SUPERPOWER_CO_0 = "ACTION_ACTIVATE_SUPERPOWER_CO_0";
const QString NormalBehavioralModule::ACTION_ACTIVATE_SUPERPOWER_CO_1 = "ACTION_ACTIVATE_SUPERPOWER_CO_1";
const QString NormalBehavioralModule::ACTION_CO_UNIT_0 = "ACTION_CO_UNIT_0";
const QString NormalBehavioralModule::ACTION_CO_UNIT_1 = "ACTION_CO_UNIT_1";
const QString NormalBehavioralModule::ACTION_EXPLODE = "ACTION_EXPLODE";
const QString NormalBehavioralModule::ACTION_FLARE = "ACTION_FLARE";


NormalBehavioralModule::NormalBehavioralModule(AdaptaAI* ai) : AdaptaModule(), m_pAdapta(ai), m_COPowerTree("resources/aidata/copower.tree", "resources/aidata/copower.txt")
{

}

NormalBehavioralModule::~NormalBehavioralModule() {
    Console::print("Deleting normal behavioral module!", Console::eDEBUG);
}

bool NormalBehavioralModule::readIni(QString filename) {

    if(QFile::exists(filename)) {
        QSettings moduleSettings(filename, QSettings::IniFormat);

        bool ok = false;

        moduleSettings.beginGroup("info");
        QString normalAIFilepath = moduleSettings.value("NormalAIFilePath", "resources/aidata/normal/normal.ini").toString();
        m_defaultBid = moduleSettings.value("DefaultBid", 0.1f).toFloat(&ok);
        if(!ok)
            m_defaultBid = 0.1f;
        moduleSettings.endGroup();

        if (QFile::exists(normalAIFilepath))
        {
            QSettings settings(normalAIFilepath, QSettings::IniFormat);

            settings.beginGroup("general");
            m_minMovementDamage = settings.value("MinMovementDamage", 0.3f).toFloat(&ok);
            if(!ok)
            {
                m_minMovementDamage = 0.3f;
            }
            m_minAttackFunds = settings.value("MinAttackFunds", 0).toFloat(&ok);
            if(!ok)
            {
                m_minAttackFunds = 0;
            }
            m_minSuicideDamage = settings.value("MinSuicideDamage", 0.75f).toFloat(&ok);
            if(!ok)
            {
                m_minSuicideDamage = 0.75f;
            }
            m_spamingFunds = settings.value("SpamingFunds", 7500).toFloat(&ok);
            if(!ok)
            {
                m_spamingFunds = 0;
            }
            m_ownUnitValue = settings.value("OwnUnitValue", 2.0f).toFloat(&ok);
            if(!ok)
            {
                m_ownUnitValue = 2.0f;
            }
            m_buildingValue = settings.value("BuildingValue", 1.0f).toFloat(&ok);
            if(!ok)
            {
                m_buildingValue = 1.0f;
            }
            m_notAttackableDamage = settings.value("NotAttackableDamage", 25).toFloat(&ok);
            if(!ok)
            {
                m_notAttackableDamage = 25;
            }
            m_midDamage = settings.value("MidDamage", 55).toFloat(&ok);
            if(!ok)
            {
                m_midDamage = 55;
            }
            m_highDamage = settings.value("HighDamage", 65).toFloat(&ok);
            if(!ok)
            {
                m_highDamage = 65;
            }
            m_directIndirectRatio = settings.value("DirectIndirectRatio", 1.75f).toFloat(&ok);
            if(!ok)
            {
                m_directIndirectRatio = 1.75f;
            }
            m_minSiloDamage = settings.value("MinSiloDamage", 7000).toFloat(&ok);
            if(!ok)
            {
                m_minSiloDamage = 7000;
            }
            settings.endGroup();
            settings.beginGroup("CoUnit");
            m_coUnitValue = settings.value("CoUnitValue", 6000).toInt(&ok);
            if(!ok)
            {
                m_coUnitValue = 6000;
            }
            m_minCoUnitScore = settings.value("MinCoUnitScore", 5000).toFloat(&ok);
            if(!ok)
            {
                m_minCoUnitScore = 5000;
            }
            m_coUnitRankReduction = settings.value("CoUnitRankReduction", 1000).toFloat(&ok);
            if(!ok)
            {
                m_coUnitRankReduction = 1000;
            }
            m_coUnitScoreMultiplier = settings.value("CoUnitScoreMultiplier", 1.1f).toFloat(&ok);
            if(!ok)
            {
                m_coUnitScoreMultiplier = 1.1f;
            }
            m_minCoUnitCount = settings.value("MinCoUnitCount", 1.1f).toInt(&ok);
            if(!ok)
            {
                m_minCoUnitCount = 5;
            }
            settings.endGroup();
            settings.beginGroup("Repairing");
            m_minUnitHealth = settings.value("MinUnitHealth", 3).toInt(&ok);
            if(!ok)
            {
                m_minUnitHealth = 3;
            }
            m_maxUnitHealth = settings.value("MaxUnitHealth", 7).toInt(&ok);
            if(!ok)
            {
                m_maxUnitHealth = 7;
            }
            m_fuelResupply = settings.value("FuelResupply", 0.33f).toFloat(&ok);
            if(!ok)
            {
                m_fuelResupply = 0.33f;
            }
            m_ammoResupply = settings.value("AmmoResupply", 0.25f).toFloat(&ok);
            if(!ok)
            {
                m_ammoResupply = 0.25f;
            }
            settings.endGroup();
            settings.beginGroup("Moving");
            m_lockedUnitHp = settings.value("LockedUnitHp", 4).toFloat(&ok);
            if(!ok)
            {
                m_lockedUnitHp = 4;
            }
            m_noMoveAttackHp = settings.value("NoMoveAttackHp", 3.5f).toFloat(&ok);
            if(!ok)
            {
                m_noMoveAttackHp = 3.5f;
            }
            settings.endGroup();
            settings.beginGroup("Attacking");
            m_ownIndirectAttackValue = settings.value("OwnIndirectAttackValue", 2.0f).toFloat(&ok);
            if(!ok)
            {
                m_ownIndirectAttackValue= 2.0f;
            }
            m_enemyKillBonus = settings.value("EnemyKillBonus", 2.0f).toFloat(&ok);
            if(!ok)
            {
                m_enemyKillBonus= 2.0f;
            }
            m_enemyIndirectBonus = settings.value("EnemyIndirectBonus", 3.0f).toFloat(&ok);
            if(!ok)
            {
                m_enemyIndirectBonus= 3.0f;
            }

            m_antiCaptureHqBonus = settings.value("AntiCaptureHqBonus", 50.0f).toFloat(&ok);
            if(!ok)
            {
                m_antiCaptureHqBonus= 50.0f;
            }
            m_antiCaptureBonus = settings.value("AntiCaptureBonus", 21.0f).toFloat(&ok);
            if(!ok)
            {
                m_antiCaptureBonus= 21.0f;
            }
            m_antiCaptureBonusScoreReduction = settings.value("AntiCaptureBonusScoreReduction", 6.0f).toFloat(&ok);
            if(!ok)
            {
                m_antiCaptureBonusScoreReduction= 6.0f;
            }
            m_antiCaptureBonusScoreReduction = settings.value("AntiCaptureBonusScoreDivider", 2.0f).toFloat(&ok);
            if(!ok || m_antiCaptureBonusScoreReduction < 0)
            {
                m_antiCaptureBonusScoreReduction = 2.0f;
            }
            m_enemyCounterDamageMultiplier = settings.value("EnemyCounterDamageMultiplier", 10.0f).toFloat(&ok);
            if(!ok)
            {
                m_enemyCounterDamageMultiplier = 10.0f;
            }
            m_watermineDamage = settings.value("WatermineDamage", 4.0f).toFloat(&ok);
            if(!ok)
            {
                m_watermineDamage = 4.0f;
            }
            m_enemyUnitCountDamageReductionMultiplier = settings.value("EnemyUnitCountDamageReductionMultiplier", 0.5f).toFloat(&ok);
            if(!ok)
            {
                m_enemyUnitCountDamageReductionMultiplier = 0.5f;
            }
            settings.endGroup();
            settings.beginGroup("Production");
            m_fundsPerBuildingFactorA = settings.value("FundsPerBuildingFactorA", 2.5f).toFloat(&ok);
            if(!ok)
            {
                m_fundsPerBuildingFactorA = 2.5f;
            }
            m_fundsPerBuildingFactorB = settings.value("FundsPerBuildingFactorB", 1.65f).toFloat(&ok);
            if(!ok)
            {
                m_fundsPerBuildingFactorB = 1.65f;
            }
            m_ownUnitEnemyUnitRatioAverager = settings.value("OwnUnitEnemyUnitRatioAverager", 10).toFloat(&ok);
            if(!ok)
            {
                m_ownUnitEnemyUnitRatioAverager = 10;
            }
            m_maxDayScoreVariancer = settings.value("MaxTransporterDayScoreVariancer", 10).toFloat(&ok);
            if(!ok)
            {
                m_maxDayScoreVariancer = 10;
            }
            m_directIndirectUnitBonusFactor = settings.value("DirectIndirectUnitBonusFactor", 1.2f).toFloat(&ok);
            if(!ok)
            {
                m_directIndirectUnitBonusFactor = 1.2f;
            }
            m_maxBuildingTargetFindLoops = settings.value("MaxBuildingTargetFindLoops", 5).toFloat(&ok);
            if(!ok)
            {
                m_maxBuildingTargetFindLoops = 5;
            }
            m_scoringCutOffDamageHigh = settings.value("ScoringCutOffDamageHigh", Unit::DAMAGE_100).toFloat(&ok);
            if(!ok)
            {
                m_scoringCutOffDamageHigh = Unit::DAMAGE_100;
            }
            m_scoringCutOffDamageLow = settings.value("ScoringCutOffDamageLow", 7.5f).toFloat(&ok);
            if(!ok)
            {
                m_scoringCutOffDamageLow = 7.5f;
            }
            m_smoothingValue = settings.value("SmoothingValue", 3).toFloat(&ok);
            if(!ok)
            {
                m_smoothingValue = 3;
            }
            m_maxDistanceMultiplier = settings.value("MaxDistanceMultiplier", 1.5f).toFloat(&ok);
            if(!ok)
            {
                m_maxDistanceMultiplier = 1.5f;
            }
            m_sameIslandBonusInRangeDays = settings.value("SameIslandBonusInRangeDays", 2).toFloat(&ok);
            if(!ok)
            {
                m_sameIslandBonusInRangeDays = 2;
            }
            m_sameIslandOutOfDayMalusFactor = settings.value("SameIslandOutOfDayMalusFactor", 0.2f).toFloat(&ok);
            if(!ok)
            {
                m_sameIslandOutOfDayMalusFactor = 0.2f;
            }
            m_highDamageBonus = settings.value("HighDamageBonus", 2).toFloat(&ok);
            if(!ok)
            {
                m_highDamageBonus = 2;
            }
            m_midDamageBonus = settings.value("MidDamageBonus", 1.5f).toFloat(&ok);
            if(!ok)
            {
                m_midDamageBonus = 1.5f;
            }

            m_lowDamageBonus = settings.value("LowDamageBonus", 1).toFloat(&ok);
            if(!ok)
            {
                m_lowDamageBonus = 1;
            }

            m_veryLowDamageBonus = settings.value("VeryLowDamageBonus", 0.5f).toFloat(&ok);
            if(!ok)
            {
                m_veryLowDamageBonus = 0.5f;
            }
            m_transportBonus = settings.value("TransportBonus", 0.125f).toFloat(&ok);
            if(!ok)
            {
                m_transportBonus = 0.125f;
            }
            m_currentlyNotAttackableBonus = settings.value("CurrentlyNotAttackableBonus", 0.5f).toFloat(&ok);
            if(!ok)
            {
                m_currentlyNotAttackableBonus = 0.5f;
            }
            m_differentIslandBonusInRangeDays = settings.value("DifferentIslandBonusInRangeDays", 1).toFloat(&ok);
            if(!ok)
            {
                m_differentIslandBonusInRangeDays = 1;
            }
            m_differentIslandOutOfDayMalusFactor = settings.value("DifferentIslandOutOfDayMalusFactor", 0.33f).toFloat(&ok);
            if(!ok)
            {
                m_differentIslandOutOfDayMalusFactor = 0.33f;
            }

            m_noTransporterBonus = settings.value("NoTransporterBonus", 70).toFloat(&ok);
            if(!ok)
            {
                m_noTransporterBonus = 70;
            }
            m_transporterToRequiredPlaceFactor = settings.value("TransporterToRequiredPlaceFactor", 3).toFloat(&ok);
            if(!ok)
            {
                m_transporterToRequiredPlaceFactor = 3;
            }

            m_minFlyingTransportScoreForBonus = settings.value("MinFlyingTransportScoreForBonus", 15).toFloat(&ok);
            if(!ok)
            {
                m_minFlyingTransportScoreForBonus = 15;
            }
            m_flyingTransporterBonus = settings.value("FlyingTransporterBonus", 15).toFloat(&ok);
            if(!ok)
            {
                m_flyingTransporterBonus = 15;
            }
            m_smallTransporterBonus = settings.value("SmallTransporterBonus", 30).toFloat(&ok);
            if(!ok)
            {
                m_smallTransporterBonus = 30;
            }
            m_unitToSmallTransporterRatio = settings.value("UnitToSmallTransporterRatio", 5).toFloat(&ok);
            if(!ok)
            {
                m_unitToSmallTransporterRatio = 5;
            }
            m_additionalLoadingUnitBonus = settings.value("AdditionalLoadingUnitBonus", 5).toFloat(&ok);
            if(!ok)
            {
                m_additionalLoadingUnitBonus = 5;
            }
            m_indirectUnitAttackCountMalus = settings.value("IndirectUnitAttackCountMalus", 4).toInt(&ok);
            if(!ok)
            {
                m_indirectUnitAttackCountMalus = 4;
            }
            m_minAttackCountBonus = settings.value("MinAttackCountBonus", 5).toFloat(&ok);
            if(!ok)
            {
                m_minAttackCountBonus = 5;
            }
            m_lowIndirectUnitBonus = settings.value("LowIndirectUnitBonus", 0.3f).toFloat(&ok);
            if(!ok)
            {
                m_lowIndirectUnitBonus = 0.3f;
            }
            m_lowIndirectMalus = settings.value("LowIndirectMalus", 0.5f).toFloat(&ok);
            if(!ok)
            {
                m_lowIndirectMalus = 0.5f;
            }
            m_highIndirectMalus = settings.value("HighIndirectMalus", 0.6f).toFloat(&ok);
            if(!ok)
            {
                m_highIndirectMalus = 0.6f;
            }
            m_lowDirectUnitBonus = settings.value("LowDirectUnitBonus", 0.35f).toFloat(&ok);
            if(!ok)
            {
                m_lowDirectUnitBonus = 0.35f;
            }
            m_lowDirectMalus = settings.value("LowDirectMalus", 0.3f).toFloat(&ok);
            if(!ok)
            {
                m_lowDirectMalus = 0.3f;
            }
            m_highDirectMalus = settings.value("HighDirectMalus", 0.6f).toFloat(&ok);
            if(!ok)
            {
                m_highDirectMalus = 0.6f;
            }

            m_minUnitCountForDamageBonus = settings.value("MinUnitCountForDamageBonus", 3).toFloat(&ok);
            if(!ok)
            {
                m_minUnitCountForDamageBonus = 3;
            }
            m_currentlyNotAttackableScoreBonus = settings.value("CurrentlyNotAttackableScoreBonus", 30).toFloat(&ok);
            if(!ok)
            {
                m_currentlyNotAttackableScoreBonus = 30;
            }
            m_coUnitBuffBonus = settings.value("CoUnitBuffBonus", 17).toFloat(&ok);
            if(!ok)
            {
                m_coUnitBuffBonus = 17;
            }
            m_nearEnemyBonus = settings.value("NearEnemyBonus", 10).toFloat(&ok);
            if(!ok)
            {
                m_nearEnemyBonus = 10;
            }
            m_movementpointBonus = settings.value("MovementpointBonus", 0.33f).toFloat(&ok);
            if(!ok)
            {
                m_movementpointBonus = 0.33f;
            }
            m_damageToUnitCostRatioBonus = settings.value("DamageToUnitCostRatioBonus", 20).toFloat(&ok);
            if(!ok)
            {
                m_damageToUnitCostRatioBonus = 20;
            }
            m_superiorityRatio = settings.value("SuperiorityRatio", 2.5f).toFloat(&ok);
            if(!ok)
            {
                m_superiorityRatio = 2.5f;
            }
            m_cheapUnitRatio = settings.value("CheapUnitRatio", 0.9f).toFloat(&ok);
            if(!ok)
            {
                m_cheapUnitRatio = 0.9f;
            }
            m_cheapUnitBonusMultiplier = settings.value("CheapUnitBonusMultiplier", 40).toFloat(&ok);
            if(!ok)
            {
                m_cheapUnitBonusMultiplier = 40;
            }
            m_normalUnitBonusMultiplier = settings.value("NormalUnitBonusMultiplier", 10).toFloat(&ok);
            if(!ok)
            {
                m_normalUnitBonusMultiplier = 10;
            }
            m_damageToUnitCostRatioBonus = settings.value("ExpensiveUnitBonusMultiplier", 5).toFloat(&ok);
            if(!ok)
            {
                m_damageToUnitCostRatioBonus = 5;
            }

            m_lowOwnBuildingEnemyBuildingRatio = settings.value("LowOwnBuildingEnemyBuildingRatio", 1.25f).toFloat(&ok);
            if(!ok)
            {
                m_lowOwnBuildingEnemyBuildingRatio = 1.25f;
            }
            m_lowInfantryRatio = settings.value("LowInfantryRatio", 0.4f).toFloat(&ok);
            if(!ok)
            {
                m_lowInfantryRatio = 0.4f;
            }
            m_lowIncomeInfantryBonusMultiplier = settings.value("LowIncomeInfantryBonusMultiplier", 50).toFloat(&ok);
            if(!ok)
            {
                m_lowIncomeInfantryBonusMultiplier = 50;
            }

            m_ProducingTransportSearchrange = settings.value("ProducingTransportSearchrange", 6).toFloat(&ok);
            if(!ok)
            {
                m_ProducingTransportSearchrange = 6;
            }
            m_ProducingTransportSizeBonus = settings.value("ProducingTransportSizeBonus", 10).toFloat(&ok);
            if(!ok)
            {
                m_ProducingTransportSizeBonus = 10;
            }
            m_ProducingTransportRatioBonus = settings.value("ProducingTransportRatioBonus", 10.0f).toFloat(&ok);
            if(!ok)
            {
                m_ProducingTransportRatioBonus = 10.0f;
            }
            m_ProducingTransportLoadingBonus = settings.value("ProducingTransportLoadingBonus", 15.0f).toFloat(&ok);
            if(!ok)
            {
                m_ProducingTransportLoadingBonus = 15.0f;
            }
            m_ProducingTransportMinLoadingTransportRatio = settings.value("ProducingTransportMinLoadingTransportRatio", 3.0f).toFloat(&ok);
            if(!ok)
            {
                m_ProducingTransportMinLoadingTransportRatio = 3.0f;
            }
            settings.endGroup();

            return true;
        }
    }

    return false;
}

void NormalBehavioralModule::init(Player *pPlayer) {
    if(m_pPlayer == nullptr && pPlayer != nullptr) {
        //AdaptaModule init
        AdaptaModule::init(pPlayer);
        //CoreAI init stuff
        spGameMap pMap = GameMap::getInstance();
        qint32 heigth = pMap->getMapHeight();
        qint32 width = pMap->getMapWidth();
        for (qint32 x = 0; x < width; x++)
        {
            m_MoveCostMap.append(QVector<std::tuple<qint32, bool>>());
            for (qint32 y = 0; y < heigth; y++)
            {
                m_MoveCostMap[x].append(std::tuple<qint32, bool>(0, false));
            }
        }
    } else {
        Console::print("NormalBehavioral module was initializated but m_pPlayer is already set or the passed pPlayer is nullptr!", Console::eWARNING);
    }
}

void NormalBehavioralModule::processStartOfTurn() {
    turnMode = GameEnums::AiTurnMode_StartOfDay;
}

bool NormalBehavioralModule::processHighestBidUnit() {
    spQmlVectorBuilding pBuildings = m_pPlayer->getBuildings();
    pBuildings->randomize();
    spQmlVectorUnit pUnits = nullptr;
    spQmlVectorUnit pEnemyUnits = nullptr;
    spQmlVectorBuilding pEnemyBuildings = nullptr;
    qint32 cost = 0;
    m_pPlayer->getSiloRockettarget(2, 3, cost);
    m_missileTarget = (cost >= m_minSiloDamage);

    if (useBuilding(pBuildings)){}
    else
    {
        pUnits = m_pPlayer->getUnits();
        pUnits->sortShortestMovementRange(true);
        pEnemyUnits = m_pPlayer->getEnemyUnits();
        pEnemyUnits->randomize();
        pEnemyBuildings = m_pPlayer->getEnemyBuildings();
        pEnemyBuildings->randomize();
        updateEnemyData(pUnits);
        if (useCOPower(pUnits, pEnemyUnits))
        {
            clearEnemyData();
        }
        else
        {
            turnMode = GameEnums::AiTurnMode_DuringDay;
            if (performActionSteps(pUnits, pEnemyUnits, pBuildings, pEnemyBuildings)){}
            else
            {
                if (aiStep == NB_AISteps::buildUnits)
                {
                    aiStep = NB_AISteps::moveUnits;
                }
                if (performActionSteps(pUnits, pEnemyUnits, pBuildings, pEnemyBuildings)){}
                else
                {
                    if (aiStep == NB_AISteps::buildUnits)
                    {
                        aiStep = NB_AISteps::moveUnits;
                    }
                    clearEnemyData();
                    m_IslandMaps.clear();
                    turnMode = GameEnums::AiTurnMode_EndOfDay;
                    if (useCOPower(pUnits, pEnemyUnits))
                    {
                        usedTransportSystem = false;
                        turnMode = GameEnums::AiTurnMode_DuringDay;
                    }
                    else
                    {
                        turnMode = GameEnums::AiTurnMode_StartOfDay;
                        //instead of finishing turn, return false since no action was taken
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool NormalBehavioralModule::processUnit(Unit* pUnit) {
    //to suppress warning
    if(pUnit == nullptr) return false;
    return false;
}

void NormalBehavioralModule::notifyUnitUsed(Unit *pUnit) {
    //to suppress warning
    if(pUnit == nullptr) return;
}

float NormalBehavioralModule::getBidFor(Unit *pUnit) {
    if(pUnit != nullptr) {
        if(pUnit->getHasMoved())
            return 0.0f;
    }
    return m_defaultBid;
}

float NormalBehavioralModule::getHighestBid(bool weighted) {
    if(getHighestBidUnit() == nullptr)
        return 0.0f;
    if(weighted)
        return m_defaultBid * m_moduleWeight;
    else
        return m_defaultBid;
}

Unit* NormalBehavioralModule::getHighestBidUnit() {
    spQmlVectorUnit spUnits = m_pPlayer->getUnits();
    for(qint32 i=0; i<spUnits->size(); i++) {
        if(!(spUnits->at(i)->getHasMoved()))
            return spUnits->at(i);
    }
    return nullptr;
}

//CoreAI Methods
bool NormalBehavioralModule::contains(QVector<QVector3D>& points, QPoint point)
{
    for (qint32 i = 0; i < points.size(); i++)
    {
        if (static_cast<qint32>(points[i].x()) == point.x() &&
            static_cast<qint32>(points[i].y()) == point.y())
        {
            return true;
        }
    }
    return false;
}

qint32 NormalBehavioralModule::index(QVector<QVector3D>& points, QPoint point)
{
    for (qint32 i = 0; i < points.size(); i++)
    {
        if (static_cast<qint32>(points[i].x()) == point.x() &&
            static_cast<qint32>(points[i].y()) == point.y())
        {
            return i;
        }
    }
    return -1;
}

bool NormalBehavioralModule::useCOPower(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits)
{
    Console::print("NormalBehavioralModule::useCOPower()", Console::eDEBUG);
    QVector<float> data;
    data.append(-1);
    data.append(0);
    data.append(-1);
    data.append(pUnits->size());
    qint32 repairUnits = 0;
    qint32 indirectUnits = 0;
    qint32 directUnits = 0;
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        if (pUnit->getHpRounded() < Unit::MAX_UNIT_HP)
        {
            repairUnits++;
        }
        if (pUnit->getBaseMaxRange() > 1)
        {
            indirectUnits++;
        }
        else
        {
            directUnits++;
        }
    }
    data.append(repairUnits);
    data.append(indirectUnits);
    data.append(directUnits);
    data.append(pEnemyUnits->size());
    data.append(m_pPlayer->getFunds());
    data.append(static_cast<float>(turnMode));

    for (quint8 i = 0; i <= 1; i++)
    {
        CO* pCO = m_pPlayer->getCO(i);
        if (pCO != nullptr)
        {
            data[0] = COSpriteManager::getInstance()->getIndex(pCO->getCoID());
            if (pCO->canUseSuperpower())
            {
                data[1] = 2;
            }
            else if (pCO->canUsePower())
            {
                data[1] = 1;
            }
            else
            {
                data[1] = 0;
            }
            data[2] = pCO->getPowerFilled() - pCO->getPowerStars();



            GameEnums::PowerMode result = pCO->getAiUsePower(data[2], pUnits->size(), repairUnits, indirectUnits,
                    directUnits, pEnemyUnits->size(), turnMode);
            if (result == GameEnums::PowerMode_Unknown)
            {
                result = static_cast<GameEnums::PowerMode>(m_COPowerTree.getDecision(data));
            }

            if (result == GameEnums::PowerMode_Power)
            {
                spGameAction pAction = spGameAction::create(ACTION_ACTIVATE_POWER_CO_0);
                if (i == 1)
                {
                    pAction->setActionID(ACTION_ACTIVATE_POWER_CO_1);
                }
                if (pAction->canBePerformed())
                {
                    emit m_pAdapta->performAction(pAction);
                    return true;
                }
            }
            else if (result == GameEnums::PowerMode_Superpower)
            {
                spGameAction pAction = spGameAction::create(ACTION_ACTIVATE_SUPERPOWER_CO_0);
                if (i == 1)
                {
                    pAction->setActionID(ACTION_ACTIVATE_SUPERPOWER_CO_1);
                }
                if (pAction->canBePerformed())
                {
                    pAction->setActionID(ACTION_ACTIVATE_TAGPOWER);
                    if (pAction->canBePerformed())
                    {
                        emit m_pAdapta->performAction(pAction);
                        return true;
                    }
                    else if (i == 1)
                    {
                        pAction->setActionID(ACTION_ACTIVATE_SUPERPOWER_CO_1);
                    }
                    else
                    {
                        pAction->setActionID(ACTION_ACTIVATE_SUPERPOWER_CO_0);
                    }
                    emit m_pAdapta->performAction(pAction);
                    return true;
                }
            }
        }
    }
    return false;
}

float NormalBehavioralModule::calcBuildingDamage(Unit* pUnit, QPoint newPosition, Building* pBuilding)
{
    float counterDamage = 0.0f;
    GameEnums::BuildingTarget targets = pBuilding->getBuildingTargets();
    if (targets == GameEnums::BuildingTarget_All ||
        (targets == GameEnums::BuildingTarget_Enemy && m_pPlayer->isEnemy(pBuilding->getOwner())) ||
        (targets == GameEnums::BuildingTarget_Own && m_pPlayer == pBuilding->getOwner()))
    {
        if (pBuilding->getFireCount() <= 1 &&
            pBuilding->getOwner() != nullptr)
        {
            QPoint pos = newPosition - pBuilding->getActionTargetOffset() - pBuilding->getPosition();
            spQmlVectorPoint pTargets = pBuilding->getActionTargetFields();
            if (pTargets.get() != nullptr)
            {
                if (pTargets->contains(pos))
                {
                    float damage = pBuilding->getDamage(pUnit);
                    if (damage > pUnit->getHp())
                    {
                        damage = pBuilding->getHp();
                    }
                    counterDamage = damage / Unit::MAX_UNIT_HP * pUnit->getUnitCosts();
                }
            }
        }
    }
    return counterDamage;
}

void NormalBehavioralModule::createMovementMap(spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    Console::print("NormalBehavioralModule::createMovementMap", Console::eDEBUG);
    spGameMap pMap = GameMap::getInstance();
    qint32 heigth = pMap->getMapHeight();
    qint32 width = pMap->getMapWidth();
    for (qint32 x = 0; x < width; x++)
    {
        for (qint32 y = 0; y < heigth; y++)
        {
            if (std::get<1>(m_MoveCostMap[x][y]) == false)
            {
                std::get<0>(m_MoveCostMap[x][y]) = 0.0f;
            }
        }
    }
    for (qint32 i = 0; i < pBuildings->size(); i++)
    {
        Building* pBuilding = pBuildings->at(i);
        float damage = pBuilding->getDamage(nullptr);
        addMovementMap(pBuilding, damage);
    }
    for (qint32 i = 0; i < pEnemyBuildings->size(); i++)
    {
        Building* pBuilding = pEnemyBuildings->at(i);
        if (pBuilding->getOwner() != nullptr)
        {
            float damage = pBuilding->getDamage(nullptr);
            addMovementMap(pBuilding, damage);
        }
    }
}

void NormalBehavioralModule::addMovementMap(Building* pBuilding, float damage)
{
    GameEnums::BuildingTarget targets = pBuilding->getBuildingTargets();
    QPoint offset = pBuilding->getPosition() + pBuilding->getActionTargetOffset() ;
    if (targets == GameEnums::BuildingTarget_All ||
        (targets == GameEnums::BuildingTarget_Enemy && m_pPlayer->isEnemy(pBuilding->getOwner())))
    {
        if (pBuilding->getFireCount() <= 1)
        {
            spQmlVectorPoint pTargets = pBuilding->getActionTargetFields();
            if (pTargets.get() != nullptr)
            {
                for (qint32 i2 = 0; i2 < pTargets->size(); i2++)
                {
                    QPoint point = pTargets->at(i2) + offset;
                    if ((m_MoveCostMap.size() > point.x() && point.x() >= 0) &&
                        (m_MoveCostMap[point.x()].size() > point.y() && point.y() >= 0))
                    {
                        if (std::get<1>(m_MoveCostMap[point.x()][point.y()]) == false)
                        {
                            std::get<0>(m_MoveCostMap[point.x()][point.y()]) += damage;
                        }
                    }
                }
            }
        }
    }
}

bool NormalBehavioralModule::useBuilding(spQmlVectorBuilding pBuildings)
{
    Console::print("NormalBehavioralModule::useBuilding", Console::eDEBUG);
    spGameMap pMap = GameMap::getInstance();
    for (qint32 i = 0; i < pBuildings->size(); i++)
    {
        Building* pBuilding = pBuildings->at(i);
        QStringList actions = pBuilding->getActionList();
        if (actions.size() >= 1 &&
            !actions[0].isEmpty())
        {
            for (qint32 i = 0; i < actions.size(); i++)
            {
                if (actions[i] != ACTION_BUILD_UNITS)
                {
                    spGameAction pAction = spGameAction::create(actions[i]);
                    pAction->setTarget(QPoint(pBuilding->Building::getX(), pBuilding->Building::getY()));
                    if (pAction->canBePerformed())
                    {
                        if (pAction->isFinalStep())
                        {
                            emit m_pAdapta->performAction(pAction);
                            return true;
                        }
                        else
                        {
                            while (!pAction->isFinalStep())
                            {
                                if (pAction->getStepInputType() == "FIELD")
                                {
                                    spMarkedFieldData pData = pAction->getMarkedFieldStepData();
                                    QVector<QPoint>* points = pData->getPoints();
                                    qint32 index = -1;
                                    QPoint target;
                                    qint32 maxValue = std::numeric_limits<qint32>::lowest();
                                    for (qint32 i2 = 0; i2 < points->size(); i2++)
                                    {
                                        Unit* pUnit = pMap->getTerrain(points->at(i2).x(), points->at(i2).y())->getUnit();
                                        qint32 unitValue = pUnit->getUnitValue();
                                        if (pUnit != nullptr && unitValue > maxValue)
                                        {
                                            maxValue = unitValue;
                                            index = i2;
                                        }
                                    }
                                    if (index < 0)
                                    {
                                        target = points->at(GlobalUtils::randIntBase(0, points->size() -1));
                                    }
                                    else
                                    {
                                        target = points->at(index);
                                    }
                                    addSelectedFieldData(pAction, target);
                                }
                                else if (pAction->getStepInputType() == "MENU")
                                {
                                    spMenuData pData = pAction->getMenuStepData();
                                    if (pData->validData())
                                    {
                                        QStringList items = pData->getActionIDs();
                                        auto enable = pData->getEnabledList();
                                        qint32 i = 0;
                                        while (i < enable.size())
                                        {
                                            if (enable[i])
                                            {
                                                i++;
                                            }
                                            else
                                            {
                                                items.removeAt(i);
                                                enable.removeAt(i);
                                            }
                                        }
                                        qint32 selection = GlobalUtils::randIntBase(0, items.size() - 1);
                                        addMenuItemData(pAction, items[selection], pData->getCostList()[selection]);
                                    }
                                }
                                else
                                {
                                    break;
                                }
                            }
                            if (pAction->isFinalStep())
                            {
                                emit m_pAdapta->performAction(pAction);
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::moveOoziums(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits)
{
    Console::print("moveOoziums()", Console::eDEBUG);
    QVector<QVector3D> targets;
    for (qint32 i = 0; i < pEnemyUnits->size(); i++)
    {
        Unit* pUnit = pEnemyUnits->at(i);
        targets.append(QVector3D(pUnit->Unit::getX(), pUnit->Unit::getY(), 1));
    }

    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        if (!pUnit->getHasMoved())
        {
            if (pUnit->getActionList().contains(ACTION_HOELLIUM_WAIT))
            {
                TargetedUnitPathFindingSystem pfs(pUnit, targets, &m_MoveCostMap);
                pfs.explore();
                qint32 movepoints = pUnit->getMovementpoints(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                QPoint targetFields = pfs.getReachableTargetField(movepoints);
                if (targetFields.x() >= 0)
                {
                    UnitPathFindingSystem turnPfs(pUnit);
                    turnPfs.explore();
                    spGameAction pAction = spGameAction::create(ACTION_HOELLIUM_WAIT);
                    pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                    QVector<QPoint> path = turnPfs.getClosestReachableMovePath(targetFields);
                    pAction->setMovepath(path, turnPfs.getCosts(path));
                    if (pAction->canBePerformed())
                    {
                        emit m_pAdapta->performAction(pAction);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::moveFlares(spQmlVectorUnit pUnits)
{
    Console::print("moveFlares()", Console::eDEBUG);
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        if (!pUnit->getHasMoved())
        {
            if (pUnit->getActionList().contains(ACTION_FLARE))
            {
                UnitPathFindingSystem turnPfs(pUnit);
                turnPfs.explore();
                spGameAction pAction = spGameAction::create(ACTION_FLARE);
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                QPoint moveTarget;
                QPoint flareTarget;
                getBestFlareTarget(pUnit, pAction, &turnPfs, flareTarget, moveTarget);
                // found something?
                if (moveTarget.x() >= 0)
                {
                    QVector<QPoint> path = turnPfs.getPath(moveTarget.x(), moveTarget.y());
                    pAction->setMovepath(path, turnPfs.getCosts(path));
                    if (pAction->canBePerformed())
                    {
                        addSelectedFieldData(pAction, flareTarget);
                        emit m_pAdapta->performAction(pAction);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::moveBlackBombs(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits)
{
    Console::print("moveBlackBombs()", Console::eDEBUG);
    spGameMap pMap = GameMap::getInstance();
    QVector<QVector3D> enemyTargets;
    spQmlVectorPoint enemyFields = GlobalUtils::getCircle(1, 1);
    for (qint32 i = 0; i < pEnemyUnits->size(); i++)
    {
        Unit* pUnit = pEnemyUnits->at(i);
        for (qint32 i2 = 0; i2 < enemyFields->size(); i2++)
        {
            if (pMap->onMap(pUnit->Unit::getX() + enemyFields->at(i2).x(), pUnit->Unit::getY() + enemyFields->at(i2).y()))
            {
                QVector3D point = QVector3D(pUnit->Unit::getX() + enemyFields->at(i2).x(), pUnit->Unit::getY() + enemyFields->at(i2).y(), 1);
                if (!enemyTargets.contains(point))
                {
                    enemyTargets.append(point);
                }
            }
        }
    }
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        if (!pUnit->getHasMoved())
        {
            if (pUnit->getActionList().contains(ACTION_EXPLODE))
            {
                UnitPathFindingSystem turnPfs(pUnit);
                turnPfs.explore();
                spQmlVectorPoint pPoints = GlobalUtils::getCircle(1, 3);
                QVector<QPoint> targets = turnPfs.getAllNodePoints();
                qint32 maxDamage = 0;
                QVector<QPoint> bestTargets;
                spGameAction pAction = spGameAction::create(ACTION_EXPLODE);
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                for (qint32 i2 = 0; i2 < targets.size(); i2++)
                {
                    qint32 damageDone = m_pPlayer->getRocketTargetDamage(targets[i2].x(), targets[i2].y(), pPoints.get(), 5, 1.2f, GameEnums::RocketTarget_Money, true);
                    if (damageDone > maxDamage)
                    {
                        bestTargets.clear();
                        bestTargets.append(targets[i2]);
                        maxDamage = damageDone;
                    }
                    else if (damageDone == maxDamage)
                    {
                        bestTargets.append(targets[i2]);
                        maxDamage = damageDone;
                    }
                }
                if (bestTargets.size() > 0 && maxDamage > 0)
                {
                    QPoint target = bestTargets[GlobalUtils::randIntBase(0, bestTargets.size() - 1)];
                    QVector<QPoint> path = turnPfs.getPath(target.x(), target.y());
                    pAction->setMovepath(path, turnPfs.getCosts(path));
                    addSelectedFieldData(pAction, target);
                    if (pAction->canBePerformed())
                    {
                        emit m_pAdapta->performAction(pAction);
                        return true;
                    }
                }
                else
                {
                    TargetedUnitPathFindingSystem pfs(pUnit, enemyTargets, &m_MoveCostMap);
                    pfs.explore();
                    qint32 movepoints = pUnit->getMovementpoints(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                    QPoint targetFields = pfs.getReachableTargetField(movepoints);
                    if (targetFields.x() >= 0)
                    {
                        pAction->setActionID(ACTION_WAIT);
                        QVector<QPoint> path = turnPfs.getClosestReachableMovePath(targetFields);
                        pAction->setMovepath(path, turnPfs.getCosts(path));
                        if (pAction->canBePerformed())
                        {
                            emit m_pAdapta->performAction(pAction);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::moveSupport(NB_AISteps step, spQmlVectorUnit pUnits, bool useTransporters)
{
    Console::print("NormalBehavioralModule::moveSupport", Console::eDEBUG);
    aiStep = step;
    spGameMap pMap = GameMap::getInstance();
    QVector<QVector3D> unitTargets;
    QVector<QPoint> unitPos;
    spQmlVectorPoint unitFields = GlobalUtils::getCircle(1, 1);
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        if (pUnit->getHpRounded() < Unit::MAX_UNIT_HP && pUnit->getUnitCosts() / Unit::MAX_UNIT_HP <= m_pPlayer->getFunds())
        {
            for (qint32 i2 = 0; i2 < unitFields->size(); i2++)
            {
                if (pMap->onMap(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y()) &&
                    pMap->getTerrain(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y())->getUnit() == nullptr)
                {
                    QVector3D point = QVector3D(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y(), 1);
                    if (!unitTargets.contains(point) )
                    {
                        unitTargets.append(point);
                        unitPos.append(pUnit->getPosition());
                    }
                }
            }
        }
    }
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        for (qint32 i2 = 0; i2 < unitFields->size(); i2++)
        {
            if (pMap->onMap(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y()) &&
                pMap->getTerrain(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y())->getUnit() == nullptr)
            {
                QVector3D point = QVector3D(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y(), 1);
                if (!unitTargets.contains(point) )
                {
                    unitTargets.append(point);
                    unitPos.append(pUnit->getPosition());
                }
            }
        }
    }
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        if (!pUnit->getHasMoved() &&
            (pUnit->getLoadedUnitCount() == 0) &&
            (pUnit->getLoadingPlace() == 0 || useTransporters))
        {
            QStringList actions = pUnit->getActionList();
            for (const auto& action : actions)
            {
                spGameAction pAction = spGameAction::create(action);
                if (action.startsWith(ACTION_SUPPORTSINGLE) ||
                    action.startsWith(ACTION_SUPPORTALL))
                {
                    UnitPathFindingSystem turnPfs(pUnit);
                    turnPfs.explore();
                    QVector<QPoint> targets = turnPfs.getAllNodePoints();
                    pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                    for (qint32 i2 = 0; i2 < targets.size(); i2++)
                    {
                        qint32 index = NormalBehavioralModule::index(unitTargets, targets[i2]);
                        if (index >= 0 && pUnit->getPosition() != unitPos[index])
                        {
                            if (action.startsWith(ACTION_SUPPORTSINGLE))
                            {
                                addSelectedFieldData(pAction, unitPos[index]);
                            }
                            QVector<QPoint> path = turnPfs.getPath(targets[i2].x(), targets[i2].y());
                            pAction->setMovepath(path, turnPfs.getCosts(path));
                            if (pAction->canBePerformed())
                            {
                                emit m_pAdapta->performAction(pAction);
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

QRectF NormalBehavioralModule::calcUnitDamage(spGameAction pAction, QPoint target)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "calcBattleDamage";
    QJSValueList args1;
    QJSValue obj1 = pInterpreter->newQObject(pAction.get());
    args1 << obj1;
    args1 << target.x();
    args1 << target.y();
    args1 << static_cast<qint32>(GameEnums::LuckDamageMode_Average);
    QJSValue erg = pInterpreter->doFunction(ACTION_FIRE, function1, args1);
    return erg.toVariant().toRectF();
}

QRectF NormalBehavioralModule::calcVirtuelUnitDamage(Unit* pAttacker, float attackerTakenDamage, QPoint atkPos,
                                     Unit* pDefender, float defenderTakenDamage, QPoint defPos,
                                     bool ignoreOutOfVisionRange)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "calcBattleDamage3";
    QJSValueList args1;
    QJSValue obj3 = pInterpreter->newQObject(nullptr);
    args1 << obj3;
    QJSValue obj1 = pInterpreter->newQObject(pAttacker);
    args1 << obj1;
    args1 << attackerTakenDamage;
    args1 << atkPos.x();
    args1 << atkPos.y();
    QJSValue obj2 = pInterpreter->newQObject(pDefender);
    args1 << obj2;
    args1 << defPos.x();
    args1 << defPos.y();
    args1 << defenderTakenDamage;
    args1 << static_cast<qint32>(GameEnums::LuckDamageMode_Average);
    args1 << ignoreOutOfVisionRange;
    QJSValue erg = pInterpreter->doFunction(ACTION_FIRE, function1, args1);
    return erg.toVariant().toRectF();
}

void NormalBehavioralModule::getBestTarget(Unit* pUnit, spGameAction pAction, UnitPathFindingSystem* pPfs, QVector<QVector3D>& ret, QVector<QVector3D>& moveTargetFields)
{
    pAction->setMovepath(QVector<QPoint>(1, QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())), 0);
    getBestAttacksFromField(pUnit, pAction, ret, moveTargetFields);
    if (pUnit->canMoveAndFire(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())))
    {
        spGameMap pMap = GameMap::getInstance();
        QVector<QPoint> targets = pPfs->getAllNodePoints();
        for (qint32 i2 = 0; i2 < targets.size(); i2++)
        {
            if (pMap->getTerrain(targets[i2].x(), targets[i2].y())->getUnit() == nullptr)
            {
                pAction->setMovepath(QVector<QPoint>(1, targets[i2]), 0);
                getBestAttacksFromField(pUnit, pAction, ret, moveTargetFields);
            }
        }
    }
}

void NormalBehavioralModule::getBestAttacksFromField(Unit* pUnit, spGameAction pAction, QVector<QVector3D>& ret, QVector<QVector3D>& moveTargetFields)
{
    spGameMap pMap = GameMap::getInstance();
    // much easier case
    // check if the action can be performed here
    if (pAction->canBePerformed())
    {
        spMarkedFieldData pMarkedFieldData = pAction->getMarkedFieldStepData();
        for (qint32 i = 0; i < pMarkedFieldData->getPoints()->size(); i++)
        {
            QPoint target = pMarkedFieldData->getPoints()->at(i);
            QRectF damage = calcUnitDamage(pAction, target);
            Terrain* pTerrain = pMap->getTerrain(target.x(), target.y());
            Unit* pDef = pTerrain->getUnit();
            if (pDef != nullptr)
            {
                QPointF dmg = calcFundsDamage(damage, pUnit, pDef);
                if (ret.size() == 0)
                {
                    ret.append(QVector3D(target.x(), target.y(), dmg.y()));
                    QPoint point = pAction->getActionTarget();
                    moveTargetFields.append(QVector3D(point.x(), point.y(), 1));
                }
                else if (static_cast<float>(ret[0].z()) == dmg.y())
                {
                    ret.append(QVector3D(target.x(), target.y(), dmg.y()));
                    QPoint point = pAction->getActionTarget();
                    moveTargetFields.append(QVector3D(point.x(), point.y(), 1));
                }
                else if (dmg.y() > ret[0].z())
                {
                    ret.clear();
                    moveTargetFields.clear();
                    ret.append(QVector3D(target.x(), target.y(), dmg.y()));
                    QPoint point = pAction->getActionTarget();
                    moveTargetFields.append(QVector3D(point.x(), point.y(), 1));
                }
            }
            else
            {
                if (isAttackOnTerrainAllowed(pTerrain))
                {
                    if (ret.size() == 0)
                    {
                        ret.append(QVector3D(target.x(), target.y(), static_cast<float>(damage.x()) * m_buildingValue));
                        QPoint point = pAction->getActionTarget();
                        moveTargetFields.append(QVector3D(point.x(), point.y(), 1));
                    }
                    else if (ret[0].z() == static_cast<float>(damage.x()) * m_buildingValue)
                    {
                        ret.append(QVector3D(target.x(), target.y(), static_cast<float>(damage.x()) * m_buildingValue));
                        QPoint point = pAction->getActionTarget();
                        moveTargetFields.append(QVector3D(point.x(), point.y(), 1));
                    }
                    else if (static_cast<float>(damage.x()) > ret[0].z())
                    {
                        ret.clear();
                        moveTargetFields.clear();
                        ret.append(QVector3D(target.x(), target.y(), static_cast<float>(damage.x()) * m_buildingValue));
                        QPoint point = pAction->getActionTarget();
                        moveTargetFields.append(QVector3D(point.x(), point.y(), 1));
                    }
                }
            }
        }
    }
}

void NormalBehavioralModule::getAttackTargets(Unit* pUnit, spGameAction pAction, UnitPathFindingSystem* pPfs, QVector<QVector4D>& ret, QVector<QVector3D>& moveTargetFields)
{
    pAction->setMovepath(QVector<QPoint>(1, QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())), 0);
    getAttacksFromField(pUnit, pAction, ret, moveTargetFields);
    if (pUnit->canMoveAndFire(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())))
    {
        spGameMap pMap = GameMap::getInstance();
        QVector<QPoint> targets = pPfs->getAllNodePoints();
        for (qint32 i2 = 0; i2 < targets.size(); i2++)
        {
            if (pMap->getTerrain(targets[i2].x(), targets[i2].y())->getUnit() == nullptr)
            {
                pAction->setMovepath(QVector<QPoint>(1, targets[i2]), 0);
                getAttacksFromField(pUnit, pAction, ret, moveTargetFields);
            }
        }
    }
}

void NormalBehavioralModule::getAttacksFromField(Unit* pUnit, spGameAction pAction, QVector<QVector4D>& ret, QVector<QVector3D>& moveTargetFields)
{
    spGameMap pMap = GameMap::getInstance();
    // much easier case
    // check if the action can be performed here
    if (pAction->canBePerformed())
    {
        spMarkedFieldData pMarkedFieldData = pAction->getMarkedFieldStepData();
        for (qint32 i = 0; i < pMarkedFieldData->getPoints()->size(); i++)
        {
            QPoint target = pMarkedFieldData->getPoints()->at(i);
            QRectF damage = calcUnitDamage(pAction, target);
            Terrain* pTerrain = pMap->getTerrain(target.x(), target.y());
            Unit* pDef = pTerrain->getUnit();
            if (pDef != nullptr)
            {
                qint32 stealthMalus = 0;
                bool terrainHide = false;
                if (pDef->isStatusStealthedAndInvisible(m_pPlayer, terrainHide))
                {
                    stealthMalus = 4;
                    if (terrainHide)
                    {
                        stealthMalus /= 2;
                    }
                }
                QPointF dmg = calcFundsDamage(damage, pUnit, pDef);
                ret.append(QVector4D(target.x(), target.y(), dmg.y(), dmg.x()));
                QPoint point = pAction->getActionTarget();
                moveTargetFields.append(QVector3D(point.x(), point.y(), 1 + stealthMalus));
            }
            else
            {
                if (isAttackOnTerrainAllowed(pTerrain))
                {
                    ret.append(QVector4D(target.x(), target.y(), static_cast<float>(damage.x()) * m_buildingValue, damage.x()));
                    QPoint point = pAction->getActionTarget();
                    moveTargetFields.append(QVector3D(point.x(), point.y(), 1));
                }
            }
        }
    }
}

bool NormalBehavioralModule::moveAwayFromProduction(spQmlVectorUnit pUnits)
{
    Console::print("NormalBehavioralModule::moveAwayFromProduction", Console::eDEBUG);
    aiStep = NB_AISteps::moveAway;
    spGameMap pMap = GameMap::getInstance();
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        // can we use the unit and does it block a production center cause it has nothing to do this turn?
        if (!pUnit->getHasMoved() &&
            pUnit->getTerrain()->getBuilding() != nullptr &&
            !m_pPlayer->isEnemy(pUnit->getTerrain()->getBuilding()->getOwner()) &&
            pUnit->getTerrain()->getBuilding()->isProductionBuilding())
        {
            UnitPathFindingSystem turnPfs(pUnit);
            turnPfs.explore();
            QVector<QPoint> points = turnPfs.getAllNodePoints();
            QPoint target(-1 , -1);
            for (qint32 i = 0; i < points.size(); i++)
            {
                Terrain* pTerrain = pMap->getTerrain(points[i].x(), points[i].y());
                if (pTerrain->getUnit() == nullptr)
                {
                    if (pTerrain->getBuilding() == nullptr)
                    {
                        target = points[i];
                        break;
                    }
                    else if (!pTerrain->getBuilding()->isProductionBuilding())
                    {
                        target = points[i];
                        break;
                    }
                }
            }
            if (target.x() >= 0 && target.y() >= 0)
            {
                spGameAction pAction = spGameAction::create(ACTION_WAIT);
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                QVector<QPoint> path = turnPfs.getPath(target.x(), target.y());
                pAction->setMovepath(path, turnPfs.getCosts(path));
                emit m_pAdapta->performAction(pAction);
                return true;
            }
        }
    }
    return false;
}

QPointF NormalBehavioralModule::calcFundsDamage(QRectF damage, Unit* pAtk, Unit* pDef)
{
    float atkDamage = static_cast<float>(damage.x()) / Unit::MAX_UNIT_HP;
    if (atkDamage > pDef->getHp())
    {
        atkDamage = pDef->getHp();
    }
    float fundsDamage = pDef->getUnitCosts() * atkDamage / Unit::MAX_UNIT_HP;
    if (damage.width() >= 0.0)
    {
        atkDamage = static_cast<float>(damage.width()) / Unit::MAX_UNIT_HP;
        if (atkDamage > pAtk->getHp())
        {
            atkDamage = pAtk->getHp();
        }
        fundsDamage -= pAtk->getUnitCosts() * atkDamage / Unit::MAX_UNIT_HP * m_ownUnitValue;
    }
    return QPointF(atkDamage, fundsDamage);
}

void NormalBehavioralModule::appendAttackTargets(Unit* pUnit, spQmlVectorUnit pEnemyUnits, QVector<QVector3D>& targets)
{
    spGameMap pMap = GameMap::getInstance();
    qint32 firerange = pUnit->getMaxRange(pUnit->getPosition());
    spQmlVectorPoint pTargetFields = GlobalUtils::getCircle(firerange, firerange);
    for (qint32 i2 = 0; i2 < pEnemyUnits->size(); i2++)
    {
        Unit* pEnemy = pEnemyUnits->at(i2);
        if (pUnit->isAttackable(pEnemy, true))
        {
            for (qint32 i3 = 0; i3 < pTargetFields->size(); i3++)
            {
                qint32 x = pTargetFields->at(i3).x() + pEnemy->Unit::getX();
                qint32 y = pTargetFields->at(i3).y() + pEnemy->Unit::getY();
                if (pMap->onMap(x, y) &&
                    pMap->getTerrain(x, y)->getUnit() == nullptr)
                {
                    if (pUnit->canMoveOver(x, y))
                    {
                        qint32 stealthMalus = 0;
                        bool terrainHide = false;
                        if (pEnemy->isStatusStealthedAndInvisible(m_pPlayer, terrainHide))
                        {
                            stealthMalus = 4;
                            if (terrainHide)
                            {
                                stealthMalus /= 2;
                            }
                        }
                        QVector3D possibleTarget(x, y, 1 + stealthMalus);
                        if (!targets.contains(possibleTarget))
                        {
                            targets.append(possibleTarget);
                        }
                    }
                }
            }
        }
    }
}

void NormalBehavioralModule::addMenuItemData(spGameAction pGameAction, QString itemID, qint32 cost)
{
    Console::print("NormalBehavioralModule::addMenuItemData()", Console::eDEBUG);
    pGameAction->writeDataString(itemID);
    // increase costs and input step
    pGameAction->setCosts(pGameAction->getCosts() + cost);
    pGameAction->setInputStep(pGameAction->getInputStep() + 1);
}

void NormalBehavioralModule::addSelectedFieldData(spGameAction pGameAction, QPoint point)
{
    Console::print("NormalBehavioralModule::addSelectedFieldData()", Console::eDEBUG);
    pGameAction->writeDataInt32(point.x());
    pGameAction->writeDataInt32(point.y());
    pGameAction->setInputStep(pGameAction->getInputStep() + 1);
}

bool NormalBehavioralModule::isAttackOnTerrainAllowed(Terrain* pTerrain)
{
    Building* pBuilding = pTerrain->getBuilding();
    if ((m_enableNeutralTerrainAttack && pTerrain->getHp() > 0) ||
        (pBuilding != nullptr && pBuilding->getHp() > 0 && pBuilding->getOwner() != m_pPlayer) ||
        (m_enableNeutralTerrainAttack && pBuilding != nullptr && pBuilding->getHp() > 0 && pBuilding->getOwner() == nullptr))
    {
        return true;
    }
    return false;
}

void NormalBehavioralModule::appendSupportTargets(QStringList actions, Unit* pCurrentUnit, spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits, QVector<QVector3D>& targets)
{
    spQmlVectorPoint unitFields = GlobalUtils::getCircle(1, 1);
    spGameMap pMap = GameMap::getInstance();
    for (const auto& action : actions)
    {
        if (action.startsWith(ACTION_SUPPORTSINGLE) ||
            action.startsWith(ACTION_SUPPORTALL))
        {
            for (qint32 i = 0; i < pUnits->size(); i++)
            {
                Unit* pUnit = pUnits->at(i);
                if (pUnit != pCurrentUnit)
                {
                    for (qint32 i2 = 0; i2 < unitFields->size(); i2++)
                    {
                        if (pMap->onMap(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y()) &&
                            pMap->getTerrain(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y())->getUnit() == nullptr)
                        {
                            QVector3D point = QVector3D(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y(), 2);
                            if (!targets.contains(point) )
                            {
                                targets.append(point);
                            }
                        }
                    }
                }
            }
        }
        else if (action.startsWith(ACTION_PLACE))
        {
            for (qint32 i = 0; i < pEnemyUnits->size(); i++)
            {
                Unit* pUnit = pEnemyUnits->at(i);
                for (qint32 i2 = 0; i2 < unitFields->size(); i2++)
                {
                    if (pMap->onMap(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y()) &&
                        pMap->getTerrain(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y())->getUnit() == nullptr)
                    {
                        QVector3D point = QVector3D(pUnit->Unit::getX() + unitFields->at(i2).x(), pUnit->Unit::getY() + unitFields->at(i2).y(), 2);
                        if (!targets.contains(point) )
                        {
                            targets.append(point);
                        }
                    }
                }
            }
        }
    }
}

void NormalBehavioralModule::appendCaptureTargets(QStringList actions, Unit* pUnit, spQmlVectorBuilding pEnemyBuildings, QVector<QVector3D>& targets)
{
    if (actions.contains(ACTION_CAPTURE) ||
        actions.contains(ACTION_MISSILE))
    {
        bool missileTarget = hasMissileTarget();
        for (qint32 i2 = 0; i2 < pEnemyBuildings->size(); i2++)
        {
            Building* pBuilding = pEnemyBuildings->at(i2);
            QPoint point(pBuilding->Building::getX(), pBuilding->Building::getY());
            if (pUnit->canMoveOver(pBuilding->Building::getX(), pBuilding->Building::getY()))
            {
                if (pBuilding->isCaptureOrMissileBuilding(missileTarget) &&
                    pBuilding->getTerrain()->getUnit() == nullptr)
                {
                    targets.append(QVector3D(pBuilding->Building::getX(), pBuilding->Building::getY(), 1));
                }
            }
        }
    }
}

void NormalBehavioralModule::appendAttackTargetsIgnoreOwnUnits(Unit* pUnit, spQmlVectorUnit pEnemyUnits, QVector<QVector3D>& targets)
{
    spGameMap pMap = GameMap::getInstance();
    for (qint32 i2 = 0; i2 < pEnemyUnits->size(); i2++)
    {
        Unit* pEnemy = pEnemyUnits->at(i2);
        if (pUnit->isAttackable(pEnemy, true))
        {
            qint32 firerange = pUnit->getMaxRange(pUnit->getPosition());
            spQmlVectorPoint pTargetFields = GlobalUtils::getCircle(firerange, firerange);
            for (qint32 i3 = 0; i3 < pTargetFields->size(); i3++)
            {
                qint32 x = pTargetFields->at(i3).x() + pEnemy->Unit::getX();
                qint32 y = pTargetFields->at(i3).y() + pEnemy->Unit::getY();
                if (pMap->onMap(x, y))
                {
                    Unit* pTargetUnit = pMap->getTerrain(x, y)->getUnit();
                    if (pUnit->canMoveOver(x, y) &&
                        pTargetUnit != nullptr &&
                        pTargetUnit->getOwner()->checkAlliance(m_pPlayer) == GameEnums::Alliance_Friend)
                    {
                        qint32 stealthMalus = 0;
                        bool terrainHide = false;
                        if (pEnemy->isStatusStealthedAndInvisible(m_pPlayer, terrainHide))
                        {
                            stealthMalus = 6;
                            if (terrainHide)
                            {
                                stealthMalus /= 2;
                            }
                        }
                        QVector3D possibleTarget(x, y, 4 + stealthMalus);
                        if (!targets.contains(possibleTarget))
                        {
                            targets.append(possibleTarget);
                        }
                    }
                }
            }
        }
    }
}

void NormalBehavioralModule::appendRepairTargets(Unit* pUnit, spQmlVectorBuilding pBuildings, QVector<QVector3D>& targets)
{
    spGameMap pMap = GameMap::getInstance();
    for (qint32 i2 = 0; i2 < pBuildings->size(); i2++)
    {
        Building* pBuilding = pBuildings->at(i2);
        QPoint point(pBuilding->Building::getX(), pBuilding->Building::getY());
        if (pMap->getTerrain(point.x(), point.y())->getUnit() == nullptr &&
            pBuilding->canRepair(pUnit))
        {
            targets.append(QVector3D(pBuilding->Building::getX(), pBuilding->Building::getY(), 1));
        }
    }
}

void NormalBehavioralModule::appendSupplyTargets(Unit* pUnit, spQmlVectorUnit pUnits, QVector<QVector3D>& targets)
{
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pSupplyUnit = pUnits->at(i);
        if (pSupplyUnit != pUnit)
        {
            if ((pSupplyUnit->getMaxAmmo1() > 0 && pSupplyUnit->hasAmmo1() < pSupplyUnit->getMaxAmmo1()) ||
                (pSupplyUnit->getMaxAmmo2() > 0 && pSupplyUnit->hasAmmo2() < pSupplyUnit->getMaxAmmo2()) ||
                (pSupplyUnit->getMaxFuel() > 0 && static_cast<float>(pSupplyUnit->getFuel()) / static_cast<float>(pSupplyUnit->getMaxFuel()) < 0.5f))
            {
                targets.append(QVector3D(pSupplyUnit->Unit::getX(), pSupplyUnit->Unit::getY(), 1));
            }
        }
    }
}

void NormalBehavioralModule::appendTransporterTargets(Unit* pUnit, spQmlVectorUnit pUnits, QVector<QVector3D>& targets)
{
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pTransporterUnit = pUnits->at(i);
        if (pTransporterUnit != pUnit)
        {
            if (pTransporterUnit->canTransportUnit(pUnit))
            {
                targets.append(QVector3D(pTransporterUnit->Unit::getX(), pTransporterUnit->Unit::getY(), 1));
            }
        }
    }
}

void NormalBehavioralModule::appendCaptureTransporterTargets(Unit* pUnit, spQmlVectorUnit pUnits, spQmlVectorBuilding pEnemyBuildings, QVector<QVector3D>& targets)
{
    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);
    spGameMap pMap = GameMap::getInstance();
    bool missileTarget = hasMissileTarget();
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pTransporterUnit = pUnits->at(i);
        if (pTransporterUnit != pUnit)
        {
            // assuming unit transporter only have space for one unit
            if (pTransporterUnit->canTransportUnit(pUnit) &&
                pTransporterUnit->getLoadingPlace() == 1)
            {
                bool goodTransporter = false;
                // check captures on this island
                qint32 transporterIslandIdx = getIslandIndex(pTransporterUnit);
                qint32 transporterIsland = getIsland(pTransporterUnit);
                for (qint32 i2 = 0; i2 < pEnemyBuildings->size(); i2++)
                {
                    qint32 x = pEnemyBuildings->at(i2)->Building::getX();
                    qint32 y = pEnemyBuildings->at(i2)->Building::getY();
                    // check if both can move there on this island
                    // so we avoid loading and unloading a unit recursivly cause we think we need to transport it to another island
                    // eventhough it has something to do here
                    if ((m_IslandMaps[unitIslandIdx]->getIsland(x, y) == unitIsland) &&
                        (m_IslandMaps[transporterIslandIdx]->getIsland(x, y) == transporterIsland) &&
                        (pMap->getTerrain(x, y)->getUnit() == nullptr) &&
                        (pEnemyBuildings->at(i2)->isCaptureOrMissileBuilding(missileTarget)))
                    {
                        goodTransporter = true;
                        break;
                    }
                }
                if (goodTransporter)
                {

                    targets.append(QVector3D(pTransporterUnit->Unit::getX(), pTransporterUnit->Unit::getY(), 1));
                }
            }
        }
    }
}

QVector<Unit*> NormalBehavioralModule::appendLoadingTargets(Unit* pUnit, spQmlVectorUnit pUnits,
                                            spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                                            bool ignoreCaptureTargets, bool virtualLoading, QVector<QVector3D>& targets,
                                            bool all)
{
    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);
    spGameMap pMap = GameMap::getInstance();
    qint32 width = pMap->getMapWidth();
    qint32 heigth = pMap->getMapHeight();
    QVector<Unit*> transportUnits;
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pLoadingUnit = pUnits->at(i);
        if (pLoadingUnit != pUnit)
        {
            // can we transport it?
            if (pUnit->canTransportUnit(pLoadingUnit, virtualLoading))
            {
                bool found = false;
                bool canCapture = pLoadingUnit->getActionList().contains(ACTION_CAPTURE);
                qint32 loadingIslandIdx = getIslandIndex(pLoadingUnit);
                qint32 loadingIsland = getIsland(pLoadingUnit);
                QPoint loadingUnitPos = pLoadingUnit->getPosition();
                if (ignoreCaptureTargets && canCapture)
                {
                    // no targets found -> try to speed up those infis
                }
                else
                {
                    found = hasTargets(pLoadingUnit, canCapture, pEnemyUnits, pEnemyBuildings,
                                       loadingIslandIdx, loadingIsland);
                }
                if (!found)
                {
                    // no targets for this unit :(
                    found = false;
                    qint32 distance = std::numeric_limits<qint32>::max();
                    qint32 targetX;
                    qint32 targetY;
                    for (qint32 x = 0; x < width; x++)
                    {
                        for (qint32 y = 0; y < heigth; y++)
                        {
                            // can be reached by both units and is empty
                            // and not added yet
                            if ((m_IslandMaps[loadingIslandIdx]->getIsland(x, y) == loadingIsland) &&
                                (m_IslandMaps[unitIslandIdx]->getIsland(x, y) == unitIsland) &&
                                ((pMap->getTerrain(x, y)->getUnit() == nullptr) ||
                                 (pMap->getTerrain(x, y)->getUnit() == pUnit)))
                            {
                                qint32 dist = GlobalUtils::getDistance(loadingUnitPos, QPoint(x, y));
                                if (dist < distance)
                                {
                                    found = true;
                                    distance = dist;
                                    targetX = x;
                                    targetY = y;
                                    if (all)
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                        if (all && found)
                        {
                            break;
                        }
                    }
                    if (found && (virtualLoading || !targets.contains(QVector3D(targetX, targetY, 1))))
                    {
                        targets.append(QVector3D(targetX, targetY, 1));
                        transportUnits.append(pLoadingUnit);
                        if (!all)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    return transportUnits;
}

bool NormalBehavioralModule::hasTargets(Unit* pLoadingUnit, bool canCapture, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                        qint32 loadingIslandIdx, qint32 loadingIsland)
{
    bool found = false;
    // check if we have anything to do here :)
    for (qint32 i2 = 0; i2 < pEnemyUnits->size(); i2++)
    {
        Unit* pEnemy = pEnemyUnits->at(i2);
        if (m_IslandMaps[loadingIslandIdx]->getIsland(pEnemy->Unit::getX(), pEnemy->Unit::getY()) == loadingIsland &&
            pLoadingUnit->isAttackable(pEnemy, true))
        {
            // this unit can do stuff skip it
            found = true;
            break;
        }
    }
    if (!found)
    {
        // check for capturing or missiles next
        if (canCapture)
        {
            bool missileTarget = hasMissileTarget();
            for (qint32 i2 = 0; i2 < pEnemyBuildings->size(); i2++)
            {
                Building* pBuilding = pEnemyBuildings->at(i2);
                if (m_IslandMaps[loadingIslandIdx]->getIsland(pBuilding->Building::getX(), pBuilding->Building::getY()) == loadingIsland &&
                    pBuilding->isCaptureOrMissileBuilding(missileTarget))
                {
                    // this unit can do stuff skip it
                    found = true;
                    break;
                }
            }
        }
    }
    return found;
}

void NormalBehavioralModule::appendNearestUnloadTargets(Unit* pUnit, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings, QVector<QVector3D>& targets)
{
    QVector<QVector<qint32>> checkedIslands;
    QVector<qint32> loadedUnitIslandIdx;

    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);
    QVector<Unit*> captureUnits;
    for (qint32 i2 = 0; i2 < pUnit->getLoadedUnitCount(); i2++)
    {
        Unit* pLoadedUnit = pUnit->getLoadedUnit(i2);
        if (pLoadedUnit->getActionList().contains(ACTION_CAPTURE))
        {
            captureUnits.append(pLoadedUnit);
        }
        loadedUnitIslandIdx.append(getIslandIndex(pLoadedUnit));
        checkedIslands.append(QVector<qint32>());
    }
    spQmlVectorPoint pUnloadArea = GlobalUtils::getCircle(1, 1);
    // check for enemis
    for (qint32 i = 0; i < pEnemyUnits->size(); i++)
    {
        Unit* pEnemy = pEnemyUnits->at(i);
        for (qint32 i2 = 0; i2 < pUnit->getLoadedUnitCount(); i2++)
        {
            Unit* pLoadedUnit = pUnit->getLoadedUnit(i2);
            qint32 targetIsland = m_IslandMaps[loadedUnitIslandIdx[i2]]->getIsland(pEnemy->Unit::getX(), pEnemy->Unit::getY());
            // check if we could reach the enemy if we would be on his island
            // and we didn't checked this island yet -> improves the speed
            if (targetIsland >= 0 && !checkedIslands[i2].contains(targetIsland))
            {
                // could we beat his ass? -> i mean can we attack him
                // if so this is a great island
                if (pLoadedUnit->isAttackable(pEnemy, true))
                {
                    checkIslandForUnloading(pUnit, pLoadedUnit, checkedIslands[i2], unitIslandIdx, unitIsland,
                                            loadedUnitIslandIdx[i2], targetIsland, pUnloadArea.get(), targets);
                }
            }
        }
    }
    // check for capturable buildings
    if (captureUnits.size() > 0)
    {
        bool missileTarget = hasMissileTarget();
        for (qint32 i = 0; i < pEnemyBuildings->size(); i++)
        {
            Building* pEnemyBuilding = pEnemyBuildings->at(i);
            for (qint32 i2 = 0; i2 < captureUnits.size(); i2++)
            {
                Unit* pLoadedUnit = captureUnits[i2];
                qint32 targetIsland = m_IslandMaps[loadedUnitIslandIdx[i2]]->getIsland(pEnemyBuilding->Building::getX(), pEnemyBuilding->Building::getY());
                // check if we could reach the enemy if we would be on his island
                // and we didn't checked this island yet -> improves the speed
                if (targetIsland >= 0 && !checkedIslands[i2].contains(targetIsland))
                {
                    if (pEnemyBuilding->isCaptureOrMissileBuilding(missileTarget))
                    {
                        checkIslandForUnloading(pUnit, pLoadedUnit, checkedIslands[i2], unitIslandIdx, unitIsland,
                                                loadedUnitIslandIdx[i2], targetIsland, pUnloadArea.get(), targets);
                    }
                }
            }
        }
    }
}

void NormalBehavioralModule::appendUnloadTargetsForCapturing(Unit* pUnit, spQmlVectorBuilding pEnemyBuildings, QVector<QVector3D>& targets)
{
    spGameMap pMap = GameMap::getInstance();

    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);

    QVector<Unit*> capturUnits;
    for (qint32 i2 = 0; i2 < pUnit->getLoadedUnitCount(); i2++)
    {
        Unit* pLoadedUnit = pUnit->getLoadedUnit(i2);
        if (pLoadedUnit->getActionList().contains(ACTION_CAPTURE))
        {
            capturUnits.append(pLoadedUnit);
        }
    }
    if (capturUnits.size() > 0)
    {
        spQmlVectorPoint pUnloadArea = GlobalUtils::getCircle(1, 1);
        GameAction testAction;
        testAction.setTargetUnit(capturUnits[0]);
        // store has moved
        bool hasMoved = capturUnits[0]->getHasMoved();
        // simulate a not moved unit for checking if we can capture the building or fire a missile from it.
        capturUnits[0]->setHasMoved(false);
        bool missileTarget = hasMissileTarget();
        for (qint32 i = 0; i < pEnemyBuildings->size(); i++)
        {
            Building* pBuilding = pEnemyBuildings->at(i);
            QPoint point(pBuilding->Building::getX(), pBuilding->Building::getY());
            if (capturUnits[0]->canMoveOver(pBuilding->Building::getX(), pBuilding->Building::getY()))
            {
                // we can capture it :)
                if (pBuilding->isCaptureOrMissileBuilding(missileTarget) &&
                    pBuilding->getTerrain()->getUnit() == nullptr)
                {
                    // check unload fields
                    for (qint32 i2 = 0; i2 < pUnloadArea->size(); i2++)
                    {
                        qint32 x = point.x() + pUnloadArea->at(i2).x();
                        qint32 y = point.y() + pUnloadArea->at(i2).y();
                        if (pMap->onMap(x, y) &&
                            pMap->getTerrain(x, y)->getUnit() == nullptr &&
                            !targets.contains(QVector3D(x, y, 1)))
                        {
                            if (isUnloadTerrain(pUnit, pMap->getTerrain(x, y)))
                            {
                                // we can reach this unload field?
                                if (m_IslandMaps[unitIslandIdx]->getIsland(x, y) == unitIsland)
                                {
                                    for (qint32 i3 = 0; i3 < capturUnits.size(); i3++)
                                    {
                                        if (capturUnits[i3]->canMoveOver(x, y))
                                        {
                                            targets.append(QVector3D(x, y, 1));
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        capturUnits[0]->setHasMoved(hasMoved);
    }
}

void NormalBehavioralModule::appendTerrainBuildingAttackTargets(Unit* pUnit, spQmlVectorBuilding pEnemyBuildings, QVector<QVector3D>& targets)
{
    spGameMap pMap = GameMap::getInstance();
    qint32 firerange = pUnit->getMaxRange(pUnit->getPosition());
    spQmlVectorPoint pTargetFields = GlobalUtils::getCircle(firerange, firerange);
    for (qint32 i = 0; i < pEnemyBuildings->size(); i++)
    {
        Building* pBuilding = pEnemyBuildings->at(i);
        if (pBuilding->getHp() > 0 &&
            pUnit->isEnvironmentAttackable(pBuilding->getBuildingID()))
        {
            qint32 width = pBuilding->getBuildingWidth();
            qint32 heigth = pBuilding->getBuildingHeigth();
            QPoint pos = pBuilding->getPosition();
            QVector<QPoint> attackPosition;
            // find all attackable fields
            for (qint32 x = -width; x <= 0; x++)
            {
                for (qint32 y = -heigth; y <= 0; y++)
                {
                    if (pBuilding->getIsAttackable(x + pos.x(), y + pos.y()))
                    {
                        attackPosition.append(QPoint(pos.x() + x, pos.y() + y));
                    }
                }
            }
            // find attackable fields
            for (qint32 i3 = 0; i3 < pTargetFields->size(); i3++)
            {
                for (qint32 i4 = 0; i4 < attackPosition.size(); i4++)
                {
                    qint32 x = pTargetFields->at(i3).x() + attackPosition[i4].x();
                    qint32 y = pTargetFields->at(i3).y() + attackPosition[i4].y();
                    if (pMap->onMap(x, y) &&
                        pMap->getTerrain(x, y)->getUnit() == nullptr)
                    {
                        if (pUnit->canMoveOver(x, y))
                        {
                            QVector3D possibleTarget(x, y, 2);
                            if (!targets.contains(possibleTarget))
                            {
                                targets.append(possibleTarget);
                            }
                        }
                    }
                }
            }
        }
    }
    if (m_enableNeutralTerrainAttack)
    {
        // find terrains
        qint32 width = pMap->getMapWidth();
        qint32 heigth = pMap->getMapHeight();
        for (qint32 x = 0; x < width; x++)
        {
            for (qint32 y = 0; y < heigth; y++)
            {
                Terrain* pTerrain = pMap->getTerrain(x, y);
                if (isAttackOnTerrainAllowed(pTerrain) &&
                    pUnit->isEnvironmentAttackable(pTerrain->getID()))
                {
                    for (qint32 i3 = 0; i3 < pTargetFields->size(); i3++)
                    {
                        qint32 x1 = pTargetFields->at(i3).x() + x;
                        qint32 y1 = pTargetFields->at(i3).y() + y;
                        if (pMap->onMap(x1, y1) &&
                            pMap->getTerrain(x1, y1)->getUnit() == nullptr)
                        {
                            if (pUnit->canMoveOver(x1, y1))
                            {
                                QVector3D possibleTarget(x1, y1, 2);
                                if (!targets.contains(possibleTarget))
                                {
                                    targets.append(possibleTarget);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void NormalBehavioralModule::checkIslandForUnloading(Unit* pUnit, Unit* pLoadedUnit, QVector<qint32>& checkedIslands,
                                     qint32 unitIslandIdx, qint32 unitIsland,
                                     qint32 loadedUnitIslandIdx, qint32 targetIsland,
                                     QmlVectorPoint* pUnloadArea, QVector<QVector3D>& targets)
{
    spGameMap pMap = GameMap::getInstance();
    qint32 width = pMap->getMapWidth();
    qint32 heigth = pMap->getMapHeight();
    checkedIslands.append(loadedUnitIslandIdx);
    for (qint32 x = 0; x < width; x++)
    {
        for (qint32 y = 0; y < heigth; y++)
        {
            // check if this is the same island as we search for
            // check if it's the same island our transporter is on if so we can reach the field
            // the unloading area is also free
            if (m_IslandMaps[loadedUnitIslandIdx]->getIsland(x, y) == targetIsland &&
                m_IslandMaps[unitIslandIdx]->getIsland(x, y) == unitIsland &&
                (pMap->getTerrain(x, y)->getUnit() == nullptr ||
                 pMap->getTerrain(x, y)->getUnit() == pUnit))
            {
                if (isUnloadTerrain(pUnit, pMap->getTerrain(x, y)))
                {
                    // and on top of that we have same free fields to unload the unit
                    for (qint32 i3 = 0; i3 < pUnloadArea->size(); i3++)
                    {
                        qint32 unloadX = x + pUnloadArea->at(i3).x();
                        qint32 unloadY = y + pUnloadArea->at(i3).y();
                        if (pMap->onMap(unloadX, unloadY) &&
                            pMap->getTerrain(unloadX, unloadY)->getUnit() == nullptr &&
                            pLoadedUnit->getBaseMovementCosts(unloadX, unloadY, unloadX, unloadY) > 0 &&
                            !targets.contains(QVector3D(x, y, 1)) &&
                            pUnit->getBaseMovementCosts(x, y, x, y) > 0)
                        {
                            targets.append(QVector3D(x, y, 1));
                            break;
                        }
                    }
                }
            }
        }
    }
}

void NormalBehavioralModule::getBestFlareTarget(Unit* pUnit, spGameAction pAction, UnitPathFindingSystem* pPfs, QPoint& flareTarget, QPoint& moveTargetField)
{
    flareTarget  = QPoint(-1, -1);
    moveTargetField  = QPoint(-1, -1);
    spQmlVectorPoint pUnfogCircle = GlobalUtils::getCircle(0, m_flareInfo.unfogRange);
    spQmlVectorPoint pTargetCircle = GlobalUtils::getCircle(m_flareInfo.minRange, m_flareInfo.maxRange);
    pAction->setMovepath(QVector<QPoint>(1, QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())), 0);
    spGameMap pMap = GameMap::getInstance();
    QVector<QPoint> targets = pPfs->getAllNodePoints();
    qint32 score = std::numeric_limits<qint32>::min();
    for (qint32 i = 0; i < targets.size(); i++)
    {
        QVector<QPoint> path = pPfs->getPath(targets[i].x(), targets[i].y());
        pAction->setMovepath(path, pPfs->getCosts(path));
        if (pAction->canBePerformed())
        {
            Unit* pFieldUnit = pMap->getTerrain(targets[i].x(), targets[i].y())->getUnit();
            if (pFieldUnit == nullptr ||
                pFieldUnit == pUnit)
            {
                for (qint32 i2 = 0; i2 < pTargetCircle->size(); ++i2)
                {
                    QPoint target = pTargetCircle->at(i2) + targets[i];
                    if (pMap->onMap(target.x(), target.y()))
                    {
                        qint32 currentScore = getFlareTargetScore(targets[i], target, pUnfogCircle);
                        if (currentScore > 0)
                        {
                            if (score < currentScore)
                            {
                                score = currentScore;
                                flareTarget  = target;
                                moveTargetField  = targets[i];
                            }
                            else if (score == currentScore && GlobalUtils::randInt(0, 10) > 5)
                            {
                                flareTarget  = target;
                                moveTargetField  = targets[i];
                            }
                        }
                    }
                }
            }
        }
    }
    if (score > 0)
    {
        Console::print("Found flare target with score: " + QString::number(score), Console::eDEBUG);
    }
}

qint32 NormalBehavioralModule::getFlareTargetScore(const QPoint& moveTarget, const QPoint& flareTarget, const spQmlVectorPoint& pUnfogCircle)
{
    spGameMap pMap = GameMap::getInstance();
    qint32 score = 0;
    for (qint32 i = 0; i < pUnfogCircle->size(); ++i)
    {
        QPoint target = pUnfogCircle->at(i) + flareTarget;
        if (pMap->onMap(target.x(), target.y()))
        {
            Unit* pFieldUnit = pMap->getTerrain(target.x(), target.y())->getUnit();
            if (pFieldUnit != nullptr && m_pPlayer->isEnemyUnit(pFieldUnit))
            {
                if (pFieldUnit->isStealthed(m_pPlayer))
                {
                    // unstealthing stealthed units is more important.
                    // than unfogging plain fields
                    if (pFieldUnit->isStatusStealthed() ||
                        pFieldUnit->hasTerrainHide(m_pPlayer))
                    {
                        score += 2;
                    }
                    else
                    {
                        score++;
                    }
                }
            }
        }
    }
    if (score > 0)
    {
        // the farther away from the flare the better it is usually
        score += GlobalUtils::getDistance(moveTarget, flareTarget) * pUnfogCircle->size() * 3;
    }
    else
    {
        score = std::numeric_limits<qint32>::min();
    }
    return score;
}

bool NormalBehavioralModule::isUnloadTerrain(Unit* pUnit, Terrain* pTerrain)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValueList args;
    QJSValue obj = pInterpreter->newQObject(pUnit);
    args << obj;
    QJSValue obj1 = pInterpreter->newQObject(pTerrain);
    args << obj1;
    QJSValue ret = pInterpreter->doFunction(ACTION_UNLOAD, "isUnloadTerrain", args);
    if (ret.isBool())
    {
        return ret.toBool();
    }
    return false;
}

void NormalBehavioralModule::rebuildIsland(spQmlVectorUnit pUnits)
{
    Console::print("NormalBehavioralModule::rebuildIsland", Console::eDEBUG);
    // and create one
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        if (getIslandIndex(pUnit) < 0)
        {
            createIslandMap(pUnit->getMovementType(), pUnit->getUnitID());
        }
        for (qint32 i2 = 0; i2 < pUnit->getLoadedUnitCount(); i2++)
        {
            if (getIslandIndex(pUnit->getLoadedUnit(i2)) < 0)
            {
                createIslandMap(pUnit->getLoadedUnit(i2)->getMovementType(), pUnit->getLoadedUnit(i2)->getUnitID());
            }
        }
    }
}

bool NormalBehavioralModule::onSameIsland(Unit* pUnit1, Unit* pUnit2)
{
    for (auto i = 0; i < m_IslandMaps.size(); i++)
    {
        if (m_IslandMaps[i]->getMovementType() == pUnit1->getMovementType())
        {
            if (m_IslandMaps[i]->getIsland(pUnit1->Unit::getX(), pUnit1->Unit::getY()) ==
                m_IslandMaps[i]->getIsland(pUnit2->Unit::getX(), pUnit2->Unit::getY()))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::onSameIsland(Unit* pUnit1, Building* pBuilding)
{
    for (auto i = 0; i < m_IslandMaps.size(); i++)
    {
        if (m_IslandMaps[i]->getMovementType() == pUnit1->getMovementType())
        {
            if (m_IslandMaps[i]->getIsland(pUnit1->Unit::getX(), pUnit1->Unit::getY()) ==
                m_IslandMaps[i]->getIsland(pBuilding->Building::getX(), pBuilding->Building::getY()))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::onSameIsland(QString movemnetType, qint32 x, qint32 y, qint32 x1, qint32 y1)
{
    for (auto i = 0; i < m_IslandMaps.size(); i++)
    {
        if (m_IslandMaps[i]->getMovementType() == movemnetType)
        {
            return onSameIsland(i, x, y, x1, y1);
        }
    }
    return false;
}

bool NormalBehavioralModule::onSameIsland(qint32 islandIdx, qint32 x, qint32 y, qint32 x1, qint32 y1)
{
    return m_IslandMaps[islandIdx]->sameIsland(x, y, x1, y1);
}

qint32 NormalBehavioralModule::getIsland(Unit* pUnit)
{
    for (qint32 i = 0; i < m_IslandMaps.size(); i++)
    {
        if (m_IslandMaps[i]->getMovementType() == pUnit->getMovementType())
        {
            return m_IslandMaps[i]->getIsland(pUnit->Unit::getX(), pUnit->Unit::getY());
        }
    }
    return -1;
}

qint32 NormalBehavioralModule::getIslandIndex(Unit* pUnit)
{
    for (qint32 i = 0; i < m_IslandMaps.size(); i++)
    {
        if (m_IslandMaps[i]->getMovementType() == pUnit->getMovementType())
        {
            return i;
        }
    }
    return -1;
}

void NormalBehavioralModule::createIslandMap(QString movementType, QString unitID)
{
    bool found = false;

    for (qint32 i = 0; i < m_IslandMaps.size(); i++)
    {
        if (m_IslandMaps[i]->getMovementType() == movementType)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        m_IslandMaps.append(spIslandMap::create(unitID, m_pPlayer));
    }
}

bool NormalBehavioralModule::needsRefuel(Unit *pUnit)
{
    if (pUnit->getMaxFuel() > 0 &&
        pUnit->getFuel() / static_cast<float>(pUnit->getMaxFuel()) < m_fuelResupply)
    {
        return true;
    }
    if (pUnit->getMaxAmmo1() > 0 &&
        pUnit->getAmmo1() / static_cast<float>(pUnit->getMaxAmmo1()) < m_ammoResupply &&
        !pUnit->getWeapon1ID().isEmpty())
    {
        return true;
    }
    if (pUnit->getMaxAmmo2() > 0 &&
        pUnit->getAmmo2() / static_cast<float>(pUnit->getMaxAmmo2()) < m_ammoResupply &&
        !pUnit->getWeapon2ID().isEmpty())
    {
        return true;
    }
    return false;
}

float NormalBehavioralModule::getAiCoUnitMultiplier(CO* pCO, Unit* pUnit)
{
    bool valid = false;
    float multiplier = 0.0f;
    if (pCO != nullptr)
    {
        multiplier = pCO->getAiCoUnitBonus(pUnit, valid);
        if (!valid)
        {
            if (pCO->getOffensiveBonus(nullptr, pUnit, QPoint(-1, -1), nullptr, QPoint(-1, -1), false) > 0 ||
                pCO->getDeffensiveBonus(nullptr, nullptr, QPoint(-1, -1), pUnit, QPoint(-1, -1), false) > 0 ||
                pCO->getFirerangeModifier(pUnit, QPoint(-1, -1)) > 0)
            {
                multiplier = 1.0f;
            }
        }
    }
    return multiplier;
}

bool NormalBehavioralModule::buildCOUnit(spQmlVectorUnit pUnits)
{
    Console::print("NormalBehavioralModule::buildCOUnit", Console::eDEBUG);
    spGameAction pAction = spGameAction::create();
    for (quint8 i2 = 0; i2 < 2; i2++)
    {
        if (i2 == 0)
        {
            pAction->setActionID(ACTION_CO_UNIT_0);
        }
        else
        {
            pAction->setActionID(ACTION_CO_UNIT_1);
        }
        CO* pCO = m_pPlayer->getCO(i2);
        qint32 bestScore = 0;
        qint32 unitIdx = -1;
        if (pCO != nullptr &&
            pCO->getCOUnit() == nullptr)
        {
            qint32 active = 0;
            bool expensive = false;
            for (qint32 i = 0; i < pUnits->size(); i++)
            {
                Unit* pUnit = pUnits->at(i);

                if (pUnit->getUnitValue() >= m_coUnitValue && pUnit->getUnitRank() >= GameEnums::UnitRank_None)
                {
                    active++;
                }
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                if (pAction->canBePerformed())
                {
                    if (!pUnit->getHasMoved())
                    {
                        if (pUnit->hasWeapons())
                        {
                            qint32 score = 0;
                            score += pUnit->getUnitValue() * m_coUnitScoreMultiplier * getAiCoUnitMultiplier(pCO, pUnit);
                            if (pUnit->getUnitValue() >= m_coUnitValue)
                            {
                                expensive = true;
                            }
                            score -= m_coUnitRankReduction * pUnit->getUnitRank();
                            if (score > bestScore)
                            {
                                bestScore = score;
                                unitIdx = i;
                            }
                        }
                    }
                }
            }
            if (unitIdx >= 0 && bestScore > m_minCoUnitScore && (active > m_minCoUnitCount || expensive))
            {
                Unit* pUnit = pUnits->at(unitIdx);
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                emit m_pAdapta->performAction(pAction);
                return true;
            }
        }
    }
    return false;
}

//NormalAI Methods

bool NormalBehavioralModule::performActionSteps(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits,
                                  spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    if (aiStep <= NB_AISteps::moveUnits && buildCOUnit(pUnits)){}
    else if (aiStep <= NB_AISteps::moveUnits && NormalBehavioralModule::moveFlares(pUnits)){}
    else if (aiStep <= NB_AISteps::moveUnits && NormalBehavioralModule::moveOoziums(pUnits, pEnemyUnits)){}
    else if (aiStep <= NB_AISteps::moveUnits && NormalBehavioralModule::moveBlackBombs(pUnits, pEnemyUnits)){}
    else if (aiStep <= NB_AISteps::moveUnits && captureBuildings(pUnits)){}
    else if (aiStep <= NB_AISteps::moveUnits && moveSupport(NB_AISteps::moveUnits, pUnits, false)){}
    // indirect units
    else if (aiStep <= NB_AISteps::moveUnits && fireWithUnits(pUnits, 2, std::numeric_limits<qint32>::max(), pBuildings, pEnemyBuildings)){}
    // direct units
    else if (aiStep <= NB_AISteps::moveUnits && fireWithUnits(pUnits, 1, 1, pBuildings, pEnemyBuildings)){}
    else if (aiStep <= NB_AISteps::moveUnits && repairUnits(pUnits, pBuildings, pEnemyBuildings)){}
    else if (aiStep <= NB_AISteps::moveToTargets && refillUnits(pUnits, pBuildings, pEnemyBuildings)){}
    else if (aiStep <= NB_AISteps::moveToTargets && moveUnits(pUnits, pBuildings, pEnemyUnits, pEnemyBuildings, 1, 1)){}
    else if (aiStep <= NB_AISteps::moveToTargets && moveUnits(pUnits, pBuildings, pEnemyUnits, pEnemyBuildings, 2, std::numeric_limits<qint32>::max())){}
    else if (aiStep <= NB_AISteps::loadUnits && !usedTransportSystem && loadUnits(pUnits, pBuildings, pEnemyBuildings)){}
    else if (aiStep <= NB_AISteps::moveTransporters && !usedTransportSystem && moveTransporters(pUnits, pEnemyUnits, pBuildings, pEnemyBuildings)){}
    else
    {
        if (!usedTransportSystem)
        {
            usedTransportSystem = true;
            aiStep = NB_AISteps::moveUnits;
            return performActionSteps(pUnits, pEnemyUnits,  pBuildings, pEnemyBuildings);
        }
        else if (aiStep <= NB_AISteps::loadUnits && loadUnits(pUnits, pBuildings, pEnemyBuildings)){}
        else if (aiStep <= NB_AISteps::moveSupportUnits && moveSupport(NB_AISteps::moveSupportUnits, pUnits, true)){}
        else if (aiStep <= NB_AISteps::moveSupportUnits && moveUnits(pUnits, pBuildings, pEnemyUnits, pEnemyBuildings, 1, std::numeric_limits<qint32>::max(), true)){}
        else if (aiStep <= NB_AISteps::moveAway && moveAwayFromProduction(pUnits)){}
        //in normal AI there's a final step which builds units, but here we don't do that since is not a building module
        //else if (aiStep <= NB_AISteps::buildUnits && buildUnits(pBuildings, pUnits, pEnemyUnits, pEnemyBuildings)){}
        else
        {
            return false;
        }
    }
    return true;
}

bool NormalBehavioralModule::isUsingUnit(Unit* pUnit)
{
    if (needsRefuel(pUnit))
    {
        return false;
    }
    spGameMap pMap = GameMap::getInstance();
    if (pMap->onMap(pUnit->Unit::getX(), pUnit->Unit::getY()))
    {
        Building* pBuilding = pMap->getTerrain(pUnit->Unit::getX(), pUnit->Unit::getY())->getBuilding();
        if (pBuilding == nullptr && pUnit->getHpRounded() <= m_minUnitHealth)
        {
            return false;
        }
        else if (pBuilding != nullptr && pBuilding->getOwner() == m_pPlayer &&
                 pUnit->getHpRounded() <= m_maxUnitHealth)
        {
            return false;
        }
    }
    if (pUnit->getHasMoved())
    {
        return false;
    }
    return true;
}


bool NormalBehavioralModule::moveUnits(spQmlVectorUnit pUnits, spQmlVectorBuilding pBuildings,
                         spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                         qint32 minfireRange, qint32 maxfireRange, bool supportUnits)
{
    Console::print("NormalBehavioralModule::moveUnits()", Console::eDEBUG);
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        // can we use the unit?
        if ((isUsingUnit(pUnit) || usedTransportSystem) &&
            !pUnit->getHasMoved() &&
            pUnit->getBaseMaxRange() >= minfireRange &&
            pUnit->getBaseMaxRange() <= maxfireRange &&
            pUnit->hasWeapons() && pUnit->getLoadedUnitCount() == 0)
        {
            QVector<QVector3D> targets;
            QVector<QVector3D> transporterTargets;
            spGameAction pAction = spGameAction::create(ACTION_WAIT);
            QStringList actions = pUnit->getActionList();
            // find possible targets for this unit
            pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));

            // find some cool targets
            appendCaptureTargets(actions, pUnit, pEnemyBuildings, targets);
            if (targets.size() > 0)
            {
                appendCaptureTransporterTargets(pUnit, pUnits, pEnemyBuildings, transporterTargets);
                targets.append(transporterTargets);
            }
            appendAttackTargets(pUnit, pEnemyUnits, targets);
            appendAttackTargetsIgnoreOwnUnits(pUnit, pEnemyUnits, targets);
            appendTerrainBuildingAttackTargets(pUnit, pEnemyBuildings, targets);
            if (targets.size() == 0)
            {
                appendRepairTargets(pUnit, pBuildings, targets);
            }
            if (supportUnits)
            {
                appendSupportTargets(actions, pUnit, pUnits, pEnemyUnits, targets);
            }
            if (moveUnit(pAction, pUnit, pUnits, actions, targets, transporterTargets, true, pBuildings, pEnemyBuildings))
            {
                return true;
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::refillUnits(spQmlVectorUnit pUnits, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    Console::print("NormalBehavioralModule::refillUnits()", Console::eDEBUG);
    if (aiStep < NB_AISteps::moveToTargets)
    {
        createMovementMap(pBuildings, pEnemyBuildings);
    }
    aiStep = NB_AISteps::moveToTargets;
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        // can we use the unit?
        if (isUsingUnit(pUnit))
        {
            QStringList actions = pUnit->getActionList();
            if (actions.contains(ACTION_SUPPORTALL_RATION) ||
                actions.contains(ACTION_SUPPORTSINGLE_REPAIR) ||
                actions.contains(ACTION_SUPPORTSINGLE_FREEREPAIR))
            {
                spGameAction pAction = spGameAction::create(ACTION_WAIT);
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                UnitPathFindingSystem pfs(pUnit);
                pfs.explore();
                bool found = false;
                QPoint moveTarget;
                if (actions.contains(ACTION_SUPPORTALL_RATION))
                {
                    pAction->setActionID(ACTION_SUPPORTALL_RATION);
                    QPoint refillTarget;
                    found = getBestRefillTarget(pfs, 4, moveTarget, refillTarget);
                }
                else if (actions.contains(ACTION_SUPPORTSINGLE_REPAIR))
                {
                    pAction->setActionID(ACTION_SUPPORTSINGLE_REPAIR);
                    QPoint refillTarget;
                    found = getBestRefillTarget(pfs, 1, moveTarget, refillTarget);
                    NormalBehavioralModule::addSelectedFieldData(pAction, refillTarget);
                }
                else if (actions.contains(ACTION_SUPPORTSINGLE_FREEREPAIR))
                {
                    pAction->setActionID(ACTION_SUPPORTSINGLE_FREEREPAIR);
                    QPoint refillTarget;
                    found = getBestRefillTarget(pfs, 1, moveTarget, refillTarget);
                    NormalBehavioralModule::addSelectedFieldData(pAction, refillTarget);
                }
                if (found)
                {
                    QVector<QPoint> path = pfs.getPath(moveTarget.x(), moveTarget.y());
                    pAction->setMovepath(path, pfs.getCosts(path));
                    if (pAction->canBePerformed())
                    {
                        m_updatePoints.append(pUnit->getPosition());
                        m_updatePoints.append(pAction->getActionTarget());
                        emit m_pAdapta->performAction(pAction);
                        return true;
                    }
                }
                else if (usedTransportSystem)
                {
                    Console::print("move to supply needed units", Console::eDEBUG);
                    QVector<QVector3D> targets;
                    QVector<QVector3D> transporterTargets;
                    pAction->setActionID(ACTION_WAIT);
                    appendRefillTargets(actions, pUnit, pUnits, targets);
                    if (moveUnit(pAction, pUnit, pUnits, actions, targets, transporterTargets, true, pBuildings, pEnemyBuildings))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool NormalBehavioralModule::getBestRefillTarget(UnitPathFindingSystem & pfs, qint32 maxRefillCount, QPoint & moveTarget, QPoint & refillTarget)
{
    bool ret = false;
    const auto points = pfs.getAllNodePoints();
    spGameMap pMap = GameMap::getInstance();
    spQmlVectorPoint circle = GlobalUtils::getCircle(1, 1);
    qint32 highestCount = 0;
    for (const auto & point : points)
    {
        if (pMap->getTerrain(point.x(), point.y())->getUnit() == nullptr)
        {
            qint32 count = 0;
            for (qint32 i = 0; i < circle->size(); ++i)
            {
                qint32 x = point.x() + circle->at(i).x();
                qint32 y = point.y() + circle->at(i).y();
                if (pMap->onMap(x, y))
                {
                    Unit* pSupplyUnit = pMap->getTerrain(x, y)->getUnit();
                    if (pSupplyUnit != nullptr &&
                        pSupplyUnit->getOwner() == m_pPlayer &&
                        needsRefuel(pSupplyUnit))
                    {
                        ++count;
                        ret = true;
                        if (count == maxRefillCount)
                        {
                            moveTarget = point;
                            refillTarget = QPoint(x, y);
                            break;
                        }
                    }
                }
            }
            if (count == maxRefillCount)
            {
                break;
            }
            else if (count > highestCount)
            {
                moveTarget = point;
                highestCount = count;
            }
        }
    }
    return ret;
}

void NormalBehavioralModule::appendRefillTargets(QStringList actions, Unit* pUnit, spQmlVectorUnit pUnits, QVector<QVector3D>& targets)
{
    if (actions.contains(ACTION_SUPPORTALL_RATION) ||
        actions.contains(ACTION_SUPPORTSINGLE_REPAIR) ||
        actions.contains(ACTION_SUPPORTSINGLE_FREEREPAIR))
    {
        spQmlVectorPoint circle = GlobalUtils::getCircle(1, 1);
        spGameMap pMap = GameMap::getInstance();
        qint32 islandIdx = getIslandIndex(pUnit);
        qint32 curX = pUnit->Unit::getX();
        qint32 curY = pUnit->Unit::getY();
        for (qint32 i = 0; i < pUnits->size(); i++)
        {
            Unit* pSupplyUnit = pUnits->at(i);
            if (needsRefuel(pSupplyUnit))
            {
                qint32 unitX = pSupplyUnit->Unit::getX();
                qint32 unitY = pSupplyUnit->Unit::getY();
                for (qint32 i2 = 0; i2 < circle->size(); ++i2)
                {
                    qint32 x = unitX + circle->at(i2).x();
                    qint32 y = unitY + circle->at(i2).y();
                    if (pMap->onMap(x, y))
                    {
                        if (onSameIsland(islandIdx, curX, curY, x, y))
                        {
                            if (!targets.contains(QVector3D(x, y, 1)))
                            {
                                targets.append(QVector3D(x, y, 1));
                            }
                        }
                    }
                }
            }
        }
    }
}

bool NormalBehavioralModule::moveUnit(spGameAction pAction, Unit* pUnit, spQmlVectorUnit pUnits, QStringList& actions,
                        QVector<QVector3D>& targets, QVector<QVector3D>& transporterTargets,
                        bool shortenPathForTarget,
                        spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    Console::print("NormalBehavioralModule::moveUnit()", Console::eDEBUG);
    TargetedUnitPathFindingSystem pfs(pUnit, targets, &m_MoveCostMap);
    pfs.explore();
    qint32 movepoints = pUnit->getMovementpoints(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
    QPoint targetFields = pfs.getReachableTargetField(movepoints);
    if (targetFields.x() >= 0)
    {
        UnitPathFindingSystem turnPfs(pUnit);
        turnPfs.explore();
        if (NormalBehavioralModule::contains(transporterTargets, targetFields))
        {
            QVector<QPoint> path = turnPfs.getPath(targetFields.x(), targetFields.y());
            pAction->setMovepath(path, turnPfs.getCosts(path));
            pAction->setActionID(ACTION_LOAD);
            if (pAction->canBePerformed())
            {
                m_updatePoints.append(pUnit->getPosition());
                m_updatePoints.append(pAction->getActionTarget());
                emit m_pAdapta->performAction(pAction);
                return true;
            }
        }
        else if (!shortenPathForTarget && NormalBehavioralModule::contains(targets, targetFields))
        {
            QVector<QPoint> movePath = turnPfs.getClosestReachableMovePath(targetFields);
            pAction->setMovepath(movePath, turnPfs.getCosts(movePath));
            pAction->setActionID(ACTION_WAIT);
            if (pAction->canBePerformed())
            {
                m_updatePoints.append(pUnit->getPosition());
                m_updatePoints.append(pAction->getActionTarget());
                emit m_pAdapta->performAction(pAction);
                return true;
            }
        }
        else
        {
            QVector<QPoint> movePath = turnPfs.getClosestReachableMovePath(targetFields);
            if (movePath.size() == 0)
            {
                movePath.append(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
            }
            qint32 idx = getMoveTargetField(pUnit, pUnits, turnPfs, movePath, pBuildings, pEnemyBuildings);
            if (idx < 0)
            {
                std::tuple<QPoint, float, bool> target = moveToSafety(pUnit, pUnits, turnPfs, movePath[0], pBuildings, pEnemyBuildings);
                QPoint ret = std::get<0>(target);
                float minDamage = std::get<1>(target);
                bool allEqual = std::get<2>(target);
                if (((ret.x() == pUnit->Unit::getX() && ret.y() == pUnit->Unit::getY()) ||
                     minDamage > pUnit->getUnitValue() / 2 ||
                     allEqual) && minDamage > 0.0f)
                {
                    if (suicide(pAction, pUnit, turnPfs))
                    {
                        return true;
                    }
                    else
                    {
                        QVector<QPoint> movePath = turnPfs.getPath(ret.x(), ret.y());
                        pAction->setMovepath(movePath, turnPfs.getCosts(movePath));
                    }
                }
                else
                {
                    QVector<QPoint> movePath = turnPfs.getPath(ret.x(), ret.y());
                    pAction->setMovepath(movePath, turnPfs.getCosts(movePath));
                }
            }
            else
            {
                QVector<QPoint> path = turnPfs.getPath(movePath[idx].x(), movePath[idx].y());
                pAction->setMovepath(path, turnPfs.getCosts(path));
            }
            bool lockedUnit = (pAction->getMovePath().size() == 1) &&
                              (pUnit->getHp() < m_lockedUnitHp);
            // when we don't move try to attack if possible
            if ((pUnit->getHp() > m_noMoveAttackHp) ||
                lockedUnit)
            {
                pAction->setActionID(ACTION_FIRE);
                QVector<QVector3D> ret;
                QVector<QVector3D> moveTargetFields;
                getBestAttacksFromField(pUnit, pAction, ret, moveTargetFields);
                if (ret.size() > 0 &&
                    (ret[0].z() >= -pUnit->getUnitValue()  * m_minSuicideDamage ||
                     lockedUnit))
                {
                    qint32 selection = GlobalUtils::randIntBase(0, ret.size() - 1);
                    QVector3D target = ret[selection];
                    NormalBehavioralModule::addSelectedFieldData(pAction, QPoint(static_cast<qint32>(target.x()),
                                                                 static_cast<qint32>(target.y())));
                    if (pAction->isFinalStep() && pAction->canBePerformed())
                    {
                        m_updatePoints.append(pUnit->getPosition());
                        m_updatePoints.append(pAction->getActionTarget());
                        m_updatePoints.append(QPoint(static_cast<qint32>(target.x()),
                                                   static_cast<qint32>(target.y())));
                        emit m_pAdapta->performAction(pAction);
                        return true;
                    }
                }
            }
            if (pAction->getMovePath().size() > 0)
            {
                m_updatePoints.append(pUnit->getPosition());
                m_updatePoints.append(pAction->getActionTarget());
                for (const auto & action : actions)
                {
                    if (action.startsWith(ACTION_SUPPORTALL))
                    {
                        pAction->setActionID(action);
                        if (pAction->canBePerformed())
                        {
                            emit m_pAdapta->performAction(pAction);
                            return true;
                        }
                    }
                }
                if (actions.contains(ACTION_STEALTH))
                {
                    pAction->setActionID(ACTION_STEALTH);
                    if (pAction->canBePerformed())
                    {
                        emit m_pAdapta->performAction(pAction);
                        return true;
                    }
                }
                if (actions.contains(ACTION_UNSTEALTH))
                {
                    pAction->setActionID(ACTION_UNSTEALTH);
                    if (pAction->canBePerformed())
                    {
                        float counterDamage = calculateCounterDamage(pUnit, pUnits, pAction->getActionTarget(), nullptr, 0, pBuildings, pEnemyBuildings, true);
                        if (counterDamage <= 0)
                        {
                            emit m_pAdapta->performAction(pAction);
                            return true;
                        }
                    }
                }
                for (const auto & action : actions)
                {
                    if (action.startsWith(ACTION_PLACE))
                    {
                        pAction->setActionID(action);
                        if (pAction->canBePerformed())
                        {
                            spMarkedFieldData pData = pAction->getMarkedFieldStepData();
                            QPoint point = pData->getPoints()->at(GlobalUtils::randIntBase(0, pData->getPoints()->size() - 1));
                            NormalBehavioralModule::addSelectedFieldData(pAction, point);
                            emit m_pAdapta->performAction(pAction);
                            return true;
                        }
                    }
                }
                if (pUnit->canMoveAndFire(pAction->getActionTarget()) ||
                    pUnit->getPosition() == pAction->getActionTarget())
                {
                    pAction->setActionID(ACTION_FIRE);
                    // if we run away and still find a target we should attack it
                    QVector<QVector3D> moveTargets(1, QVector3D(pAction->getActionTarget().x(),
                                                                pAction->getActionTarget().y(), 1));
                    QVector<QVector3D> ret;
                    getBestAttacksFromField(pUnit, pAction, ret, moveTargets);
                    if (ret.size() > 0 && ret[0].z() >= -pUnit->getUnitValue()  * m_minSuicideDamage)
                    {
                        qint32 selection = GlobalUtils::randIntBase(0, ret.size() - 1);
                        QVector3D target = ret[selection];
                        NormalBehavioralModule::addSelectedFieldData(pAction, QPoint(static_cast<qint32>(target.x()),
                                                                     static_cast<qint32>(target.y())));
                        if (pAction->isFinalStep() && pAction->canBePerformed())
                        {
                            m_updatePoints.append(pUnit->getPosition());
                            m_updatePoints.append(pAction->getActionTarget());
                            m_updatePoints.append(QPoint(static_cast<qint32>(target.x()),
                                                       static_cast<qint32>(target.y())));
                            emit m_pAdapta->performAction(pAction);
                            return true;
                        }
                    }
                }
                pAction->setActionID(ACTION_WAIT);
                if (pAction->canBePerformed())
                {
                    emit m_pAdapta->performAction(pAction);
                    return true;
                }
                else
                {
                    Q_ASSERT(false);
                }
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::loadUnits(spQmlVectorUnit pUnits, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    Console::print("NormalBehavioralModule::loadUnits()", Console::eDEBUG);
    aiStep = NB_AISteps::loadUnits;
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        // can we use the unit?
        if (!pUnit->getHasMoved() &&
            // we don't support multi transporting for the ai for now this will break the system trust me
            pUnit->getLoadingPlace() <= 0)
        {
            QVector<QVector3D> targets;
            QVector<QVector3D> transporterTargets;
            spGameAction pAction = spGameAction::create(ACTION_LOAD);
            QStringList actions = pUnit->getActionList();
            // find possible targets for this unit
            pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));

            // find some cool targets
            appendTransporterTargets(pUnit, pUnits, transporterTargets);
            targets.append(transporterTargets);
            // till now the selected targets are a little bit lame cause we only search for reachable transporters
            // but not for reachable loading places.
            if (moveUnit(pAction, pUnit, pUnits, actions, targets, transporterTargets, true, pBuildings, pEnemyBuildings))
            {
                return true;
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::moveTransporters(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    Console::print("NormalBehavioralModule::moveTransporters()", Console::eDEBUG);
    aiStep = NB_AISteps::moveTransporters;
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        // can we use the unit?
        if (!pUnit->getHasMoved() && pUnit->getLoadingPlace() > 0)
        {
            // wooohooo it's a transporter
            if (pUnit->getLoadedUnitCount() > 0)
            {
                spGameAction pAction = spGameAction::create(ACTION_WAIT);
                QStringList actions = pUnit->getActionList();
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                // find possible targets for this unit
                QVector<QVector3D> targets;
                // can one of our units can capture buildings?
                for (qint32 i = 0; i < pUnit->getLoadedUnitCount(); i++)
                {
                    Unit* pLoaded = pUnit->getLoadedUnit(i);
                    if (pLoaded->getActionList().contains(ACTION_CAPTURE))
                    {
                        appendUnloadTargetsForCapturing(pUnit, pEnemyBuildings, targets);
                        break;
                    }
                }
                // if not find closest unloading field
                if (targets.size() == 0 || pUnit->getLoadedUnitCount() > 1)
                {
                    appendNearestUnloadTargets(pUnit, pEnemyUnits, pEnemyBuildings, targets);
                }
                if (moveToUnloadArea(pAction, pUnit, pUnits, actions, targets, pBuildings, pEnemyBuildings))
                {
                    return true;
                }
            }
            else
            {
                spGameAction pAction = spGameAction::create(ACTION_WAIT);
                QStringList actions = pUnit->getActionList();
                // find possible targets for this unit
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                // we need to move to a loading place
                QVector<QVector3D> targets;
                QVector<QVector3D> transporterTargets;
                appendLoadingTargets(pUnit, pUnits, pEnemyUnits, pEnemyBuildings, false, false, targets);
                if (targets.size() == 0)
                {
                    appendLoadingTargets(pUnit, pUnits, pEnemyUnits, pEnemyBuildings, true, false, targets);
                }
                if (moveUnit(pAction, pUnit, pUnits, actions, targets, transporterTargets, true, pBuildings, pEnemyBuildings))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::moveToUnloadArea(spGameAction pAction, Unit* pUnit, spQmlVectorUnit pUnits, QStringList& actions,
                                QVector<QVector3D>& targets,
                                spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    Console::print("NormalBehavioralModule::moveToUnloadArea()", Console::eDEBUG);
    spGameMap pMap = GameMap::getInstance();
    Interpreter* pInterpreter = Interpreter::getInstance();
    TargetedUnitPathFindingSystem pfs(pUnit, targets, &m_MoveCostMap);
    pfs.explore();
    qint32 movepoints = pUnit->getMovementpoints(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
    QPoint targetFields = pfs.getReachableTargetField(movepoints);
    if (targetFields.x() >= 0)
    {
        if (NormalBehavioralModule::contains(targets, targetFields))
        {
            UnitPathFindingSystem turnPfs(pUnit);
            turnPfs.explore();
            QVector<QPoint> path = turnPfs.getPath(targetFields.x(), targetFields.y());
            pAction->setMovepath(path, turnPfs.getCosts(path));
            pAction->setActionID(ACTION_UNLOAD);
            if (pAction->canBePerformed())
            {
                bool unloaded = false;
                QVector<qint32> unloadedUnits;
                do
                {
                    unloaded = false;
                    spMenuData pDataMenu = pAction->getMenuStepData();
                    if (pDataMenu->validData())
                    {
                        QStringList actions = pDataMenu->getActionIDs();
                        QVector<qint32> unitIDx = pDataMenu->getCostList();
                        QVector<QList<QVariant>> unloadFields;
                        for (qint32 i = 0; i < unitIDx.size() - 1; i++)
                        {
                            QString function1 = "getUnloadFields";
                            QJSValueList args1;
                            QJSValue obj1 = pInterpreter->newQObject(pAction.get());
                            args1 << obj1;
                            args1 << unitIDx[i];
                            QJSValue ret = pInterpreter->doFunction("ACTION_UNLOAD", function1, args1);
                            unloadFields.append(ret.toVariant().toList());
                        }
                        if (actions.size() > 1)
                        {
                            for (qint32 i = 0; i < unloadFields.size(); i++)
                            {
                                if (!needsRefuel(pUnit->getLoadedUnit(i)))
                                {
                                    if (!unloadedUnits.contains(unitIDx[i]))
                                    {
                                        if (unloadFields[i].size() == 1)
                                        {
                                            addMenuItemData(pAction, actions[i], unitIDx[i]);
                                            spMarkedFieldData pFields = pAction->getMarkedFieldStepData();
                                            addSelectedFieldData(pAction, pFields->getPoints()->at(0));
                                            unloaded = true;
                                            unloadedUnits.append(unitIDx[i]);
                                            break;
                                        }
                                        else if (unloadFields[i].size() > 0 &&
                                                 pUnit->getLoadedUnit(i)->getActionList().contains(ACTION_CAPTURE))
                                        {
                                            for (qint32 i2 = 0; i2 < unloadFields[i].size(); i2++)
                                            {
                                                QPoint unloadField = unloadFields[i][i2].toPoint();
                                                Building* pBuilding = pMap->getTerrain(unloadField.x(),
                                                                                       unloadField.y())->getBuilding();
                                                if (pBuilding != nullptr && m_pPlayer->isEnemy(pBuilding->getOwner()))
                                                {
                                                    addMenuItemData(pAction, actions[i], unitIDx[i]);
                                                    addSelectedFieldData(pAction, unloadField);
                                                    unloaded = true;
                                                    unloadedUnits.append(unitIDx[i]);
                                                    break;
                                                }
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                            if (unloaded == false &&
                                !needsRefuel(pUnit->getLoadedUnit(0)))
                            {
                                qint32 costs = pDataMenu->getCostList()[0];
                                addMenuItemData(pAction, actions[0], costs);
                                unloaded = true;
                                spMarkedFieldData pFields = pAction->getMarkedFieldStepData();
                                qint32 field = GlobalUtils::randIntBase(0, pFields->getPoints()->size() - 1);
                                addSelectedFieldData(pAction, pFields->getPoints()->at(field));
                            }
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
                while (unloaded);
                addMenuItemData(pAction, ACTION_WAIT, 0);
                m_updatePoints.append(pUnit->getPosition());
                m_updatePoints.append(pAction->getActionTarget());
                if (pAction->canBePerformed())
                {
                    emit m_pAdapta->performAction(pAction);
                    return true;
                }
            }
        }
        else
        {
            return moveUnit(pAction, pUnit, pUnits, actions, targets, targets, true, pBuildings, pEnemyBuildings);
        }
    }
    return false;
}

bool NormalBehavioralModule::repairUnits(spQmlVectorUnit pUnits, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    Console::print("NormalBehavioralModule::repairUnits()", Console::eDEBUG);
    aiStep = NB_AISteps::moveUnits;
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        // can we use the unit?
        if (!isUsingUnit(pUnit) && !pUnit->getHasMoved())
        {
            QVector<QVector3D> targets;
            QVector<QVector3D> transporterTargets;
            spGameAction pAction = spGameAction::create(ACTION_WAIT);
            QStringList actions = pUnit->getActionList();
            // find possible targets for this unit
            pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
            appendRepairTargets(pUnit, pBuildings, targets);
            if (needsRefuel(pUnit))
            {
                appendTransporterTargets(pUnit, pUnits, transporterTargets);
                targets.append(transporterTargets);
            }
            if (moveUnit(pAction, pUnit, pUnits, actions, targets, transporterTargets, false, pBuildings, pEnemyBuildings))
            {
                return true;
            }
            else
            {
                pAction = spGameAction::create(ACTION_WAIT);
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                UnitPathFindingSystem turnPfs(pUnit);
                turnPfs.explore();
                if (suicide(pAction, pUnit, turnPfs))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

qint32 NormalBehavioralModule::getMoveTargetField(Unit* pUnit, spQmlVectorUnit pUnits, UnitPathFindingSystem& turnPfs,
                                    QVector<QPoint>& movePath, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    spGameMap pMap = GameMap::getInstance();
    for (qint32 i = 0; i < movePath.size(); i++)
    {
        // empty or own field
        qint32 x = movePath[i].x();
        qint32 y = movePath[i].y();
        if ((pMap->getTerrain(x, y)->getUnit() == nullptr ||
             pMap->getTerrain(x, y)->getUnit() == pUnit) &&
            turnPfs.getCosts(turnPfs.getIndex(x, y), x, y, x, y) > 0)
        {
            float counterDamage = calculateCounterDamage(pUnit, pUnits, movePath[i], nullptr, 0.0f, pBuildings, pEnemyBuildings);
            if (counterDamage < pUnit->getUnitValue() * m_minMovementDamage)
            {
                return i;
            }
        }
    }
    return -1;
}

std::tuple<QPoint, float, bool> NormalBehavioralModule::moveToSafety(Unit* pUnit, spQmlVectorUnit pUnits,
                                                       UnitPathFindingSystem& turnPfs, QPoint target,
                                                       spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    Console::print("NormalBehavioralModule::moveToSafety", Console::eDEBUG);
    spGameMap pMap = GameMap::getInstance();
    QVector<QPoint> targets = turnPfs.getAllNodePoints();
    QPoint ret(pUnit->Unit::getX(), pUnit->Unit::getY());
    float leastDamageField = std::numeric_limits<float>::max();
    qint32 shortestDistance = std::numeric_limits<qint32>::max();
    bool allFieldsEqual = true;
    for (qint32 i = 0; i < targets.size(); i++)
    {
        qint32 x = targets[i].x();
        qint32 y = targets[i].y();
        if (pMap->getTerrain(x, y)->getUnit() == nullptr &&
            turnPfs.getCosts(turnPfs.getIndex(x, y), x, y, x, y) > 0)
        {
            float currentDamage = calculateCounterDamage(pUnit, pUnits, targets[i], nullptr, 0.0f, pBuildings, pEnemyBuildings);
            if (leastDamageField < std::numeric_limits<float>::max() &&
                static_cast<qint32>(leastDamageField) != static_cast<qint32>(currentDamage))
            {
                allFieldsEqual = false;
            }
            qint32 distance = GlobalUtils::getDistance(target, targets[i]);
            if (currentDamage < leastDamageField)
            {
                ret = targets[i];
                leastDamageField = currentDamage;
                shortestDistance = distance;
            }
            else if (static_cast<qint32>(currentDamage) == static_cast<qint32>(leastDamageField) && distance < shortestDistance)
            {
                ret = targets[i];
                leastDamageField = currentDamage;
                shortestDistance = distance;
            }
        }
    }
    return std::tuple<QPoint, float, bool>(ret, leastDamageField, allFieldsEqual);
}

bool NormalBehavioralModule::captureBuildings(spQmlVectorUnit pUnits)
{
    Console::print("NormalBehavioralModule::captureBuildings()", Console::eDEBUG);
    QVector<QVector3D> captureBuildings;
    qint32 cost = 0;
    QPoint rocketTarget = m_pPlayer->getSiloRockettarget(2, 3, cost);
    bool fireSilos = hasMissileTarget();
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        if (!pUnit->getHasMoved() && pUnit->getActionList().contains(ACTION_CAPTURE))
        {
            if (pUnit->getCapturePoints() > 0)
            {
                spGameAction pAction = spGameAction::create(ACTION_CAPTURE);
                pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                if (pAction->canBePerformed())
                {
                    emit m_pAdapta->performAction(pAction);
                    return true;
                }
            }
            else
            {
                GameAction action(ACTION_CAPTURE);
                action.setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                UnitPathFindingSystem pfs(pUnit);
                pfs.explore();
                QVector<QPoint> targets = pfs.getAllNodePoints();
                for (qint32 i2 = 0; i2 < targets.size(); i2++)
                {
                    action.setActionID(ACTION_CAPTURE);
                    action.setMovepath(QVector<QPoint>(1, targets[i2]), 0);
                    if (action.canBePerformed())
                    {
                        captureBuildings.append(QVector3D(targets[i2].x(), targets[i2].y(), i));
                    }
                    else
                    {
                        action.setActionID(ACTION_MISSILE);
                        if (action.canBePerformed() && fireSilos)
                        {
                            captureBuildings.append(QVector3D(targets[i2].x(), targets[i2].y(), i));
                        }
                    }
                }
            }
        }
    }
    if (captureBuildings.size() > 0)
    {
        spGameMap pMap = GameMap::getInstance();
        for (qint32 i = 0; i < pUnits->size(); i++)
        {
            Unit* pUnit = pUnits->at(i);
            if (!pUnit->getHasMoved() && pUnit->getActionList().contains(ACTION_CAPTURE))
            {
                QVector<QVector3D> captures;
                for (qint32 i2 = 0; i2 < captureBuildings.size(); i2++)
                {
                    if (static_cast<qint32>(captureBuildings[i2].z()) == i)
                    {
                        captures.append(captureBuildings[i2]);
                    }
                }
                bool perform = false;
                qint32 targetIndex = 0;
                bool productionBuilding = false;
                if (captures.size() > 0)
                {
                    if (captures.size() == 0)
                    {
                        // we have only one target go for it
                        targetIndex = 0;
                        perform = true;
                    }
                    else
                    {
                        // check if we have a building only we can capture and capture it
                        for (qint32 i2 = 0; i2 < captures.size(); i2++)
                        {
                            qint32 captureCount = 0;
                            for (qint32 i3 = 0; i3 < captureBuildings.size(); i3++)
                            {
                                if (static_cast<qint32>(captureBuildings[i3].x()) == static_cast<qint32>(captures[i2].x()) &&
                                    static_cast<qint32>(captureBuildings[i3].y()) == static_cast<qint32>(captures[i2].y()))
                                {
                                    captureCount++;
                                }
                            }
                            bool isProductionBuilding = pMap->getTerrain(static_cast<qint32>(captures[i2].x()), static_cast<qint32>(captures[i2].y()))->getBuilding()->getActionList().contains(ACTION_BUILD_UNITS);
                            if ((captureCount == 1 && perform == false) ||
                                (captureCount == 1 && productionBuilding == false && perform == true && isProductionBuilding))
                            {
                                productionBuilding = isProductionBuilding;
                                targetIndex = i2;
                                perform = true;
                            }
                        }
                        // check if there unique captures open
                        bool skipUnit = false;
                        for (qint32 i2 = 0; i2 < captureBuildings.size(); i2++)
                        {
                            qint32 captureCount = 0;
                            for (qint32 i3 = 0; i3 < captureBuildings.size(); i3++)
                            {
                                if (static_cast<qint32>(captureBuildings[i3].x()) == static_cast<qint32>(captureBuildings[i2].x()) &&
                                    static_cast<qint32>(captureBuildings[i3].y()) == static_cast<qint32>(captureBuildings[i2].y()))
                                {
                                    captureCount++;
                                }
                            }
                            if (captureCount == 1)
                            {
                                skipUnit = true;
                            }
                        }
                        // if not we can select a target from the list
                        if (!skipUnit)
                        {
                            targetIndex = 0;
                            qint32 prio = -1;
                            // priorities production buildings over over captures
                            for (qint32 i2 = 0; i2 < captures.size(); i2++)
                            {
                                Building* pBuilding = pMap->getTerrain(static_cast<qint32>(captures[i2].x()), static_cast<qint32>(captures[i2].y()))->getBuilding();
                                qint32 testPrio = std::numeric_limits<qint32>::min();
                                if (pBuilding->getBuildingID() == "HQ")
                                {
                                    testPrio = std::numeric_limits<qint32>::max();
                                }
                                else if (pBuilding->getActionList().contains(ACTION_BUILD_UNITS))
                                {
                                    testPrio = pBuilding->getConstructionList().size();
                                }
                                if (testPrio > prio)
                                {
                                    targetIndex = i2;
                                    prio = testPrio;
                                }
                            }
                            perform = true;
                        }
                    }
                }
                // perform capturing
                if (perform)
                {
                    UnitPathFindingSystem pfs(pUnit);
                    pfs.explore();
                    spGameAction pAction = spGameAction::create(ACTION_CAPTURE);
                    pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
                    QVector<QPoint> path = pfs.getPath(static_cast<qint32>(captures[targetIndex].x()), static_cast<qint32>(captures[targetIndex].y()));
                    pAction->setMovepath(path, pfs.getCosts(path));
                    m_updatePoints.append(pUnit->getPosition());
                    m_updatePoints.append(pAction->getActionTarget());
                    if (pAction->canBePerformed())
                    {
                        emit m_pAdapta->performAction(pAction);
                        return true;
                    }
                    else
                    {
                        NormalBehavioralModule::addSelectedFieldData(pAction, rocketTarget);
                        pAction->setActionID(ACTION_MISSILE);
                        if (pAction->canBePerformed())
                        {
                            emit m_pAdapta->performAction(pAction);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::fireWithUnits(spQmlVectorUnit pUnits, qint32 minfireRange, qint32 maxfireRange,
                             spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    Console::print("NormalBehavioralModule::fireWithUnits()", Console::eDEBUG);
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        if (!pUnit->getHasMoved() &&
            pUnit->getBaseMaxRange() >= minfireRange &&
            pUnit->getBaseMaxRange() <= maxfireRange &&
            (pUnit->getAmmo1() > 0 || pUnit->getAmmo2() > 0) &&
            pUnit->hasAction(NormalBehavioralModule::ACTION_FIRE))
        {
            spGameAction pAction = spGameAction::create(ACTION_FIRE);
            pAction->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
            UnitPathFindingSystem pfs(pUnit);
            pfs.explore();
            QVector<QVector4D> ret;
            QVector<QVector3D> moveTargetFields;
            NormalBehavioralModule::getAttackTargets(pUnit, pAction, &pfs, ret, moveTargetFields);
            qint32 targetIdx = getBestAttackTarget(pUnit, pUnits, ret, moveTargetFields, pBuildings, pEnemyBuildings);
            if (targetIdx >= 0)
            {
                QVector4D target = ret[targetIdx];
                QVector<QPoint> path = pfs.getPath(static_cast<qint32>(moveTargetFields[targetIdx].x()),
                                                   static_cast<qint32>(moveTargetFields[targetIdx].y()));
                pAction->setMovepath(path, pfs.getCosts(path));
                NormalBehavioralModule::addSelectedFieldData(pAction, QPoint(static_cast<qint32>(target.x()), static_cast<qint32>(target.y())));
                if (GameMap::getInstance()->getTerrain(static_cast<qint32>(target.x()), static_cast<qint32>(target.y()))->getUnit() == nullptr)
                {
                    m_IslandMaps.clear();
                }
                if (pAction->isFinalStep() && pAction->canBePerformed())
                {
                    m_updatePoints.append(pUnit->getPosition());
                    m_updatePoints.append(pAction->getActionTarget());
                    m_updatePoints.append(QPoint(static_cast<qint32>(target.x()), static_cast<qint32>(target.y())));
                    emit m_pAdapta->performAction(pAction);
                    return true;
                }
            }
        }
    }
    return false;
}

bool NormalBehavioralModule::suicide(spGameAction pAction, Unit* pUnit, UnitPathFindingSystem& turnPfs)
{
    Console::print("NormalBehavioralModule::suicide", Console::eDEBUG);
    // we don't have a good option do the best that we can attack with an all in attack :D
    pAction->setActionID(ACTION_FIRE);
    QVector<QVector3D> ret;
    QVector<QVector3D> moveTargetFields;
    NormalBehavioralModule::getBestTarget(pUnit, pAction, &turnPfs, ret, moveTargetFields);
    if (ret.size() > 0 && ret[0].z() >= -pUnit->getUnitValue() * m_minSuicideDamage)
    {
        qint32 selection = GlobalUtils::randIntBase(0, ret.size() - 1);
        QVector3D target = ret[selection];
        QVector<QPoint> path = turnPfs.getPath(static_cast<qint32>(moveTargetFields[selection].x()),
                                               static_cast<qint32>(moveTargetFields[selection].y()));
        pAction->setMovepath(path, turnPfs.getCosts(path));
        NormalBehavioralModule::addSelectedFieldData(pAction, QPoint(static_cast<qint32>(target.x()),
                                                     static_cast<qint32>(target.y())));
        if (pAction->isFinalStep() && pAction->canBePerformed())
        {
            m_updatePoints.append(pUnit->getPosition());
            m_updatePoints.append(pAction->getActionTarget());
            m_updatePoints.append(QPoint(static_cast<qint32>(target.x()),
                                       static_cast<qint32>(target.y())));
            emit m_pAdapta->performAction(pAction);
            return true;
        }
    }
    return false;
}

qint32 NormalBehavioralModule::getBestAttackTarget(Unit* pUnit, spQmlVectorUnit pUnits, QVector<QVector4D>& ret,
                                     QVector<QVector3D>& moveTargetFields,
                                     spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    spGameMap pMap = GameMap::getInstance();
    qint32 target = -1;
    qint32 currentDamage = std::numeric_limits<qint32>::min();
    qint32 deffense = 0;

    float minFundsDamage = -pUnit->getUnitValue() * m_minAttackFunds;

    for (qint32 i = 0; i < ret.size(); i++)
    {
        QPoint moveTarget(static_cast<qint32>(moveTargetFields[i].x()), static_cast<qint32>(moveTargetFields[i].y()));
        Unit* pEnemy = pMap->getTerrain(static_cast<qint32>(ret[i].x()), static_cast<qint32>(ret[i].y()))->getUnit();
        qint32 minfireRange = pUnit->getMinRange(moveTarget);
        qint32 fundsDamage = 0;
        float newHp = 0.0f;
        if (pEnemy != nullptr)
        {
            newHp = pEnemy->getHp() - static_cast<float>(ret[i].w());
            fundsDamage = static_cast<qint32>(ret[i].z() * calculateCaptureBonus(pEnemy, newHp));
            if (minfireRange > 1)
            {
                fundsDamage *= m_ownIndirectAttackValue;
            }
            if (newHp <= 0)
            {
                fundsDamage *= m_enemyKillBonus;
            }
            if (pEnemy->getMinRange(pEnemy->getPosition()) > 1)
            {
                fundsDamage *= m_enemyIndirectBonus;
            }

        }
        else
        {
            fundsDamage = static_cast<qint32>(ret[i].z());
        }
        float counterDamage = calculateCounterDamage(pUnit, pUnits, moveTarget, pEnemy, ret[i].w(), pBuildings, pEnemyBuildings);
        if (counterDamage < 0)
        {
            counterDamage = 0;
        }
        fundsDamage -= counterDamage;
        qint32 targetDefense = pMap->getTerrain(static_cast<qint32>(ret[i].x()), static_cast<qint32>(ret[i].y()))->getDefense(pUnit);
        if (fundsDamage >= minFundsDamage)
        {
            if (fundsDamage > currentDamage)
            {
                currentDamage = fundsDamage;
                target = i;
                deffense = targetDefense;
            }
            else if (fundsDamage == currentDamage && targetDefense > deffense)
            {
                currentDamage = fundsDamage;
                target = i;
                deffense = targetDefense;
            }
        }
    }
    return target;
}

void NormalBehavioralModule::updateEnemyData(spQmlVectorUnit pUnits)
{
    Console::print("NormalBehavioralModule::updateEnemyData", Console::eDEBUG);
    rebuildIsland(pUnits);
    if (m_EnemyUnits.size() == 0)
    {
        m_EnemyUnits = m_pPlayer->getSpEnemyUnits();
        for (qint32 i = 0; i < m_EnemyUnits.size(); i++)
        {
            m_EnemyPfs.append(spUnitPathFindingSystem::create(m_EnemyUnits[i].get()));
            m_EnemyPfs[i]->explore();
            m_VirtualEnemyData.append(QPointF(0, 0));
        }
        calcVirtualDamage(pUnits);
    }
    else
    {
        qint32 i = 0;
        while (i < m_EnemyUnits.size())
        {
            if (m_EnemyUnits[i]->getHp() <= 0 ||
                m_EnemyUnits[i]->getTerrain() == nullptr)
            {
                m_EnemyUnits.removeAt(i);
                m_EnemyPfs.removeAt(i);
                m_VirtualEnemyData.removeAt(i);
            }
            else
            {
                i++;
            }
        }
    }
    QVector<qint32> updated;
    for (qint32 i = 0; i < m_updatePoints.size(); i++)
    {
        for (qint32 i2 = 0; i2 < m_EnemyUnits.size(); i2++)
        {
            if (!updated.contains(i2))
            {
                if (m_EnemyUnits[i2]->getHp() > 0 && m_EnemyUnits[i2]->getTerrain() != nullptr)
                {
                    if (qAbs(m_updatePoints[i].x() - m_EnemyUnits[i2]->Unit::getX()) +
                        qAbs(m_updatePoints[i].y() - m_EnemyUnits[i2]->Unit::getY()) <=
                        m_EnemyUnits[i2]->getMovementpoints(QPoint(m_EnemyUnits[i2]->Unit::getX(), m_EnemyUnits[i2]->Unit::getY())) + 2)
                    {
                        m_EnemyPfs[i2] = spUnitPathFindingSystem::create(m_EnemyUnits[i2].get());
                        m_EnemyPfs[i2]->explore();
                    }
                    updated.push_back(i2);
                }
            }
        }
    }
    m_updatePoints.clear();
}

void NormalBehavioralModule::calcVirtualDamage(spQmlVectorUnit pUnits)
{
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        QVector<QPoint> attackedUnits;
        QVector<float> damage;
        if (isUsingUnit(pUnit))
        {
            spGameAction action = spGameAction::create(ACTION_FIRE);
            action->setTarget(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY()));
            UnitPathFindingSystem pfs(pUnit);
            pfs.explore();
            QVector<QVector4D> ret;
            QVector<QVector3D> moveTargetFields;
            NormalBehavioralModule::getAttackTargets(pUnit, action, &pfs, ret, moveTargetFields);
            for (qint32 i2 = 0; i2 < ret.size(); i2++)
            {
                QPoint pos(static_cast<qint32>(ret[i2].x()), static_cast<qint32>(ret[i2].y()));
                if (!attackedUnits.contains(pos))
                {
                    attackedUnits.append(pos);
                    damage.append(ret[i2].w());
                }
            }
        }
        for (qint32 i2 = 0; i2 < attackedUnits.size(); i2++)
        {
            for (qint32 i3 = 0; i3 < m_EnemyUnits.size(); i3++)
            {
                if (m_EnemyUnits[i3]->Unit::getX() == attackedUnits[i2].x() &&
                    m_EnemyUnits[i3]->Unit::getY() == attackedUnits[i2].y())
                {
                    m_VirtualEnemyData[i3].setX(m_VirtualEnemyData[i3].x() + m_enemyUnitCountDamageReductionMultiplier * static_cast<double>(damage[i2]) / (damage.size()));
                    break;
                }
            }
        }
    }
}

float NormalBehavioralModule::calculateCaptureBonus(Unit* pUnit, float newLife)
{
    float ret = 1.0f;
    qint32 capturePoints = pUnit->getCapturePoints();
    Building* pBuilding = pUnit->getTerrain()->getBuilding();
    if (capturePoints > 0)
    {
        qint32 restCapture = 20 - capturePoints;
        qint32 currentHp = pUnit->getHpRounded();
        qint32 newHp = GlobalUtils::roundUp(newLife);
        qint32 remainingDays = GlobalUtils::roundUp(restCapture / static_cast<float>(currentHp));
        if (newHp <= 0)
        {
            if (remainingDays > 0)
            {
                ret = 1 + (m_antiCaptureBonus - currentHp) / currentHp;
            }
            else
            {
                ret = m_antiCaptureBonus + 1.0f;
            }
        }
        else
        {
            qint32 newRemainingDays = GlobalUtils::roundUp(restCapture / static_cast<float>(newHp));
            if (remainingDays > newRemainingDays)
            {
                ret = 0.8f;
            }
            else if (remainingDays == newRemainingDays && remainingDays < 2)
            {
                ret = 1.0f;
            }
            else if (remainingDays == 0)
            {
                ret = 1.0f;
            }
            else
            {
                ret = 1 + (newRemainingDays - remainingDays) / remainingDays;
            }
            if (ret > m_antiCaptureBonusScoreReduction)
            {
                ret = ret / m_antiCaptureBonusScoreDivider + m_antiCaptureBonusScoreReduction / m_antiCaptureBonusScoreDivider;
            }
        }
    }
    if (pBuilding != nullptr &&
        pBuilding->getOwner() == m_pPlayer &&
        pBuilding->getBuildingID() == "HQ" &&
        pUnit->getActionList().contains(ACTION_CAPTURE))
    {
        ret *= m_antiCaptureHqBonus;
    }
    return ret;
}

float NormalBehavioralModule::calculateCounterDamage(Unit* pUnit, spQmlVectorUnit pUnits, QPoint newPosition,
                                       Unit* pEnemyUnit, float enemyTakenDamage,
                                       spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings,
                                       bool ignoreOutOfVisionRange)
{
    qint32 baseCost = pUnit->getUnitValue();
    QVector<qint32> baseCosts;
    for (qint32 i3 = 0; i3 < pUnits->size(); i3++)
    {
        baseCosts.append(pUnits->at(i3)->getUnitValue());
    }
    spGameMap pMap = GameMap::getInstance();
    float counterDamage = 0;
    for (qint32 i = 0; i < m_EnemyUnits.size(); i++)
    {
        spUnit pNextEnemy = m_EnemyUnits[i];
        if (pNextEnemy->getHp() > 0 && pNextEnemy->getTerrain() != nullptr)
        {
            QPoint enemyPos = QPoint(pNextEnemy->Unit::getX(), pNextEnemy->Unit::getY());
            qint32 minFireRange = pNextEnemy->getMinRange(enemyPos);
            qint32 maxFireRange = pNextEnemy->getMaxRange(enemyPos);
            qint32 moveRange = 0;
            qint32 distance = GlobalUtils::getDistance(newPosition, enemyPos);
            bool canMoveAndFire = pNextEnemy->canMoveAndFire(enemyPos);
            if (canMoveAndFire)
            {
                moveRange = pNextEnemy->getMovementpoints(enemyPos);
            }
            if (distance <= moveRange + maxFireRange &&
                pNextEnemy->isAttackable(pUnit, true))
            {
                float enemyDamage = static_cast<float>(m_VirtualEnemyData[i].x());
                if (pNextEnemy.get() == pEnemyUnit)
                {
                    enemyDamage += enemyTakenDamage;
                }
                enemyDamage *= m_enemyCounterDamageMultiplier;
                QRectF damageData;
                QVector<QPoint> targets = m_EnemyPfs[i]->getAllNodePoints();
                if (distance >= minFireRange && distance <= maxFireRange)
                {
                    damageData = NormalBehavioralModule::calcVirtuelUnitDamage(pNextEnemy.get(), enemyDamage, enemyPos, pUnit, 0, newPosition, ignoreOutOfVisionRange);
                    for (qint32 i3 = 0; i3 < pUnits->size(); i3++)
                    {
                        distance = GlobalUtils::getDistance(QPoint(pUnits->at(i3)->Unit::getX(), pUnits->at(i3)->Unit::getY()), enemyPos);
                        if (distance >= minFireRange && distance <= maxFireRange &&
                            pNextEnemy->isAttackable(pUnits->at(i3), true))
                        {
                            if (baseCosts[i3] > 0 && baseCost > 0)
                            {
                                if (baseCost > baseCosts[i3])
                                {
                                    // reduce damage the more units it can attack
                                    damageData.setX(damageData.x() -  damageData.x() * baseCosts[i3] / baseCost / 2);
                                }
                                else
                                {
                                    damageData.setX(damageData.x() -  damageData.x() *  baseCost / baseCosts[i3] / 2);
                                }
                            }
                        }
                    }
                }
                else if (canMoveAndFire)
                {
                    for (qint32 i2 = 0; i2 < targets.size(); i2++)
                    {
                        distance = GlobalUtils::getDistance(newPosition, targets[i2]);
                        if (distance >= minFireRange && distance <= maxFireRange &&
                            (pMap->getTerrain(targets[i2].x(), targets[i2].y())->getUnit() == nullptr ||
                             (targets[i2].x() == pNextEnemy->Unit::getX() && targets[i2].y() == pNextEnemy->Unit::getY())))
                        {
                            damageData = NormalBehavioralModule::calcVirtuelUnitDamage(pNextEnemy.get(), enemyDamage, targets[i2], pUnit, 0, newPosition, ignoreOutOfVisionRange);
                            break;
                        }
                    }
                    for (qint32 i2 = 0; i2 < targets.size(); i2++)
                    {
                        for (qint32 i3 = 0; i3 < pUnits->size(); i3++)
                        {
                            distance = GlobalUtils::getDistance(QPoint(pUnits->at(i3)->Unit::getX(), pUnits->at(i3)->Unit::getY()), targets[i2]);
                            if (distance >= minFireRange && distance <= maxFireRange &&
                                (pMap->getTerrain(targets[i2].x(), targets[i2].y())->getUnit() == nullptr ||
                                 pMap->getTerrain(targets[i2].x(), targets[i2].y())->getUnit()->getOwner()->isAlly(m_pPlayer)) &&
                                pNextEnemy->isAttackable(pUnits->at(i3), true))
                            {
                                if (baseCosts[i3] > 0 && baseCost > 0)
                                {
                                    if (baseCost > baseCosts[i3])
                                    {
                                        // reduce damage the more units it can attack
                                        damageData.setX(damageData.x() -  damageData.x() * baseCosts[i3] / baseCost / 2);
                                    }
                                    else
                                    {
                                        damageData.setX(damageData.x() -  damageData.x() *  baseCost / baseCosts[i3] / 2);
                                    }
                                }
                            }
                        }
                    }
                }

                if (damageData.x() < 0)
                {
                    damageData.setX(0);
                }
                if (damageData.x() > 0)
                {
                    counterDamage += static_cast<qint32>(calcFundsDamage(damageData, pNextEnemy.get(), pUnit).y());
                }
            }
        }
    }
    counterDamage += calculateCounteBuildingDamage(pUnit, newPosition, pBuildings, pEnemyBuildings);
    static qint32 min = std::numeric_limits<qint32>::max();
    static qint32 max = std::numeric_limits<qint32>::min();
    if (counterDamage < min)
    {
        min = counterDamage;
    }
    if (counterDamage > max)
    {
        max = counterDamage;
    }
    return counterDamage;
}

float NormalBehavioralModule::calculateCounteBuildingDamage(Unit* pUnit, QPoint newPosition, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings)
{
    float counterDamage = 0.0f;
    for (qint32 i = 0; i < pBuildings->size(); i++)
    {
        Building* pBuilding = pBuildings->at(i);
        counterDamage += calcBuildingDamage(pUnit, newPosition, pBuilding);
    }
    for (qint32 i = 0; i < pEnemyBuildings->size(); i++)
    {
        Building* pBuilding = pEnemyBuildings->at(i);
        counterDamage += calcBuildingDamage(pUnit, newPosition, pBuilding);
    }
    spQmlVectorPoint pCircle = GlobalUtils::getCircle(1, 2);
    spGameMap pMap = GameMap::getInstance();
    for (qint32 i = 0; i < pCircle->size(); i++)
    {
        QPoint pos = newPosition + pCircle->at(i);
        if (pMap->onMap(pos.x(), pos.y()))
        {
            Unit* pMine = pMap->getTerrain(pos.x(), pos.y())->getUnit();
            if (pMine != nullptr &&
                !pMine->isStealthed(m_pPlayer) &&
                pMine->getUnitID() == "WATERMINE")
            {
                counterDamage +=  m_watermineDamage;
            }
        }
    }
    return counterDamage;
}

void NormalBehavioralModule::clearEnemyData()
{
    m_VirtualEnemyData.clear();
    m_EnemyUnits.clear();
    m_EnemyPfs.clear();
}

void NormalBehavioralModule::getEnemyDamageCounts(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits, QVector<QVector4D> & attackCount)
{
    WeaponManager* pWeaponManager = WeaponManager::getInstance();
    for (qint32 i2 = 0; i2 < pEnemyUnits->size(); i2++)
    {
        for (qint32 i = 0; i < pUnits->size(); i++)
        {
            Unit* pUnit = pUnits->at(i);
            float dmg1 = 0.0f;
            float hpValue = pUnit->getHpRounded() / Unit::MAX_UNIT_HP;
            Unit* pEnemyUnit = pEnemyUnits->at(i2);
            // get weapon 1 damage
            if (!pUnit->getWeapon1ID().isEmpty())
            {
                dmg1 = pWeaponManager->getBaseDamage(pUnit->getWeapon1ID(), pEnemyUnit) * hpValue;
            }
            // get weapon 2 damage
            float dmg2 = 0.0f;
            if (!pUnit->getWeapon2ID().isEmpty())
            {
                dmg2 = pWeaponManager->getBaseDamage(pUnit->getWeapon2ID(), pEnemyUnit) * hpValue;
            }

            if ((dmg1 > m_notAttackableDamage || dmg2 > m_notAttackableDamage) &&
                pEnemyUnit->getMovementpoints(QPoint(pEnemyUnit->Unit::getX(), pEnemyUnit->Unit::getY())) - pUnit->getMovementpoints(QPoint(pUnit->Unit::getX(), pUnit->Unit::getY())) < 2)
            {
                if (onSameIsland(pUnit, pEnemyUnits->at(i2)))
                {
                    attackCount[i2].setY(attackCount[i2].y() + 1);
                }
                attackCount[i2].setX(attackCount[i2].x() + 1);
            }
            if (dmg1 > m_midDamage || dmg2 > m_midDamage)
            {
                attackCount[i2].setZ(attackCount[i2].z() + 1);
            }
            if (dmg1 > m_highDamage || dmg2 > m_highDamage)
            {
                attackCount[i2].setW(attackCount[i2].w() + 1);
            }
        }
    }
}

std::tuple<float, qint32> NormalBehavioralModule::calcExpectedFundsDamage(qint32 posX, qint32 posY, Unit& dummy, spQmlVectorUnit pEnemyUnits, QVector<QVector4D> attackCount, float bonusFactor)
{
    Console::print("NormalBehavioralModule::calcExpectedFundsDamage() find enemies", Console::eDEBUG);
    WeaponManager* pWeaponManager = WeaponManager::getInstance();
    qint32 notAttackableCount = 0;
    float damageCount = 0;
    float attacksCount = 0;
    float myMovepoints = dummy.getBaseMovementPoints();
    if (myMovepoints == 0)
    {
        myMovepoints = 1;
    }
    spGameMap pMap = GameMap::getInstance();
    qint32 maxCounter = pMap->getMapWidth() * pMap->getMapHeight() / (myMovepoints * 2);
    float myFirerange = dummy.getBaseMaxRange();
    float enemyFirerange = dummy.getBaseMaxRange();
    QPoint position = dummy.getPosition();
    qint32 counter = 1;
    while (attacksCount  < m_maxBuildingTargetFindLoops &&
           pEnemyUnits->size() > m_maxBuildingTargetFindLoops &&
           counter <= maxCounter)
    {
        for (qint32 i3 = 0; i3 < pEnemyUnits->size(); i3++)
        {
            Unit* pEnemyUnit = pEnemyUnits->at(i3);
            QPoint enemyPosition = pEnemyUnit->getPosition();
            float distance = GlobalUtils::getDistance(position, enemyPosition);
            if (distance / myMovepoints <= counter &&
                distance / myMovepoints >= (counter - 1))
            {
                float dmg = 0.0f;
                if (!dummy.getWeapon1ID().isEmpty())
                {
                    dmg = pWeaponManager->getBaseDamage(dummy.getWeapon1ID(), pEnemyUnit);
                }
                if (!dummy.getWeapon2ID().isEmpty())
                {
                    float dmg2 = pWeaponManager->getBaseDamage(dummy.getWeapon2ID(), pEnemyUnit);
                    if (dmg2 > dmg)
                    {
                        dmg = dmg2;
                    }
                }
                if (dmg > pEnemyUnit->getHp() * Unit::MAX_UNIT_HP)
                {
                    dmg = pEnemyUnit->getHp() * Unit::MAX_UNIT_HP;
                }
                if (dmg > 0.0f)
                {
                    float counterDmg = 0;
                    if (!pEnemyUnit->getWeapon1ID().isEmpty())
                    {
                        counterDmg = pWeaponManager->getBaseDamage(pEnemyUnit->getWeapon1ID(), &dummy);
                    }
                    if (!pEnemyUnit->getWeapon2ID().isEmpty())
                    {
                        float dmg2 = pWeaponManager->getBaseDamage(pEnemyUnit->getWeapon2ID(), &dummy);
                        if (dmg2 > counterDmg)
                        {
                            counterDmg = dmg2;
                        }
                    }
                    if (counterDmg > m_scoringCutOffDamageHigh)
                    {
                        counterDmg = m_scoringCutOffDamageHigh;
                    }
                    else if (counterDmg < m_scoringCutOffDamageLow)
                    {
                        counterDmg = 0.0f;
                    }
                    float resDamage = 0;

                    float enemyMovepoints = pEnemyUnit->getBaseMovementPoints();
                    if (myMovepoints + myFirerange >= enemyMovepoints)
                    {
                        float mult = (myMovepoints + myFirerange + m_smoothingValue) / (enemyMovepoints + enemyFirerange + m_smoothingValue);
                        if (mult > m_maxDistanceMultiplier)
                        {
                            mult = m_maxDistanceMultiplier;
                        }
                        if (myFirerange > 1)
                        {
                            // increased bonus for indirects since they should be attacked less often and attack more often
                            mult += 1.0f;
                        }
                        resDamage = dmg / (pEnemyUnit->getHp() * Unit::MAX_UNIT_HP) * pEnemyUnit->getUnitValue() * mult * bonusFactor -
                                    counterDmg / Unit::DAMAGE_100 * pEnemyUnit->getUnitValue();
                    }
                    else
                    {
                        float mult = (enemyMovepoints + enemyFirerange + m_smoothingValue) / (myMovepoints + myFirerange + m_smoothingValue);
                        if (mult > m_maxDistanceMultiplier)
                        {
                            mult = m_maxDistanceMultiplier;
                        }
                        resDamage = dmg / (pEnemyUnit->getHp() * Unit::MAX_UNIT_HP) * pEnemyUnit->getUnitValue() * bonusFactor -
                                    counterDmg / Unit::DAMAGE_100 * pEnemyUnit->getUnitValue() * mult;
                    }
                    if (resDamage > pEnemyUnit->getUnitValue())
                    {
                        resDamage = pEnemyUnit->getUnitValue();
                    }
                    float factor = 1.0f;
                    if (dmg > m_highDamage)
                    {
                        factor += (attackCount[i3].w() + m_smoothingValue) / (attackCount[i3].x() + m_smoothingValue);
                    }
                    else if (dmg > m_midDamage)
                    {
                        factor += (attackCount[i3].z() + m_smoothingValue) / (attackCount[i3].z() + m_smoothingValue);
                    }
                    if (onSameIsland(dummy.getMovementType(), posX, posY, pEnemyUnit->Unit::getX(), pEnemyUnit->Unit::getY()))
                    {
                        factor += (m_sameIslandBonusInRangeDays - (distance / static_cast<float>(myMovepoints) * m_sameIslandOutOfDayMalusFactor));
                        if (pEnemyUnit->hasWeapons())
                        {
                            float notAttackableValue = 0.0f;
                            if (dmg > m_highDamage)
                            {
                                notAttackableValue = m_highDamageBonus;
                            }
                            else if (dmg > m_midDamage)
                            {
                                notAttackableValue = m_midDamageBonus;
                            }
                            else if (dmg > m_notAttackableDamage)
                            {
                                notAttackableValue = m_lowDamageBonus;
                            }
                            else
                            {
                                factor *= m_veryLowDamageBonus;
                            }
                            if (attackCount[i3].y() == 0.0f &&
                                attackCount[i3].x() == 0.0f &&
                                attackCount[i3].z() == 0.0f &&
                                attackCount[i3].w() == 0.0f)
                            {
                                notAttackableCount += notAttackableValue;
                            }
                            else if (attackCount[i3].y() == 0.0f &&
                                     attackCount[i3].z() == 0.0f &&
                                     attackCount[i3].w() == 0.0f)
                            {
                                notAttackableCount  += notAttackableValue * m_currentlyNotAttackableBonus;
                            }
                        }
                        else
                        {
                            factor *= m_transportBonus;
                        }
                    }
                    else
                    {
                        factor += (m_differentIslandBonusInRangeDays - (distance / static_cast<float>(myMovepoints) * m_differentIslandOutOfDayMalusFactor));
                        if (pEnemyUnit->hasWeapons())
                        {
                            float notAttackableValue = 0.0f;
                            if (dmg > m_highDamage)
                            {
                                notAttackableValue = m_highDamageBonus;
                            }
                            else if (dmg > m_midDamage)
                            {
                                notAttackableValue = m_midDamageBonus;
                            }
                            else if (dmg > m_notAttackableDamage)
                            {
                                notAttackableValue = m_lowDamageBonus;
                            }
                            else
                            {
                                factor *= m_veryLowDamageBonus;
                            }
                            if (attackCount[i3].y() == 0.0f &&
                                attackCount[i3].x() == 0.0f &&
                                attackCount[i3].z() == 0.0f &&
                                attackCount[i3].w() == 0.0f)
                            {
                                notAttackableCount += notAttackableValue * m_currentlyNotAttackableBonus;
                            }
                            else if (attackCount[i3].x() == 0.0f &&
                                     attackCount[i3].z() == 0.0f &&
                                     attackCount[i3].w() == 0.0f)
                            {
                                notAttackableCount  += notAttackableValue * m_currentlyNotAttackableBonus * m_currentlyNotAttackableBonus;
                            }
                        }
                        else
                        {
                            factor += m_transportBonus;
                        }
                    }
                    if (factor < 0)
                    {
                        factor = 0;
                    }
                    damageCount += resDamage * factor;
                    attacksCount++;
                }
            }
        }
        counter++;
    }
    Console::print("NormalBehavioralModule::calcExpectedFundsDamage() calc damage", Console::eDEBUG);
    if (attacksCount <= 0)
    {
        attacksCount = 1;
    }
    float damage = damageCount / attacksCount;
    if (damage > 0)
    {
        float value = (attacksCount) / static_cast<float>(pEnemyUnits->size());
        if (attacksCount > m_minAttackCountBonus)
        {
            damage *= (attacksCount + m_minAttackCountBonus) / static_cast<float>(pEnemyUnits->size());
        }
        else
        {
            damage *= value;
        }
        // reduce effectiveness of units who can't attack a lot of units
        if (dummy.getMinRange(position) > 1)
        {
            for (qint32 i = m_indirectUnitAttackCountMalus; i > 1; --i)
            {
                float factor = 1 / static_cast<float>(i);
                if (value < factor)
                {
                    notAttackableCount *= factor;
                    damage *= factor;
                    break;
                }
            }
        }
    }
    return std::tuple<float, qint32>(damage, notAttackableCount);
}

qint32 NormalBehavioralModule::getClosestTargetDistance(qint32 posX, qint32 posY, Unit& dummy, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings)
{
    qint32 minDistance = std::numeric_limits<qint32>::max();
    QPoint pos(posX, posY);
    qint32 islandIdx = NormalBehavioralModule::getIslandIndex(&dummy);
    for (qint32 i = 0; i < pEnemyUnits->size(); i++)
    {
        Unit* pEnemyUnit = pEnemyUnits->at(i);
        if (onSameIsland(islandIdx, posX, posY, pEnemyUnit->Unit::getX(), pEnemyUnit->Unit::getY()))
        {
            if (dummy.isAttackable(pEnemyUnit, true))
            {
                qint32 distance = GlobalUtils::getDistance(pos, pEnemyUnit->getPosition());
                if (minDistance > distance)
                {
                    minDistance = distance;
                }
            }
        }
    }
    if (dummy.getActionList().contains(ACTION_CAPTURE))
    {
        bool missileTarget = hasMissileTarget();
        for (qint32 i = 0; i < pEnemyBuildings->size(); i++)
        {
            Building* pBuilding = pEnemyBuildings->at(i);
            if (dummy.canMoveOver(pBuilding->Building::getX(), pBuilding->Building::getY()))
            {
                if (pBuilding->isCaptureOrMissileBuilding(missileTarget) &&
                    pBuilding->getTerrain()->getUnit() == nullptr)
                {
                    qint32 distance = GlobalUtils::getDistance(pos, pBuilding->getPosition());
                    if (minDistance > distance)
                    {
                        minDistance = distance;
                    }
                }
            }
        }
    }
    return minDistance;
}

