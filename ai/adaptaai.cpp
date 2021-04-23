#include "adaptaai.h"

AdaptaAI::AdaptaAI() : CoreAI(GameEnums::AiTypes_Adapta), m_isFirstProcessOfTurn(true)
{

}

void AdaptaAI::readIni(QString name) {
    //todo
}

void AdaptaAI::process() {

    spQmlVectorUnit pUnits = m_pPlayer->getUnits();

    if(m_isFirstProcessOfTurn) {
        m_isFirstProcessOfTurn = false;
        // remove island maps of the last turn
        m_IslandMaps.clear();

        //compute all modules' bids, based on current state of game
        for(AdaptaModule &module : m_modules) {
            module.processStartOfTurn();
        }
    }

    rebuildIsland(pUnits.get());

    Unit* pUsedUnit;
    float highestBid = 0;
    qint32 selectedModuleIndex = -1;
    //for each active unit, sorted by highest bid (?), select the module who bid the most and make it do its action
    //(each unit with a bid of 0 by every module is selected to do action wait)
    for(qint32 i = 0; i < m_modules.size(); i++) {
        if(m_modules[i].getHighestBid() > highestBid) {
            highestBid = m_modules[i].getHighestBid();
            selectedModuleIndex = i;
        }
    }
    //select the highest bid unit and make that module use that unit
    if(selectedModuleIndex != -1) {
        pUsedUnit = m_modules[selectedModuleIndex].getHighestBidUnit();
        m_modules[selectedModuleIndex].processHighestBidUnit();
        //notify all other modules that that unit was used
        for(qint32 i=0; i < m_modules.size(); i++) {
            if(i!=selectedModuleIndex)
                m_modules[i].notifyUnitUsed(pUsedUnit);
        }
    }
    //when no module makes a bid, all units are used, so bid for building stuff and build until there is money
    //and factories to do so, according to bids
    else {
        //here highestBid = 0 and selectedModuleIndex = -1

        //see if any module has to build something
        for(qint32 i=0; i < m_buildingModules.size(); i++) {
            m_buildingModules[i].processWhatToBuild();
            if(m_buildingModules[i].getHighestBid() > highestBid) {
                highestBid = m_buildingModules[i].getHighestBid();
                selectedModuleIndex = i;
            }
        }
        //select the module which had the highest bid, and make it create a unit
        if(selectedModuleIndex != -1) {
            m_buildingModules[selectedModuleIndex].buildHighestBidUnit();
        } else {
            //if no module nor buildingModule has done an action, end turn
            finishTurn();
        }
    }
}




void AdaptaAI::finishTurn()
{
    m_isFirstProcessOfTurn = true;
    CoreAI::finishTurn();
}
