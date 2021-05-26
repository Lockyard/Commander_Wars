#include "adaptamodule.h"

AdaptaModule::AdaptaModule() {

}

AdaptaModule::AdaptaModule(Player* pPlayer) : m_pPlayer(pPlayer)
{
}

AdaptaModule::AdaptaModule(const AdaptaModule &other) {
    m_moduleWeight = other.m_moduleWeight;
}

AdaptaModule::AdaptaModule(AdaptaModule &&other) {
    m_moduleWeight = other.m_moduleWeight;
    other.m_moduleWeight = 0.0;
}

void AdaptaModule::operator=(const AdaptaModule &other) {
    m_moduleWeight = other.m_moduleWeight;
}

float AdaptaModule::getBidFor(Unit* pUnit) {
    for(qint32 i = 0; i < m_unitsBids.size(); i++) {
        if(m_unitsBids.at(i).first == pUnit)
            return m_unitsBids[i].second;
    }
    return 0.0f;
}

float AdaptaModule::getHighestBid(bool weighted /*=true*/) {
    if(m_isUnitBidsSorted && m_unitsBids.size() > 0) {
        return weighted ? m_unitsBids[0].second * m_moduleWeight : m_unitsBids[0].second;
    }

    float highestBid = 0;
    for(qint32 i=0; i < m_unitsBids.size(); i++) {
        if(m_unitsBids[i].second > highestBid) {
            highestBid = m_unitsBids[i].second;
        }
    }
    if(weighted)
        return highestBid * m_moduleWeight;
    else
        return highestBid;
}

Unit* AdaptaModule::getHighestBidUnit() {
    if(m_isUnitBidsSorted && m_unitsBids.size() > 0) {
        return m_unitsBids[0].first;
    }

    float highestBid = 0;
    qint32 highestBidIndex = -1;
    for(qint32 i=0; i < m_unitsBids.size(); i++) {
        if(m_unitsBids[i].second > highestBid) {
            highestBid = m_unitsBids[i].second;
            highestBidIndex = i;
        }
    }
    if(highestBidIndex == -1)
        return nullptr;
    else
        return m_unitsBids.at(highestBidIndex).first;
}


//getters and setters
float AdaptaModule::moduleWeight() const
{
    return m_moduleWeight;
}

void AdaptaModule::setModuleWeight(float moduleWeight)
{
    m_moduleWeight = moduleWeight;
}

Player *AdaptaModule::getPPlayer() const
{
    return m_pPlayer;
}

void AdaptaModule::setPPlayer(Player *pPlayer)
{
    m_pPlayer = pPlayer;
}

void AdaptaModule::sortUnitBids() {
    std::sort(m_unitsBids.begin(), m_unitsBids.end(), [](QPair<Unit*, float> p1, QPair<Unit*, float> p2) -> bool {return p1.second > p2.second;});
    m_isUnitBidsSorted = true;
}
