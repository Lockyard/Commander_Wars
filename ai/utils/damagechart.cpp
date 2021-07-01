#include "damagechart.h"
#include "resource_management/weaponmanager.h"
#include "game/gamemap.h"
#include "coreengine/console.h"
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

float DamageChart::getDmg(Unit* pAttacker, Unit* pDefender, qint32 mAttacker, qint32 mDefender, float attackerHp, float defenderHp) {
    //if < 0, which is default value, use actual units hps, otherwise use the given virtual hp (even if higher than 10, althought it doesn't really make sense)
    if(attackerHp < 0)
        attackerHp = pAttacker->getHp();
    if(defenderHp < 0)
        defenderHp = pDefender->getHp();
    float dmg1 = 0.0f;
    float dmg2 = 0.0f;
    if(pAttacker->hasAmmo1()) {
        dmg1 = dmgChart1At(mAttacker, mDefender);
    }
    if(pAttacker->hasAmmo2()) {
        dmg2 = dmgChart2At(mAttacker, mDefender);
    }
    //dmg1 is used to store t, total damage
    dmg1 = dmg1 > dmg2 ? dmg1 : dmg2; //pick the highest damage the unit can do
    dmg1 *= attackerHp / Unit::MAX_UNIT_HP;
    float defHpLost = Unit::MAX_UNIT_HP - defenderHp;
    qint32 defStars = GameMap::getInstance()->getTerrain(pDefender->getX(), pDefender->getY())->getDefense(pDefender);
    //it's the same as [ dmg1 - defStars * (dmg1 * .1f - dmg1 * .01f * defHpLost) ]
    return dmg1 * (1 - defStars * (.1f - defHpLost * .01f));
}


std::pair<float, float> DamageChart::getFundsDmgBidirectional(Unit* pAttacker, Unit* pDefender, qint32 mAttacker, qint32 mDefender) {
    float dmgFundsDone = getFundsDmg(pAttacker, pDefender, mAttacker, mDefender);
    float dmgFundsReceived = 0.0f;
    //if one unit is indirect, there's no counterdamage
    if(pAttacker->getBaseMinRange() > 1 || pDefender->getBaseMinRange() > 1) {
        float dmgToEnemyDone = getDmg(pAttacker, pDefender, mAttacker, mDefender);
        dmgFundsReceived = getFundsDmg(pDefender, pAttacker, mDefender, mAttacker, qMax(0.0f, pDefender->getHp() - dmgToEnemyDone));
    }
    return std::pair<float, float>(dmgFundsDone, dmgFundsReceived);
}

QString DamageChart::toQString() {
    QString res = "Damage Chart: Unit List IDs:\n[";

    for(quint32 i=0; i<m_unitIDs.size(); i++) {
        res+= "(" + QString::number(i) + ")" + m_unitIDs[i] +
                (i==m_unitIDs.size()-1 ? "]\n" : ", ");
    }
    qint32 M = m_totalUnits;

    res += "Full chart 1:\n";
    for(qint32 mAtt = 0; mAtt < M; mAtt++) {
        res += "|";
        for(qint32 mDef=0; mDef < M; mDef++) {
            QString numString = QString::number(dmgChart1At(mAtt, mDef));
            while(numString.length() < 3) {
                numString.prepend(" ");
            }
            res += numString + "|";
        }
        res += "\n";
    }
    res += "Full chart 2:\n";
    for(qint32 mAtt = 0; mAtt < M; mAtt++) {
        res += "|";
        for(qint32 mDef=0; mDef < M; mDef++) {
            QString numString = QString::number(dmgChart2At(mAtt, mDef));
            while(numString.length() < 3) {
                numString.prepend(" ");
            }
            res += numString + "|";
        }
        res += "\n";
    }

    return res;
}


//private methods
void DamageChart::initializeCharts() {
    WeaponManager* weaponManager = WeaponManager::getInstance();
    qint32 M = m_totalUnits;
    m_dmgChart1.resize(M*M, 0.0f);
    m_dmgChart2.resize(M*M, 0.0f);
    Player* pPlayer = GameMap::getInstance()->getCurrentPlayer();
    Player* pOpponent = GameMap::getInstance()->getPlayer(0) == pPlayer ?
                GameMap::getInstance()->getPlayer(1) : GameMap::getInstance()->getPlayer(0);

    for(qint32 mAtt = 0; mAtt < M; mAtt++) {
        //create a fake unit just to init it and get the weapon IDs
        spUnit pAttacker = new Unit(m_unitIDs[mAtt], pPlayer, false);
        QString weapon1ID = pAttacker->getWeapon1ID();
        QString weapon2ID = pAttacker->getWeapon2ID();

        //Unit attacker2(m_unitIDs[mAtt], pPlayer, false);
        for(qint32 mDef = 0; mDef < M; mDef++) {
            spUnit pDefender = new Unit(m_unitIDs[mDef], pOpponent, false);
            if(!pAttacker->getWeapon1ID().isEmpty())
                m_dmgChart1[mAtt*M + mDef] = qMax(0.0f, weaponManager->getBaseDamage(pAttacker->getWeapon1ID(), pDefender.get()));
            if(!pAttacker->getWeapon2ID().isEmpty())
                m_dmgChart2[mAtt*M + mDef] = qMax(0.0f, weaponManager->getBaseDamage(pAttacker->getWeapon2ID(), pDefender.get()));
        }
    }
}
