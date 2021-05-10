#include "multiinfluencenetworkmodule.h"

#include "ai/coreai.h"
#include <QSettings>
#include "game/gamemap.h"
#include "ai/utils/AiUtilsFunctions.h"
#include "coreengine/console.h"




MultiInfluenceNetworkModule::MultiInfluenceNetworkModule(Player* pPlayer, AdaptaAI* ai) : AdaptaModule(pPlayer), m_pAdapta(ai) {
}


void MultiInfluenceNetworkModule::readIni(QString filename) {
    if(QFile::exists(filename)) {
        QSettings settings(filename, QSettings::IniFormat);

        bool ok = false;

        settings.beginGroup("General");
        //get the list of units this module can use and what this module can have against (N)
        m_unitList = settings.value("UnitList").toStringList();
        if(m_unitList.size() == 0)
            defaultInitializeUnitList(m_unitList);

        m_unitAmount = m_unitList.size();//N
        m_unitOutputMaps.resize(m_unitAmount);

        m_unitCount.resize(m_unitList.size());

        m_unitListFull = settings.value("UnitListFull").toStringList();
        if(m_unitList.size() == 0)
            defaultInitializeUnitList(m_unitListFull);

        //to be safe, add if not present any unit listed in unitList into unitListFull, and
        //initialize n to M indexes map
        m_nToMIndexes.resize(m_unitAmount, -1);
        for(qint32 i=0; i < m_unitAmount; i++) {
            if(!m_unitListFull.contains(m_unitList[i]))
                m_unitListFull.append(m_unitList[i]);
            m_nToMIndexes[i] = m_unitListFull.indexOf(m_unitList[i]);
        }

        m_fullUnitAmount = m_unitListFull.size(); //M

        QStringList unitInfMaps = settings.value("UnitInfluenceMaps", "NONE").toStringList();

        m_customMapsPerUnit = 0; //K, incremented now if any
        m_stdMapsPerUnit = 0; //S, incremented now if any
        m_localMapsPerUnit = 0;

        //initialize all maps for each unit
        if(!unitInfMaps.contains("NONE") && unitInfMaps.size() > 0) {
            for(qint32 i = 0; i < unitInfMaps.size(); i++) {
                if(unitInfMaps[i].startsWith("CUSTOM"))
                    m_customMapsPerUnit++;
                else
                    m_stdMapsPerUnit++;
            } //here K and S are correct
            m_localMapsPerUnit = unitInfMaps.size(); //L = S + K

            m_unitInfluenceMaps.reserve(m_unitAmount * m_localMapsPerUnit); //N * (K + S)
            for(qint32 n=0; n < m_unitAmount; n++) {
                for(qint32 l=0; l < m_localMapsPerUnit; l++) {
                    //create an influence map of the specified type
                    m_unitInfluenceMaps.push_back(InfluenceMap(adaenums::getInfluenceMapTypeFromString(unitInfMaps.at(l))));
                }
            }
        }

        m_customWeightsPerUnitAmount = m_fullUnitAmount * m_customMapsPerUnit; //M*K

        //initializate global inf map vector
        QStringList globalInfMaps = settings.value("GlobalInfluenceMaps").toStringList();
        m_globalMapsAmount = 0; //G
        m_globalCustomMapsAmount = 0; //GK
        if(!globalInfMaps.contains("NONE") && globalInfMaps.size() > 0) {
            m_globalMapsAmount = globalInfMaps.size();
            m_globalInfluenceMaps.reserve(m_globalMapsAmount);
            for(qint32 infMapCount=0; infMapCount < globalInfMaps.size(); infMapCount++) {
                //create an influence map of the specified type
                m_globalInfluenceMaps.push_back(InfluenceMap(adaenums::getInfluenceMapTypeFromString(globalInfMaps.at(infMapCount), adaenums::iMapType::STD_VALUE)));
                //if is a custom map, increment the amount of global custom maps
                if(globalInfMaps.at(infMapCount).startsWith("CUSTOM"))
                    m_globalCustomMapsAmount++;
            }
            m_globalCustomInfluenceMapsUnitWeights2D.resize(m_globalCustomMapsAmount * m_fullUnitAmount); //GK * M
            m_allGlobalInfluenceMapWeightsForUnit2D.resize(globalInfMaps.size() * m_unitAmount); //G * N
        }

        //calculate how long the weightVector should be
        qint32 weightsPerUnit = 0;
        //S + K*(M+1) + G
        weightsPerUnit = m_stdMapsPerUnit + m_customMapsPerUnit * (m_fullUnitAmount + 1) + m_globalMapsAmount;
        //N*(S + K*(M+1) + G) + GK*M
        m_requiredVectorLength = m_unitAmount * weightsPerUnit + m_globalCustomMapsAmount*m_fullUnitAmount;

        m_weightPerStar = settings.value("WeightPerStar", 10).toDouble(&ok);
        if(!ok)
            m_weightPerStar = 10;
        m_friendlyBuildingMultiplier = settings.value("FriendlyBuildingMultiplier", 1.5f).toDouble(&ok);
        if(!ok)
            m_friendlyBuildingMultiplier = 1.5f;
        m_friendlyFactoryMultiplier = settings.value("FriendlyFactoryMultiplier", 0.1f).toDouble(&ok);
        if(!ok)
            m_friendlyFactoryMultiplier = 0.1f;

        settings.endGroup();
    }
    else {
        Console::print("MIN module couldn't load file '" + filename + "'!", Console::eWARNING);
    }
}

