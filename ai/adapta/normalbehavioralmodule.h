#ifndef NORMALBEHAVIORALMODULE_H
#define NORMALBEHAVIORALMODULE_H

#include <QObject>
#include "adaptamodule.h"
#include "ai/adaptaai.h"

//include from CoreAI
#include <qtextstream.h>
#include <qvector.h>
#include <qvector3d.h>
#include <qvector4d.h>
#include <QRectF>
#include "ai/decisionquestion.h"
#include "ai/islandmap.h"
#include "ai/decisiontree.h"
#include "coreengine/qmlvector.h"
#include "coreengine/LUPDATE_MACROS.h"
//include from BaseGameInputIF
#include "3rd_party/oxygine-framework/oxygine-framework.h"
#include "coreengine/fileserializable.h"
#include "game/GameEnums.h"


/**
 * @brief NormalBehavioralModule is a module which implements just a behavioral AI. It will be basically a copy/reimplementation
 * of the existing normal AI by Robosturm adapted to be a module for the AdaptaAI, maybe with some changes (credits to him in
 * any case), but nothing more since a behavioral AI is not the focus of the AdaptaAI project.
 * This module should be used so that the Adapta AI has at least an action to do with each unit, if some units are not supported
 * by any other module.
 * Although it's a copy of the normal AI this module DOES NOT build units since is not a building module.
 */
class NormalBehavioralModule : public AdaptaModule
{
    //CoreAI stuff
    /**
     * @brief NB_AISteps = Normal Behavior AISteps
     */
    ENUM_CLASS NB_AISteps
    {
        moveUnits = 0,
                moveToTargets,
                loadUnits,
                moveTransporters,
                moveSupportUnits,
                moveAway,
                buildUnits,
    };
    Q_ENUM(NB_AISteps)
    // static string list of actions so we only define them once
    static const QString ACTION_WAIT;
    static const QString ACTION_HOELLIUM_WAIT;
    static const QString ACTION_SUPPORTSINGLE;
    static const QString ACTION_SUPPORTSINGLE_REPAIR;
    static const QString ACTION_SUPPORTSINGLE_FREEREPAIR;
    static const QString ACTION_SUPPORTALL;
    static const QString ACTION_SUPPORTALL_RATION;
    static const QString ACTION_UNSTEALTH;
    static const QString ACTION_PLACE;
    static const QString ACTION_STEALTH;
    static const QString ACTION_BUILD_UNITS;
    static const QString ACTION_CAPTURE;
    static const QString ACTION_MISSILE;
    static const QString ACTION_FIRE;
    static const QString ACTION_UNLOAD;
    static const QString ACTION_LOAD;
    static const QString ACTION_NEXT_PLAYER;
    static const QString ACTION_SWAP_COS;
    static const QString ACTION_ACTIVATE_TAGPOWER;
    static const QString ACTION_ACTIVATE_POWER_CO_0;
    static const QString ACTION_ACTIVATE_POWER_CO_1;
    static const QString ACTION_ACTIVATE_SUPERPOWER_CO_0;
    static const QString ACTION_ACTIVATE_SUPERPOWER_CO_1;
    static const QString ACTION_CO_UNIT_0;
    static const QString ACTION_CO_UNIT_1;
    static const QString ACTION_EXPLODE;
    static const QString ACTION_FLARE;



public:

    NormalBehavioralModule(AdaptaAI* ai);

    // AdaptaModule interface
    void readIni(QString name) override;
    void init(Player* pPlayer) override;
    void processStartOfTurn() override;
    bool processHighestBidUnit() override;
    bool processUnit(Unit *pUnit) override;
    void notifyUnitUsed(Unit *pUnit) override;
    float getBidFor(Unit *pUnit) override;
    float getHighestBid(bool weighted) override;
    Unit *getHighestBidUnit() override;

private:
    //own variables
    AdaptaAI* m_pAdapta;
    float m_defaultBid{.1f};

