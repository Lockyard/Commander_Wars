#include "influencemap.h"

#include "coreengine/mainapp.h"
#include "coreengine/globalutils.h"
#include "ai/coreai.h"

#include "game/gamemap.h"
#include "game/unitpathfindingsystem.h"
#include "resource_management/unitspritemanager.h"
#include "resource_management/fontmanager.h"
#include "coreengine/console.h"

#include <qfile.h>


/**
 * @brief InfluenceMap::InfluenceMap create with specified islands. Set to 0 all map
 * @param islands
 */
InfluenceMap::InfluenceMap()
{
    Mainapp* pApp = Mainapp::getInstance();
    this->moveToThread(pApp->getWorkerthread());
    Interpreter::setCppOwnerShip(this);
    spGameMap pMap = GameMap::getInstance();
    m_mapWidth = pMap->getMapWidth();
    m_mapHeight = pMap->getMapHeight();

    m_influenceMap2D.resize(m_mapWidth * m_mapHeight);
}

InfluenceMap::InfluenceMap(adaenums::iMapType type) {
    Mainapp* pApp = Mainapp::getInstance();
    this->moveToThread(pApp->getWorkerthread());
    Interpreter::setCppOwnerShip(this);
    spGameMap pMap = GameMap::getInstance();
    m_mapWidth = pMap->getMapWidth();
    m_mapHeight = pMap->getMapHeight();

    m_influenceMap2D.resize(m_mapWidth * m_mapHeight);

    m_type = type;
}

InfluenceMap::InfluenceMap(const InfluenceMap &other) {
    Mainapp* pApp = Mainapp::getInstance();
    this->moveToThread(pApp->getWorkerthread());
    Interpreter::setCppOwnerShip(this);
    spGameMap pMap = GameMap::getInstance();
    m_mapWidth = other.m_mapWidth;
    m_mapHeight = other.m_mapHeight;

    m_influenceMap2D.reserve(m_mapWidth * m_mapHeight);

    for (qint32 i=0; i<m_mapWidth*m_mapHeight; i++) {
        m_influenceMap2D.push_back(other.m_influenceMap2D.at(i));
    }

    m_type = other.m_type;
    m_weight = other.m_weight;
}

InfluenceMap::InfluenceMap(InfluenceMap &&other) {
    Mainapp* pApp = Mainapp::getInstance();
    this->moveToThread(pApp->getWorkerthread());
    Interpreter::setCppOwnerShip(this);
    spGameMap pMap = GameMap::getInstance();
    m_mapWidth = other.m_mapWidth;
    m_mapHeight = other.m_mapHeight;

    m_influenceMap2D.reserve(m_mapWidth * m_mapHeight);

    for (qint32 i=0; i<m_mapWidth*m_mapHeight; i++) {
        m_influenceMap2D.push_back(other.m_influenceMap2D.at(i));
    }

    other.m_influenceMap2D.clear();

    m_type = other.m_type;
    m_weight = other.m_weight;
}


InfluenceMap::~InfluenceMap(){
    if(isInfoTilesMapInitialized) {
        for(auto & sprite : m_infoTilesMap) {
            sprite->detach();
        }

        for (auto & textField : m_infoTextMap) {
            textField->detach();
        }
    }
}

void InfluenceMap::reset() {
    for(quint32 i = 0; i < m_influenceMap2D.size(); i++) {
        m_influenceMap2D[i] = 0;
    }
}


