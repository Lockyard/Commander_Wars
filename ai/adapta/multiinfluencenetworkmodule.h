#ifndef MULTIINFLUENCENETWORKMODULE_H
#define MULTIINFLUENCENETWORKMODULE_H

#include "adaptamodule.h"
#include "ai/adapta/influencemap.h"
#include "ai/genetic/weightvector.h"
#include "ai/utils/damagechart.h"
#include "ai/adaptaai.h"
#include "game/unitpathfindingsystem.h"
#include <set>
#include <QVector>

class MultiInfluenceNetworkModule;
typedef oxygine::intrusive_ptr<MultiInfluenceNetworkModule> spMultiInfluenceNetworkModule;

/**
 * @brief The MultiInfluenceNetworkModule class is a module based on the one explained in the adapta paper.
 * It uses multiple influence maps and a vector of weight for each unit and map to get a final different map for each
 * type of unit, which is used to determine the best action to perform
 */
class MultiInfluenceNetworkModule : public AdaptaModule
{
public:
    struct UnitStatus {
        float hp;
        qint32 posX;
        qint32 posY;
    };

    struct UnitData {
        Unit* pUnit;
        QString unitID;
        spUnitPathFindingSystem pPfs;
        float bid; //bid for this unit
        //the last status unit N has seen for this unit
        std::vector<UnitStatus> statusForUnit;
        bool isEnemy;
        bool isKilled;

        /*
        inline bool operator<(const UnitData &other) const {
            return uniqueID < other.uniqueID;
        }

        inline bool operator==(const UnitData &other) const {
            return uniqueID == other.uniqueID;
        }

        inline bool operator>(const UnitData &other) const {
            return uniqueID > other.uniqueID;
        }//*/
    };

    MultiInfluenceNetworkModule() = default;

    MultiInfluenceNetworkModule(AdaptaAI* ai);

    MultiInfluenceNetworkModule(MultiInfluenceNetworkModule &other) = default;
    MultiInfluenceNetworkModule(MultiInfluenceNetworkModule &&other) = default;
    MultiInfluenceNetworkModule &operator=(const MultiInfluenceNetworkModule &other);

    virtual ~MultiInfluenceNetworkModule() = default;

    virtual void readIni(QString filename) override;

    virtual void init(Player* pPlayer) override;

    bool loadVectorFromFile(QString file);

    /**
     * @brief processStartOfTurn calculate bids for this turn and process stuff for the start of turn
     */
    virtual void processStartOfTurn() override;

    /**
     * @brief processHighestBidUnit process the action for the highest bid unit of this module
     * @param weighted
     */
    virtual bool processHighestBidUnit() override;
    virtual bool processUnit(Unit* pUnit) override;
    virtual void notifyUnitUsed(Unit* pUnit) override;

    virtual float getBidFor(Unit* pUnit) override;
    /**
     * @brief getHighestBid get the value of the highest bid done by this module. By default the bid is weighted by
     * this module's weight (see [set]moduleWeight())
     */
    virtual float getHighestBid(bool weighted = true) override;
    virtual Unit* getHighestBidUnit() override;

    /**
     * @brief assignWeightVector assign a WV. The length must be ok (use getREquiredWeightVectorLength() to check)
     * Its format is this:
     * N = num of units used by this module
     * M = num of total units supported by this module
     * S = num of standard influence maps used by each unit
     * K = num of custom influence maps used by each unit
     * G = num of global influence maps
     * GK = num of global custom influence maps
     * GS = num of global standard influence maps
     * For performance, it's organized like this (undercase letters are an index of the corresponding uppercase. If
     * it ends with an m then it's relative to a unit, if it ends with a letter relative to an influence map (s, g, k)
     * then it's a weight relative to the whole influence map, for a certain unit n:
     *
     * [n0_k0_m0,.., n0_k0_mM, n0_k1_m0,.., n0_kK_mM, n1_k0_m0,.., nN_kK_mM,
     *  gk0_m0,.., gk0_mM, gk1_m0,.., gk1_mM,.., gkGK_mM,
     *  n0_s0,..n0_sS, n0_k0,.., n0_kK, n0_g0,.., n0_gG, n1_s0,..nN_gG]
     *
     * First there are all weights for custom map 0 for unit 0. Each weight is how much each target unit m weights wrt the unit n considered. This is repeated for every custom map assigned to
     * unit 0 (k0, k1, etc to kK). Then there's the same for unit 1, up to unit N. After this there are weights for each map assigned to each unit for each std, custom and global map
     * Each map is weighted differently for each unit
     * @param assignedWV
     * @return true if the assignment was correctly done, false otherwise
     */
    bool assignWeightVector(WeightVector assignedWV);