/**
 * @brief processStartOfTurn calculate bids for this turn and process stuff for the start of turn
 */
void MultiInfluenceNetworkModule::processStartOfTurn() {
    //if not initialized, initialize the types vectors
    initializeWithPlayerPtr();
    //do unit count at start of turn
    for(qint32 i=0; i < m_unitList.size(); i++) {
        m_unitCount[i] = m_pPlayer->getUnitCount(m_unitList[i]);
    }
    //and unit init
    spQmlVectorUnit spUnits = m_pPlayer->getUnits();
    spQmlVectorUnit spEnemies = m_pPlayer->getEnemyUnits();
    initUnitData(m_armyUnitData, spUnits.get());
    initUnitData(m_enemyUnitData, spEnemies.get());

    //compute global influence maps
    quint32 k = 0; //k counts the number of the current custom map
    for(quint32 i=0; i<m_globalInfluenceMaps.size(); i++) {
        if(adaenums::isCustomType(m_globalInfluenceMaps[i].getType())) {
            computeGlobalInfluenceMap(m_globalInfluenceMaps[i], true, k);
            k++;
        } else {
            computeGlobalInfluenceMap(m_globalInfluenceMaps[i], false);
        }
    }

    //for each unit which this player have at least count > 0, compute the standard influence maps, if any
    for(quint32 n=0; n < m_unitCount.size(); n++) {
        Console::print("Maps of unit " + m_unitList[n] + ":", Console::eDEBUG);
        //if there is at least 1 unit of a type, compute its influence maps
        if(m_unitCount[n] > 0) {
            k=0; //reset k
            m_unitOutputMaps[n].reset(); //reset output map to 0
            //compute each local map and add it to the output, weighted
            for(qint32 l=0; l < m_localMapsPerUnit; l++) {
                InfluenceMap &infMap = influenceMapOfUnit(n, l);
                if(adaenums::isCustomType(infMap.getType())) {
                    computeLocalInfluenceMap(infMap, n, true, k);
                    k++;
                } else {
                    computeLocalInfluenceMap(infMap, n, false);
                }
                m_unitOutputMaps[n].weightedAddMap(infMap);
                Console::print(infMap.toQStringFull(2), Console::eDEBUG);
            }
            //add also each global map, weighted. Here global maps are already computed
            for(quint32 g=0; g < m_globalInfluenceMaps.size(); g++) {
                m_unitOutputMaps[n].weightedAddMap(m_globalInfluenceMaps[g], glbMapWeightForUnit(n, g));
            }
            Console::print(">>Final Map:\n" + m_unitOutputMaps[n].toQString(2), Console::eDEBUG);
        } //here the output map is done, for unit n
    }
    //now all maps are computed
}

/**
 * @brief processHighestBidUnit process the action for the highest bid unit of this module
 * @param weighted
 */
bool MultiInfluenceNetworkModule::processHighestBidUnit() {
    //todo do bid system, for now is just process any usable unit
    spQmlVectorUnit pUnits = m_pPlayer->getUnits();
    for(qint32 i=0; i < pUnits->size(); i++) {
        if(!(pUnits->at(i)->getHasMoved()) && m_unitList.contains(pUnits->at(i)->getUnitID())) {
            return processUnit(pUnits->at(i));
        }
    }
    return false;
}

