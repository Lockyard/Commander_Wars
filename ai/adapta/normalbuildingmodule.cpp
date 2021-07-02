#include "normalbuildingmodule.h"
#include "game/gamemap.h"
#include "coreengine/console.h"
#include <QSettings>

//normalai includes
#include "coreengine/globalutils.h"
#include "resource_management/weaponmanager.h"


const QString NormalBuildingModule::MODULE_ID = "NORMAL_BUILDING";

NormalBuildingModule::NormalBuildingModule(AdaptaAI* pAdapta) : BuildingModule(), m_pAdapta(pAdapta)
{
}

bool NormalBuildingModule::readIni(QString filename) {
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
            m_spamingFunds = settings.value("SpamingFunds", 7500).toFloat(&ok);
            if(!ok)
            {
                m_spamingFunds = 0;
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

void NormalBuildingModule::init(Player *pPlayer) {
    if(m_pPlayer == nullptr && pPlayer != nullptr) {
        BuildingModule::init(pPlayer);
    } else {
        Console::print("NormalBuilding module was initializated but m_pPlayer is already set or the passed pPlayer is nullptr!", Console::eWARNING);
    }

}

void NormalBuildingModule::processStartOfTurn() {
    m_IslandMaps.clear();
    m_productionData.clear();
}

//does nothing, since there's no real processing to do to get a bid
//actually updates m_missileTarget
void NormalBuildingModule::processWhatToBuild()
{
    qint32 cost = 0;
    m_pPlayer->getSiloRockettarget(2, 3, cost);
    m_missileTarget = (cost >= m_minSiloDamage);
    return;
}

bool NormalBuildingModule::buildHighestBidUnit()
{
    spQmlVectorUnit pUnits = m_pAdapta->getCurrentProcessSpUnits();
    spQmlVectorUnit pEnemyUnits = m_pAdapta->getCurrentProcessSpEnemies();
    spQmlVectorBuilding pBuildings = m_pAdapta->getCurrentProcessSpBuildings();
    spQmlVectorBuilding pEnemyBuildings = m_pAdapta->getCurrentProcessSpEnemyBuildings();

    return buildUnits(pBuildings, pUnits, pEnemyUnits, pEnemyBuildings);
}

/**
 * @brief get the default bid of the module if there's at least 1 free production building
 */
float NormalBuildingModule::getHighestBid(bool weighted/*=true*/) {
    QStringList nonUsableBuildingTypes;
    spQmlVectorBuilding spBuildings = m_pPlayer->getBuildings();

    for(qint32 i=0; i<spBuildings->size(); i++) {
        Building* pBuilding = spBuildings->at(i);
        if(pBuilding->isProductionBuilding() &&
                GameMap::getInstance()->getTerrain(pBuilding->Building::getX(), pBuilding->Building::getY())->getUnit() == nullptr) {

            if(nonUsableBuildingTypes.contains(pBuilding->getBuildingID()))
                continue;
            if(canProductionBuildingBeUsedWithCurrentFunds(pBuilding)) {
                return m_defaultBid * (weighted ? m_moduleWeight : 1);
            } else {
                nonUsableBuildingTypes.append(pBuilding->getBuildingID());
            }
        }
    }
    return 0.0f;
}

//private own methods

bool NormalBuildingModule::canProductionBuildingBeUsedWithCurrentFunds(Building *pBuilding) {
    QStringList unitBuildIDs = pBuilding->getConstructionList();
    qint32 funds = m_pPlayer->getFunds();
    qint32 minCost = std::numeric_limits<qint32>::max();
    qint32 unitCost;
    for(QString& unitID : unitBuildIDs) {
        unitCost = m_pPlayer->getCosts(unitID);
        if(unitCost < minCost){
            minCost = unitCost;
        }
    }
    return funds >= minCost;
}

//BasegameinputIF Methods
float NormalBuildingModule::getUnitBuildValue(QString unitID)
{
    float modifier = m_pPlayer->getUnitBuildValue(unitID);
    for (qint32 i = 0; i < m_BuildingChanceModifier.size(); i++)
    {
        if (std::get<0>(m_BuildingChanceModifier[i]) == unitID)
        {
            return std::get<1>(m_BuildingChanceModifier[i]) + modifier;
        }
    }
    return 1.0f + modifier;
}

//CoreAI Methods
void NormalBuildingModule::addMenuItemData(spGameAction pGameAction, QString itemID, qint32 cost)
{
    Console::print("NormalBuildingModule::addMenuItemData()", Console::eDEBUG);
    pGameAction->writeDataString(itemID);
    // increase costs and input step
    pGameAction->setCosts(pGameAction->getCosts() + cost);
    pGameAction->setInputStep(pGameAction->getInputStep() + 1);
}

void NormalBuildingModule::GetOwnUnitCounts(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                              qint32 & infantryUnits, qint32 & indirectUnits,
                              qint32 & directUnits, QVector<std::tuple<Unit*, Unit*>> & transportTargets)
{
    for (qint32 i = 0; i < pUnits->size(); i++)
    {
        Unit* pUnit = pUnits->at(i);
        if (pUnit->getActionList().contains(CoreAI::ACTION_CAPTURE))
        {
            infantryUnits++;
        }
        else if (pUnit->hasWeapons())
        {
            if (pUnit->getBaseMaxRange() > 1)
            {
                indirectUnits++;
            }
            else
            {
                directUnits++;
            }
        }
        if (pUnit->getLoadingPlace() > 0)
        {
            QVector<QVector3D> ret;
            QVector<Unit*> transportUnits = appendLoadingTargets(pUnit, pUnits, pEnemyUnits, pEnemyBuildings, false, true, ret, true);
            for (qint32 i2 = 0; i2 < transportUnits.size(); i2++)
            {
                transportTargets.append(std::tuple<Unit*, Unit*>(pUnit, transportUnits[i2]));
            }
        }
    }
}

QVector<Unit*> NormalBuildingModule::appendLoadingTargets(Unit* pUnit, spQmlVectorUnit pUnits,
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
                bool canCapture = pLoadingUnit->getActionList().contains(CoreAI::ACTION_CAPTURE);
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

void NormalBuildingModule::checkIslandForUnloading(Unit* pUnit, Unit* pLoadedUnit, QVector<qint32>& checkedIslands,
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

bool NormalBuildingModule::isUnloadTerrain(Unit* pUnit, Terrain* pTerrain)
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QJSValueList args;
    QJSValue obj = pInterpreter->newQObject(pUnit);
    args << obj;
    QJSValue obj1 = pInterpreter->newQObject(pTerrain);
    args << obj1;
    QJSValue ret = pInterpreter->doFunction(CoreAI::ACTION_UNLOAD, "isUnloadTerrain", args);
    if (ret.isBool())
    {
        return ret.toBool();
    }
    return false;
}


bool NormalBuildingModule::hasTargets(Unit* pLoadingUnit, bool canCapture, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
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


void NormalBuildingModule::rebuildIsland(spQmlVectorUnit pUnits)
{
    Console::print("NormalBuildingModule::rebuildIsland", Console::eDEBUG);
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

bool NormalBuildingModule::onSameIsland(Unit* pUnit1, Unit* pUnit2)
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

bool NormalBuildingModule::onSameIsland(Unit* pUnit1, Building* pBuilding)
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

bool NormalBuildingModule::onSameIsland(QString movemnetType, qint32 x, qint32 y, qint32 x1, qint32 y1)
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

bool NormalBuildingModule::onSameIsland(qint32 islandIdx, qint32 x, qint32 y, qint32 x1, qint32 y1)
{
    return m_IslandMaps[islandIdx]->sameIsland(x, y, x1, y1);
}


qint32 NormalBuildingModule::getIsland(Unit* pUnit)
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

qint32 NormalBuildingModule::getIslandIndex(Unit* pUnit)
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

void NormalBuildingModule::createIslandMap(QString movementType, QString unitID)
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

float NormalBuildingModule::getAiCoUnitMultiplier(CO* pCO, Unit* pUnit)
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


//NormalAI Methods

bool NormalBuildingModule::buildUnits(spQmlVectorBuilding pBuildings, spQmlVectorUnit pUnits,
                          spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings)
{
    Console::print("NormalBuildingModule::buildUnits()", Console::eDEBUG);
    spGameMap pMap = GameMap::getInstance();

    qint32 enemeyCount = 0;
    for (qint32 i = 0; i < pMap->getPlayerCount(); i++)
    {
        if (m_pPlayer->isEnemy(pMap->getPlayer(i)) && !pMap->getPlayer(i)->getIsDefeated())
        {
            enemeyCount++;
        }
    }

    QVector<float> data(BuildItems::Max, 0);
    qint32 productionBuildings = 0;
    for (qint32 i = 0; i < pBuildings->size(); i++)
    {
        Building* pBuilding = pBuildings->at(i);
        if (pBuilding->isProductionBuilding() &&
            pMap->getTerrain(pBuilding->Building::getX(), pBuilding->Building::getY())->getUnit() == nullptr)
        {
            productionBuildings++;
        }
    }
    qint32 infantryUnits = 0;
    qint32 indirectUnits = 0;
    qint32 directUnits = 0;
    QVector<std::tuple<Unit*, Unit*>> transportTargets;
    GetOwnUnitCounts(pUnits, pEnemyUnits, pEnemyBuildings,
                     infantryUnits, indirectUnits, directUnits,
                     transportTargets);
    QVector<QVector4D> attackCount(pEnemyUnits->size(), QVector4D(0, 0, 0, 0));
    getEnemyDamageCounts(pUnits, pEnemyUnits, attackCount);
    float funds = m_pPlayer->getFunds();
    // calc average costs if we would build same cost units on every building
    float fundsPerFactory = funds / (static_cast<float>(productionBuildings));
    if (productionBuildings > 2)
    {
        // if we have a bigger number of buildings we wanna spam out units but not at an average costs overall buildings
        // but more a small amount of strong ones and a large amount of cheap ones
        // so we use a small (x - a) / (x - b) function here
        float test = funds * ((productionBuildings - m_fundsPerBuildingFactorA) / (static_cast<float>(productionBuildings) - m_fundsPerBuildingFactorB));
        if (test > m_spamingFunds)
        {
            test = m_spamingFunds;
        }
        if (test > fundsPerFactory)
        {
            fundsPerFactory = test;
        }
    }
    // position 0 direct to indirect ratio
    if (indirectUnits > 0)
    {
        data[DirectUnitRatio] = static_cast<float>(directUnits) / static_cast<float>(indirectUnits);
    }
    else
    {
        data[DirectUnitRatio] = static_cast<float>(directUnits);
    }
    // position 1 infatry to unit count ratio
    if (pUnits->size() > 0)
    {
        data[InfantryUnitRatio] = infantryUnits / static_cast<float>(pUnits->size());
    }
    else
    {
        data[InfantryUnitRatio] = 0.0;
    }
    data[InfantryCount] = infantryUnits;
    data[UnitEnemyRatio] = (pUnits->size() + m_ownUnitEnemyUnitRatioAverager) / (pEnemyUnits->size() + m_ownUnitEnemyUnitRatioAverager);
    if (enemeyCount > 1)
    {
        data[UnitEnemyRatio] *= (enemeyCount - 1);
    }
    data[UnitCount] = pUnits->size();

    spGameAction pAction = spGameAction::create(CoreAI::ACTION_BUILD_UNITS);
    float bestScore = std::numeric_limits<float>::lowest();
    QVector<qint32> buildingIdx;
    QVector<qint32> unitIDx;
    QVector<float> scores;
    QVector<bool> transporters;
    float variance = pMap->getCurrentDay() - 1;
    if (variance > m_maxDayScoreVariancer)
    {
        variance = m_maxDayScoreVariancer;
    }
    spQmlVectorPoint pFields = GlobalUtils::getCircle(1, 1);
    for (qint32 i = 0; i < pBuildings->size(); i++)
    {
        Building* pBuilding = pBuildings->at(i);
        if (pBuilding->isProductionBuilding() &&
            pBuilding->getTerrain()->getUnit() == nullptr)
        {
            pAction->setTarget(QPoint(pBuilding->Building::getX(), pBuilding->Building::getY()));
            if (pAction->canBePerformed())
            {
                // we're allowed to build units here
                spMenuData pData = pAction->getMenuStepData();
                if (pData->validData())
                {
                    auto enableList = pData->getEnabledList();
                    auto actionIds = pData->getActionIDs();
                    qint32 index = getIndexInProductionData(pBuilding);
                    auto & buildingData = m_productionData[index];
                    for (qint32 i2 = 0; i2 < pData->getActionIDs().size(); i2++)
                    {
                        if (enableList[i2])
                        {
                            float score = 0.0f;
                            qint32 unitIdx = getUnitProductionIdx(index, actionIds[i2],
                                                                  pUnits, transportTargets,
                                                                  pEnemyUnits, pEnemyBuildings,
                                                                  attackCount, data);
                            bool isTransporter = false;
                            if (unitIdx >= 0)
                            {
                                auto & unitData = buildingData.m_buildData[unitIdx];
                                data[UnitCost] = unitData.cost;
                                if (unitData.canMove)
                                {
                                    data[Movementpoints] = unitData.movePoints;
                                    data[ReachDistance] = unitData.closestTarget;
                                    data[COBonus] = unitData.coBonus;
                                    data[NotAttackableCount] = unitData.notAttackableCount;
                                    data[DamageData] =  unitData.damage;
                                    data[FundsFactoryRatio] = unitData.cost / fundsPerFactory;
                                    if (pEnemyBuildings->size() > 0 && enemeyCount > 0)
                                    {
                                        data[BuildingEnemyRatio] = pBuildings->size() / (static_cast<float>(pEnemyBuildings->size()) / static_cast<float>(enemeyCount));
                                    }
                                    else
                                    {
                                        data[BuildingEnemyRatio] = 0.0;
                                    }
                                    if (unitData.infantryUnit)
                                    {
                                        data[InfantryUnit] = 1.0;
                                    }
                                    else
                                    {
                                        data[InfantryUnit] = 0.0;
                                    }
                                    if (unitData.indirectUnit)
                                    {
                                        data[IndirectUnit] = 1.0;
                                        data[DirectUnit] = 0.0;
                                    }
                                    else
                                    {
                                        data[IndirectUnit] = 0.0;
                                        data[DirectUnit] = 1.0;
                                    }
                                    if (!unitData.isTransporter)
                                    {

                                        score = calcBuildScore(data);
                                    }
                                    else
                                    {
                                        score = calcTransporterScore(unitData, pUnits, data);
                                        isTransporter = true;
                                    }
                                }
                            }
                            score *= getUnitBuildValue(actionIds[i2]);
                            if (score > bestScore)
                            {
                                bestScore = score;
                                buildingIdx.append(i);
                                unitIDx.append(i2);
                                scores.append(score);
                                transporters.append(isTransporter);
                                qint32 index = 0;
                                while (index < scores.size())
                                {
                                    if (scores[index] < bestScore - variance)
                                    {
                                        buildingIdx.removeAt(index);
                                        unitIDx.removeAt(index);
                                        scores.removeAt(index);
                                        transporters.removeAt(index);
                                    }
                                    else
                                    {
                                        index++;
                                    }
                                }
                            }
                            else if (score >= bestScore - variance)
                            {
                                buildingIdx.append(i);
                                unitIDx.append(i2);
                                scores.append(score);
                                transporters.append(isTransporter);
                            }
                        }
                    }
                }
            }
        }
    }

    if (buildingIdx.size() > 0)
    {
        qint32 item = GlobalUtils::randIntBase(0, buildingIdx.size() - 1);
        Building* pBuilding = pBuildings->at(buildingIdx[item]);
        pAction->setTarget(QPoint(pBuilding->Building::getX(), pBuilding->Building::getY()));
        if (transporters[item])
        {
            for (auto & building : m_productionData)
            {
                for (auto & unit : building.m_buildData)
                {
                    if (unit.isTransporter)
                    {
                        unit.noTransporterBonus = 0;
                        unit.transportCount += 1;
                    }
                }
            }
        }
        spMenuData pData = pAction->getMenuStepData();
        if (pData->validData())
        {
            addMenuItemData(pAction, pData->getActionIDs()[unitIDx[item]], pData->getCostList()[unitIDx[item]]);
        }
        // produce the unit
        if (pAction->isFinalStep())
        {
            //m_updatePoints.append(pAction->getActionTarget()); //is not used by anyone in the building module
            emit m_pAdapta->performAction(pAction);
            return true;
        }
    }
    return false;
}

void NormalBuildingModule::getEnemyDamageCounts(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits, QVector<QVector4D> & attackCount)
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

qint32 NormalBuildingModule::getIndexInProductionData(Building* pBuilding)
{
    Console::print("NormalAi::getIndexInProductionData()", Console::eDEBUG);
    qint32 x = pBuilding->Building::getX();
    qint32 y = pBuilding->Building::getY();
    qint32 ret = -1;
    for (qint32 i = 0; i < m_productionData.length(); ++i)
    {
        if (m_productionData[i].m_x == x &&
            m_productionData[i].m_y == y)
        {
            ret = i;
            break;
        }
    }
    if (ret < 0)
    {
        m_productionData.append(ProductionData());
        ret = m_productionData.length() - 1;
        m_productionData[ret].m_x = x;
        m_productionData[ret].m_y = y;
    }
    return ret;
}

qint32 NormalBuildingModule::getUnitProductionIdx(qint32 index, QString unitId,
                                      spQmlVectorUnit pUnits, QVector<std::tuple<Unit*, Unit*>> & transportTargets,
                                      spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                                      QVector<QVector4D> & attackCount, QVector<float> & buildData)
{
    Console::print("NormalAi::getUnitProductionIdx()", Console::eDEBUG);
    qint32 ret = -1;
    auto & data = m_productionData[index];
    for (qint32 i = 0; i < data.m_buildData.length(); ++i)
    {
        auto & unitData = data.m_buildData[i];
        if (unitData.unitId == unitId)
        {
            if (unitData.notAttackableCount > 0)
            {
                Unit dummy(unitId, m_pPlayer, false);
                dummy.setVirtuellX(data.m_x);
                dummy.setVirtuellY(data.m_y);
                float bonusFactor = 1.0f;
                if ((buildData[DirectUnitRatio] > m_directIndirectRatio && unitData.baseRange > 1) ||
                    (buildData[DirectUnitRatio] < m_directIndirectRatio && unitData.baseRange == 1))
                {
                    bonusFactor = m_directIndirectUnitBonusFactor;
                }
                auto damageData = calcExpectedFundsDamage(data.m_x, data.m_y, dummy, pEnemyUnits, attackCount, bonusFactor);
                unitData.notAttackableCount = std::get<1>(damageData);
                unitData.damage =  std::get<0>(damageData);
            }
            ret = i;
            break;
        }
    }
    if (ret < 0)
    {
        data.m_buildData.append(UnitBuildData());
        ret = data.m_buildData.length() - 1;
        auto & unitBuildData = data.m_buildData[ret];
        unitBuildData.unitId = unitId;
        createUnitBuildData(data.m_x, data.m_y, unitBuildData,
                            pUnits, transportTargets,
                            pEnemyUnits, pEnemyBuildings,
                            attackCount, buildData);
    }
    return ret;
}

float NormalBuildingModule::calcBuildScore(QVector<float>& data)
{
    float score = 0;
    // used index 0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 11
    if (data[IndirectUnit] == 1.0f)
    {
        // indirect unit
        if (data[DirectUnitRatio] > m_directIndirectRatio)
        {
            score += m_lowIndirectUnitBonus * (data[DirectUnitRatio] - m_directIndirectRatio);
        }
        else if (data[DirectUnitRatio] < m_directIndirectRatio / 2)
        {
            score -= m_highIndirectMalus * (m_directIndirectRatio - data[DirectUnitRatio]);
        }
        else if (data[DirectUnitRatio] < m_directIndirectRatio)
        {
            score -= m_lowIndirectMalus * (m_directIndirectRatio - data[DirectUnitRatio]);
        }
    }
    else if (data[DirectUnit] == 1.0f)
    {
        // direct unit
        if (data[DirectUnitRatio] < m_directIndirectRatio)
        {
            score += m_lowDirectUnitBonus * (m_directIndirectRatio - data[DirectUnitRatio]);
        }
        else if (data[DirectUnitRatio] > m_directIndirectRatio * 2)
        {
            score -= m_highDirectMalus * (data[DirectUnitRatio] - m_directIndirectRatio);
        }
        else if (data[DirectUnitRatio] > m_directIndirectRatio)
        {
            score -= m_lowDirectMalus * (data[DirectUnitRatio] - m_directIndirectRatio);
        }
    }
    if (data[UnitCount] > m_minUnitCountForDamageBonus)
    {
        // apply damage bonus
        score += data[DamageData] / Unit::DAMAGE_100;
    }
    // infantry bonus
    if (data[InfantryUnit] == 1.0f)
    {
        if (data[InfantryCount] <= m_minInfantryCount && data[BuildingEnemyRatio] < m_lowOwnBuildingEnemyBuildingRatio)
        {
            score += (m_minInfantryCount - data[InfantryCount]) * m_minInfantryCount + (m_lowOwnBuildingEnemyBuildingRatio - data[BuildingEnemyRatio]) * m_lowIncomeInfantryBonusMultiplier;
        }
        else if (data[InfantryUnitRatio] < m_lowInfantryRatio)
        {
            score += (m_lowOwnBuildingEnemyBuildingRatio - data[BuildingEnemyRatio]) * m_buildingBonusMultiplier;
        }
        else
        {
            score += (m_lowOwnBuildingEnemyBuildingRatio - data[BuildingEnemyRatio]) * m_buildingBonusMultiplier;
        }
        if (data[InfantryCount] >= m_minInfantryCount)
        {
            score -= data[InfantryCount];
        }
    }
    score += calcCostScore(data);
    // apply movement bonus
    score += data[Movementpoints] * m_movementpointBonus;
    if (data[UnitCount] > m_minUnitCountForDamageBonus)
    {
        // apply not attackable unit bonus
        score += data[NotAttackableCount] * m_currentlyNotAttackableScoreBonus;
    }
    if (data[UnitCost] > 0)
    {
        score += data[DamageData] * m_damageToUnitCostRatioBonus / data[UnitCost];
    }
    // apply co buff bonus
    score += data[COBonus] * m_coUnitBuffBonus;

    if (data[ReachDistance] > 0 && data[Movementpoints] > 0)
    {
        score += m_nearEnemyBonus * data[Movementpoints] / GlobalUtils::roundUp(data[ReachDistance]);
    }
    return score;
}

void NormalBuildingModule::createUnitBuildData(qint32 x, qint32 y, UnitBuildData & unitBuildData,
                                   spQmlVectorUnit pUnits, QVector<std::tuple<Unit*, Unit*>> & transportTargets,
                                   spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                                   QVector<QVector4D> & attackCount, QVector<float> & buildData)
{
    Console::print("NormalAi::createUnitBuildData()", Console::eDEBUG);
    Unit dummy(unitBuildData.unitId, m_pPlayer, false);
    dummy.setVirtuellX(x);
    dummy.setVirtuellY(y);
    createIslandMap(dummy.getMovementType(), dummy.getUnitID());
    UnitPathFindingSystem pfs(&dummy, m_pPlayer);
    pfs.explore();
    unitBuildData.movePoints = dummy.getMovementpoints(QPoint(x, y));
    auto points = pfs.getAllNodePoints();
    if (points.length() >= unitBuildData.movePoints * 1.5f)
    {
        unitBuildData.canMove = true;
        unitBuildData.isTransporter = (dummy.getWeapon1ID().isEmpty() &&
                                       dummy.getWeapon2ID().isEmpty());
        if (unitBuildData.isTransporter)
        {
            getTransporterData(unitBuildData, dummy, pUnits, pEnemyUnits, pEnemyBuildings, transportTargets);
        }
        else
        {
            unitBuildData.closestTarget = getClosestTargetDistance(x, y, dummy, pEnemyUnits, pEnemyBuildings);
            unitBuildData.coBonus += getAiCoUnitMultiplier(m_pPlayer->getCO(0), &dummy);
            unitBuildData.coBonus += getAiCoUnitMultiplier(m_pPlayer->getCO(1), &dummy);
            unitBuildData.baseRange = dummy.getBaseMaxRange();
            float bonusFactor = 1.0f;
            if ((buildData[DirectUnitRatio] > m_directIndirectRatio && unitBuildData.baseRange > 1) ||
                (buildData[DirectUnitRatio] < m_directIndirectRatio && unitBuildData.baseRange == 1))
            {
                bonusFactor = m_directIndirectUnitBonusFactor;
            }
            auto damageData = calcExpectedFundsDamage(x, y, dummy, pEnemyUnits, attackCount, bonusFactor);
            unitBuildData.notAttackableCount = std::get<1>(damageData);
            unitBuildData.damage =  std::get<0>(damageData);
            unitBuildData.cost = dummy.getUnitCosts();
            unitBuildData.infantryUnit = (dummy.getActionList().contains(CoreAI::ACTION_CAPTURE) &&
                                          dummy.getLoadingPlace() == 0);
            unitBuildData.indirectUnit = dummy.getBaseMaxRange() > 1;
        }
    }
}


float NormalBuildingModule::calcCostScore(QVector<float>& data)
{
    float score = 0;
    // funds bonus;
    if (data[FundsFactoryRatio] > m_superiorityRatio + data[UnitEnemyRatio])
    {
        score -= (data[FundsFactoryRatio] - m_superiorityRatio + data[UnitEnemyRatio]) * m_expensiveUnitBonusMultiplier;
    }
    else if (data[FundsFactoryRatio] < m_cheapUnitRatio)
    {
        score += (m_cheapUnitRatio - data[FundsFactoryRatio]) * m_cheapUnitBonusMultiplier;
    }
    else
    {
        score += (m_superiorityRatio + data[UnitEnemyRatio] - data[FundsFactoryRatio]) * m_normalUnitBonusMultiplier;
    }
    return score;
}

void NormalBuildingModule::getTransporterData(UnitBuildData & unitBuildData, Unit& dummy, spQmlVectorUnit pUnits,
                                  spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                                  QVector<std::tuple<Unit*, Unit*>>& transportTargets)
{
    Console::print("NormalAi::getTransporterData()", Console::eDEBUG);
    QVector<QVector3D> targets;
    spQmlVectorUnit relevantUnits = spQmlVectorUnit::create();
    QPoint position = dummy.getPosition();
    qint32 movement = dummy.getBaseMovementPoints();
    if (movement == 0)
    {
        movement = 1;
    }
    spGameMap pMap = GameMap::getInstance();
    qint32 loadingPlace = dummy.getLoadingPlace();
    qint32 smallTransporterCount = 0;
    qint32 maxCounter = m_ProducingTransportSearchrange;
    qint32 counter = 1;
    while (relevantUnits->size()  < loadingPlace * m_transporterToRequiredPlaceFactor &&
           pUnits->size() > loadingPlace * (m_transporterToRequiredPlaceFactor - 1) &&
           counter <= maxCounter)
    {
        for (qint32 i = 0; i < pUnits->size(); i++)
        {
            qint32 distance = GlobalUtils::getDistance(position, pUnits->at(i)->getPosition());
            if (distance / movement <= counter * maxDayDistance &&
                distance / movement >= (counter - 1) * maxDayDistance)
            {
                relevantUnits->append(pUnits->at(i));
            }
            if (loadingPlace == 1)
            {
                if (pUnits->at(i)->getLoadingPlace() == 1)
                {
                    smallTransporterCount++;
                }
            }
        }
        counter++;
    }
    QVector<Unit*> loadingUnits = appendLoadingTargets(&dummy, relevantUnits, pEnemyUnits, pEnemyBuildings, false, true, targets, true);
    QVector<Unit*> transporterUnits;
    for (qint32 i2 = 0; i2 < transportTargets.size(); i2++)
    {
        if (!transporterUnits.contains(std::get<0>(transportTargets[i2])))
        {
            transporterUnits.append(std::get<0>(transportTargets[i2]));
        }
    }
    qint32 i = 0;
    while ( i < loadingUnits.size())
    {
        if (canTransportToEnemy(&dummy, loadingUnits[i], pEnemyUnits, pEnemyBuildings))
        {
            qint32 transporter = 0;
            for (qint32 i2 = 0; i2 < transportTargets.size(); i2++)
            {
                if (std::get<1>(transportTargets[i2])->getPosition() == loadingUnits[i]->getPosition())
                {
                    transporter++;
                    break;
                }
            }

            if (transporter == 0)
            {
                unitBuildData.noTransporterBonus += m_noTransporterBonus;
            }
            i++;
        }
        else
        {
            loadingUnits.removeAt(i);
        }
    }
    unitBuildData.smallTransporterCount = smallTransporterCount;
    unitBuildData.loadingPlace = dummy.getLoadingPlace();
    unitBuildData.transportCount = transporterUnits.size();
    unitBuildData.loadingCount = loadingUnits.size();
    unitBuildData.flying = !dummy.useTerrainDefense();
}

float NormalBuildingModule::calcTransporterScore(UnitBuildData & unitBuildData,  spQmlVectorUnit pUnits, QVector<float>& data)
{
    float score = 0.0f;
    if (score == 0.0f && pUnits->size() / (unitBuildData.smallTransporterCount + 1) > m_unitToSmallTransporterRatio && unitBuildData.loadingPlace == 1)
    {
        spGameMap pMap = GameMap::getInstance();
        if (unitBuildData.smallTransporterCount > 0)
        {
            score += qMin(m_smallTransporterBonus,  pUnits->size() / static_cast<float>(unitBuildData.smallTransporterCount + 1.0f) * 10.0f);

        }
        else
        {
            score += m_smallTransporterBonus;
        }
        // give a bonus to t-heli's or similar units cause they are mostlikly much faster
        if (unitBuildData.flying && score > m_minFlyingTransportScoreForBonus)
        {
            score += m_flyingTransporterBonus;
        }
    }
    if (unitBuildData.transportCount > 0 && unitBuildData.loadingCount > 0)
    {
        score += (unitBuildData.loadingCount / static_cast<float>(unitBuildData.transportCount)) * m_ProducingTransportRatioBonus;
    }
    else
    {
        score += unitBuildData.loadingCount * m_ProducingTransportLoadingBonus;
    }
    if (unitBuildData.loadingCount > 0 || unitBuildData.loadingCount == 1)
    {
        if (unitBuildData.transportCount <= 0 ||
            static_cast<float>(unitBuildData.loadingCount) / static_cast<float>(unitBuildData.transportCount) < m_ProducingTransportMinLoadingTransportRatio ||
            unitBuildData.loadingCount == 1)
        {
            score += calcCostScore(data);
            score += unitBuildData.loadingPlace * m_additionalLoadingUnitBonus;
            score += unitBuildData.loadingCount * m_additionalLoadingUnitBonus;
        }
    }
    else
    {
        score = std::numeric_limits<float>::lowest();
    }
    return score;
}

bool NormalBuildingModule::canTransportToEnemy(Unit* pUnit, Unit* pLoadedUnit, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings)
{
    spQmlVectorPoint pUnloadArea = GlobalUtils::getCircle(1, 1);
    // check for enemis
    qint32 loadedUnitIslandIdx = getIslandIndex(pLoadedUnit);
    qint32 unitIslandIdx = getIslandIndex(pUnit);
    qint32 unitIsland = getIsland(pUnit);
    QVector<qint32> checkedIslands;
    QVector<QVector3D> targets;
    for (qint32 i = 0; i < pEnemyUnits->size(); i++)
    {
        Unit* pEnemy = pEnemyUnits->at(i);
        qint32 targetIsland = m_IslandMaps[loadedUnitIslandIdx]->getIsland(pEnemy->Unit::getX(), pEnemy->Unit::getY());
        // check if we could reach the enemy if we would be on his island
        // and we didn't checked this island yet -> improves the speed
        if (targetIsland >= 0 )
        {
            // could we beat his ass? -> i mean can we attack him
            // if so this is a great island
            if (pLoadedUnit->isAttackable(pEnemy, true))
            {
                checkIslandForUnloading(pUnit, pLoadedUnit, checkedIslands, unitIslandIdx, unitIsland,
                                        loadedUnitIslandIdx, targetIsland, pUnloadArea.get(), targets);
                if (targets.size() > 0)
                {
                    break;
                }
            }
        }
    }
    // check for capturable buildings
    if (pLoadedUnit->getActionList().contains(CoreAI::ACTION_CAPTURE) && targets.size() == 0)
    {
        bool missileTarget = hasMissileTarget();
        for (qint32 i = 0; i < pEnemyBuildings->size(); i++)
        {
            Building* pEnemyBuilding = pEnemyBuildings->at(i);

            qint32 targetIsland = m_IslandMaps[loadedUnitIslandIdx]->getIsland(pEnemyBuilding->Building::getX(), pEnemyBuilding->Building::getY());
            // check if we could reach the enemy if we would be on his island
            // and we didn't checked this island yet -> improves the speed
            if (targetIsland >= 0 )
            {
                if (pEnemyBuilding->isCaptureOrMissileBuilding(missileTarget))
                {
                    checkIslandForUnloading(pUnit, pLoadedUnit, checkedIslands, unitIslandIdx, unitIsland,
                                            loadedUnitIslandIdx, targetIsland, pUnloadArea.get(), targets);
                    if (targets.size() > 0)
                    {
                        break;
                    }
                }
            }
        }
    }
    if (targets.size() > 0)
    {
        return true;
    }
    return false;
}

std::tuple<float, qint32> NormalBuildingModule::calcExpectedFundsDamage(qint32 posX, qint32 posY, Unit& dummy, spQmlVectorUnit pEnemyUnits, QVector<QVector4D> attackCount, float bonusFactor)
{
    Console::print("NormalBuildingModule::calcExpectedFundsDamage() find enemies", Console::eDEBUG);
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
    Console::print("NormalAi::calcExpectedFundsDamage() calc damage", Console::eDEBUG);
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

qint32 NormalBuildingModule::getClosestTargetDistance(qint32 posX, qint32 posY, Unit& dummy, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings)
{
    qint32 minDistance = std::numeric_limits<qint32>::max();
    QPoint pos(posX, posY);
    qint32 islandIdx = getIslandIndex(&dummy);
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
    if (dummy.getActionList().contains(CoreAI::ACTION_CAPTURE))
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


//*/
