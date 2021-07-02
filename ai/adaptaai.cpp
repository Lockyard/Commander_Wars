#include "adaptaai.h"

#include <QSettings>

#include "adapta/convolutionalnnmodule.h"
#include "adapta/multiinfluencenetworkmodule.h"
#include "adapta/normalbehavioralmodule.h"
#include "adapta/normalbuildingmodule.h"
#include "coreengine/console.h"
#include "ai/adapta/trainingmanager.h"
#include "ai/utils/aiutils.h"

#include "menue/mapselectionmapsmenue.h"

const QString AdaptaAI::DEFAULT_CONFIG_NAME = "DefaultConfiguration";
const QString AdaptaAI::NO_MODULE_ID = "NONE";

AdaptaAI::AdaptaAI() : CoreAI(GameEnums::AiTypes_Adapta), m_isFirstProcessOfTurn(true)
{
    setObjectName("AdaptaAI");
    Interpreter::setCppOwnerShip(this);
    Mainapp* pApp = Mainapp::getInstance();
    this->moveToThread(pApp->getWorkerthread());
    loadIni("adapta/adaptaai.ini");
    //*/


}

void AdaptaAI::readIni(QString filename) {
    if(QFile::exists(filename)) {
        QSettings settings(filename, QSettings::IniFormat);

        settings.beginGroup("LoadConfiguration");
        QString configName = settings.value("ConfigName", "DefaultConfiguration").toString();
        settings.endGroup();

        bool requireAllLoadsOk;
        QStringList adaptaModulesIDs;
        QStringList buildingModulesIDs;
        QStringList adaptaModulesIniFiles;
        QStringList buildingModulesIniFiles;

        settings.beginGroup(configName);
        requireAllLoadsOk = settings.value("RequireAllLoadsOk", true).toBool();
        adaptaModulesIDs = settings.value("AdaptaModules").toStringList();
        buildingModulesIDs = settings.value("BuildingModules").toStringList();
        adaptaModulesIniFiles = settings.value("AdaptaIniFiles").toStringList();
        buildingModulesIniFiles = settings.value("BuildingIniFiles").toStringList();
        settings.endGroup();

        bool ok = loadModulesFromConfig(requireAllLoadsOk, adaptaModulesIDs, buildingModulesIDs, adaptaModulesIniFiles, buildingModulesIniFiles);

        //if load failed for whatever reason, load the default configuration
        if(!ok) {
            Console::print("Configuration '" + configName + "' failed to load! Loading default configuration '" + DEFAULT_CONFIG_NAME + "'!", Console::eWARNING);
            settings.beginGroup(DEFAULT_CONFIG_NAME);
            requireAllLoadsOk = settings.value("RequireAllLoadsOk", true).toBool();
            adaptaModulesIDs = settings.value("AdaptaModules").toStringList();
            buildingModulesIDs = settings.value("BuildingModules").toStringList();
            adaptaModulesIniFiles = settings.value("AdaptaIniFiles").toStringList();
            buildingModulesIniFiles = settings.value("BuildingIniFiles").toStringList();
            settings.endGroup();

            ok = loadModulesFromConfig(requireAllLoadsOk, adaptaModulesIDs, buildingModulesIDs, adaptaModulesIniFiles, buildingModulesIniFiles);

            //if even default config fails, which shouldn't really, but who knows, then just hardcode instantiate a
            //default config which should be the same anyway, a normal AI behavioral + building
            if(!ok) {
                Console::print("Default configuration failed! Instantiating a default Normal AI Behavior + Building configuration!", Console::eWARNING);
                loadDefaultConfig();
            } else {
                Console::print("AdaptaAI: Default Configuration was loaded", Console::eDEBUG);
            }
        } else {
            Console::print("AdaptaAI: Configuration '" + configName + "' was loaded", Console::eDEBUG);
        }
    } else {
        Console::print("AdaptaAI ini file not found. Instantiating a default Normal AI Behavior + Building configuration.", Console::eWARNING);
        loadDefaultConfig();
    }
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


bool AdaptaAI::loadModulesFromConfig(bool requireAllLoadsOk, QStringList adaptaModulesIDs, QStringList buildingModulesIDs,
                           QStringList adaptaModulesIniFiles, QStringList buildingModulesIniFiles) {
    if(adaptaModulesIDs.isEmpty() || buildingModulesIDs.isEmpty()) {
        Console::print("Loading AdaptaAI modules from file: Configuration was not found or is incomplete, since at least one of the lists is empty. "
                        " (if you want to have an empty list of modules, use '" + NO_MODULE_ID + "' as specified in the adapta ini file)", Console::eWARNING);
        return false;
    }

    if(!adaptaModulesIDs.contains(NO_MODULE_ID) && adaptaModulesIDs.size() != adaptaModulesIniFiles.size()) {
        Console::print("Loading AdaptaAI modules from file: mismatch of length of adaptaModules (" + QString::number(adaptaModulesIDs.size()) +
                       ") and length of their ini files (" + QString::number(adaptaModulesIniFiles.size()) + ")!", Console::eWARNING);
        return false;
    }
    if(!buildingModulesIDs.contains(NO_MODULE_ID) && buildingModulesIDs.size() != buildingModulesIniFiles.size()) {
        Console::print("Loading AdaptaAI modules from file: mismatch of length of adaptaModules (" + QString::number(adaptaModulesIDs.size()) +
                       ") and length of their ini files (" + QString::number(adaptaModulesIniFiles.size()) + ")!", Console::eWARNING);
        return false;
    }

    m_modules.reserve(adaptaModulesIDs.size());

    if(!adaptaModulesIDs.contains(NO_MODULE_ID)) {
        //here we are sure sizes of ids and inifiles lists are the same. they may be 0,
        for(qint32 i=0; i < adaptaModulesIDs.size(); i++) {
            spAdaptaModule loadedModule = adaenums::generateAdaptaModuleFromString(adaptaModulesIDs.at(i), this);
            //if generation of the module was not ok abort
            if(loadedModule.get() == nullptr) {
                if(requireAllLoadsOk) {
                    m_modules.clear();
                    Console::print("Error while generating module '" + adaptaModulesIDs.at(i) +"'!", Console::eWARNING);
                    return false;
                } else {
                    Console::print("Module '" + adaptaModulesIDs.at(i) + "' was not generated, skipping it since current configuration "
                    "doesn't require all modules", Console::eWARNING);
                    continue;
                }
            }
            //load the ini. If load goes well, add the module
            if(loadedModule->readIni(adaptaModulesIniFiles.at(i))) {
                m_modules.append(loadedModule);
                Console::print("'" + adaptaModulesIDs.at(i) + "' module loaded correctly with config file '" +
                               adaptaModulesIniFiles.at(i) + "'", Console::eDEBUG);
            }
            //if load fails, abort if all modules are required, else continue
            else {
                if(requireAllLoadsOk) {
                    m_modules.clear();
                    Console::print("Error while loading ini file '" + adaptaModulesIniFiles.at(i) + "' of module '" + adaptaModulesIDs.at(i) +"'", Console::eWARNING);
                    return false;
                } else {
                    Console::print("Module '" + adaptaModulesIDs.at(i) + "' didn't load ini correctly, skipping it since current configuration "
                                    "doesn't require all modules", Console::eWARNING);
                    continue;
                }
            }
        }
    }

    //skip building modules if there is contained the keyword which states not to have building modules, or if the list is empty
    if(buildingModulesIDs.contains(NO_MODULE_ID) || buildingModulesIDs.isEmpty())
        return true;

    m_buildingModules.reserve(buildingModulesIDs.size());

    for(qint32 i=0; i<buildingModulesIDs.size(); i++) {
        spBuildingModule loadedBuildingModule = adaenums::generateBuildingModuleFromString(buildingModulesIDs.at(i), this);
        //if generation of the module was not ok abort
        if(loadedBuildingModule.get() == nullptr) {
            if(requireAllLoadsOk) {
                m_modules.clear();
                m_buildingModules.clear();
                Console::print("Error while generating building module '" + buildingModulesIDs.at(i) + "'!", Console::eWARNING);
                return false;
            } else {
                Console::print("BuildingModule '" + buildingModulesIDs.at(i) + "' was not generated, skipping it since current configuration "
                                "doesn't require all modules", Console::eWARNING);
                continue;
            }
        }
        //load ini config. If load goes well, add the module
        if(loadedBuildingModule->readIni(buildingModulesIniFiles.at(i))) {
            m_buildingModules.append(loadedBuildingModule);
            Console::print("'" + buildingModulesIDs.at(i) + "' building module loaded correctly with config file '" +
                           buildingModulesIniFiles.at(i) + "'", Console::eDEBUG);
        }
        //if load fails, abort if all modules are required, else continue
        else {
            if(requireAllLoadsOk) {
                m_modules.clear();
                m_buildingModules.clear();
                Console::print("Error while loading ini file '" + buildingModulesIniFiles.at(i) + "' of building module '" + buildingModulesIDs.at(i) +"'", Console::eWARNING);
                return false;
            } else {
                Console::print("BuildingModule '" + buildingModulesIDs.at(i) + "' didn't load ini correctly, skipping it since current configuration "
                                "doesn't require all modules", Console::eWARNING);
                continue;
            }
        }
    }
    return true;
}


void AdaptaAI::loadDefaultConfig() {
    m_modules.clear();
    m_buildingModules.clear();
    //since they are the assigned to an intrusive ptr in the append(), they will be destroyed correctly
    NormalBehavioralModule* nbm = new NormalBehavioralModule(this);
    NormalBuildingModule* nbmBuild = new NormalBuildingModule(this);
    nbm->readIni("resources/aidata/adapta/normalbehavior.ini");
    nbmBuild->readIni("resources/aidata/adapta/normalbuilding.ini");
    m_modules.append(nbm);
    m_buildingModules.append(nbmBuild);
    Console::print("AdaptaAI: Default Normal AI Behavior and Building modules configuration loaded", Console::eDEBUG);
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