bool MultiInfluenceNetworkModule::processUnit(Unit* pUnit) {
    if(pUnit->getHasMoved())
        return false;
    InfluenceMap &finalMap = m_unitOutputMaps[m_unitList.indexOf(pUnit->getUnitID())];
    //if is indirect, boost the weight on its tile if there are targets
    if(pUnit->getBaseMinRange() > 1 ) {
        QVector3D bestTarget = findNearestHighestDmg(pUnit->getPosition(), pUnit);
        if(bestTarget.z() > 0) {
            finalMap.setInfluenceValueAt(finalMap.getInfluenceValueAt(pUnit->getX(), pUnit->getY()) * m_indirectsTileStillMultiplier,
                                         pUnit->getX(), pUnit->getY());
        }
    }

    Console::print("Final Map influence for unit [" + pUnit->getUnitID() + ", (" + QString::number(pUnit->getX()) + ", " +
                   QString::number(pUnit->getY()) + ")]:\n" + finalMap.toQString(), Console::eDEBUG);

    UnitPathFindingSystem* pPfs;
    for(quint32 i=0; i < m_armyUnitData.size(); i++) {
        if(m_armyUnitData[i].m_pUnit == pUnit) {
            pPfs = m_armyUnitData[i].m_pPfs.get();
            break;
        }
    }
    pPfs->setStartPoint(pUnit->getX(), pUnit->getY());
    pPfs->setMovepoints(pUnit->getMovementpoints(pUnit->getPosition()));
    pPfs->setIgnoreEnemies(false);
    pPfs->explore();

    auto points = pPfs->getAllNodePoints();
    finalMap.sortNodePointsByInfluence(points);


    //check in each point, sorted by best influence, if the unit can perform an attack action or a wait action, if it can't,
    //try next point
    for(qint32 i=0; i < points.size(); i++) {
        Console::print("Best point N°" + QString::number(i+1) + ": (x: " + QString::number(points[i].x()) + ", y: " +
                       QString::number(points[i].y()) +", val: " +
                       QString::number(finalMap.getInfluenceValueAt(points[i].x(), points[i].y())) + ")", Console::eDEBUG);
        QVector3D bestTarget = findNearestHighestDmg(points[i], pUnit);

        //if found something to attack, do action, go to point and attack
        if(bestTarget.z() > 0) {
            spGameAction pAction = new GameAction(CoreAI::ACTION_FIRE);
            pAction->setTarget(pUnit->getPosition());
            QVector<QPoint> path = pPfs->getPath(static_cast<qint32>(points[i].x()), static_cast<qint32>(points[i].y()));
            pAction->setMovepath(path, pPfs->getCosts(path));
            m_pAdapta->addSelectedFieldData(pAction, bestTarget.x(), bestTarget.y());

            if (pAction->isFinalStep() && pAction->canBePerformed())
            {
                emit m_pAdapta->performAction(pAction);
                return true;
            }
        }
        //Here an attackable unit was not found. If the next point, if any, has same influence, go on and try that point.
        else if(i< points.size() - 1 &&
                finalMap.getInfluenceValueAt(points[i].x(), points[i].y()) == finalMap.getInfluenceValueAt(points[i+1].x(), points[i+1].y())) {
            continue;
        }
        //if the best point has no associated attackable unit then just move there
        else {
            spGameAction pAction = new GameAction(CoreAI::ACTION_WAIT);
            pAction->setTarget(pUnit->getPosition());
            QVector<QPoint> path = pPfs->getPath(static_cast<qint32>(points[i].x()), static_cast<qint32>(points[i].y()));
            pAction->setMovepath(path, pPfs->getCosts(path));

            if(pAction->canBePerformed()) {
                emit m_pAdapta->performAction(pAction);
                return true;
            }
        }
    }
    return false;
}

void MultiInfluenceNetworkModule::notifyUnitUsed(Unit* pUnit) {
    //do nothing here really, just a tmch
    pUnit->getUnitID();
}


