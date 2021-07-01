#ifndef NORMALBUILDINGMODULE_H
#define NORMALBUILDINGMODULE_H

#include "buildingmodule.h"
#include "ai/adaptaai.h"

/**
 * @brief This class is a copy of the NormalAI by Robosturm, adapted to be a building module of the adapta (much like the
 * NormalBehavioralModule for AI actions), so credits to him. Since is not the focus of the project to have a behavioral AI
 * it's just copied and adapted to work as a module. Only building-related functions and parameters are copied from the normal AI
 */
class NormalBuildingModule : public BuildingModule
{
public:
    NormalBuildingModule(AdaptaAI* pAdapta);

    // BuildingModule interface
public:
    void readIni(QString filename) override;
    void init(Player* pPlayer) override;
    void processStartOfTurn() override;
    void processWhatToBuild() override;
    bool buildHighestBidUnit() override;

    float getHighestBid(bool weighted = true) override;

private:
    //Own Methods

    /**
     * @return true if the production building can be used to create at least the cheapest unit (in terms of funds only, since it
     * doesn't check if the space is free)
     */
    bool canProductionBuildingBeUsedWithCurrentFunds(Building *pBuilding);
    //CoreAI structs/enums/const

    //NormalAI structs/enums/const
    struct UnitBuildData
    {
        QString unitId;
        bool isTransporter{false};
        bool canMove{false};
        bool indirectUnit{false};
        bool infantryUnit{false};
        qint32 cost{0};
        qint32 baseRange{0};
        float damage{0.0f};
        qint32 notAttackableCount{0};
        float coBonus{0.0f};
        float closestTarget{0.0f};
        qint32 movePoints{0};

        qint32 smallTransporterCount{0};
        qint32 loadingPlace{0};
        qint32 noTransporterBonus{0};
        qint32 transportCount{0};
        qint32 loadingCount{0};
        bool flying{false};
        bool isSmallTransporter{false};
    };
    struct ProductionData
    {
        qint32 m_x = -1;
        qint32 m_y = -1;
        QVector<UnitBuildData> m_buildData;
    };
    enum BuildItems
    {
        DirectUnitRatio = 0,
        InfantryUnitRatio = 1,
        IndirectUnit = 2,
        DirectUnit = 3,
        InfantryUnit = 4,
        FundsFactoryRatio = 5,
        BuildingEnemyRatio = 6,
        NotAttackableCount = 7,
        DamageData = 8,
        InfantryCount = 9,
        COBonus = 10,
        Movementpoints = 11,
        UnitEnemyRatio = 12,
        UnitCount = 13,
        ReachDistance = 14,
        UnitCost = 15,
        Max,
    };
    static constexpr float maxDayDistance = 6.0f;;


    //own variables
    float m_defaultBid{0.1f};
    AdaptaAI* m_pAdapta;

    //BasegameinputIF variables
    QVector<std::tuple<QString, float>> m_BuildingChanceModifier;

    //CoreAI variables
    QVector<spIslandMap> m_IslandMaps;
    bool m_missileTarget{false};

    //NormalAI variables
    /**
     * @brief m_productionData
     */
    QVector<ProductionData> m_productionData;

    float m_notAttackableDamage{25.0f};
    float m_midDamage{55.0f};
    float m_highDamage{65.0f};
    float m_directIndirectRatio{1.75f};
    qint32 m_minSiloDamage{7000};
    float m_spamingFunds{7500};

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

    //BasegameinputIF Methods
    /**
     * @brief getUnitBuildValue only applies to Normal AI and better
     * @param unitID unit to alter the building score
     * @return value score modification value. value <= 0 means won't be produced. value means 1.0 normal chance. value 2.0 means doubled score mostlikely AI will only build this unit
     */
    float getUnitBuildValue(QString unitID);

    //CoreAI Methods
    void addMenuItemData(spGameAction pGameAction, QString itemID, qint32 cost);
    /**
     * @brief GetUnitCounts
     * @param pUnits
     * @param infantryUnits
     * @param indirectUnits
     * @param directUnits
     * @param transportTargets
     */
    void GetOwnUnitCounts(spQmlVectorUnit pUnits, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                          qint32 & infantryUnits, qint32 & indirectUnits,
                          qint32 & directUnits, QVector<std::tuple<Unit*, Unit*>> & transportTargets);

    QVector<Unit*> appendLoadingTargets(Unit* pUnit, spQmlVectorUnit pUnits,
                                        spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                                        bool ignoreCaptureTargets, bool virtualLoading, QVector<QVector3D>& targets,
                                        bool all = false);

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
     * @brief isUnloadTerrain
     * @param pUnit
     * @param pTerrain
     * @return
     */
    bool isUnloadTerrain(Unit* pUnit, Terrain* pTerrain);

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

    //NormalAI methods
    bool buildUnits(spQmlVectorBuilding pBuildings, spQmlVectorUnit pUnits,
                    spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings);

    /**
     * @brief getEnemyDamageCounts
     * @param pUnits
     * @param pEnemyUnits
     * @param attackCount
     */
    void getEnemyDamageCounts(spQmlVectorUnit pUnits,spQmlVectorUnit pEnemyUnits, QVector<QVector4D> & attackCount);

    /**
     * @brief getIndexInProductionData
     * @param pBuilding
     * @return
     */
    qint32 getIndexInProductionData(Building* pBuilding);
    /**
     * @brief getUnitProductionIdx
     * @param index
     * @param unitId
     * @return
     */
    qint32 getUnitProductionIdx(qint32 index, QString unitId,
                                spQmlVectorUnit pUnits, QVector<std::tuple<Unit*, Unit*>> & transportTargets,
                                spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                                QVector<QVector4D> & attackCount, QVector<float> & buildData);
    /**
     * @brief calcBuildScore
     * @param data
     * @return
     */
    float calcBuildScore(QVector<float>& data);
    /**
     * @brief createUnitBuildData
     * @param x
     * @param y
     * @param unitBuildData
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @param attackCount
     * @param buildData
     */
    void createUnitBuildData(qint32 x, qint32 y, UnitBuildData & unitBuildData,
                             spQmlVectorUnit pUnits, QVector<std::tuple<Unit*, Unit*>> & transportTargets,
                             spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                             QVector<QVector4D> & attackCount, QVector<float> & buildData);
    /**
     * @brief calcCostScore
     * @param data
     * @return
     */
    float calcCostScore(QVector<float>& data);
    /**
     * @brief getTransporterData
     * @param unitBuildData
     * @param dummy
     * @param pUnits
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @param transportTargets
     */
    void getTransporterData(UnitBuildData & unitBuildData, Unit& dummy, spQmlVectorUnit pUnits,
                            spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings,
                            QVector<std::tuple<Unit*, Unit*>>& transportTargets);
    /**
     * @brief calcTransporterScore
     * @param posX
     * @param posY
     * @param dummy
     * @param pUnits
     * @param pEnemyUnits
     * @return
     */
    float calcTransporterScore(UnitBuildData & unitBuildData,  spQmlVectorUnit pUnits, QVector<float>& data);

    /**
     * @brief canTransportToEnemy
     * @param pUnit
     * @param pLoadedUnit
     * @param pEnemyUnits
     * @param pEnemyBuildings
     * @return
     */
    bool canTransportToEnemy(Unit* pUnit, Unit* pLoadedUnit, spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings);

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

#endif // NORMALBUILDINGMODULE_H