void InfluenceMap::addUnitAtkInfluence(Unit *pUnit, float attackWeight, float stepMultiplier, qint32 steps, qint32 stepsForIndirect, qint32 unitX, qint32 unitY) {
    qint32 movePoints = pUnit->getMovementpoints(pUnit->getPosition());
    if(stepsForIndirect == -3)
        stepsForIndirect = steps;
    spUnitPathFindingSystem spPfs = new UnitPathFindingSystem(pUnit, pUnit->getOwner());

    //if requested, leave the requested virtual position
    if(unitX < 0)
        unitX = pUnit->getX();
    if(unitY < 0)
        unitY = pUnit->getY();

    //Console::print("Adding atk influence (val: " + QString::number(attackWeight, 'f', 3) +  ") of [" + pUnit->getUnitID() + " (" + QString::number(unitX) + ", " + QString::number(unitY) + ")]", Console::eINFO);

    //if is an indirect unit
    if(pUnit->getBaseMaxRange() > 1) {

        std::vector<qint32> atkTilesMap2D(m_mapHeight*m_mapWidth, PathFindingSystem::infinite);
        //this is a 2D map overlapping the real tile map used to mark some tiles while calculating where a unit can attack
        //values >= 0 indicate in how many steps the unit REACHES that tile, (-1, so 0 are the tiles reachable in 1 step, etc)
        std::vector<qint32> stepTilesMap2D(m_mapHeight*m_mapWidth, PathFindingSystem::infinite);

        if(movePoints != 0) {
            spPfs->setIgnoreEnemies(false);
            //set virtual start point if any or normal start point
            spPfs->setStartPoint(unitX, unitY);
            //double the move points of the unit. used to calculate the influence on 2 steps
            spPfs->setMovepoints(movePoints*stepsForIndirect);
            spPfs->explore();

            auto points = spPfs->getAllNodePoints();
            //write the map of stepTiles to know on each tile how many turns this unit require to reach it, if it can (with max the given steps)
            for (const auto & point : points) {
                //in how many turns (steps) the unit reaches a tile (here 1 for tiles reachable in 1 step, 2 for 2 etc. 0 is for the tile where the unit is)
                if(spPfs->isReachable(point.x(), point.y())) {
                    qint32 cost = spPfs->getTargetCosts(point.x(), point.y());
                    if(cost == 0) {
                        stepTilesMap2D[point.y()*m_mapWidth + point.x()] = 0; //0 where the indirect unit is
                    } else {
                        stepTilesMap2D[point.y()*m_mapWidth + point.x()] = ((cost - 1) / movePoints) + 1; //1 one each tile it takes the unit to reach that point. So on for 2, 3 turns
                    }

                }
            }
        }
        //if movePoints == 0, the unit cannot move, just mark its tile as 0, and leave the others to infinite
        else {
            stepTilesMap2D[unitY*m_mapWidth + unitX] = 0;
        }

        //mark the attack map based on how many turn it takes to being able to attack a cell
        for(qint32 y=0; y < m_mapHeight; y++) {
            for(qint32 x=0; x < m_mapWidth; x++) {
                if(stepTilesMap2D[y*m_mapWidth + x] < PathFindingSystem::infinite) {
                    QPoint unitPos(unitX, unitY);
                    markAttackArea(pUnit->getMinRange(unitPos), pUnit->getMaxRange(unitPos), x, y, atkTilesMap2D, stepTilesMap2D[y*m_mapWidth + x]);
                }
            }
        }
        //now add the influence based on the attack range and turns to reach it
        for(qint32 y=0; y < m_mapHeight; y++) {
            for(qint32 x=0; x < m_mapWidth; x++) {
                if(atkTilesMap2D[y*m_mapWidth + x] < PathFindingSystem::infinite) {
                    //get how many steps it takes for this unit to reach current examined point, and update the multiplier accordingly
                    float multiplier = qPow(stepMultiplier, atkTilesMap2D[y*m_mapWidth + x]);
                    addValueAt(attackWeight * multiplier, x, y);
                }
            }
        }

    }
    //if is a direct unit
    else {
        //this is a 2D map overlapping the real tile map used to mark some tiles while calculating where a unit can attack
        //values >= 0 indicate in how many steps the unit REACHES that tile, (-1, so 0 are the tiles reachable in 1 step, etc)
        std::vector<qint32> stepTilesMap2D(m_mapHeight*m_mapWidth, PathFindingSystem::infinite);
        //a parallel map used to calculate the value of attacking each tile instead of moving on them
        //values >= 0 indicat in how many steps the unit CAN ATTACK that tile (-1 again)
        std::vector<qint32> atkTilesMap2D(m_mapHeight*m_mapWidth, -1);

        if(movePoints != 0) {
            spPfs->setIgnoreEnemies(false);
            //set virtual start point if any or normal start point
            spPfs->setStartPoint(unitX, unitY);
            //double the move points of the unit. used to calculate the influence on 2 steps
            spPfs->setMovepoints(movePoints*steps);
            spPfs->explore();

            auto points = spPfs->getAllNodePoints();
            //write the map of stepTiles to know on each tile how many turns this unit require to reach it, if it can (with max the given steps)
            for (const auto & point : points) {
                //in how many turns (steps) the unit reaches a tile (-1, so 0 is the immediate reachable ones, so on)
                if(spPfs->isReachable(point.x(), point.y())) {
                    stepTilesMap2D[point.y()*m_mapWidth + point.x()] = qMax(0, (spPfs->getTargetCosts(point.x(), point.y()) - 1) / movePoints);
                }
            }
        }
        //if movePoints == 0, the unit cannot move, just mark its tile as 0, and leave the others to infinite
        else {
            stepTilesMap2D[unitY*m_mapWidth + unitX] = 0;
        }

        qint32 currStepVal, currNearStepVal;

        //set the value for the atkTilesMap, which is the real used one
        for(qint32 y=0; y < m_mapHeight; y++) {
            for(qint32 x=0; x < m_mapWidth; x++) {
                qint32 currIndex = y*m_mapWidth + x;
                currStepVal = stepTilesMap2D[currIndex];
                if(x < m_mapWidth-1) {
                    currNearStepVal = stepTilesMap2D[currIndex + 1];
                    if(currStepVal > currNearStepVal) {
                        atkTilesMap2D[currIndex] = currNearStepVal; //save always the lesser value
                    } else if (currStepVal < currNearStepVal) {
                        atkTilesMap2D[currIndex + 1] = currStepVal;
                    }
                }
                if(y < m_mapHeight -1) {
                    currNearStepVal = stepTilesMap2D[(y+1)*m_mapWidth + x];
                    if(currStepVal > currNearStepVal) {
                        atkTilesMap2D[currIndex] = currNearStepVal; //save always the lesser value
                    } else if (currStepVal < currNearStepVal) {
                        atkTilesMap2D[(y+1)*m_mapWidth + x] = currStepVal;
                    }
                }
                //if value is not set, copy it from the stepTilesMap
                if(atkTilesMap2D[currIndex] == -1)
                    atkTilesMap2D[currIndex] = stepTilesMap2D[currIndex];
            }
        }

        //now for each tile add its influence based on the fact that the unit can attack it or not
        for(qint32 x=0; x<m_mapWidth; x++) {
            for(qint32 y=0; y<m_mapHeight; y++) {
                //if value is infinite then the tiles cannot even be attacked, so don't add influence. Otherwise add it
                if(atkTilesMap2D[y*m_mapWidth + x] != PathFindingSystem::infinite) {
                    //get how many steps it takes for this unit to reach current examined point, and update the multiplier accordingly
                    float multiplier = qPow(stepMultiplier, atkTilesMap2D[y*m_mapWidth + x]);
                    //increment the influence on this specific point, according to the multiplier
                    addValueAt(attackWeight * multiplier, x, y);
                }
            }
        }
    }
}

