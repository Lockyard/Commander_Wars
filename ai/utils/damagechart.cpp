#include "damagechart.h"
#include "resource_management/weaponmanager.h"
#include <QtMath>

DamageChart::DamageChart() : m_totalUnits(0) {
}

DamageChart::DamageChart(QStringList unitIDsList) {
    m_unitIDs.reserve(unitIDsList.size());
    for(qint32 i=0; i < unitIDsList.size(); i++) {
        m_unitIDs.push_back(unitIDsList[i]);
    }
    m_totalUnits = m_unitIDs.size();

    initializeCharts();
}

DamageChart::DamageChart(std::vector<QString> unitIDsVector) {
    m_unitIDs = unitIDsVector;
    m_totalUnits = m_unitIDs.size();
    initializeCharts();
}

DamageChart::DamageChart(DamageChart &other) {
    m_unitIDs = other.m_unitIDs;
    m_dmgChart1 = other.m_dmgChart1;
    m_dmgChart2 = other.m_dmgChart2;
    m_totalUnits = m_unitIDs.size();
}

DamageChart::DamageChart(DamageChart &&other) {
    m_unitIDs = other.m_unitIDs;
    m_dmgChart1 = other.m_dmgChart1;
    m_dmgChart2 = other.m_dmgChart2;
    m_totalUnits = m_unitIDs.size();
    other.m_unitIDs.clear();
    other.m_dmgChart1.clear();
    other.m_dmgChart2.clear();
}

void DamageChart::initialize(QStringList unitIDsList) {
    m_unitIDs.reserve(unitIDsList.size());
    for(qint32 i=0; i < unitIDsList.size(); i++) {
        m_unitIDs.push_back(unitIDsList[i]);
    }
    m_totalUnits = m_unitIDs.size();

    initializeCharts();
}

void DamageChart::initialize(std::vector<QString> unitIDsVector) {
    m_unitIDs = unitIDsVector;
    m_totalUnits = m_unitIDs.size();
    initializeCharts();
}


inline float DamageChart::getBaseDmg(qint32 mAttacker, qint32 mDefender) {
    return dmgChart1At(mAttacker, mDefender) > dmgChart2At(mAttacker, mDefender) ? dmgChart1At(mAttacker, mDefender) : dmgChart2At(mAttacker, mDefender);
}

float DamageChart::getBaseDmg(QString attackerID, QString defenderID) {
    qint32 i_att = 0, i_def = 0;
    for(qint32 i=0; i < m_totalUnits; i++) {
        if(attackerID == m_unitIDs[i])
            i_att = i;
        if(defenderID == m_unitIDs[i])
            i_def = i;
        if(i_att && i_def)
            break;
    }

    return getBaseDmg(i_att, i_def);
}

float DamageChart::getBaseDmgWithAmmo(Unit *pUnit, qint32 mAttacker, qint32 mDefender) {
    float dmg1 = 0.0f;
    float dmg2 = 0.0f;
    if(pUnit->hasAmmo1()) {
        dmg1 = dmgChart1At(mAttacker, mDefender);
    }
    if(pUnit->hasAmmo2()) {
        dmg2 = dmgChart2At(mAttacker, mDefender);
    }
    return qMax(dmg1, dmg2);
}




void DamageChart::initializeCharts() {
    WeaponManager* weaponManager = WeaponManager::getInstance();
    qint32 M = m_totalUnits;
    m_dmgChart1.resize(M*M, 0.0f);
    m_dmgChart2.resize(M*M, 0.0f);

    for(qint32 mAtt = 0; mAtt < M; mAtt++) {
        Unit attacker(m_unitIDs[mAtt], nullptr, false);
        for(qint32 mDef = 0; mDef < M; mDef++) {
            Unit defender(m_unitIDs[mDef], nullptr, false);
            m_dmgChart1[mAtt*M + mDef] = qMin(0.0f, weaponManager->getBaseDamage(attacker.getWeapon1ID(), &defender));
            m_dmgChart2[mAtt*M + mDef] = qMin(0.0f, weaponManager->getBaseDamage(attacker.getWeapon2ID(), &defender));
        }
    }
}