float MultiInfluenceNetworkModule::getBidFor(Unit* pUnit) {
    //todo change bid system, for now is just > 0 if the unit can be used, 0 otherwise
    if(!pUnit->getHasMoved() && m_unitList.contains(pUnit->getUnitID())) {
        return m_moduleWeight;
    }
    else {
        return 0.0f;
    }
}
/**
 * @brief getHighestBid get the value of the highest bid done by this module. By default the bid is weighted by
 * this module's weight (see [set]moduleWeight())
 */
float MultiInfluenceNetworkModule::getHighestBid(bool weighted/*=true*/) {

    //todo change this when there will be actual bids

    float weight = weighted ? m_moduleWeight : 1.0f;
    //for now return a bid > 0 if there are available units that this module can move
    bool availableUnits = false;
    spQmlVectorUnit pUnits = m_pPlayer->getUnits();
    for(qint32 i=0; i < pUnits->size(); i++) {
        if(!(pUnits->at(i)->getHasMoved()) && m_unitList.contains(pUnits->at(i)->getUnitID())) {
            availableUnits = true;
            break;
        }
    }
    if(availableUnits) {
        return weight;
    } else {
        return 0.0f;
    }
}

Unit* MultiInfluenceNetworkModule::getHighestBidUnit() {
    //todo change bid system
    spQmlVectorUnit pUnits = m_pPlayer->getUnits();
    for(qint32 i=0; i<pUnits->size(); i++) {
        if(!pUnits->at(i)->getHasMoved() && m_unitList.contains(pUnits->at(i)->getUnitID())) {
            return pUnits->at(i);
        }
    }
    return nullptr;
}


bool MultiInfluenceNetworkModule::assignWeightVector(WeightVector assignedWV) {
    //if this is not true then this module is not even initialized
    if(m_requiredVectorLength==0 || assignedWV.size() != m_requiredVectorLength)
        return false;

    //copy the wv
    m_weightVector = assignedWV;

    qint32 N = m_unitAmount; //N = how many unit this module can use
    qint32 M = m_fullUnitAmount; //M = how many unit are recognized by this module
    qint32 G = m_globalMapsAmount; //G = how many global inf maps this module has
    qint32 GK = m_globalCustomMapsAmount; //GK = global custom maps amount
    qint32 K = m_customMapsPerUnit; //K = local custom maps amount
    qint32 S = m_stdMapsPerUnit; //S = local standard maps amount
    qint32 L = m_localMapsPerUnit; //L = S + K, all local maps amount

    qint32 globalMapsUnitWeightsOffset = N*K*M;
    qint32 mapWeightsOffset = globalMapsUnitWeightsOffset + M*GK;
    qint32 mapWeightsPerUnit = S + K + G;

    //copy some parts of the wv in other vectors for faster access

    //weights for each global custom map gk, of each unit m
    for(qint32 gk = 0; gk < GK; gk++) {
        for(qint32 m = 0; m < M; m++) {
            m_globalCustomInfluenceMapsUnitWeights2D [gk*M + m] = m_weightVector[globalMapsUnitWeightsOffset + gk*M + m];
        }
    }

    //assign weights of all global maps for each unit n
    for(qint32 n = 0; n < N; n++) {
        for(qint32 g=0; g < G; g++) {
            m_allGlobalInfluenceMapWeightsForUnit2D[n*G + g] =
                    m_weightVector[mapWeightsOffset + (n * mapWeightsPerUnit) + S + K + g];
        }
    }

    //assign weights to each custom and standard local map
    for(qint32 n = 0; n < N; n++) {
        for(qint32 s=0, k=0; s + k < L;) {
            //if is custom
            if(adaenums::isCustomType(influenceMapOfUnit(n, s+k).getType())) {
                influenceMapOfUnit(n, s+k).setWeight(m_weightVector[mapWeightsOffset + (n*mapWeightsPerUnit) + S + k]);
                k++;
            }
            //if is standard
            else {
                influenceMapOfUnit(n, s+k).setWeight(m_weightVector[mapWeightsOffset + (n*mapWeightsPerUnit) + s]);
                s++;
            }
        }
    }
    return true;
}

qint32 MultiInfluenceNetworkModule::getRequiredWeightVectorLength() {
    return m_requiredVectorLength;
}


