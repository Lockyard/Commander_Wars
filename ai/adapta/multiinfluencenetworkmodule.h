#ifndef MULTIINFLUENCENETWORKMODULE_H
#define MULTIINFLUENCENETWORKMODULE_H

#include "adaptamodule.h"
#include "ai/adapta/influencemap.h"
#include "ai/genetic/weightvector.h"
#include "ai/utils/damagechart.h"
#include "ai/adaptaai.h"
#include "game/unitpathfindingsystem.h"
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
    struct UnitData {
        Unit* m_pUnit;
        spUnitPathFindingSystem m_pPfs;
        float m_bid; //bid for this unit
    };

    MultiInfluenceNetworkModule() = default;

    MultiInfluenceNetworkModule(Player* pPlayer, AdaptaAI* ai);

    MultiInfluenceNetworkModule(MultiInfluenceNetworkModule &other) = default;
    MultiInfluenceNetworkModule(MultiInfluenceNetworkModule &&other) = default;
    MultiInfluenceNetworkModule &operator=(const MultiInfluenceNetworkModule &other);

    virtual ~MultiInfluenceNetworkModule() = default;

    virtual void readIni(QString filename) override;

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
    virtual bool assignWeightVector(WeightVector assignedWV) override;


    qint32 getRequiredWeightVectorLength();

    virtual QString toQString() override;


    AdaptaAI *getPAdapta() const;
    void setPAdapta(AdaptaAI *pAdapta);

private:
    AdaptaAI* m_pAdapta;

    //units supported by this module. The first are the ones it can use, the second are all the units which will be supported
    //if a unit will be in battle and will be not supported, there'll be a std behaviour (ignore it or give it a default value
    //I guess)
    QStringList m_unitList;
    QStringList m_unitListFull;

    //this is long N (like m_unitList) and contains at each position in parallel an instantiation of a unit with that ID
    //this unit is not in game but is used to retrieve the unit type's properties
    std::vector<spUnit> m_unitTypesVector;
    bool m_arePlayerPtrStuffInitialized{false};

    std::vector<qint32> m_unitCount;

    //[n0_l0, n0_l1,.., n0_lL, n1_l0,.., nN,lL]
    std::vector<InfluenceMap> m_unitInfluenceMaps;
    //[g0, g1,.., gG]
    std::vector<InfluenceMap> m_globalInfluenceMaps;

    float m_minMapWeight{-1.0};
    float m_maxMapWeight{1.0};

    std::vector<UnitData> m_armyUnitData;
    std::vector<UnitData> m_enemyUnitData;

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

    void initUnitData(std::vector<UnitData> &unitDataVector, QmlVectorUnit* pUnits);

    void computeGlobalInfluenceMap(InfluenceMap &influenceMap, bool isCustom=false, quint32 customNum=0);
    /**
     * @brief compute the local influence map &influenceMap of unit #unitNum (n), and if isCustom, the #customNum (k)
     */
    void computeLocalInfluenceMap(InfluenceMap &influenceMap, quint32 unitNum, bool isCustom=false, quint32 customNum=0);

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


};

#endif // MULTIINFLUENCENETWORKMODULE_H
