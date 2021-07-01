#include "buildingmodule.h"

BuildingModule::BuildingModule() {}

BuildingModule::BuildingModule(const BuildingModule &other) {
    m_moduleWeight = other.m_moduleWeight;
    m_pPlayer = other.m_pPlayer;
}

BuildingModule::BuildingModule(BuildingModule &&other) {
    m_moduleWeight = other.m_moduleWeight;
    m_pPlayer = other.m_pPlayer;
    other.m_moduleWeight = 0.0;
    other.m_pPlayer = nullptr;
}

BuildingModule &BuildingModule::operator=(const BuildingModule &other) {
    m_moduleWeight = other.m_moduleWeight;
    m_pPlayer = other.m_pPlayer;
    return *this;
}

void BuildingModule::init(Player *pPlayer) {
    m_pPlayer = pPlayer;
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
    if(weighted)
        return highestBid * m_moduleWeight;
    else
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