void InfluenceMap::addUnitValueInfluence(Unit *pUnit, QPoint startPoint, bool ignoreEnemies, float unitWeight) {
    //tmch
    pUnit->getAmmo1();

    spUnitPathFindingSystem spPfs = new UnitPathFindingSystem(pUnit, GameMap::getInstance()->getCurrentPlayer());

    spPfs->setIgnoreEnemies(ignoreEnemies);
    spPfs->setMovepoints(-2);
    spPfs->setStartPoint(startPoint.x(), startPoint.y());
    spPfs->explore();

    auto points = spPfs->getAllNodePoints();
    //for each reachable point by this unit
    for (const auto & point : points) {
        float multiplier = 1.0f;

        //if is the starting point, add 0
        if(point.x() == startPoint.x() && point.y() == startPoint.y()) {
            multiplier = 0.0f;
        }
        //if is adiacent
        else if((point.x() == startPoint.x() + 1 && point.y() == startPoint.y()) ||
                (point.x() == startPoint.x() - 1 && point.y() == startPoint.y()) ||
                (point.x() == startPoint.x() && point.y() == startPoint.y() - 1) ||
                (point.x() == startPoint.x() && point.y() == startPoint.y() + 1)) {
            if(spPfs->getTargetCosts(point.x(), point.y()) == PathFindingSystem::infinite)
                multiplier = 0.0f;
            else
                multiplier = 1.0f;
        }
        //if is another reachable point, give weight asintotically from 1 to 0 based on distance
        else {
            qint32 fieldCost = spPfs->getTargetCosts(point.x(), point.y());
            multiplier = qExp(-fieldCost);
        }

        //increment the influence on this specific point, according to the multiplier
        addValueAt(unitWeight * multiplier, point.x(), point.y());
    }
}