    //variables from BaseGameInputIF
    bool m_enableNeutralTerrainAttack{true};
    /**
     * @brief m_MoveCostMap move cost modifier map for the ai
     */
    QVector<QVector<std::tuple<qint32, bool>>> m_MoveCostMap;

    //variables from CoreAI
    DecisionTree m_COPowerTree;
    QVector<spIslandMap> m_IslandMaps;
    float m_buildingValue{1.0f};
    float m_ownUnitValue{1.0f};
    GameEnums::AiTurnMode turnMode{GameEnums::AiTurnMode_StartOfDay};
    NB_AISteps aiStep;
    bool usedTransportSystem{false};
    bool m_missileTarget{false};
    float m_fuelResupply{0.33f};
    float m_ammoResupply{0.25f};

    float m_minCoUnitScore{5000.0f};
    qint32 m_coUnitValue{1000};
    float m_coUnitRankReduction{1000.0f};
    float m_coUnitScoreMultiplier{1.1f};
    qint32 m_minCoUnitCount{5};

    bool finish{false};
    struct FlareInfo
    {
        qint32 minRange{0};
        qint32 maxRange{0};
        qint32 unfogRange{0};
    };
    FlareInfo m_flareInfo;
    QStringList m_files;

    //variables from normal AI
    /**
     * @brief m_EnemyUnits all enemy units that exists at the start of turn
     */
    QVector<spUnit> m_EnemyUnits;
    /**
     * @brief m_EnemyPfs all enemy pfs currently correct.
     */
    QVector<spUnitPathFindingSystem> m_EnemyPfs;
    /**
     * @brief updatePoints points we need to update for the next pfs
     */
    QVector<QPoint> m_updatePoints;
    /**
     * @brief m_VirtualEnemyData
     */
    QVector<QPointF> m_VirtualEnemyData;

    static constexpr float maxDayDistance = 6.0f;;
    float m_notAttackableDamage{25.0f};
    float m_midDamage{55.0f};
    float m_highDamage{65.0f};
    float m_directIndirectRatio{1.75f};
    qint32 m_minSiloDamage{7000};
    float m_minMovementDamage{0.3f};
    float m_minAttackFunds{0.0f};
    float m_minSuicideDamage{0.75f};
    float m_spamingFunds{7500};

    qint32 m_minUnitHealth{3};
    qint32 m_maxUnitHealth{7};
    float m_lockedUnitHp{4};
    float m_noMoveAttackHp{3.5f};
    float m_ownIndirectAttackValue{2.0f};
    float m_enemyKillBonus{2.0f};
    float m_enemyIndirectBonus{3.0f};
    float m_antiCaptureHqBonus{50.0f};
    float m_antiCaptureBonus{21.0f};
    float m_antiCaptureBonusScoreReduction{6.0f};
    float m_antiCaptureBonusScoreDivider{2.0f};
    float m_enemyCounterDamageMultiplier{10.0f};
    float m_watermineDamage{4.0f};
    float m_enemyUnitCountDamageReductionMultiplier{0.5f};
    float m_fundsPerBuildingFactorA{2.5f};
    float m_fundsPerBuildingFactorB{1.65f};
    float m_ownUnitEnemyUnitRatioAverager{10};
    float m_maxDayScoreVariancer{10};
    float m_directIndirectUnitBonusFactor{1.2f};

