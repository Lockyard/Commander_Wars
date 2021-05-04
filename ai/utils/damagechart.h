#ifndef DAMAGECHART_H
#define DAMAGECHART_H
#include <vector>
#include <QStringList>
#include "game/unit.h"

/**
 * @brief The DamageChart class stores a damage chart to have a faster retrieaval of base damage of unit type a to unit type b,
 * with both types of ammo. The chart is custom in size and types of units.
 * Since this class is meant to be faster than calling the JS scripts, it does the calculations on its own and doesn't call
 * scripts function (except maybe in future explicitly called methods which do so)
 */
class DamageChart
{
public:
    DamageChart();
    explicit DamageChart(QStringList unitIDsList);
    explicit DamageChart(std::vector<QString> unitIDsVector);
    DamageChart(DamageChart &other);
    DamageChart(DamageChart &&other);
    ~DamageChart() = default;

    void initialize(QStringList unitIDsList);
    void initialize(std::vector<QString> unitIDsVector);

    /**
     * @brief fast way to retrieve the damage done by attacker to defender. the passed indices are relative to the
     * stringlist/vector passed on the constructor
     */
    inline float getBaseDmg(qint32 mAttacker, qint32 mDefender);

    /**
     * @brief a bit slower way to retrieve the damage done by attacker to defender, using IDs
     */
    float getBaseDmg(QString attackerID, QString defenderID);

    /**
     * @brief get the highest base damage the unit attacker do to unit defender (indices are like in getBaseDmg(qint32, qint32)
     * This accounts also if the unit has ammos left to determine highest damage
     */
    float getBaseDmgWithAmmo(Unit* pUnit, qint32 mAttacker, qint32 mDefender);

    /**
     * @brief get the damage done by the attacker type of unit to the defender according to the real AWDS formula, except for
     * luck. This consider defense and ammos
     * Formula is from AWDS
     * if attackerStrength and defenderStrength are set, in range (0, 10], this virtual strength of the unit will be considered
     * they can be set higher than 10 but it shouldn't have sense
     */
    float getDmg(Unit* pAttacker, Unit* pDefender, qint32 mAttacker, qint32 mDefender, float attackerVirtualHp = -1, float defenderVirtualHp = -1);

    /**
     * @brief get the damage done in funds by the attacker type of unit to the defender according to the real AWDS formula,
     * except for luck. This consider defense and ammos
     * Formula is from AWDS
     */
    inline float getFundsDmg(Unit* pAttacker, Unit* pDefender, qint32 mAttacker, qint32 mDefender, float attackerVirtualHp = -1, float defenderVirtualHp = -1) {
        return qMin(pDefender->getCosts() * pDefender->getHp() / Unit::MAX_UNIT_HP,
                    getDmg(pAttacker, pDefender, mAttacker, mDefender, attackerVirtualHp, defenderVirtualHp) * pDefender->getCosts());
    }


    /**
     * @brief get funds damage both done and received after attack
     */
    std::pair<float, float> getFundsDmgBidirectional(Unit* pAttacker, Unit* pDefender, qint32 mAttacker, qint32 mDefender);


private:
    std::vector<QString> m_unitIDs;
    std::vector<float> m_dmgChart1; //MxM the base damage unit m1 does to unit m2 with weapon 1
    std::vector<float> m_dmgChart2; //MxM same but with weap 2
    qint32 m_totalUnits; //number of total units. It's the size of the unit IDs vector

    void initializeCharts();

    float inline dmgChart1At(qint32 mAtt, qint32 mDef) {
        return m_dmgChart1[mAtt*m_totalUnits + mDef];
    }

    float inline dmgChart2At(qint32 mAtt, qint32 mDef) {
        return m_dmgChart2[mAtt*m_totalUnits + mDef];
    }
};

#endif // DAMAGECHART_H