void InfluenceMap::addUnitDirectDmgValueInfluence(Unit* pAttackerTypeUnit, QPoint enemyTargetPoint, float dmgValue) {
    //tmch
    pAttackerTypeUnit->getAmmo1();

    spUnitPathFindingSystem spPfs = new UnitPathFindingSystem(pAttackerTypeUnit, GameMap::getInstance()->getCurrentPlayer());

    spPfs->setIgnoreEnemies(true);
    spPfs->setMovepoints(-2);
    spPfs->setStartPoint(enemyTargetPoint.x(), enemyTargetPoint.y());
    spPfs->explore();

    auto points = spPfs->getAllNodePoints();
    //for each reachable point by this unit
    for (const auto & point : points) {
        float multiplier = 1.0f;

        //if is the starting point, add 0
        if(point.x() == enemyTargetPoint.x() && point.y() == enemyTargetPoint.y()) {
            multiplier = 0.0f;
        }
        //if is adiacent
        else if((point.x() == enemyTargetPoint.x() + 1 && point.y() == enemyTargetPoint.y()) ||
                (point.x() == enemyTargetPoint.x() - 1 && point.y() == enemyTargetPoint.y()) ||
                (point.x() == enemyTargetPoint.x() && point.y() == enemyTargetPoint.y() - 1) ||
                (point.x() == enemyTargetPoint.x() && point.y() == enemyTargetPoint.y() + 1)) {
            qint32 fieldCost = spPfs->getTargetCosts(point.x(), point.y());
            if(fieldCost < 0) //not reachable
                multiplier = 0.0f;
            else
                multiplier = 1.0f;
        }
        //if is another reachable point, give weight asintotically from 1 to 0 based on distance
        else {
            qint32 fieldCost = spPfs->getTargetCosts(point.x(), point.y());
            multiplier = qPow(fieldCost, -1);
        }

        //overwrite weight only if is better than current one
        if(getInfluenceValueAt(point.x(), point.y()) < dmgValue * multiplier) {
            //increment the influence on this specific point, according to the multiplier
            setValueAt(dmgValue * multiplier, point.x(), point.y());
        }
    }
}