    float m_maxBuildingTargetFindLoops{5};
    float m_scoringCutOffDamageHigh{Unit::DAMAGE_100};
    float m_scoringCutOffDamageLow{7.5f};
    float m_smoothingValue{3};
    float m_maxDistanceMultiplier{1.5f};
    float m_sameIslandBonusInRangeDays{2};
    float m_sameIslandOutOfDayMalusFactor{0.2f};
    float m_highDamageBonus{2};
    float m_midDamageBonus{1.5f};
    float m_lowDamageBonus{1};
    float m_veryLowDamageBonus{0.5f};
    float m_transportBonus{0.125f};
    float m_currentlyNotAttackableBonus{0.5};
    float m_differentIslandBonusInRangeDays{1};
    float m_differentIslandOutOfDayMalusFactor{0.33f};
    float m_noTransporterBonus{70};
    float m_transporterToRequiredPlaceFactor{3};
    float m_minFlyingTransportScoreForBonus{15};
    float m_flyingTransporterBonus{15};
    float m_smallTransporterBonus{30};
    float m_unitToSmallTransporterRatio{5};
    float m_additionalLoadingUnitBonus{5};
    qint32 m_indirectUnitAttackCountMalus{4};
    float m_minAttackCountBonus{5};
    float m_lowIndirectUnitBonus{0.3f};
    float m_lowIndirectMalus{0.5f};
    float m_highIndirectMalus{0.6f};
    float m_lowDirectUnitBonus{0.35f};
    float m_lowDirectMalus{0.3f};
    float m_highDirectMalus{0.6f};
    float m_minUnitCountForDamageBonus{3};
    float m_minInfantryCount{5};
    float m_currentlyNotAttackableScoreBonus{30};
    float m_coUnitBuffBonus{17};
    float m_nearEnemyBonus{10};
    float m_lowOwnBuildingEnemyBuildingRatio{1.25f};
    float m_lowInfantryRatio{0.4f};
    float m_buildingBonusMultiplier{0.75f};
    float m_lowIncomeInfantryBonusMultiplier{50};
    float m_movementpointBonus{0.33f};
    float m_damageToUnitCostRatioBonus{20};
    float m_superiorityRatio{2.5f};
    float m_cheapUnitRatio{0.9f};
    float m_cheapUnitBonusMultiplier{40};
    float m_normalUnitBonusMultiplier{10};
    float m_expensiveUnitBonusMultiplier{5};

    float m_ProducingTransportSearchrange{6};
    float m_ProducingTransportSizeBonus{10};
    float m_ProducingTransportRatioBonus{1.7f};
    float m_ProducingTransportLoadingBonus{15.0f};
    float m_ProducingTransportMinLoadingTransportRatio{3.0f};