QString MultiInfluenceNetworkModule::toQString() {
    QString res = "";
    //units in unit list (N)
    res+= "Unit List: \n";
    for(qint32 n=0; n< m_unitList.size(); n++) {
        res += m_unitList[n] + (n == m_unitList.size() - 1 ? "\nUnit List Full:\n" : ", ");
    }
    //units in unit list full (M)
    for(qint32 m=0; m < m_unitListFull.size(); m++) {
        res += m_unitListFull[m] + (m == m_unitListFull.size() - 1 ? "\nLocal Maps, for each unit:\n" : ", ");
    }
    //local maps of each unitand their single mapweights (L)
    for(qint32 n=0; n < m_unitAmount; n++) {
        res += "> Unit " + m_unitList.at(n) + ":\n";
        for(qint32 l=0; l< m_localMapsPerUnit; l++) {
            res += adaenums::iMapTypeToQString(influenceMapOfUnit(n, l).getType()) + " [" + QString::number(influenceMapOfUnit(n, l).getWeight(), 'f', 3) +
                    (l==m_localMapsPerUnit-1 ? "]\n" : "], ");
        }
    }

    res += "Global Maps with weight for each unit:\n";
    //global maps (G)
    if(m_globalInfluenceMaps.size() > 0) {
        for(quint32 g = 0; g < m_globalInfluenceMaps.size(); g++) {
            res += adaenums::iMapTypeToQString(m_globalInfluenceMaps[g].getType()) + "[";
            //print each weight of unit n/N for global map g/G
            for(qint32 n=0; n < m_unitAmount; n++) {
                res += m_unitList.at(n) + ": " + QString::number(glbMapWeightForUnit(n, g), 'f', 3) +
                        (n==m_unitAmount-1 ? "]\n" : ", ");
            }
        }
    } else {
        res += "None\n";
    }
    //local custom unit weights
    if(m_customMapsPerUnit > 0) {
        //local custom weights
        for(qint32 n=0; n < m_unitAmount; n++) {
            res += "Unit " + m_unitList[n] + ":\n";
            for(qint32 k=0, l=0; l < m_localMapsPerUnit; l++) {
                //if for unit n, map l is custom, add all weights
                if(adaenums::isCustomType(influenceMapOfUnit(n, l).getType())) {
                    res += ">Custom map " + QString::number(k) + ":\n[";
                    for(qint32 m=0; m<m_fullUnitAmount; m++) {
                        res+= m_unitListFull[m] + ": " + QString::number(customInfluenceMapUnitWeight(n, k, m), 'f', 3) +
                                (m==m_fullUnitAmount-1 ? "]\n" : ", ");
                    }
                    k++;
                }
            }
        }
    } else {
        res += "No custom maps for units.\n";
    }
    //global custom units weights
    if(m_globalCustomMapsAmount > 0) {
        for(qint32 g=0, gk=0; g < m_globalMapsAmount; g++) {
            if(adaenums::isCustomType(m_globalInfluenceMaps[g].getType())) {
                res += "Weights for global custom map " + QString::number(gk) + ":\n[";
                for(qint32 m=0; m < m_fullUnitAmount; m++) {
                    res += m_unitListFull[m] + ": " + QString::number(glbCustomMapUnitWeight(gk, m), 'f', 3) +
                            (m==m_fullUnitAmount-1 ? "]\n" : ", ");
                }
                gk++;
            }
        }
    } else {
        res += "No global custom maps.";
    }
    return res;
}

AdaptaAI *MultiInfluenceNetworkModule::getPAdapta() const
{
    return m_pAdapta;
}

void MultiInfluenceNetworkModule::setPAdapta(AdaptaAI *pAdapta)
{
    m_pAdapta = pAdapta;
}

//private methods///////////////////////////////////////////////////////////////////////////////

void MultiInfluenceNetworkModule::defaultInitializeUnitList(QStringList &unitList) {
    unitList.clear();
    //by default append all units which can be built by all 3 construction buildings, without duplicates
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString function1 = "getConstructionList";
    QJSValueList args1;
    QJSValue obj1 = pInterpreter->newQObject(this);
    args1 << obj1;
    QJSValue ret = pInterpreter->doFunction("FACTORY", function1, args1);
    QStringList buildList = ret.toVariant().toStringList();
    unitList.append(buildList);

    ret = pInterpreter->doFunction("HARBOUR", function1, args1);
    buildList = ret.toVariant().toStringList();
    for(qint32 i=0; i<buildList.size(); i++) {
        if(!unitList.contains(buildList[i])) {
            unitList.append(buildList[i]);
        }
    }

    ret = pInterpreter->doFunction("AIRPORT", function1, args1);
    buildList = ret.toVariant().toStringList();
    for(qint32 i=0; i<buildList.size(); i++) {
        if(!unitList.contains(buildList[i])) {
            unitList.append(buildList[i]);
        }
    }
}