void InfluenceMap::addUnitIndirectDmgValueInfluence(Unit* pAttackerTypeUnit, QPoint enemyTargetPoint, float dmgValue) {
    qint32 minRange = pAttackerTypeUnit->getBaseMinRange();
    qint32 maxRange = pAttackerTypeUnit->getBaseMaxRange();
    qint32 targetX = enemyTargetPoint.x();
    qint32 targetY = enemyTargetPoint.y();

    std::vector<qint32> markedTiles2D(m_mapWidth * m_mapHeight, PathFindingSystem::infinite);

    markAttackAreaIfCanMoveOver(pAttackerTypeUnit, minRange, maxRange, targetX, targetY, markedTiles2D, 0);

    spUnitPathFindingSystem spPfs = new UnitPathFindingSystem(pAttackerTypeUnit, GameMap::getInstance()->getCurrentPlayer());
    spPfs->setMovepoints(-2);
    spPfs->setIgnoreEnemies(true);

    qint32 fieldCost;

    for(qint32 y=0; y < m_mapHeight; y++) {
        for(qint32 x=0; x < m_mapWidth; x++) {
            //if is a tile attackable in 1 turn, explore from there
            if(markedTiles2D[y*m_mapWidth + x] == 0) {
                spPfs->setStartPoint(x, y);
                spPfs->explore();

                auto points = spPfs->getAllNodePoints();
                for(const auto &point : points) {
                    //if on the marked tile there's a higher cost, overwrite it
                    fieldCost = spPfs->getTargetCosts(point.x(), point.y());
                    if(fieldCost < 0)
                        fieldCost = PathFindingSystem::infinite;
                    if(markedTiles2D[point.y()*m_mapWidth + point.x()] > fieldCost) {
                        markedTiles2D[point.y()*m_mapWidth + point.x()] = fieldCost;
                    }
                }
            }
        }
    }
    //now markedTiles2D is set. Each tile contains how does it cost to reach any tile to shoot to the desired unit
    for(qint32 y=0; y < m_mapHeight; y++) {
        for(qint32 x=0; x < m_mapWidth; x++) {
            fieldCost = markedTiles2D[y*m_mapWidth + x];
            if(fieldCost==0) {
                setValueIfGreaterAt(dmgValue, x, y);
            } else if(fieldCost != PathFindingSystem::infinite) {
                setValueIfGreaterAt(dmgValue * qPow(fieldCost+1, -1), x, y);
            }

        }
    }

}

void InfluenceMap::addUnitIndirectDmgValueInfluenceFast(Unit* pAttackerTypeUnit, QPoint enemyTargetPoint, float dmgValue, float negExp) {
    qint32 minRange = pAttackerTypeUnit->getBaseMinRange();
    qint32 maxRange = pAttackerTypeUnit->getBaseMaxRange();
    qint32 targetX = enemyTargetPoint.x();
    qint32 targetY = enemyTargetPoint.y();

    std::vector<qint32> markedTiles2D(m_mapWidth * m_mapHeight, PathFindingSystem::infinite);

    //mark all tiles at distance 0 to maxRange with 0
    markAttackAreaIfCanMoveOver(pAttackerTypeUnit, 0, maxRange, targetX, targetY, markedTiles2D, 0);

    //set now values for tiles from where the unit can attack the target
    for(qint32 y=0; y < m_mapHeight; y++) {
        for(qint32 x=0; x < m_mapWidth; x++) {
            //now set the actual value on the tiles marked with 0, only if they are at distance gte than minRange
            if(markedTiles2D[y*m_mapWidth + x] == 0) {
                if(pointDistance(x, y, targetX, targetY) >= minRange)
                    setValueIfGreaterAt(dmgValue, x, y);
            }
        }
    }

    spUnitPathFindingSystem spPfs = new UnitPathFindingSystem(pAttackerTypeUnit, GameMap::getInstance()->getCurrentPlayer());

    spPfs->setMovepoints(-2);
    spPfs->setIgnoreEnemies(true);
    spPfs->setStartPoint(targetX, targetY);
    spPfs->explore();

    qint32 fieldCost;

    auto points = spPfs->getAllNodePoints();
    for(const auto &point : points) {
        //exclude the marked tiles, which are the one too near to the unit, already marked or excluded
        if(markedTiles2D[point.y()*m_mapWidth + point.x()] != 0) {
            fieldCost = spPfs->getTargetCosts(point.x(), point.y());
            setValueIfGreaterAt(dmgValue * qPow(fieldCost, negExp), point.x(), point.y());
        }
    }
}