    qint32 getRequiredWeightVectorLength();

    virtual QString toQString() override;


    AdaptaAI *getPAdapta() const;
    void setPAdapta(AdaptaAI *pAdapta);

    /**
     * @brief generateTransferLearningMask returns 2 vectors, which represent the weights to be transferred from an origin MIN
     * module to a target one. For instance if the first has weights for INFANTRY and LIGHT_TANK only, and the second has also
     * ARTILLERY weights, it will be generated a vector suitable for the second one which has 3 units, but weights of INFANTRY and
     * LIGHT_TANK will be set based on the first one.
     * The vector of bools tells which weights have to be transferred, the float vector contains the weights
     */
    static std::pair<std::vector<bool>, std::vector<float>> generateTransferLearningMask(MultiInfluenceNetworkModule &originMIN,
                                                                                         MultiInfluenceNetworkModule &targetMIN);

private:
    AdaptaAI* m_pAdapta;

    //units supported by this module. The first are the ones it can use, the second are all the units which will be supported
    //if a unit will be in battle and will be not supported, there'll be a std behaviour (ignore it or give it a default value
    //I guess)
    QStringList m_unitList;
    QStringList m_unitListFull;
    //these 2 map a unit ID to the corresponding position in the above m_unitList and m_unitListFull. Since they are maps, it's
    //faster than an QStringList.indexOf()
    std::map<QString, qint32> m_unitIDToN;
    std::map<QString, qint32> m_unitIDToM;

    //this is long N (like m_unitList) and contains at each position in parallel an instantiation of a unit with that ID
    //this unit is not in game but is used to retrieve the unit type's properties
    std::vector<spUnit> m_unitTypesVector;
    //these are long M and contain M fake units of a player and the opponent respectively
    std::vector<spUnit> m_armyUnitTypesVector;
    std::vector<spUnit> m_enemyUnitTypesVector;

    std::vector<qint32> m_unitCount;
    //has the influence map of unit n been computed, for this turn?
    std::vector<bool> m_isMapNComputed;

    //[n0_l0, n0_l1,.., n0_lL, n1_l0,.., nN,lL]
    std::vector<InfluenceMap> m_unitInfluenceMaps;
    //[g0, g1,.., gG]
    std::vector<InfluenceMap> m_globalInfluenceMaps;

    float m_minMapWeight{-1.0};
    float m_maxMapWeight{1.0};

    std::map<qint32, UnitData> m_armyUnitData;
    std::map<qint32, UnitData> m_enemyUnitData;
    qint32 m_armyUnitDataKillCount;
    qint32 m_enemyUnitDataKillCount;

    //weightvector and stuff to sort better the weights loaded from it
    /**
     * WeightVector stores all the weights, and also is used for its first part to retrieve all custom local weights
     * [n0_k0_m0,.., n0_k0_mM, n0_k1_m0,.., n0_kK_mM, n1_k0_m0,.., nN_kK_mM,
     *  gk0_m0,.., gk0_mM, gk1_m0,.., gk1_mM,.., gkGK_mM,
     *  n0_s0,..n0_sS, n0_k0,.., n0_kK, n0_g0,.., n0_gG, n1_s0,..nN_gG]
     */
    WeightVector m_weightVector;

    qint32 m_requiredVectorLength;

    qint32 m_unitAmount; //N
    qint32 m_fullUnitAmount; //M
    qint32 m_stdMapsPerUnit; //S
    qint32 m_customMapsPerUnit; //K
    qint32 m_localMapsPerUnit; //L = S + K (stands for local maps, compared to global)
    qint32 m_globalMapsAmount; //G
    qint32 m_globalCustomMapsAmount; //GK
    qint32 m_customWeightsPerUnitAmount; //M*K, calculated to ease a tiny bit performance
    //[gk0_m0,..,gk0_mM, gk1_m0,..gk1_mM, .., gGK_mM]
    std::vector<float> m_globalCustomInfluenceMapsUnitWeights2D; //size: GK * M
    //[n0_g0, n0_g1,..,n0_gG, n1_g0,.., n1_gG,.., nN_gG]
    std::vector<float> m_allGlobalInfluenceMapWeightsForUnit2D; //size: G * N
    //[n0_outmap, n1_outmap...]
    std::vector<InfluenceMap> m_unitOutputMaps;