void MultiInfluenceNetworkModule::initializeWithPlayerPtr() {
    if(m_arePlayerPtrStuffInitialized || !(m_pAdapta->getPlayer()))
        return;
    m_pPlayer = m_pAdapta->getPlayer();
    //initialize types vectors
    m_unitTypesVector.reserve(m_unitAmount);
    for(qint32 i=0; i < m_unitAmount; i++) {
        m_unitTypesVector.push_back(new Unit(m_unitList[i], m_pPlayer, false));
        UnitData data;
        data.m_pUnit = m_unitTypesVector[i].get();
        data.m_pPfs = new UnitPathFindingSystem(data.m_pUnit);
    }

    //initialize damage chart
    m_damageChart.initialize(m_unitListFull);
    Console::print(m_damageChart.toQString(), Console::eDEBUG);

    m_arePlayerPtrStuffInitialized = true;
}


void MultiInfluenceNetworkModule::initUnitData(std::vector<UnitData> &unitDataVector, QmlVectorUnit* pUnits) {
    unitDataVector.clear();
    for(qint32 i=0; i < pUnits->size(); i++) {
        UnitData data;
        data.m_pUnit = pUnits->at(i);
        data.m_pPfs = new UnitPathFindingSystem(data.m_pUnit);
        unitDataVector.push_back(data);
    }
}

void MultiInfluenceNetworkModule::computeGlobalInfluenceMap(InfluenceMap &influenceMap, bool isCustom, quint32 customNum) {
    influenceMap.reset();
    if(isCustom) {
        //todo
    }
    //if it's standard
    else {
    }
}

void MultiInfluenceNetworkModule::computeLocalInfluenceMap(InfluenceMap &influenceMap, quint32 unitNum,
                                                           bool isCustom, quint32 customNum) {
    influenceMap.reset();
    if(isCustom) {
        spQmlVectorUnit pEnemies = m_pPlayer->getEnemyUnits();
        spQmlVectorUnit pUnits = m_pPlayer->getUnits();

        switch(influenceMap.getType()) {
        //custom 1 adds influence for each unit in game, positive for allies and negative for enemies.
        //the influence is propagated with the attack area
        case adaenums::iMapType::CUSTOM_1:
            for(qint32 i=0; i < pUnits->size(); i++) {
                Unit* pUnit = pUnits->at(i);
                influenceMap.addUnitAtkInfluence(pUnit, customInfluenceMapUnitWeight(unitNum, customNum, m_unitListFull.indexOf(pUnit->getUnitID())), 0.5f, 3, 2);
            }
            for(qint32 i=0; i < pEnemies->size(); i++) {
                Unit* pEnemy = pEnemies->at(i);
                influenceMap.addUnitAtkInfluence(pEnemy, -customInfluenceMapUnitWeight(unitNum, customNum, m_unitListFull.indexOf(pEnemy->getUnitID())), 0.5f, 3, 2);
            }
            break;
            //custom allies and custom enemies together are more powerful than a custom 1 (however they are also more complex and slower to converge)
        case adaenums::iMapType::CUSTOM_ALLIES:
            for(qint32 i=0; i < pUnits->size(); i++) {
                Unit* pUnit = pUnits->at(i);
                influenceMap.addUnitAtkInfluence(pUnit, customInfluenceMapUnitWeight(unitNum, customNum, m_unitListFull.indexOf(pUnit->getUnitID())), 0.5f, 3, 2);
            }
            break;
        case adaenums::iMapType::CUSTOM_ENEMIES:
            for(qint32 i=0; i < pEnemies->size(); i++) {
                Unit* pEnemy = pEnemies->at(i);
                influenceMap.addUnitAtkInfluence(pEnemy, -customInfluenceMapUnitWeight(unitNum, customNum, m_unitListFull.indexOf(pEnemy->getUnitID())), 0.5f, 3, 2);
            }
            break;
        default:
            break;
        }
    } else {
        spQmlVectorUnit pEnemies = m_pPlayer->getEnemyUnits();
        spQmlVectorUnit pUnits = m_pPlayer->getUnits();
        switch(influenceMap.getType()) {
        //std attack adds for each enemy unit its influence (negative, since it represent in general a bad tile) of attack against the type of unit determined by unitNum
        case adaenums::iMapType::STD_ATTACK:
            for(qint32 i=0; i < pEnemies->size(); i++) {
                Unit* pEnemy = pEnemies->at(i);
                influenceMap.addUnitAtkInfluence(pEnemy, -m_damageChart.getBaseDmgWithAmmo(pEnemy, m_unitListFull.indexOf(pEnemy->getUnitID()), m_nToMIndexes[unitNum]), .5f, 3, 2);
            }
            break;
        //std damage creates a map where the type of unit given does the most damage
        case adaenums::iMapType::STD_DAMAGE: {
            Unit* pUnit = m_unitTypesVector[unitNum].get();
            //direct
            if(pUnit->getBaseMinRange() > 1) {
                for(qint32 i=0; i<pEnemies->size(); i++) {
                    influenceMap.addUnitDirectDmgValueInfluence(pUnit, pEnemies->at(i)->getPosition(), m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], m_unitListFull.indexOf(pEnemies->at(i)->getUnitID())));
                }
            }
            //indirect
            else {
                for(qint32 i=0; i<pEnemies->size(); i++) {
                    influenceMap.addUnitIndirectDmgValueInfluence(pUnit, pEnemies->at(i)->getPosition(), m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], m_unitListFull.indexOf(pEnemies->at(i)->getUnitID())));
                }
            }
        }
            break;
        //mapdefense adds on each tile the defense gained by this unit's map (if the unit can go over that tile)
        case adaenums::iMapType::STD_MAPDEFENSE: {
            spUnit unit = new Unit(m_unitList[unitNum], m_pPlayer, false);
            influenceMap.addMapDefenseInfluence(m_pPlayer, unit.get(), m_weightPerStar, m_friendlyBuildingMultiplier, m_friendlyFactoryMultiplier);
        }
            break;
        default:
            break;
        }
    }
}