void InfluenceMap::addUnitIndirectDmgValueInfluenceFaster(Unit* pAttackerTypeUnit, QPoint enemyTargetPoint, float dmgValue, float negExp) {
    qint32 minRange = pAttackerTypeUnit->getBaseMinRange();
    qint32 maxRange = pAttackerTypeUnit->getBaseMaxRange();
    qint32 targetX = enemyTargetPoint.x();
    qint32 targetY = enemyTargetPoint.y();
    qint32 yDistance, xDistance, distance;

    for(qint32 y = 0; y < m_mapWidth; y++) {
        yDistance = qAbs(targetY - y);
        xDistance = qAbs(targetX);
        distance = yDistance + xDistance;
        for(qint32 x = 0; x < m_mapWidth; x++) {
            if(pAttackerTypeUnit->canMoveOver(x, y)) {
                if(distance <= maxRange) {
                    if(distance >= minRange) {
                        //if is on tiles reachable in 1 attack
                        setValueIfGreaterAt(dmgValue, x, y);
                    } //else if is on tiles too near the attacked unit, set nothing
                }
                //if are tiles farther in range than max range, decrease with x^-negExp
                else {
                    setValueIfGreaterAt(dmgValue * qPow((distance+1), negExp), x, y);
                }
            }

            if(x < targetX) {
                distance--;
            }
            else {
                distance++;
            }

        }
    }
}

void InfluenceMap::addMapDefenseInfluence(Player* pPlayer, Unit* pUnit, float weightPerStar, float friendlyBuildingMultiplier, float friendlyFactoryMultiplier) {
    if(!pUnit->useTerrainDefense())
        return;
    Building* pBuilding;
    for(qint32 y=0; y < m_mapHeight; y++) {
        for(qint32 x=0; x < m_mapWidth; x++) {
            if(!pUnit->canMoveOver(x, y))
                continue;
            pBuilding = GameMap::getInstance()->getTerrain(x, y)->getBuilding();
            if(pBuilding) {
                if(pBuilding->getOwnerID() == pPlayer->getPlayerID()) {
                    //if is a friendly production building, add its defense plus the other 2 multipliers
                    if(pBuilding->isProductionBuilding()) {
                        addValueAt(friendlyBuildingMultiplier * friendlyFactoryMultiplier * weightPerStar * GameMap::getInstance()->getTerrain(x, y)->getBaseDefense(), x, y);
                    }
                    //if it's just a normal friendly building, add the friendly multiplier but not the factoryMultiplier
                    else {
                        addValueAt(friendlyBuildingMultiplier * weightPerStar * GameMap::getInstance()->getTerrain(x, y)->getBaseDefense(), x, y);
                    }
                }
                //if is a neutral or enemy building, just add its defense
                else {
                    addValueAt(weightPerStar * GameMap::getInstance()->getTerrain(x, y)->getBaseDefense(), x, y);
                }
            } else {
                //if is a standard terrain, just add the star amount * weightPerStar
                addValueAt(weightPerStar * GameMap::getInstance()->getTerrain(x, y)->getBaseDefense(), x, y);
            }
        }
    }
}


void InfluenceMap::addGenericMapDefenseInfluence(Player* pPlayer, float weightPerStar, float friendlyBuildingMultiplier, float friendlyFactoryMultiplier) {
    Building* pBuilding;
    for(qint32 y=0; y < m_mapHeight; y++) {
        for(qint32 x=0; x < m_mapWidth; x++) {
            pBuilding = GameMap::getInstance()->getTerrain(x, y)->getBuilding();
            if(pBuilding) {
                if(pBuilding->getOwnerID() == pPlayer->getPlayerID()) {
                    //if is a friendly production building, add its defense plus the other 2 multipliers
                    if(pBuilding->isProductionBuilding()) {
                        addValueAt(friendlyBuildingMultiplier * friendlyFactoryMultiplier * weightPerStar * GameMap::getInstance()->getTerrain(x, y)->getBaseDefense(), x, y);
                    }
                    //if it's just a normal friendly building, add the friendly multiplier but not the factoryMultiplier
                    else {
                        addValueAt(friendlyBuildingMultiplier * weightPerStar * GameMap::getInstance()->getTerrain(x, y)->getBaseDefense(), x, y);
                    }
                }
                //if is a neutral or enemy building, just add its defense
                else {
                    addValueAt(weightPerStar * GameMap::getInstance()->getTerrain(x, y)->getBaseDefense(), x, y);
                }
            } else {
                //if is a standard terrain, just add the star amount * weightPerStar
                addValueAt(weightPerStar * GameMap::getInstance()->getTerrain(x, y)->getBaseDefense(), x, y);
            }
        }
    }
}


