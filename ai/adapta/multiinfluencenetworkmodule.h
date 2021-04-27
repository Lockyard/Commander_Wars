#ifndef MULTIINFLUENCENETWORKMODULE_H
#define MULTIINFLUENCENETWORKMODULE_H

#include "adaptamodule.h"
#include "ai/adapta/influencemap.h"
#include "ai/genetic/weightvector.h"
#include <QVector>

/**
 * @brief The MultiInfluenceNetworkModule class is a module based on the one explained in the adapta paper.
 * It uses multiple influence maps and a vector of weight for each unit and map to get a final different map for each
 * type of unit, which is used to determine the best action to perform
 */
class MultiInfluenceNetworkModule : public AdaptaModule
{
    Q_OBJECT
public:
    MultiInfluenceNetworkModule();

    virtual void readIni(QString filename) override;

    /**
     * @brief processStartOfTurn calculate bids for this turn and process stuff for the start of turn
     */
    virtual void processStartOfTurn() override;

    /**
     * @brief processHighestBidUnit process the action for the highest bid unit of this module
     * @param weighted
     */
    virtual void processHighestBidUnit() override;
    virtual void processUnit(Unit* pUnit) override;
    virtual void notifyUnitUsed(Unit* pUnit) override;

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


private:
    //the influence maps on the base layer of the simple network
    std::vector<InfluenceMap> m_baseInfMapLayer;

    //units supported by this module. The first are the ones it can use, the second are all the units which will be supported
    //if a unit will be in battle and will be not supported, there'll be a std behaviour (ignore it or give it a default value
    //I guess)
    QStringList m_unitList;
    QStringList m_unitListFull;

    std::vector<qint32> m_unitCount;

    //[n0_l0, n0_l1,.., n0_lL, n1_l0,.., nN,lL]
    std::vector<InfluenceMap> m_unitInfluenceMaps;
    //[g0, g1,.., gG]
    std::vector<InfluenceMap> m_globalInfluenceMaps;

    float m_minMapWeight{-1.0};
    float m_maxMapWeight{1.0};

    //weightvector and stuff to sort better the weights loaded from it
    /*[n0_k0_m0,.., n0_k0_mM, n0_k1_m0,.., n0_kK_mM, n1_k0_m0,.., nN_kK_mM,
     *  gk0_m0,.., gk0_mM, gk1_m0,.., gk1_mM,.., gkGK_mM,
     *  n0_s0,..n0_sS, n0_k0,.., n0_kK, n0_g0,.., n0_gG, n1_s0,..nN_gG]
     */
    WeightVector m_weightVector;

    qint32 m_requiredVectorLength;

    qint32 m_unitAmount; //N
    qint32 m_fullUnitAmount; //M
    qint32 m_stdMapsPerUnit; //S
    qint32 m_customMapsPerUnit; //K
    qint32 m_totalMapsPerUnit; //L = S + K (stands for local maps, compared to global)
    qint32 m_globalMapsAmount; //G
    qint32 m_globalCustomMapsAmount; //GK
    qint32 m_customWeightsPerUnitAmount; //M*K, calculated to ease a tiny bit performance
    //[gk0_m0,..,gk0_mM, gk1_m0,..gk1_mM, .., gGK_mM]
    std::vector<float> m_globalCustomInfluenceMapsUnitWeights2D; //size: GK * M
    //[n0_g0, n0_g1,..,n0_gG, n1_g0,.., n1_gG,.., nN_gG]
    std::vector<float> m_allGlobalInfluenceMapWeightsForUnit2D; //size: G * N
    //[n0_outmap, n1_outmap...]
    std::vector<InfluenceMap> m_unitOutputMaps;


    std::vector<float> m_dmgChart;

    //private methods
    void defaultInitializeUnitList(QStringList &unitList);

    void computeGlobalInfluenceMap(InfluenceMap &influenceMap, bool isCustom=false, quint32 customNum=0);
    /**
     * @brief compute the local influence map #localMapNum (l) of unit #unitNum (n)
     */
    void computeLocalInfluenceMap(InfluenceMap &influenceMap, quint32 localMapNum, quint32 unitNum, bool isCustom=false, quint32 customNum=0);


    /**
     * @brief Get the weight that the unit #unitNumber (n) has wrt the global map #globalMapNumber
     */
    inline float glbMapWeightForUnit(qint32 unitNumber, qint32 globalMapNumber) {
        return m_allGlobalInfluenceMapWeightsForUnit2D[globalMapNumber * m_unitAmount + unitNumber];
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

    // n*K + k
    /**
     * @brief get the local map #localMapNumber associated to unit number #unitNumber
     */
    inline InfluenceMap& influenceMapOfUnit(qint32 localMapNumber, qint32 unitNumber) {
        return m_unitInfluenceMaps[unitNumber*m_customMapsPerUnit + localMapNumber];
    }

};

#endif // MULTIINFLUENCENETWORKMODULE_H
