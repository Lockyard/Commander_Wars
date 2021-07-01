#include "adaptaai.h"
#include "adapta/convolutionalnnmodule.h"
#include "adapta/multiinfluencenetworkmodule.h"
#include "adapta/normalbehavioralmodule.h"
#include "adapta/normalbuildingmodule.h"
#include "coreengine/console.h"
#include "ai/adapta/trainingmanager.h"
#include "ai/utils/aiutils.h"

#include "menue/mapselectionmapsmenue.h"

AdaptaAI::AdaptaAI() : CoreAI(GameEnums::AiTypes_Adapta), m_isFirstProcessOfTurn(true)
{
    //*/
    setObjectName("AdaptaAI");
    Interpreter::setCppOwnerShip(this);
    Mainapp* pApp = Mainapp::getInstance();
    this->moveToThread(pApp->getWorkerthread());
    //loadIni("normal/" + configurationFile);
    //*/
    //readIni("mockup");
    NormalBehavioralModule* nbm = new NormalBehavioralModule(this);
    NormalBuildingModule* nbmBuild = new NormalBuildingModule(this);

    nbm->readIni("resources/aidata/adapta/normalbehavior.ini");
    nbmBuild->readIni("resources/aidata/adapta/normalbuilding.ini");

    m_modules.append(nbm);
    m_buildingModules.append(nbmBuild);
    Console::print("Normal AI Behavior and Building modules loaded", Console::eDEBUG);

    /*/
    MultiInfluenceNetworkModule* pMin = new MultiInfluenceNetworkModule(m_pPlayer, this);
    pMin->readIni("resources/aidata/adapta/MINextermination.ini");
    TrainingManager::instance().requestWVLength(pMin->getRequiredWeightVectorLength());
    WeightVector wv = TrainingManager::instance().getAssignedWeightVector();
    Console::print("wv is of size " + QString::number(wv.size()) + " (requested: " +
                   QString::number(pMin->getRequiredWeightVectorLength()) + ")", Console::eDEBUG);
    pMin->assignWeightVector(wv);
    Console::print("MIN mockup module loaded. Result:\n" + pMin->toQString(), Console::eINFO);
    oxygine::intrusive_ptr<MultiInfluenceNetworkModule> spMin(pMin);
    m_modules.append(pMin);
    //*/
}

void AdaptaAI::readIni(QString name) {
    //todo remove this test and actually read an ini
}

void AdaptaAI::init() {
    CoreAI::init();
    initModules();
}

void AdaptaAI::process() {
    if(m_isFirstProcessOfTurn) {
        m_isFirstProcessOfTurn = false;

        setCurrentProcessInfosNonValid();

        //compute all modules' bids, based on current state of game
        for(auto spModule : qAsConst(m_modules)) {
            spModule->processStartOfTurn();
        }
        for(auto spBuildingModule : qAsConst(m_buildingModules)) {
            spBuildingModule->processStartOfTurn();
        }
    }

    float highestBid = 0;
    qint32 selectedModuleIndex = -1;
    m_lastSelectedModuleIndex = -1;
    //for each active unit, sorted by highest bid (?), select the module who bid the most and make it do its action
    //(each unit with a bid of 0 by every module is selected to do action wait)
    for(qint32 i = 0; i < m_modules.size(); i++) {
        if(m_modules[i]->getHighestBid() > highestBid) {
            highestBid = m_modules[i]->getHighestBid();
            selectedModuleIndex = i;
        }
    }
    //select the highest bid unit and make that module use that unit
    if(selectedModuleIndex != -1) {
        m_lastSelectedModuleIndex = selectedModuleIndex;
        if(!m_modules[selectedModuleIndex]->processHighestBidUnit()) {
            Console::print("Module didn't actually perform any action! Finishing turn to avoid livelock", Console::eWARNING);
            finishTurn();
            return;
        }
    }
    //when no module makes a bid, all units are used, so bid for building stuff and build until there is money
    //and factories to do so, according to bids
    else {
        //here highestBid = 0 and selectedModuleIndex = -1

        //see if any module has to build something
        for(qint32 i=0; i < m_buildingModules.size(); i++) {
            m_buildingModules[i]->processWhatToBuild();
            float currModuleHighestBid = m_buildingModules[i]->getHighestBid();
            if(currModuleHighestBid > highestBid) {
                highestBid = currModuleHighestBid;
                selectedModuleIndex = i;
            }
        }
        //select the module which had the highest bid, and make it create a unit
        if(selectedModuleIndex != -1) {
            //build something with the selected module...
            if(!m_buildingModules[selectedModuleIndex]->buildHighestBidUnit()) {
                //if the module didn't build anything, end turn to avoid livelock
                finishTurn();
                return;
            }
        }
        //if no module nor buildingModule has done an action, end turn
        else {
            finishTurn();
        }
    }
}

void AdaptaAI::addSelectedFieldData(spGameAction pGameAction, qint32 pointX, qint32 pointY) {
    pGameAction->writeDataInt32(pointX);
    pGameAction->writeDataInt32(pointY);
    pGameAction->setInputStep(pGameAction->getInputStep() + 1);
}

spQmlVectorUnit AdaptaAI::getCurrentProcessSpUnits() {
    if(!m_currProcessInfo.validSpUnits)
        updateCurrentProcessInfoUnits();
    return m_currProcessInfo.spUnits;
}

spQmlVectorUnit AdaptaAI::getCurrentProcessSpEnemies() {
    if(!m_currProcessInfo.validSpEnemies)
        updateCurrentProcessInfoEnemies();
    return m_currProcessInfo.spEnemies;
}

spQmlVectorBuilding AdaptaAI::getCurrentProcessSpBuildings() {
    if(!m_currProcessInfo.validSpBuildings)
        updateCurrentProcessInfoBuildings();
    return m_currProcessInfo.spBuildings;
}

spQmlVectorBuilding AdaptaAI::getCurrentProcessSpEnemyBuildings() {
    if(!m_currProcessInfo.validSpEnemyBuildings)
        updateCurrentProcessInfoEnemyBuildings();
    return m_currProcessInfo.spEnemyBuildings;
}

//slot
void AdaptaAI::initModules() {
    for(auto spModule : qAsConst(m_modules)) {
        spModule->init(m_pPlayer);
    }
    for(auto spBuildingModule : qAsConst(m_buildingModules)) {
        spBuildingModule->init(m_pPlayer);
    }
}


//protected
void AdaptaAI::finishTurn()
{
    m_isFirstProcessOfTurn = true;
    CoreAI::finishTurn();
}