void InfluenceMap::weightedAddMap(InfluenceMap &sumMap) {
    for(quint32 i=0; i<m_influenceMap2D.size(); i++) {
        m_influenceMap2D[i] += sumMap.m_influenceMap2D[i] * sumMap.m_weight;
    }
}

void InfluenceMap::weightedAddMap(InfluenceMap &sumMap, float customWeight) {
    for(quint32 i=0; i<m_influenceMap2D.size(); i++) {
        m_influenceMap2D[i] += sumMap.m_influenceMap2D[i] * customWeight;
    }
}

float InfluenceMap::getCurrMaxAbsInfluence() {
    float maxInfluence = 0;
    for(quint32 i=0; i < m_influenceMap2D.size(); i++) {
        if(qAbs(m_influenceMap2D[i]) > maxInfluence)
            maxInfluence = qAbs(m_influenceMap2D[i]);
    }
    return maxInfluence;
}


void InfluenceMap::sortNodePointsByInfluence(QVector<QPoint> & nodePoints) {
    std::sort(nodePoints.begin(), nodePoints.end(), [this](QPoint p1, QPoint p2) {
        return getInfluenceValueAt(p1.x(), p1.y()) > getInfluenceValueAt(p2.x(), p2.y());
    });
}


bool InfluenceMap::equalsMap(InfluenceMap &otherMap, float epsilon) {
    if(m_influenceMap2D.size() != otherMap.m_influenceMap2D.size())
        return false;
    for(quint32 i=0; i<m_influenceMap2D.size(); i++) {
        if(qAbs(m_influenceMap2D[i] - otherMap.m_influenceMap2D[i]) > epsilon)
            return false;
    }
    return true;
}


void InfluenceMap::showColoredTiles() {
    //if the map is not initialized, initialize it
    if(!isInfoTilesMapInitialized)
        initializeInfoTilesMap();

    spGameMap pMap = GameMap::getInstance();
    float maxAbsInfluenceValue = getCurrMaxAbsInfluence();
    qint32 currInfluenceValue = 0;
    for (qint32 x = 0; x < m_mapWidth; ++x)
    {
        for (qint32 y = 0; y < m_mapHeight; ++y)
        {
            currInfluenceValue = getInfluenceValueAt(x, y);
            //set color and transparency according to the value on tile (positive/negative or zero)
            getInfoTileAt(x, y)->setColor(currInfluenceValue >= 0 ? M_POSITIVE_COLOR : M_NEGATIVE_COLOR);
            currInfluenceValue = qAbs(currInfluenceValue);
            getInfoTileAt(x, y)->setAlpha(M_ALPHA_HIDE + static_cast<qint32>(M_ALPHA_SHOW * currInfluenceValue/maxAbsInfluenceValue));
        }
    }
}

void InfluenceMap::showValues() {
    //if the map is not initialized, initialize it
    if(!isInfoTilesMapInitialized)
        initializeInfoTilesMap();

    spGameMap pMap = GameMap::getInstance();
    for (qint32 x = 0; x < m_mapWidth; ++x)
    {
        for (qint32 y = 0; y < m_mapHeight; ++y)
        {
            getInfoTextAt(x, y)->setAlpha(255);
            getInfoTextAt(x,y)->setHtmlText(QString::number(getInfluenceValueAt(x, y), 'f', 1));
        }
    }
}


void InfluenceMap::showAllInfo() {
    //first call show so it initializes the map if not initialized
    showColoredTiles();
    showValues();
}





void InfluenceMap::hide() {
    if(!isInfoTilesMapInitialized)
        return;

    for (qint32 y = 0; y < m_mapHeight; ++y)
    {
        for (qint32 x = 0; x < m_mapWidth; ++x)
        {
            getInfoTileAt(x, y)->setAlpha(M_ALPHA_HIDE);
            getInfoTextAt(x, y)->setAlpha(M_ALPHA_HIDE);
        }
    }
}