    //vector where at index n is contained the index m, to quickly translate where unit n of m_unitList is in the full array
    //of units m_unitListFull
    std::vector<quint32> m_nToMIndexes;
    std::map<QString, qint32> m_unitIDToMIndexMap;

    DamageChart m_damageChart;

    //fast mode doesn't compute maps at the start of turn for every unit, but only when required by a unit.
    //this means that it can't generate bids based on the maps, but has to use another heuristic
    bool m_fastMode{true};


    //values parametrized used in functions
    //note: 1 step indicates tiles reachable in 1 turn
    //when propagating, how much each step tile is multiplied wrt the previous one
    float m_stepMultiplier{0.5f};
    //how many steps are computed for direct units
    float m_stepsForDirectUnits{2};
    //how many steps are computed for indirect units
    float m_stepsForIndirectUnits{1};
    //how much is valued each star of defense in the STD_MAPDEFENSE maps, if any. Plus other values for these maps
    float m_weightPerStar{10};
    float m_friendlyBuildingMultiplier{1.5f};
    float m_friendlyFactoryMultiplier{0.1f};
    /** How much will be the final tile on which each indirect unit is be multiplied on (wrt influence map),
     * if the indirect unit can attack some units. This is useful to incentivize the unit to attack instead of going against
     * other targets*/
    float m_indirectsTileStillMultiplier{1.5f};


    //private methods
    void defaultInitializeUnitList(QStringList &unitList);

    /**
     * Initialize stuff which require player pointer set
     */
    void initializeWithPlayerPtr();

    void generateBids(spQmlVectorUnit pUnits, bool useMapInfo);

    void initUnitData(std::map<qint32, UnitData> &unitDataSet, QmlVectorUnit* pUnits, bool isEnemy);

    void addUnitData(std::map<qint32, UnitData> &unitDataSet, Unit* pUnit, bool isEnemy);

    /**
     * @brief sync the references of army and enemy Unit data with the given, (most recent) units in game
     */
    void updateAllUnitsDataReferences(spQmlVectorUnit spUnits, spQmlVectorUnit spEnemies);
    /**
     * @brief check for all changes in UnitData since last process of this module for unit #unitNum (n)
     * if any, correct the output map of that unit type
     */
    void updateMapsChangesForUnit(qint32 unitNum);

    /**
     * @brief update all UnitData relative to unit of type #unitNum to the current map situation
     */
    void updateAllUnitDataForUnit(qint32 unitNum);
    /**
     * @brief update the content of the specified unit data with the actual values, for unit #unitNum (n)
     */
    inline void updateUnitDataForUnit(UnitData &data, qint32 unitNum) {
        if(data.isKilled) {
            data.statusForUnit[unitNum].hp = 0;
            data.statusForUnit[unitNum].posX = -1;
            data.statusForUnit[unitNum].posY = -1;
        } else {
            data.statusForUnit[unitNum].hp = data.pUnit->getHp();
            data.statusForUnit[unitNum].posX = data.pUnit->getX();
            data.statusForUnit[unitNum].posY = data.pUnit->getY();
        }
    }


    /**
     * @brief return true if UnitData is changed
     */
    inline bool isUnitDataChangedForUnit(const UnitData &data, qint32 unitNum) {
        if(data.isKilled)
            return data.statusForUnit[unitNum].hp != 0 || data.statusForUnit[unitNum].posX != -1 ||
                    data.statusForUnit[unitNum].posY != -1;
        else
            return data.pUnit->getHp() != data.statusForUnit[unitNum].hp || data.pUnit->getX() != data.statusForUnit[unitNum].posX ||
                    data.pUnit->getY() != data.statusForUnit[unitNum].posY;
    }

    inline bool isUnitDataPositionChangedForUnit(const UnitData &data, qint32 unitNum) {
        if(data.isKilled)
            return data.statusForUnit[unitNum].posX != -1 || data.statusForUnit[unitNum].posY != -1;
        else
            return  data.pUnit->getX() != data.statusForUnit[unitNum].posX ||
                    data.pUnit->getY() != data.statusForUnit[unitNum].posY;
    }

