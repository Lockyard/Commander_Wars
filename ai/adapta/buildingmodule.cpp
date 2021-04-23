#include "buildingmodule.h"

BuildingModule::BuildingModule(Player* pPlayer) : m_pPlayer(pPlayer)
{

}

BuildingModule::BuildingModule(const BuildingModule &other) {
    m_moduleWeight = other.m_moduleWeight;
}

BuildingModule::BuildingModule(BuildingModule &&other) {
    m_moduleWeight = other.m_moduleWeight;
    other.m_moduleWeight = 0.0;
}

BuildingModule &BuildingModule::operator=(const BuildingModule &other) {
    m_moduleWeight = other.m_moduleWeight;
}

/**
 * @brief getHighestBid get the value of the highest bid done by this module. By default the bid is weighted by
 * this module's weight (see [set]moduleWeight())
 */
float BuildingModule::getHighestBid(bool weighted/*=true*/) {
    float highestBid = 0;
    for(qint32 i=0; i < m_buildingsBids.size(); i++) {
        if(m_buildingsBids[i] > highestBid) {
            highestBid = m_buildingsBids[i];
        }
    }
    return highestBid;
}

Building* BuildingModule::getHighestBidBuilding() {
    float highestBid = 0;
    qint32 highestBidIndex = -1;
    for(qint32 i=0; i < m_buildingsBids.size(); i++) {
        if(m_buildingsBids[i] > highestBid) {
            highestBid = m_buildingsBids[i];
            highestBidIndex = i;
        }
    }
    if(highestBidIndex == -1)
        return nullptr;
    else
        return m_buildings->at(highestBidIndex);
}

//getters, setters
float BuildingModule::moduleWeight() const {
    return m_moduleWeight;
}

void BuildingModule::setModuleWeight(float moduleWeight) {
    m_moduleWeight = moduleWeight;
}
