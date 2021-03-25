#include <QSettings>

#include "testfirstai.h"

#include "coreengine/console.h"

#include "game/gameaction.h"
#include "game/player.h"
#include "game/co.h"
#include "game/building.h"
#include "game/unit.h"
#include "game/gamemap.h"
#include "game/unitpathfindingsystem.h"
#include "resource_management/unitspritemanager.h"

TestFirstAI::TestFirstAI() : CoreAI(GameEnums::AiTypes_TestFirst),
    m_influenceMap(m_IslandMaps), m_inffMap(m_IslandMaps)
{
    rebuildIslandMaps = true;
}


void TestFirstAI::readIni(QString name) {
    QString a = name;
}


void TestFirstAI::process() {
    spQmlVectorUnit pUnits = m_pPlayer->getUnits();
    pUnits-> randomize();
    spQmlVectorBuilding pBuildings = m_pPlayer->getBuildings();

    spQmlVectorUnit pEnemyUnits = m_pPlayer->getEnemyUnits();
    pEnemyUnits->randomize();



    // create island maps at the start of turn
    if (rebuildIslandMaps)
    {
        rebuildIslandMaps = false;
        // remove island maps of the last turn
        m_IslandMaps.clear();

        // create influence map at the start of the turn
        m_influenceMap.reset();

        for (qint32 i= 0; i < pUnits->size(); i++)
        {
            Unit* pUnit = pUnits->at(i);
            m_influenceMap.addUnitInfluence(pUnit, pEnemyUnits, pUnit->getCosts()*.001f);
        }

        for (qint32 i= 0; i < pEnemyUnits->size(); i++)
        {
            Unit* pUnit = pEnemyUnits->at(i);

            //negative weight since are enemies, and this ai's units as enemies of the considered unit
            m_influenceMap.addUnitInfluence(pUnit, pUnits, pUnit->getCosts()*-.001f);
        }

        m_influenceMap.showAllInfo();

        Console::print("TestAI front lines created", Console::eDEBUG);
    }
    rebuildIsland(pUnits.get());

    //move a unit and if no unit has been moved finish turn
    if(moveAUnit(pUnits)) {}
    else if(buildAUnit(pBuildings)){}
    else{

        finishTurn();
    }


}

bool TestFirstAI::moveAUnit(spQmlVectorUnit pUnits) {
    //just move any available unit and make it capture stuff, then build other infantries. Just for test
    for(qint32 i = 0; i < pUnits->size(); i++) {

        Unit* pUnit = pUnits->at(i);
        qint32 movePoints = pUnit->getMovementpoints(pUnit->getPosition());
        Console::print("movePoints of unit at (" + QString::number(pUnit->getPosition().x()) + ", " +
                       QString::number(pUnit->getPosition().y()) + ") are " + QString::number(movePoints), Console::eDEBUG);

        if(pUnit->hasAction(ACTION_CAPTURE) && !pUnit->getHasMoved()) {

            //if is already capturing continue capture
            if(pUnit->getCapturePoints() > 0) {
                spGameAction pAction = new GameAction(ACTION_CAPTURE);
                pAction->setTarget(QPoint(pUnit->getX(),pUnit->getY()));
                emit performAction(pAction);
                return true;
            }
            //if not capturing try to capture a nearby building in range.
             else {
                spGameAction pAction = new GameAction(ACTION_CAPTURE);
                pAction->setTarget(QPoint(pUnit->getX(), pUnit->getY()));
                UnitPathFindingSystem upfs(pUnit);
                upfs.explore();
                QVector<QPoint> targets = upfs.getAllNodePoints();
                //for each reachable field
                for(qint32 j = 0; j<targets.size(); j++) {
                    //if is in the same spot as the unit set a empty movepath and cost 0
                    if(pUnit->getX() == targets[j].x() && pUnit->getY() == targets[j].y()) {
                        pAction->setMovepath(QVector<QPoint>(), 0);
                    }

                    else {
                        QVector<QPoint> path = upfs.getPath(targets[j].x(), targets[j].y());
                        pAction->setMovepath(path, upfs.getCosts(path));
                    }

                    //if the action of capturing can be performed then do it and emit it, else go on
                    if(pAction->canBePerformed()) {
                        emit performAction(pAction);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool TestFirstAI::buildAUnit(spQmlVectorBuilding pBuildings) {
    UnitSpriteManager* pUnitSpriteManager = UnitSpriteManager::getInstance();

    for (qint32 i = 0; i < pBuildings->size(); i++) {
        Building* pBuilding = pBuildings->at(i);

        if(pBuilding->isProductionBuilding() && pBuilding->getTerrain()->getUnit() == nullptr) {
            spGameAction pAction = new GameAction(ACTION_BUILD_UNITS);
            pAction->setTarget(QPoint(pBuilding->getX(), pBuilding->getY()));

            //if we can build a unit
            if(pAction->canBePerformed()) {

                // we're allowed to build units here
                spMenuData pData = pAction->getMenuStepData();
                if (pData->validData()) {
                    //hardcoded 21 to make it create only infantries. again just for test
                    QString unitID = pUnitSpriteManager->getID(21);
                    qint32 menuIndex = pData->getActionIDs().indexOf(unitID);
                    if (menuIndex >= 0 && pData->getEnabledList()[menuIndex])
                    {
                        CoreAI::addMenuItemData(pAction, unitID, pData->getCostList()[menuIndex]);
                        // produce the unit
                        if (pAction->isFinalStep())
                        {
                            emit performAction(pAction);
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void TestFirstAI::finishTurn() {
    rebuildIslandMaps = true;
    CoreAI::finishTurn();
}