QVector3D MultiInfluenceNetworkModule::findNearestHighestDmg(QPoint fromWherePoint, Unit* pUnit) {
    GameMap* pMap = GameMap::getInstance();
    //x, y, z. x, y = pos of enemy, z = funds damage
    QVector3D currBest (-1,-1,-1);
    //works both for indirect and direct
    qint32 minRange = pUnit->getBaseMinRange();
    qint32 maxRange = pUnit->getBaseMaxRange();
    qint32 minX = qMax(fromWherePoint.x() - maxRange, 0);
    qint32 maxX = qMin(fromWherePoint.x() + maxRange, pMap->getMapWidth() - 1);
    qint32 minY = qMax(fromWherePoint.y() - maxRange, 0);
    qint32 maxY = qMin(fromWherePoint.y() + maxRange, pMap->getMapHeight() - 1);

    //search efficiently for all tiles this indirect unit can attack in range
    for(qint32 x = minX; x <= maxX; x++) {
        for(qint32 y = minY; y <= maxY; y++) {
            qint32 distance = aiutils::pointDistance(x, y, fromWherePoint.x(), fromWherePoint.y());
            //for each tile this unit can attack
            if(distance <= maxRange && distance >= minRange) {
                Unit* pEnemy = pMap->getTerrain(x, y)->getUnit();
                //if there's a unit, calculate funds damage, and if it's better write new current best
                if(pEnemy != nullptr && pEnemy->getOwner() != pUnit->getOwner()) {
                    std::pair<float, float> fundsDmg = m_damageChart.getFundsDmgBidirectional(pUnit, pEnemy, m_unitListFull.indexOf(pUnit->getUnitID()), m_unitListFull.indexOf(pEnemy->getUnitID()));
                    //if the damage in funds is at least positive wrt the counterdamage received, and it's the best so far, write current best
                    if(fundsDmg.first > fundsDmg.second && fundsDmg.first > currBest.z()) {
                        currBest.setX(x);
                        currBest.setY(y);
                        currBest.setZ(fundsDmg.first);
                    }
                }
            }
        }
    }

    return currBest;
}