    //CoreAI Methods
    //(public ones)
    /**
     * @brief contains
     * @param points
     * @param point
     * @return
     */
    static bool contains(QVector<QVector3D>& points, QPoint point);
    /**
     * @brief index
     * @param points
     * @param point
     * @return
     */
    static qint32 index(QVector<QVector3D>& points, QPoint point);
    /**
     * @brief useCOPower
     * @param pUnits
     * @param pEnemyUnits
     * @return
     */
    bool useCOPower(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits);
    /**
     * @brief calcBuildingDamage
     * @param pUnit
     * @param pBuilding
     * @return
     */
    float calcBuildingDamage(Unit* pUnit, QPoint newPosition, Building* pBuilding);
    /**
     * @brief createMovementMap
     */
    void createMovementMap(spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief addMovementMap
     * @param pBuilding
     * @param damage
     */
    void addMovementMap(Building* pBuilding, float damage);
    /**
     * @brief useBuilding
     * @param pBuildings
     * @return
     */
    bool useBuilding(spQmlVectorBuilding pBuildings);
    /**
     * @brief moveOoziums moves all those sweet nice ooziums :)
     * @param pUnits
     * @param pEnemyUnits
     */
    bool moveOoziums(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits);
    /**
     * @brief moveFlares
     * @param pUnits
     * @return
     */
    bool moveFlares(spQmlVectorUnit pUnits);
    /**
     * @brief moveBlackBombs
     * @param pUnits
     * @param pEnemyUnits
     * @return
     */
    bool moveBlackBombs(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits);
    /**
     * @brief moveSupport
     * @param pUnits
     * @return
     */
    bool moveSupport(NB_AISteps step, spQmlVectorUnit pUnits, bool useTransporters);
    /**
     * @brief nextAction
     */
    /**
     * @brief calcUnitDamage
     * @param pUnit
     * @param position
     * @param target
     * @return x = unit damage and y = counter damage
     */
    QRectF calcUnitDamage(spGameAction pAction, QPoint target);
    /**
     * @brief CoreAI::calcVirtuelUnitDamage
     * @param pAttacker
     * @param attackerTakenDamage
     * @param atkPos
     * @param defX
     * @param defY
     * @param defenderTakenDamage
     * @return
     */
    QRectF calcVirtuelUnitDamage(Unit* pAttacker, float attackerTakenDamage, QPoint atkPos,
                                 Unit* pDefender, float defenderTakenDamage, QPoint defPos,
                                 bool ignoreOutOfVisionRange = false);
    /**
     * @brief getBestTarget
     * @param pUnit
     * @param pAction
     * @param pPfs
     * @return target unit x, y and z = fonddamage
     */
    void getBestTarget(Unit* pUnit, spGameAction pAction, UnitPathFindingSystem* pPfs, QVector<QVector3D>& ret, QVector<QVector3D>& moveTargetFields);
    /**
     * @brief getAttacksFromField
     * @param pUnit
     * @param pAction
     * @param ret
     */
    void getBestAttacksFromField(Unit* pUnit, spGameAction pAction, QVector<QVector3D>& ret, QVector<QVector3D>& moveTargetFields);
    /**
     * @brief getAttackTargets
     * @param pUnit
     * @param pAction
     * @param pPfs
     * @param ret
     * @param moveTargetFields
     */
    void getAttackTargets(Unit* pUnit, spGameAction pAction, UnitPathFindingSystem* pPfs, QVector<QVector4D>& ret, QVector<QVector3D>& moveTargetFields);
    /**
     * @brief getAttacksFromField
     * @param pUnit
     * @param pAction
     * @param ret
     * @param moveTargetFields
     */
    void getAttacksFromField(Unit* pUnit, spGameAction pAction, QVector<QVector4D>& ret, QVector<QVector3D>& moveTargetFields);
    /**
     * @brief moveAwayFromProduction
     * @param pUnits
     * @return
     */
    bool moveAwayFromProduction(spQmlVectorUnit pUnits);
    /**
     * @brief CoreAI::calcFundsDamage
     * @param damage
     * @param pAtk
     * @param pDef
     * @return
     */
    QPointF calcFundsDamage(QRectF damage, Unit* pAtk, Unit* pDef);
    /**
     * @brief appendAttackTargets
     * @param pUnit
     * @param pEnemyUnits
     * @param targets
     */
    void appendAttackTargets(Unit* pUnit, spQmlVectorUnit pEnemyUnits, QVector<QVector3D>& targets);

    //protected (CoreAI Methods)
    void addMenuItemData(spGameAction pGameAction, QString itemID, qint32 cost);
    void addSelectedFieldData(spGameAction pGameAction, QPoint point);
    /**
     * @brief isAttackOnTerrainAllowed
     * @param pTerrain
     * @return
     */
    bool isAttackOnTerrainAllowed(Terrain* pTerrain);
    // helper functions to get targets for unit actions
    void appendSupportTargets(QStringList actions, Unit* pCurrentUnit, spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits, QVector<QVector3D>& targets);
    void appendCaptureTargets(QStringList actions, Unit* pUnit, spQmlVectorBuilding pEnemyBuildings,  QVector<QVector3D>& targets);
    void appendAttackTargetsIgnoreOwnUnits(Unit* pUnit, spQmlVectorUnit pEnemyUnits, QVector<QVector3D>& targets);
    void appendRepairTargets(Unit* pUnit, spQmlVectorBuilding pBuildings, QVector<QVector3D>& targets);
    void appendSupplyTargets(Unit* pUnit, spQmlVectorUnit pUnits, QVector<QVector3D>& targets);
    void appendTransporterTargets(Unit* pUnit, spQmlVectorUnit pUnits, QVector<QVector3D>& targets);
    void appendCaptureTransporterTargets(Unit* pUnit, spQmlVectorUnit pUnits,
                                         spQmlVectorBuilding pEnemyBuildings, QVector<QVector3D>& targets);
    QVector<Unit*> appendLoadingTargets(Unit* pUnit, spQmlVectorUnit pUnits,
                                        spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                                        bool ignoreCaptureTargets, bool virtualLoading, QVector<QVector3D>& targets,
                                        bool all = false);
    /**
     * @brief hasTargets checks if a unit has anything to do on this island
     * @param pLoadingUnit
     * @param canCapture
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @return
     */
    bool hasTargets(Unit* pLoadingUnit, bool canCapture, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                    qint32 loadingIslandIdx, qint32 loadingIsland);
    /**
     * @brief appendNearestUnloadTargets searches for unload fields closest to our current position
     * @param pUnit
     * @param pEnemyUnits
     * @param pEnemyBuildings
     */
    void appendNearestUnloadTargets(Unit* pUnit, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings, QVector<QVector3D>& targets);
    /**
     * @brief appendUnloadTargetsForCapturing searches unload fields near enemy buildings
     * @param pUnit
     * @param pEnemyBuildings
     */
    void appendUnloadTargetsForCapturing(Unit* pUnit, spQmlVectorBuilding pEnemyBuildings, QVector<QVector3D>& targets);
    /**
     * @brief appendTerrainBuildingAttackTargets
     * @param pUnit
     * @param pEnemyBuildings
     * @param targets
     */
    void appendTerrainBuildingAttackTargets(Unit* pUnit, spQmlVectorBuilding pEnemyBuildings, QVector<QVector3D>& targets);
    /**
     * @brief checkIslandForUnloading
     * @param pUnit
     * @param pLoadedUnit
     * @param checkedIslands
     * @param unitIslandIdx
     * @param unitIsland
     * @param loadedUnitIslandIdx
     * @param targetIsland
     * @param pUnloadArea
     * @param targets
     */
    void checkIslandForUnloading(Unit* pUnit, Unit* pLoadedUnit, QVector<qint32>& checkedIslands,
                                 qint32 unitIslandIdx, qint32 unitIsland,
                                 qint32 loadedUnitIslandIdx, qint32 targetIsland,
                                 QmlVectorPoint* pUnloadArea, QVector<QVector3D>& targets);
    /**
     * @brief getBestFlareTarget
     * @param pUnit
     * @param pAction
     * @param pPfs
     * @param flareTarget
     * @param moveTargetField
     */
    void getBestFlareTarget(Unit* pUnit, spGameAction pAction, UnitPathFindingSystem* pPfs, QPoint& flareTarget, QPoint& moveTargetField);
    /**
     * @brief getFlareTargetScore
     * @param moveTarget
     * @param flareTarget
     * @param pUnfogCircle
     * @return
     */
    qint32 getFlareTargetScore(const QPoint& moveTarget, const QPoint& flareTarget, const spQmlVectorPoint& pUnfogCircle);
    /**
     * @brief isUnloadTerrain
     * @param pUnit
     * @param pTerrain
     * @return
     */
    bool isUnloadTerrain(Unit* pUnit, Terrain* pTerrain);
    /**
     * @brief createIslandMap
     * @param pUnits
     */
    void rebuildIsland(spQmlVectorUnit pUnits);
    /**
     * @brief onSameIsland checks if unit1 can reach unit 2. This may be vice versa but isn't checked here
     * @param pUnit1
     * @param pUnit2
     * @return
     */
    bool onSameIsland(Unit* pUnit1, Unit* pUnit2);
    /**
     * @brief onSameIsland checks if unit1 can reach the building. This may be vice versa but isn't checked here
     * @param pUnit1
     * @param pBuilding
     * @return
     */
    bool onSameIsland(Unit* pUnit1, Building* pBuilding);
    /**
     * @brief onSameIsland
     * @param movemnetType
     * @param x
     * @param y
     * @param x1
     * @param y1
     * @return
     */
    bool onSameIsland(QString movemnetType, qint32 x, qint32 y, qint32 x1, qint32 y1);
    /**
     * @brief onSameIsland
     * @param islandIdx
     * @param x
     * @param y
     * @param x1
     * @param y1
     * @return
     */
    bool onSameIsland(qint32 islandIdx, qint32 x, qint32 y, qint32 x1, qint32 y1);
    /**
     * @brief getIsland
     * @param pUnit1
     * @return
     */
    qint32 getIsland(Unit* pUnit);
    /**
     * @brief getIslandIndex
     * @param pUnit1
     * @return
     */
    qint32 getIslandIndex(Unit* pUnit);
    /**
     * @brief createIslandMap
     * @param movementType
     * @param unitID
     */
    void createIslandMap(QString movementType, QString unitID);
    /**
     * @brief needsRefuel
     * @param pUnit
     * @return
     */
    bool needsRefuel(Unit* pUnit);
    /**
     * @brief hasMissileTarget
     * @return
     */
    inline bool hasMissileTarget()
    {
        return m_missileTarget;
    };
    /**
     * @brief getAiCoUnitMultiplier
     * @param pCO
     * @param pUnit
     * @return
     */
    float getAiCoUnitMultiplier(CO* pCO, Unit* pUnit);

    //this is not an actual build of a unit so goes here and not in a BuildingModule
    bool buildCOUnit(spQmlVectorUnit pUnits);

    /*
    bool buildCOUnit(spQmlVectorUnit pUnits);
    //*/

    //NormalAI Methods///////////////////////////////////////////////////////////////////////////

    bool performActionSteps(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits,
                            spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief isUsingUnit
     * @param pUnit
     * @return
     */
    bool isUsingUnit(Unit* pUnit);
    /**
     * @brief moveUnits
     * @param pUnits
     * @param pBuildings
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @return
     */
    bool moveUnits(spQmlVectorUnit pUnits, spQmlVectorBuilding pBuildings,
                   spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                   qint32 minfireRange, qint32 maxfireRange, bool supportUnits = false);
    /**
     * @brief refillUnits
     * @param pUnits
     * @param pBuildings
     * @param pEnemyBuildings
     * @return
     */
    bool refillUnits(spQmlVectorUnit pUnits, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief getBestRefillTarget
     * @param pfs
     * @param maxRefillCount
     * @param moveTarget
     * @param refillTarget
     * @return
     */
    bool getBestRefillTarget(UnitPathFindingSystem & pfs, qint32 maxRefillCount, QPoint & moveTarget, QPoint & refillTarget);
    /**
     * @brief appendRefillTargets
     * @param actions
     * @param pUnit
     * @param pUnits
     * @param targets
     */
    void appendRefillTargets(QStringList actions, Unit* pUnit, spQmlVectorUnit pUnits, QVector<QVector3D>& targets);
    /**
     * @brief moveUnit
     * @param pAction
     * @param pUnit
     * @param actions
     * @param targets
     * @param transporterTargets
     * @return
     */
    bool moveUnit(spGameAction pAction, Unit* pUnit, spQmlVectorUnit pUnits, QStringList& actions,
                  QVector<QVector3D>& targets, QVector<QVector3D>& transporterTargets,
                  bool shortenPathForTarget,
                  spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief loadUnits
     * @param pUnits
     * @return
     */
    bool loadUnits(spQmlVectorUnit pUnits, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief NormalAi::moveTransporters
     * @param pUnits
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @return
     */
    bool moveTransporters(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief moveToUnloadArea
     * @param pAction
     * @param pUnit
     * @param actions
     * @param targets
     * @return
     */
    bool moveToUnloadArea(spGameAction pAction, Unit* pUnit, spQmlVectorUnit pUnits, QStringList& actions,
                          QVector<QVector3D>& targets,
                          spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief repairUnits
     * @param pUnits
     * @param pBuildings
     * @return
     */
    bool repairUnits(spQmlVectorUnit pUnits, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief getMoveTargetField shortens the movepath so we take no damage
     * @param pUnit
     * @param movePath
     * @return
     */
    qint32 getMoveTargetField(Unit* pUnit, spQmlVectorUnit pUnits, UnitPathFindingSystem& turnPfs,
                              QVector<QPoint>& movePath, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief moveToSafety
     * @param pUnit
     * @param turnPfs
     * @return
     */
    std::tuple<QPoint, float, bool> moveToSafety(Unit* pUnit, spQmlVectorUnit pUnits,
                                                 UnitPathFindingSystem& turnPfs, QPoint target,
                                                 spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief captureBuildings
     * @param pUnits
     * @return
     */
    bool captureBuildings(spQmlVectorUnit pUnits);
    /**
     * @brief fireWithIndirectUnits
     * @param pUnits
     * @return
     */
    bool fireWithUnits(spQmlVectorUnit pUnits, qint32 minfireRange, qint32 maxfireRange,
                       spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief suicide
     * @param pAction
     * @param pUnit
     * @param turnPfs
     * @return
     */
    bool suicide(spGameAction pAction, Unit* pUnit, UnitPathFindingSystem& turnPfs);
    /**
     * @brief getBestAttackTarget
     * @param pUnit
     * @param ret
     * @param moveTargetFields
     * @return
     */
    qint32 getBestAttackTarget(Unit* pUnit, spQmlVectorUnit pUnits, QVector<QVector4D>& ret,
                               QVector<QVector3D>& moveTargetFields,
                               spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief updateEnemyData
     */
    void updateEnemyData(spQmlVectorUnit pUnits);
    /**
     * @brief calcVirtualDamage
     */
    void calcVirtualDamage(spQmlVectorUnit pUnits);
    /**
     * @brief calculateCaptureBonus
     * @param pUnit
     * @param newLife
     * @return
     */
    float calculateCaptureBonus(Unit* pUnit, float newLife);
    /**
     * @brief calculateCounterDamage
     * @param pUnit
     * @param newPosition
     * @param pEnemyUnit
     * @param enemyNewLife
     */
    float calculateCounterDamage(Unit* pUnit, spQmlVectorUnit pUnits, QPoint newPosition,
                                 Unit* pEnemyUnit, float enemyTakenDamage,
                                 spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings,
                                 bool ignoreOutOfVisionRange = false);
    /**
     * @brief calculateCounteBuildingDamage
     * @param pUnit
     * @param pBuildings
     * @param pEnemyBuildings
     * @return
     */
    float calculateCounteBuildingDamage(Unit* pUnit, QPoint newPosition, spQmlVectorBuilding pBuildings, spQmlVectorBuilding pEnemyBuildings);
    /**
     * @brief clearEnemyData
     */
    void clearEnemyData();
    /**
     * @brief getEnemyDamageCounts
     * @param pUnits
     * @param pEnemyUnits
     * @param attackCount
     */
    void getEnemyDamageCounts(spQmlVectorUnit pUnits,spQmlVectorUnit pEnemyUnits, QVector<QVector4D> & attackCount);
    /**
     * @brief NormalAi::calcExpectedFundsDamage
     * @param dummy
     * @param pEnemyUnits
     * @return
     */
    std::tuple<float, qint32> calcExpectedFundsDamage(qint32 posX, qint32 posY, Unit& dummy, spQmlVectorUnit pEnemyUnits, QVector<QVector4D> attackCount, float bonusFactor);
    /**
     * @brief getClosestTargetDistance
     * @param posX
     * @param posY
     * @param dummy
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @return
     */
    qint32 getClosestTargetDistance(qint32 posX, qint32 posY, Unit& dummy, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings);

};

#endif // NORMALBEHAVIORALMODULE_H