    /**
     * @brief correct the influence maps of unit #unitNum (n). This doesn't recompute the output map
     */
    void correctInfluenceMapsForUnit(UnitData &changedData, qint32 unitNum);

    /**
     * @brief correct influence map given wrt to the new data of a unit which is changed
     */
    void correctLocalInfluenceMap(InfluenceMap &influenceMap, UnitData &changedData, quint32 unitNum, bool isCustom=false, quint32 customNum=0);

    void computeGlobalInfluenceMap(InfluenceMap &influenceMap, bool isCustom=false, quint32 customNum=0);
    /**
     * @brief compute the local influence map &influenceMap of unit #unitNum (n), and if isCustom, the #customNum (k)
     */
    void computeLocalInfluenceMap(InfluenceMap &influenceMap, quint32 unitNum, spQmlVectorUnit spUnits, spQmlVectorUnit spEnemies, bool isCustom=false, quint32 customNum=0);

    void computeInfluenceMapsForUnit(qint32 unitNum, spQmlVectorUnit spUnits, spQmlVectorUnit spEnemies);

    /**
     * @brief compute the output map from 0 for unit #unitNum (n). By default it just compute the output, but if specified
     * it computes all maps from 0
     */
    void computeOutputMapForUnit(qint32 unitNum, bool computeAll = false, spQmlVectorUnit spUnits = nullptr, spQmlVectorUnit spEnemies = nullptr);

    /**
     * @brief find the point where a unit, considering its ammos, deals more damage, in std fund damage
     * x, y = enemy x and y, z = damage in funds
     * if z < 0 then there are no attackable units (or the attackable ones cause more damage to the unit than to the enemy in funds)
     */
    QVector3D findNearestHighestDmg(QPoint fromWherePoint, Unit* pUnit);


    //
    /**
     * @brief Get the weight that the unit #unitNumber (n) has wrt the global map #globalMapNumber
     */
    inline float glbMapWeightForUnit(qint32 unitNumber, qint32 globalMapNumber) {
        return m_allGlobalInfluenceMapWeightsForUnit2D[unitNumber * m_globalMapsAmount + globalMapNumber];
    }

    /**
     * @brief Get the weight that the global map #globalCustomMapNum has of the unit #unitNumber (m)
     */
    inline float glbCustomMapUnitWeight(qint32 globalCustomMapNum, qint32 unitNumber) {
        return m_globalCustomInfluenceMapsUnitWeights2D[globalCustomMapNum * m_fullUnitAmount + unitNumber];
    }

    //weightvector has all weights, but the first part is formatted to quickly retrieve custom unit weights
    //n, k, m -> index is n*(K*M) + k*(M) + m
    /**
     * @brief Get, for unit #usedUnitNumber (n/N), for its personal custom map #customMapNumber (k/K), the weight, for this
     * custom map, of a target unit #targetUnitNumber (m/M)
     */
    inline float customInfluenceMapUnitWeight(qint32 usedUnitNumber, qint32 customMapNumber, qint32 targetUnitNumber) {
        return m_weightVector[usedUnitNumber * m_customWeightsPerUnitAmount + customMapNumber * m_fullUnitAmount + targetUnitNumber];
    }

    // n*L + l
    /**
     * @brief get the local map #localMapNumber associated to unit number #unitNumber
     */
    inline InfluenceMap& influenceMapOfUnit(qint32 unitNumber, qint32 localMapNumber) {
        return m_unitInfluenceMaps[unitNumber*m_localMapsPerUnit + localMapNumber];
    }

    inline QString toQStringChangedDataForUnit(UnitData &data, qint32 unitNum) {
        if(data.isKilled) {
            return "[" + data.unitID + " (dead), hp: " + QString::number(data.statusForUnit[unitNum].hp) + ", (" + QString::number(data.statusForUnit[unitNum].posX) +
                    ", " + QString::number(data.statusForUnit[unitNum].posY) + ")] -> [hp:0, (-1, -1)]";
        } else {
            return "[" + data.pUnit->getUnitID() + ", hp: " + QString::number(data.statusForUnit[unitNum].hp) + ", (" + QString::number(data.statusForUnit[unitNum].posX) +
                    ", " + QString::number(data.statusForUnit[unitNum].posY) + ")] -> [hp: " + QString::number(data.pUnit->getHp()) +
                    ", (" + QString::number(data.pUnit->getX()) + ", " + QString::number(data.pUnit->getY()) + ")]";
        }
    }