QString InfluenceMap::toQString(qint32 precision) {
    float minVal = 0;
    float maxVal = 0;
    for(quint32 i=0; i<m_influenceMap2D.size(); i++) {
        if(m_influenceMap2D[i] > maxVal)
            maxVal = m_influenceMap2D[i];
        else if(m_influenceMap2D[i] < minVal)
            minVal = m_influenceMap2D[i];
    }
    qint32 padSize = qMax(QString::number(minVal, 'f', precision).length(), QString::number(maxVal, 'f', precision).length());

    QString res = "";
    for(qint32 y = 0; y < m_mapHeight; y++) {
        res += "|";
        for(qint32 x = 0; x < m_mapWidth; x++) {
            QString numS = QString::number(getInfluenceValueAt(x, y), 'f', precision);
            while(numS.length() < padSize) {
                numS.prepend(" ");
            }
            res += numS + "|";
        }
        res += "\n";
    }
    return res;
}

QString InfluenceMap::toQStringFull(qint32 precision) {
    return adaenums::iMapTypeToQString(m_type) + ", [w: " + QString::number(m_weight) + "]:\n" + toQString(precision);
}

//static
InfluenceMap InfluenceMap::weightedSum(InfluenceMap &infMap1, InfluenceMap &infMap2) {
    InfluenceMap res;

    for(quint32 i=0; i < infMap1.m_influenceMap2D.size(); i++) {
        res.m_influenceMap2D[i] = infMap1.m_influenceMap2D[i]*infMap1.m_weight +
                                    infMap2.m_influenceMap2D[i]*infMap2.m_weight;
    }

    Console::print("Summing map 1 and 2:\n" + infMap1.toQString() + infMap2.toQString() +"\nResult:" + res.toQString(),
                   Console::eDEBUG);

    return res;
}


void InfluenceMap::initializeInfoTilesMap() {
    if(isInfoTilesMapInitialized)
        return;

    //create font style
    oxygine::TextStyle style = FontManager::getMainFont24();
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_MIDDLE;
    style.hAlign = oxygine::TextStyle::HALIGN_MIDDLE;

    spGameMap pMap = GameMap::getInstance();
    m_infoTilesMap.reserve(m_mapWidth * m_mapHeight);
    m_infoTextMap.reserve(m_mapWidth * m_mapHeight);

    //for each tile on map
    for (qint32 y = 0; y < m_mapHeight; ++y)
    {
        for (qint32 x = 0; x < m_mapWidth; ++x)
        {
            //add the invisible sprite to the grid of sprites and to the map
            oxygine::spColorRectSprite sprite = new oxygine::ColorRectSprite();
            sprite->setSize(GameMap::getImageSize(), GameMap::getImageSize());
            QColor color{M_POSITIVE_COLOR};
            color.setAlphaF(0.0f);
            sprite->setColor(color);
            sprite->setPosition(x * GameMap::getImageSize(), y * GameMap::getImageSize());
            sprite->setPriority(static_cast<qint32>(Mainapp::ZOrder::MarkedFields));
            pMap->addChild(sprite);
            m_infoTilesMap.push_back(sprite);

            //add a textfield on this tile
            oxygine::spTextField pTextField = new oxygine::TextField();
            pTextField->setStyle(style);
            pTextField->setHtmlText(QString::number(0));
            pTextField->setColor(QColorConstants::White);
            pTextField->setPosition(x * GameMap::getImageSize(), y * GameMap::getImageSize());
            pTextField->setAlpha(0);
            pTextField->setSize(GameMap::getImageSize(), GameMap::getImageSize());
            pTextField->setFontSize(10);
            pTextField->setPriority(static_cast<qint32>(Mainapp::ZOrder::Animation));
            pMap->addChild(pTextField);
            m_infoTextMap.push_back(pTextField);

        }
    }

    isInfoTilesMapInitialized = true;
}
