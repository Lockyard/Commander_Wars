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

AdaptaModule &AdaptaModule::operator=(const AdaptaModule &other) {
    m_moduleWeight = other.m_moduleWeight;
}

float AdaptaModule::getBidFor(Unit* pUnit) {
    for(qint32 i = 0; i < m_units->size(); i++) {
        if(m_units->at(i) == pUnit)
            return m_unitsBids[i];
    }
    return 0.0f;
}

float AdaptaModule::getHighestBid(bool weighted /*=true*/) {
    float highestBid = 0;
    for(qint32 i=0; i < m_unitsBids.size(); i++) {
        if(m_unitsBids[i] > highestBid) {
            highestBid = m_unitsBids[i];
        }
    }
    return highestBid;
}

Unit* AdaptaModule::getHighestBidUnit() {
    float highestBid = 0;
    qint32 highestBidIndex = -1;
    for(qint32 i=0; i < m_unitsBids.size(); i++) {
        if(m_unitsBids[i] > highestBid) {
            highestBid = m_unitsBids[i];
            highestBidIndex = i;
        }
    }
    if(highestBidIndex == -1)
        return nullptr;
    else
        return m_units->at(highestBidIndex);
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