    /**
     * @brief N * K * M = offset in the weightvector where global maps' units weights start
     */
    inline qint32 globalMapsUnitWeightsOffset() {
        return m_unitAmount * m_customMapsPerUnit * m_fullUnitAmount;
    }

    /**
     * @brief (N*K*M) + M * GK = offset in the weight vector where the maps weights begin
     */
    inline qint32 mapWeightsOffset() {
        return globalMapsUnitWeightsOffset() + (m_fullUnitAmount * m_globalCustomMapsAmount);
    }

    /**
     * @brief (mapWeightsPerUnit
     * @return (S + K + G) = how many map weights there are in the vector for each unit
     */
    inline qint32 mapWeightsPerUnit() {
        return m_stdMapsPerUnit + m_customMapsPerUnit + m_globalMapsAmount;
    }

    /**
     * @brief index of the Weight Vector of std map's weight number s of unit n
     */
    inline qint32 wvIndexOfStdMapWeightOfUnit(qint32 n, qint32 s) {
        return mapWeightsOffset() + (n*mapWeightsPerUnit()) + s;
    }

    /**
     * @brief index of the Weight Vector of custom map's weight number k of unit n
     */
    inline qint32 wvIndexOfCustomMapWeightOfUnit(qint32 n, qint32 k) {
        return mapWeightsOffset() + (n*mapWeightsPerUnit()) + m_stdMapsPerUnit + k;
    }

    /**
     * @brief index of the Weight Vector of global map's weight number g for unit n
     */
    inline qint32 wvIndexOfGlobalMapWeightForUnit(qint32 n, qint32 g) {
        return mapWeightsOffset() + (n*mapWeightsPerUnit()) + m_stdMapsPerUnit + m_customMapsPerUnit + g;
    }

    /**
     * @brief index of the Weight Vector of global custom map gk's weight of unit m
     */
    inline qint32 wvIndexOfGlobalCustomInfluenceMapUnitWeight(qint32 gk, qint32 m) {
        return globalMapsUnitWeightsOffset() + gk * m_fullUnitAmount + m;
    }

    /**
     * @brief Get, for unit #usedUnitNumber (n/N), for its personal custom map #customMapNumber (k/K), the weight, for this
     * custom map, of a target unit #targetUnitNumber (m/M)
     */
    inline qint32 wvIndexCustomInfluenceMapUnitWeight(qint32 usedUnitNumber, qint32 customMapNumber, qint32 targetUnitNumber) {
        return usedUnitNumber * m_customWeightsPerUnitAmount + customMapNumber * m_fullUnitAmount + targetUnitNumber;
    }


    /**
     * @brief get the k value (local custom map number k) of the #typeNumber custom map of type type.
     * For instance CUSTOM_ALLIES and typeNumber = 2 will return the k relative to the 2nd map of type CUSTOM_ALLIES in the
     * list of local custom influence maps of unit n. -1 if there's no such map
     */
    qint32 indexOfCustomMapWeightOfTypeAndNumberOfUnit(qint32 n, adaenums::iMapType type, qint32 typeNumber);


    /**
     * @brief get the s value (local std map number s) of the #typeNumber custom map of type type.
     * For instance STD_DAMAGE and typeNumber = 2 will return the s relative to the 2nd map of type STD_DAMAGE in the
     * list of local standard influence maps of unit n. -1 if there's no such map
     */
    qint32 indexOfStdMapWeightOfTypeAndNumberOfUnit(qint32 n, adaenums::iMapType type, qint32 typeNumber);


    /**
     * @brief get the g value (global map number g) of the #typeNumber map of type type.
     * For instance STD_DAMAGE and typeNumber = 2 will return the g relative to the 2nd map of type STD_DAMAGE in the
     * list of global influence maps of unit n. -1 if there's no such map
     */
    qint32 indexOfGlobalMapWeightOfTypeAndNumber(adaenums::iMapType type, qint32 typeNumber);


    /**
     * @brief get the gk value (global custom map number gk) of the #typeNumber map of type type.
     * For instance CUSTOM_1 and typeNumber = 2 will return the gk relative to the 2nd map of type CUSTOM_1 in the
     * list of global custom influence maps of unit n. -1 if there's no such map
     */
    qint32 indexOfGlobalCustomMapWeightOfTypeAndNumber(adaenums::iMapType type, qint32 typeNumber);



};

#endif // MULTIINFLUENCENETWORKMODULE_H
