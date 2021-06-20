#include "multiinfluencenetworkmodule.h"

#include "ai/coreai.h"
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "game/gamemap.h"
#include "ai/utils/aiutils.h"
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
        m_isMapNComputed.resize(m_unitAmount, false);

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
        //now initialize mapping for unitID -> N and M
        for(qint32 i=0; i < m_unitAmount; i++) {
            m_unitIDToN.insert({m_unitList[i], i});
        }
        for(qint32 i=0; i < m_fullUnitAmount; i++) {
            m_unitIDToM.insert({m_unitListFull[i], i});
        }

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

        //load a vector if required
        bool loadWeightVector = settings.value("LoadWeightVector", false).toBool();
        QString weightVectorFile = settings.value("WeightVectorFile", "").toString();

        m_fastMode = settings.value("FastMode", true).toBool();

        settings.endGroup();

        //calculate how long the weightVector should be
        qint32 weightsPerUnit = 0;
        //S + K*(M+1) + G
        weightsPerUnit = m_stdMapsPerUnit + m_customMapsPerUnit * (m_fullUnitAmount + 1) + m_globalMapsAmount;
        //N*(S + K*(M+1) + G) + GK*M
        m_requiredVectorLength = m_unitAmount * weightsPerUnit + m_globalCustomMapsAmount*m_fullUnitAmount;

        if(loadWeightVector) {
            loadVectorFromFile(weightVectorFile);
        }

        settings.beginGroup("FunctionParameters");
        m_stepMultiplier = settings.value("StepMultiplier", .5f).toDouble(&ok);
        if(!ok)
            m_stepMultiplier = .5f;
        m_stepsForDirectUnits = settings.value("StepsForDirectUnits", 2).toInt(&ok);
        if(!ok)
            m_stepsForDirectUnits = 2;
        m_stepsForIndirectUnits = settings.value("StepsForIndirectUnits", 1).toInt(&ok);
        if(!ok)
            m_stepsForIndirectUnits = 1;
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

bool MultiInfluenceNetworkModule::loadVectorFromFile(QString file) {
    QFile loadFile(file);
    if (!loadFile.open(QIODevice::ReadOnly)) {
           Console::print("Couldn't open save file of weight vector for MIN '" + file + "'!", Console::eWARNING);
           return false;
    }
    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    if(loadDoc.isNull()) {
        Console::print("WV file '" + file +  "' for MIN was not loaded correctly!", Console::eWARNING);
        return false;
    }

    QJsonObject wvObject;

    if(loadDoc.isArray()) {
        wvObject = loadDoc.array()[0].toObject();
    } else if(loadDoc.isObject()) {
        wvObject = loadDoc.object();
    } else {
        Console::print("WV file '" + file +  "' for MIN was not loaded correctly!", Console::eWARNING);
        return false;
    }
    WeightVector wv = WeightVector::generateFromJson(wvObject);
    if(wv.size() != m_requiredVectorLength) {
        Console::print("Loaded vector from file '" + file +  "' for MIN has wrong size! Required: " +
                       QString::number(m_requiredVectorLength) + ", got: " +QString::number(wv.size()), Console::eWARNING);
        return false;
    }
    assignWeightVector(wv);
    Console::print("MIN module loaded vector correctly. Weight Vector:\n" + m_weightVector.toQStringFull(), Console::eDEBUG);
    return true;
}

/**
 * @brief processStartOfTurn calculate bids for this turn and process stuff for the start of turn
 */
void MultiInfluenceNetworkModule::processStartOfTurn() {
    Console::print("MIN module processStartOfTurn()", Console::eDEBUG);
    //if not initialized, initialize the types vectors
    initializeWithPlayerPtr();
    //do unit count at start of turn and mark all maps as not computed
    for(qint32 i=0; i < m_unitAmount; i++) {
        m_unitCount[i] = m_pPlayer->getUnitCount(m_unitList[i]);
        m_isMapNComputed[i] = false;
    }
    //and unitData init
    spQmlVectorUnit spEnemies = m_pPlayer->getEnemyUnits();
    spQmlVectorUnit spUnits = m_pPlayer->getUnits();
    initUnitData(m_armyUnitData, spUnits.get(), false);
    initUnitData(m_enemyUnitData, spEnemies.get(), true);
    m_armyUnitDataKillCount = 0;
    m_enemyUnitDataKillCount = 0;

    //compute global influence maps
    quint32 gk = 0; //gk counts the number of the current global custom map
    for(quint32 i=0; i<m_globalInfluenceMaps.size(); i++) {
        if(adaenums::isCustomType(m_globalInfluenceMaps[i].getType())) {
            computeGlobalInfluenceMap(m_globalInfluenceMaps[i], true, gk);
            gk++;
        } else {
            computeGlobalInfluenceMap(m_globalInfluenceMaps[i], false);
        }
    }

    //if fast mode is on, skip the computation of all local maps here and don't generate bids
    //else compute all maps here and generate bids
    if(!m_fastMode) {
        //for each unit which this player have at least count > 0, compute the standard influence maps, if any
        for(quint32 n=0; n < m_unitCount.size(); n++) {
            Console::print("Maps of unit " + m_unitList[n] + ":", Console::eDEBUG);
            //if there is at least 1 unit of a type, compute its influence maps
            if(m_unitCount[n] > 0) {
                computeOutputMapForUnit(n, true, spUnits, spEnemies);
                Console::print(">>Final Map:\n" + m_unitOutputMaps[n].toQString(2), Console::eDEBUG);
            } //here the output map is done, for unit n
        }
        //now all maps are computed
        generateBids(spUnits, true);
    }
}

/**
 * @brief processHighestBidUnit process the action for the highest bid unit of this module
 * @param weighted
 */
bool MultiInfluenceNetworkModule::processHighestBidUnit() {
    //todo do bid system, for now is just process any usable unit
    spQmlVectorUnit pUnits = m_pPlayer->getUnits();
    for(qint32 i=0; i < pUnits->size(); i++) {
        if(!(pUnits->at(i)->getHasMoved()) && m_unitIDToN.find(pUnits->at(i)->getUnitID()) != m_unitIDToN.end()) {
            return processUnit(pUnits->at(i));
        }
    }
    return false;
}

bool MultiInfluenceNetworkModule::processUnit(Unit* pUnit) {
    Console::print("MIN, Processing unit [" + pUnit->getUnitID() + ", (" + QString::number(pUnit->getX()) + ", " +
                   QString::number(pUnit->getY())+ ")]", Console::eDEBUG);
    if(pUnit->getHasMoved())
        return false;

    spQmlVectorUnit spEnemies = m_pPlayer->getEnemyUnits();
    spQmlVectorUnit spUnits = m_pPlayer->getUnits();

    updateAllUnitsDataReferences(spUnits, spEnemies);

    qint32 n = m_unitIDToN.find(pUnit->getUnitID())->second;
    //if map of unit of type N has already been computed, update the map wrt changes happened since it was computed
    if(m_isMapNComputed[n]) {
        updateMapsChangesForUnit(n);
        computeOutputMapForUnit(n, false);
    }
    //else just compute the output map of processed unit for first time this turn
    else {
        computeOutputMapForUnit(n, true, spUnits, spEnemies);
    }
    InfluenceMap &finalMap = m_unitOutputMaps[n];
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

    UnitPathFindingSystem* pPfs = nullptr;
    pPfs = m_armyUnitData.find(pUnit->getUniqueID())->second.pPfs.get();
    pPfs->setStartPoint(pUnit->getX(), pUnit->getY());
    pPfs->setMovepoints(pUnit->getMovementpoints(pUnit->getPosition()));
    pPfs->setIgnoreEnemies(false);
    pPfs->explore();

    auto points = pPfs->getAllNodePoints();
    finalMap.sortNodePointsByInfluence(points);


    //check in each point, sorted by best influence, if the unit can perform an attack action or a wait action, if it can't,
    //try next point
    for(qint32 i=0; i < points.size(); i++) {
        //Console::print("Best point N°" + QString::number(i+1) + ": (x: " + QString::number(points[i].x()) + ", y: " + QString::number(points[i].y()) +", val: " + QString::number(finalMap.getInfluenceValueAt(points[i].x(), points[i].y())) + ")", Console::eDEBUG);
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
                //Console::print("Unit " + pUnit->getUnitID() + " (" + QString::number(pUnit->getX()) + ", " + QString::number(pUnit->getY()) + ") attacked at (" + QString::number(bestTarget.x()) + ", " + QString::number(bestTarget.y()) + ")", Console::eDEBUG);
                return true;
            }
        }
        //Here an attackable unit was not found. If the next point, if any, has same influence, go on and try that point.
        else if(i < points.size() - 1 &&
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
            } else {
                Console::print("Action wait couldn't have been performed!", Console::eDEBUG);
            }
        }
    }
    //before not doing anything, try to wait on the spot
    spGameAction pAction = new GameAction(CoreAI::ACTION_WAIT);
    pAction->setTarget(pUnit->getPosition());
    QVector<QPoint> path = pPfs->getPath(static_cast<qint32>(pUnit->getX()), static_cast<qint32>(pUnit->getY()));
    pAction->setMovepath(path, pPfs->getCosts(path));

    if(pAction->canBePerformed()) {
        emit m_pAdapta->performAction(pAction);
        return true;
    }

    return false;
}

void MultiInfluenceNetworkModule::notifyUnitUsed(Unit* pUnit) {
    //do nothing here really, just a tmch
    pUnit->getUnitID();
}


float MultiInfluenceNetworkModule::getBidFor(Unit* pUnit) {
    //todo change bid system, for now is just > 0 if the unit can be used, 0 otherwise
    if(!pUnit->getHasMoved() && m_unitIDToN.find(pUnit->getUnitID()) != m_unitIDToN.end()) {
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
        if(!(pUnits->at(i)->getHasMoved()) && m_unitIDToN.find(pUnits->at(i)->getUnitID()) != m_unitIDToN.end()) {
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
        if(!pUnits->at(i)->getHasMoved() && m_unitIDToN.find(pUnits->at(i)->getUnitID()) != m_unitIDToN.end()) {
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


std::pair<std::vector<bool>, std::vector<float>> MultiInfluenceNetworkModule::generateTransferLearningMask(MultiInfluenceNetworkModule &originMIN, MultiInfluenceNetworkModule &targetMIN) {
    std::vector<bool> resUseWeightMask;
    std::vector<float> resWeightValueMask;

    resUseWeightMask.resize(targetMIN.m_requiredVectorLength, false);
    resWeightValueMask.resize(targetMIN.m_requiredVectorLength);

    //for each unit n in the origin MIN
    for(qint32 n = 0; n < originMIN.m_unitAmount; n++) {

        qint32 nTarget = targetMIN.m_unitList.indexOf(originMIN.m_unitList.at(n));
        //if the unit is not used by target MIN, ignore it
        if(nTarget < 0)
            continue;
        std::map<adaenums::iMapType, qint32> mapTypeCount;

        //for each local map of that unit
        for(qint32 s=0, k=0; s + k < originMIN.m_localMapsPerUnit;) {
            float mapWeight = originMIN.influenceMapOfUnit(n, s + k).getWeight();
            adaenums::iMapType mapType = originMIN.influenceMapOfUnit(n, s + k).getType();

            //increment the map type count by 1
            if(mapTypeCount.find(mapType) != mapTypeCount.end())
                mapTypeCount.at(mapType)++;
            else
                mapTypeCount.insert({mapType, 1});

            //if is custom transfer the weight
            if(adaenums::isCustomType(mapType)) {
                //find the index of the custom map's weight in the weight vector
                qint32 kTarget = targetMIN.indexOfCustomMapWeightOfTypeAndNumberOfUnit(nTarget, mapType, mapTypeCount.at(mapType));
                //if the target MIN hasn't that map, just skip this map
                if(kTarget == -1){
                    k++;
                    continue;
                }
                qint32 wvIndex = targetMIN.wvIndexOfCustomMapWeightOfUnit(nTarget, kTarget);

                resUseWeightMask[wvIndex] = true;
                resWeightValueMask[wvIndex] = mapWeight; //adding n0_k0 ... n0_kK
                //now find all weights for unit n for custom map k of units m
                for(qint32 m=0; m < originMIN.m_fullUnitAmount; m++) {
                    qint32 mTarget = targetMIN.m_unitListFull.indexOf(originMIN.m_unitListFull.at(m));
                    if(mTarget < 0) {
                        continue;
                    }
                    wvIndex = targetMIN.wvIndexCustomInfluenceMapUnitWeight(nTarget, kTarget, mTarget);
                    resUseWeightMask[wvIndex] = true;
                    resWeightValueMask[wvIndex] = originMIN.customInfluenceMapUnitWeight(n, k, m); //adding n0_k0_m0, etc
                }
                k++;
            }
            //if is not custom transfer only the weight of the map
            else {
                qint32 sTarget = targetMIN.indexOfStdMapWeightOfTypeAndNumberOfUnit(nTarget, mapType, mapTypeCount.at(mapType));
                //if the target MIN hasn't that map, just skip this map
                if(sTarget == -1) {
                    s++;
                    continue;
                }
                qint32 wvIndex = targetMIN.wvIndexOfStdMapWeightOfUnit(nTarget, sTarget);
                resUseWeightMask[wvIndex] = true;
                resWeightValueMask[wvIndex] = mapWeight; //adding n0_s0 ... n0_sS etc
                s++;
            }
        }
    }

    std::map<adaenums::iMapType, qint32> globalMapTypeCount;
    //for each global map in origin
    for(qint32 g = 0, gk = 0; g < originMIN.m_globalMapsAmount; g++) {
        adaenums::iMapType mapType = originMIN.m_globalInfluenceMaps.at(g).getType();
        //increment the map type count by 1
        if(globalMapTypeCount.find(mapType) != globalMapTypeCount.end())
            globalMapTypeCount.at(mapType)++;
        else
            globalMapTypeCount.insert({mapType, 1});

        qint32 gTarget = targetMIN.indexOfGlobalMapWeightOfTypeAndNumber(mapType, globalMapTypeCount.at(mapType));
        if(gTarget == -1) {
            if(adaenums::isCustomType(mapType))
                gk++;
            continue;
        }


        //for each unit n, add the weight of map g for unit n
        for(qint32 n=0; n < originMIN.m_unitAmount; n++) {
            qint32 nTarget = targetMIN.m_unitList.indexOf(originMIN.m_unitList.at(n));
            if(nTarget < 0)
                continue;

            float mapWeightForUnitN = originMIN.glbMapWeightForUnit(n, g);

            qint32 wvIndex = targetMIN.wvIndexOfGlobalMapWeightForUnit(nTarget, gTarget);
            resUseWeightMask[wvIndex] = true;
            resWeightValueMask[wvIndex] = mapWeightForUnitN; //adding n0_g0 ... n0_gG, n1_g0... etc
        }
        //now if the global map is custom, add all its weights for unit m
        if(adaenums::isCustomType(mapType)) {
            qint32 gkTarget = targetMIN.indexOfGlobalCustomMapWeightOfTypeAndNumber(mapType, globalMapTypeCount.at(mapType));
            if(gkTarget == -1) {
                gk++;
                continue;
            }

            //for each unit m, add weight gk0_m0, etc.
            for(qint32 m=0; m < originMIN.m_fullUnitAmount; m++) {
                qint32 mTarget = targetMIN.m_unitListFull.indexOf(originMIN.m_unitListFull.at(m));
                if(mTarget < 0) {
                    continue;
                }

                qint32 wvIndex = targetMIN.wvIndexOfGlobalCustomInfluenceMapUnitWeight(gkTarget, mTarget);

                resUseWeightMask[wvIndex] = true;
                resWeightValueMask[wvIndex] = originMIN.glbCustomMapUnitWeight(gk, m); //assigning gk0_m0, gk0_mM, etc
            }
            gk++;
        }
    }

    std::pair<std::vector<bool>, std::vector<float>> res(resUseWeightMask, resWeightValueMask);
    return res;
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
    }
    m_armyUnitTypesVector.reserve(m_fullUnitAmount);
    for(qint32 i=0; i < m_fullUnitAmount; i++) {
        m_armyUnitTypesVector.push_back(new Unit(m_unitListFull[i], m_pPlayer, false));
    }
    m_enemyUnitTypesVector.reserve(m_fullUnitAmount);
    Player* pOpponent = nullptr;
    for(qint32 i=0; i < GameMap::getInstance()->getPlayerCount(); i++) {
        if(GameMap::getInstance()->getPlayer(i)->getTeam() != m_pPlayer->getTeam()) {
            pOpponent = GameMap::getInstance()->getPlayer(i);
            break;
        }
    }
    for(qint32 i=0; i < m_fullUnitAmount; i++) {
        m_enemyUnitTypesVector.push_back(new Unit(m_unitListFull[i], pOpponent, false));
    }

    //initialize damage chart
    m_damageChart.initialize(m_unitListFull);
    Console::print(m_damageChart.toQString(), Console::eDEBUG);

    m_arePlayerPtrStuffInitialized = true;
}


void MultiInfluenceNetworkModule::generateBids(spQmlVectorUnit spUnits, bool useMapInfo) {
    m_unitsBids.clear();
    m_unitsBids.reserve(spUnits->size());
    for(qint32 i=0; i<spUnits->size(); i++) {
        m_unitsBids.append(QPair(spUnits->at(i), 1.0f));
    }
    //todo calculate a reasonable bid if not in fast mode
    if(useMapInfo) {}
}


void MultiInfluenceNetworkModule::initUnitData(std::map<qint32, UnitData> &unitDataSet, QmlVectorUnit* pUnits, bool isEnemy) {
    unitDataSet.clear();
    for(qint32 i=0; i < pUnits->size(); i++) {
        UnitData data;
        data.pUnit = pUnits->at(i);
        data.unitID = data.pUnit->getUnitID();
        data.pPfs = new UnitPathFindingSystem(data.pUnit);

        UnitStatus status;
        status.hp = data.pUnit->getHp();
        status.posX = data.pUnit->getX();
        status.posY = data.pUnit->getY();

        data.statusForUnit.resize(m_unitAmount, status);
        data.isEnemy = isEnemy;
        data.isKilled = false;
        unitDataSet.insert({data.pUnit->getUniqueID(), data});
    }
}

void MultiInfluenceNetworkModule::addUnitData(std::map<qint32, UnitData> &unitDataSet, Unit *pUnit, bool isEnemy) {
    UnitData data;
    data.pUnit = pUnit;
    data.unitID = pUnit->getUnitID();
    data.pPfs = new UnitPathFindingSystem(data.pUnit);

    UnitStatus status;
    status.hp = data.pUnit->getHp();
    status.posX = data.pUnit->getX();
    status.posY = data.pUnit->getY();

    data.statusForUnit.resize(m_unitAmount, status);
    data.isEnemy = isEnemy;
    data.isKilled = false;
    unitDataSet.insert({data.pUnit->getUniqueID(), data});
}

void MultiInfluenceNetworkModule::updateAllUnitsDataReferences(spQmlVectorUnit spUnits, spQmlVectorUnit spEnemies) {
    std::set<qint32> armyValidIDs;
    std::set<qint32> enemyValidIDs;
    //add new unit data if any
    for(qint32 i=0; i < spUnits->size(); i++) {
        auto it = m_armyUnitData.find(spUnits->at(i)->getUniqueID());
        //if didn't find anything, add the new UnitData
        if(it == m_armyUnitData.end())  {
            addUnitData(m_armyUnitData, spUnits->at(i), false);
        }
        armyValidIDs.insert(spUnits->at(i)->getUniqueID());
    }
    //add new enemy data if any
    for(qint32 i=0; i < spEnemies->size(); i++) {
        auto it = m_enemyUnitData.find(spEnemies->at(i)->getUniqueID());
        //if didn't find anything, add the new UnitData
        if(it == m_enemyUnitData.end())  {
            addUnitData(m_enemyUnitData, spEnemies->at(i), false);
        }
        enemyValidIDs.insert(spEnemies->at(i)->getUniqueID());
    }

    //if there is some old unitData no more in game and not marked as killed, mark it as killed
    if(m_armyUnitData.size() - m_armyUnitDataKillCount > armyValidIDs.size()) {
        for(auto it = m_armyUnitData.begin(); it != m_armyUnitData.end(); ++it) {
            if(armyValidIDs.find(it->first) == armyValidIDs.end() && !it->second.isKilled) {
                it->second.isKilled = true;
                m_armyUnitDataKillCount++;
            }
        }
    }

    //if there is some old unitData for enemies no more in game and not marked as killed, mark it as killed
    if(m_enemyUnitData.size() - m_enemyUnitDataKillCount > enemyValidIDs.size()) {
        for(auto it = m_enemyUnitData.begin(); it != m_enemyUnitData.end(); ++it) {
            if(enemyValidIDs.find(it->first) == enemyValidIDs.end() && !it->second.isKilled) {
                it->second.isKilled = true;
                m_enemyUnitDataKillCount++;
            }
        }
    }
}

void MultiInfluenceNetworkModule::updateMapsChangesForUnit(qint32 unitNum) {

    for(auto it = m_armyUnitData.begin(); it != m_armyUnitData.end(); ++it) {
        if(isUnitDataChangedForUnit(it->second, unitNum)) {
            correctInfluenceMapsForUnit(it->second, unitNum);
            //after updating maps, update also the unit data for this unit type
            updateUnitDataForUnit(it->second, unitNum);
        }
    }
    //update all enemy units in same way
    for(auto it = m_enemyUnitData.begin(); it != m_enemyUnitData.end(); ++it) {
        if(isUnitDataChangedForUnit(it->second, unitNum)) {
            correctInfluenceMapsForUnit(it->second, unitNum);
            //after updating maps, update also the unit data for this unit type
            updateUnitDataForUnit(it->second, unitNum);
        }
    }
}

void MultiInfluenceNetworkModule::updateAllUnitDataForUnit(qint32 unitNum) {
    for(auto it = m_armyUnitData.begin(); it != m_armyUnitData.end(); ++it) {
        updateUnitDataForUnit(it->second, unitNum);
    }
    for(auto it = m_enemyUnitData.begin(); it != m_enemyUnitData.end(); ++it) {
        updateUnitDataForUnit(it->second, unitNum);
    }
}


void MultiInfluenceNetworkModule::correctInfluenceMapsForUnit(UnitData &changedData, qint32 n) {
    qint32 k=0;
    //compute each local map and add it to the output, weighted
    for(qint32 l=0; l < m_localMapsPerUnit; l++) {
        InfluenceMap &infMap = influenceMapOfUnit(n, l);
        if(adaenums::isCustomType(infMap.getType())) {
            correctLocalInfluenceMap(infMap, changedData, n, true, k);
            k++;
        } else {
            correctLocalInfluenceMap(infMap, changedData, n, false);
        }
    }
}


void MultiInfluenceNetworkModule::correctLocalInfluenceMap(InfluenceMap &influenceMap, UnitData &changedData, quint32 unitNum, bool isCustom, quint32 customNum) {
    auto it = m_unitIDToM.find(changedData.unitID);
    if(it == m_unitIDToM.end()) { //if the unit has an ID not supported by this MIN, ignore it
        return;
    }
    qint32 m = it->second;

    if(isCustom) {
        switch(influenceMap.getType()) {
        //custom 1 adds influence for each unit in game, positive for allies and negative for enemies.
        //the influence is propagated with the attack area
        case adaenums::iMapType::CUSTOM_1:
        {
            float sign = 1.0f;
            //if is enemy, influences are negative
            if(changedData.isEnemy) {
                sign = -1.0f;
            }
            //if a unit data is changed and is killed, just remove its old influence
            if(changedData.isKilled) {
                Unit* pUnit = changedData.isEnemy ? m_enemyUnitTypesVector[m].get() : m_armyUnitTypesVector[m].get();
                influenceMap.addUnitAtkInfluence(pUnit, changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY,
                                                 -sign * customInfluenceMapUnitWeight(unitNum, customNum, m) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
            }
            //if position changed, subtract its old influence and add new influence
            else if(isUnitDataPositionChangedForUnit(changedData, unitNum)) {
                influenceMap.addUnitAtkInfluence(changedData.pUnit, changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY,
                                                 -sign * customInfluenceMapUnitWeight(unitNum, customNum, m) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                influenceMap.addUnitAtkInfluence(changedData.pUnit, sign * customInfluenceMapUnitWeight(unitNum, customNum, m) * aiutils::unitHpRatio(changedData.pUnit->getHp()), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
            }
            //if position is same, then only hp changed, remove an influence proportional to hp lost
            else {
                float hpLostDifferenceRatio = aiutils::unitHpRatio((changedData.statusForUnit[unitNum].hp - changedData.pUnit->getHp()));
                influenceMap.addUnitAtkInfluence(changedData.pUnit, - sign * customInfluenceMapUnitWeight(unitNum, customNum, m) * hpLostDifferenceRatio, m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
            }
        }
            break;
        case adaenums::iMapType::CUSTOM_ALLIES:
            if(!changedData.isEnemy) {
                //if a unit data is changed and is killed, just remove its old influence
                if(changedData.isKilled) {
                    influenceMap.addUnitAtkInfluence(m_armyUnitTypesVector[m].get(), changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY,
                                                     - customInfluenceMapUnitWeight(unitNum, customNum, m) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                }
                //if position changed, compute and add its inverse old influence and add new influence
                else if(isUnitDataPositionChangedForUnit(changedData, unitNum)) {
                    influenceMap.addUnitAtkInfluence(changedData.pUnit, changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY,
                                                     -customInfluenceMapUnitWeight(unitNum, customNum, m) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                    influenceMap.addUnitAtkInfluence(changedData.pUnit, customInfluenceMapUnitWeight(unitNum, customNum, m) * aiutils::unitHpRatio(changedData.pUnit->getHp()), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                }
                //if position is same, then only hp changed, remove an influence proportional to hp lost
                else {
                    float hpLostDifferenceRatio = aiutils::unitHpRatio((changedData.statusForUnit[unitNum].hp - changedData.pUnit->getHp()));
                    influenceMap.addUnitAtkInfluence(changedData.pUnit, -customInfluenceMapUnitWeight(unitNum, customNum, m) * hpLostDifferenceRatio, m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                }
            }
                    break;
        case adaenums::iMapType::CUSTOM_ENEMIES:
            if(changedData.isEnemy) {
                if(changedData.isKilled) {
                    influenceMap.addUnitAtkInfluence(m_enemyUnitTypesVector[m].get(), changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY,
                                                     customInfluenceMapUnitWeight(unitNum, customNum, m) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                }
                //if position changed, compute and add its inverse old influence and add new influence
                else if(isUnitDataPositionChangedForUnit(changedData, unitNum)) {
                    influenceMap.addUnitAtkInfluence(changedData.pUnit, changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY,
                                                     customInfluenceMapUnitWeight(unitNum, customNum, m) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                    influenceMap.addUnitAtkInfluence(changedData.pUnit, -customInfluenceMapUnitWeight(unitNum, customNum, m) * aiutils::unitHpRatio(changedData.pUnit->getHp()), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                }
                //if position is same, then only hp changed, remove an influence proportional to hp lost
                else {
                    float hpLostDifferenceRatio = aiutils::unitHpRatio((changedData.statusForUnit[unitNum].hp - changedData.pUnit->getHp()));
                    influenceMap.addUnitAtkInfluence(changedData.pUnit, customInfluenceMapUnitWeight(unitNum, customNum, m) * hpLostDifferenceRatio, m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                }
            }
            break;
        default:
            break;
        }
    } else if(adaenums::isStdType(influenceMap.getType())) {
        switch(influenceMap.getType()) {
        //std attack adds for each enemy unit its influence (negative, since it represent in general a bad tile) of attack against the type of unit determined by unitNum
        case adaenums::iMapType::STD_ATTACK:
        {
            if(changedData.isEnemy) {
                if(changedData.isKilled) {
                    influenceMap.addUnitAtkInfluence(m_enemyUnitTypesVector[m].get(), changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY,
                                                     m_damageChart.getBaseDmgWithAmmo(changedData.pUnit, m, m_nToMIndexes[unitNum]) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                }
                //if position changed, compute and add its inverse old influence and add new influence
                else if(isUnitDataPositionChangedForUnit(changedData, unitNum)) {
                    influenceMap.addUnitAtkInfluence(changedData.pUnit, changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY,
                                                     m_damageChart.getBaseDmgWithAmmo(changedData.pUnit, m, m_nToMIndexes[unitNum]) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                    influenceMap.addUnitAtkInfluence(changedData.pUnit, -m_damageChart.getBaseDmgWithAmmo(changedData.pUnit, m, m_nToMIndexes[unitNum]) * aiutils::unitHpRatio(changedData.pUnit->getHp()), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                }
                //if position is same, then only hp changed, remove an influence proportional to hp lost
                else {
                    float hpLostDifferenceRatio = aiutils::unitHpRatio((changedData.statusForUnit[unitNum].hp - changedData.pUnit->getHp()));
                    influenceMap.addUnitAtkInfluence(changedData.pUnit, m_damageChart.getBaseDmgWithAmmo(changedData.pUnit, m, m_nToMIndexes[unitNum]) * hpLostDifferenceRatio, m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
                }
            }
        }
            break;
        //std damage creates a map where the type of unit given does the most damage
        case adaenums::iMapType::STD_DAMAGE: {
            if(changedData.isEnemy) {
                Unit* pUnit = m_unitTypesVector[unitNum].get();
                //direct
                if(pUnit->getBaseMinRange() > 1) {
                    if(changedData.isKilled) {
                        influenceMap.addUnitDirectDmgValueInfluence(pUnit, QPoint(changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY), -m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], m) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp));
                    }
                    //if position changed, compute and add its inverse old influence and add new influence
                    else if(isUnitDataPositionChangedForUnit(changedData, unitNum)) {
                        influenceMap.addUnitDirectDmgValueInfluence(pUnit, QPoint(changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY), -m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], m) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp));
                        influenceMap.addUnitDirectDmgValueInfluence(pUnit, changedData.pUnit->getPosition(), m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], m) * aiutils::unitHpRatio(changedData.pUnit->getHp()));
                    }
                    //if position is same, then only hp changed, remove an influence proportional to hp lost
                    else {
                        float hpLostDifferenceRatio = aiutils::unitHpRatio((changedData.statusForUnit[unitNum].hp - changedData.pUnit->getHp()));
                        influenceMap.addUnitDirectDmgValueInfluence(pUnit, changedData.pUnit->getPosition(), -m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], m) * hpLostDifferenceRatio);
                    }
                }
                //indirect
                else {
                    if(changedData.isKilled) {
                        influenceMap.addUnitIndirectDmgValueInfluence(pUnit, QPoint(changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY), -m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], m) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp));
                    }
                    //if position changed, compute and add its inverse old influence and add new influence
                    else if(isUnitDataPositionChangedForUnit(changedData, unitNum)) {
                        influenceMap.addUnitIndirectDmgValueInfluence(pUnit, QPoint(changedData.statusForUnit[unitNum].posX, changedData.statusForUnit[unitNum].posY), -m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], m) * aiutils::unitHpRatio(changedData.statusForUnit[unitNum].hp));
                        influenceMap.addUnitIndirectDmgValueInfluence(pUnit, changedData.pUnit->getPosition(), m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], m) * aiutils::unitHpRatio(changedData.pUnit->getHp()));
                    }
                    //if position is same, then only hp changed, remove an influence proportional to hp lost
                    else {
                        float hpLostDifferenceRatio = aiutils::unitHpRatio((changedData.statusForUnit[unitNum].hp - changedData.pUnit->getHp()));
                        influenceMap.addUnitIndirectDmgValueInfluence(pUnit, changedData.pUnit->getPosition(), -m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], m) * hpLostDifferenceRatio);
                    }
                }
            }
        }
            break;
        default:
            break;
        }
    }
    //if it's not custom nor standard, it's a once map, so don't recompute it
}


void MultiInfluenceNetworkModule::computeGlobalInfluenceMap(InfluenceMap &influenceMap, bool isCustom, quint32 customNum) {
    if(isCustom) {
        //todo
        if(customNum >= 0)
            influenceMap.reset();
    }
    //if it's standard
    else if(adaenums::isStdType(influenceMap.getType())){
        //todo
        influenceMap.reset();
    } else {
        switch(influenceMap.getType()) {
        case adaenums::iMapType::ONCE_MAPDEFENSE:
            if(influenceMap.isComputed()){
                return;
            }
            influenceMap.reset();
            influenceMap.addGenericMapDefenseInfluence(m_pPlayer, m_weightPerStar, m_friendlyBuildingMultiplier, m_friendlyFactoryMultiplier);
            influenceMap.setComputed(true);
            break;
        default:
            break;
        }
    }
}


void MultiInfluenceNetworkModule::computeLocalInfluenceMap(InfluenceMap &influenceMap, quint32 unitNum, spQmlVectorUnit spUnits,
                                                           spQmlVectorUnit spEnemies, bool isCustom, quint32 customNum) {

    if(isCustom) {
        influenceMap.reset();

        switch(influenceMap.getType()) {
        //custom 1 adds influence for each unit in game, positive for allies and negative for enemies.
        //the influence is propagated with the attack area
        case adaenums::iMapType::CUSTOM_1:
            for(qint32 i=0; i < spUnits->size(); i++) {
                Unit* pUnit = spUnits->at(i);
                auto it = m_unitIDToM.find(pUnit->getUnitID());
                if(it == m_unitIDToM.end()) {
                    continue;//if is a type of unit not supported by this MIN, skip its influence
                }
                influenceMap.addUnitAtkInfluence(pUnit, customInfluenceMapUnitWeight(unitNum, customNum, it->second) * aiutils::unitHpRatio(pUnit->getHp()), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
            }
            for(qint32 i=0; i < spEnemies->size(); i++) {
                Unit* pEnemy = spEnemies->at(i);
                auto it = m_unitIDToM.find(pEnemy->getUnitID());
                if(it == m_unitIDToM.end()) {
                    continue;//if is a type of unit not supported by this MIN, skip its influence
                }
                influenceMap.addUnitAtkInfluence(pEnemy, -customInfluenceMapUnitWeight(unitNum, customNum, it->second) * aiutils::unitHpRatio(pEnemy->getHp()), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
            }
            break;
            //custom allies and custom enemies together are more powerful than a custom 1 (however they are also more complex and slower to converge)
        case adaenums::iMapType::CUSTOM_ALLIES:
            for(qint32 i=0; i < spUnits->size(); i++) {
                Unit* pUnit = spUnits->at(i);
                auto it = m_unitIDToM.find(pUnit->getUnitID());
                if(it == m_unitIDToM.end()) {
                    continue;//if is a type of unit not supported by this MIN, skip its influence
                }
                influenceMap.addUnitAtkInfluence(pUnit, customInfluenceMapUnitWeight(unitNum, customNum, it->second) * aiutils::unitHpRatio(pUnit->getHp()), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
            }
            break;
        case adaenums::iMapType::CUSTOM_ENEMIES:
            for(qint32 i=0; i < spEnemies->size(); i++) {
                Unit* pEnemy = spEnemies->at(i);
                auto it = m_unitIDToM.find(pEnemy->getUnitID());
                if(it == m_unitIDToM.end()) {
                    continue;//if is a type of unit not supported by this MIN, skip its influence
                }
                influenceMap.addUnitAtkInfluence(pEnemy, -customInfluenceMapUnitWeight(unitNum, customNum, it->second) * aiutils::unitHpRatio(pEnemy->getHp()), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
            }
            break;
        default:
            break;
        }
    }
    //if is standard
    else if(adaenums::isStdType(influenceMap.getType())){
        influenceMap.reset();

        switch(influenceMap.getType()) {
        //std attack adds for each enemy unit its influence (negative, since it represent in general a bad tile) of attack against the type of unit determined by unitNum
        case adaenums::iMapType::STD_ATTACK:
            for(qint32 i=0; i < spEnemies->size(); i++) {
                Unit* pEnemy = spEnemies->at(i);
                auto it = m_unitIDToM.find(pEnemy->getUnitID());
                if(it == m_unitIDToM.end()) {
                    continue;//if is a type of unit not supported by this MIN, skip its influence
                }
                influenceMap.addUnitAtkInfluence(pEnemy, -m_damageChart.getBaseDmgWithAmmo(pEnemy, it->second, m_nToMIndexes[unitNum]) * aiutils::unitHpRatio(pEnemy->getHp()), m_stepMultiplier, m_stepsForDirectUnits, m_stepsForIndirectUnits);
            }
            break;
        //std damage creates a map where the type of unit given does the most damage
        case adaenums::iMapType::STD_DAMAGE: {
            Unit* pUnit = m_unitTypesVector[unitNum].get();
            //direct
            if(pUnit->getBaseMinRange() > 1) {
                for(qint32 i=0; i < spEnemies->size(); i++) {
                    Unit* pEnemy = spEnemies->at(i);
                    auto it = m_unitIDToM.find(pEnemy->getUnitID());
                    if(it == m_unitIDToM.end()) {
                        continue;//if is a type of unit not supported by this MIN, skip its influence
                    }
                    influenceMap.addUnitDirectDmgValueInfluence(pUnit, pEnemy->getPosition(), m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], it->second) * aiutils::unitHpRatio(spEnemies->at(i)->getHp()));
                }
            }
            //indirect
            else {
                for(qint32 i=0; i < spEnemies->size(); i++) {
                    Unit* pEnemy = spEnemies->at(i);
                    auto it = m_unitIDToM.find(pEnemy->getUnitID());
                    if(it == m_unitIDToM.end()) {
                        continue;//if is a type of unit not supported by this MIN, skip its influence
                    }
                    influenceMap.addUnitIndirectDmgValueInfluence(pUnit, pEnemy->getPosition(), m_damageChart.getBaseDmg(m_nToMIndexes[unitNum], it->second) * aiutils::unitHpRatio(spEnemies->at(i)->getHp()));
                }
            }
        }
            break;
        default:
            break;
        }
    }
    //if is a once map, compute it only if not already done
    else {
        if(influenceMap.isComputed())
            return;

        switch(influenceMap.getType()) {
        //mapdefense adds on each tile the defense gained by this unit's map (if the unit can go over that tile)
        case adaenums::iMapType::ONCE_MAPDEFENSE: {
            influenceMap.reset();
            spUnit unit = new Unit(m_unitList[unitNum], m_pPlayer, false);
            influenceMap.addMapDefenseInfluence(m_pPlayer, unit.get(), m_weightPerStar, m_friendlyBuildingMultiplier, m_friendlyFactoryMultiplier);
            influenceMap.setComputed(true);
            Console::print("Computed global once map:\n" + influenceMap.toQStringFull(), Console::eDEBUG);
        }
            break;
        default:
            break;
        }
    }
}

void MultiInfluenceNetworkModule::computeInfluenceMapsForUnit(qint32 n, spQmlVectorUnit spUnits, spQmlVectorUnit spEnemies) {
    qint32 k=0;

    //compute each local map and add it to the output, weighted
    for(qint32 l=0; l < m_localMapsPerUnit; l++) {
        InfluenceMap &infMap = influenceMapOfUnit(n, l);
        if(adaenums::isCustomType(infMap.getType())) {
            computeLocalInfluenceMap(infMap, n, spUnits, spEnemies, true, k);
            k++;
        } else {
            computeLocalInfluenceMap(infMap, n, spUnits, spEnemies, false);
        }
        Console::print(infMap.toQStringFull(2), Console::eDEBUG);
    }

    //update the unitData of this unit to now
    updateAllUnitDataForUnit(n);
}


void MultiInfluenceNetworkModule::computeOutputMapForUnit(qint32 n, bool computeAll, spQmlVectorUnit spUnits, spQmlVectorUnit spEnemies) {
    m_unitOutputMaps[n].reset(); //reset output map to 0
    if(computeAll)
        computeInfluenceMapsForUnit(n, spUnits, spEnemies);

    for(qint32 l=0; l < m_localMapsPerUnit; l++) {
        m_unitOutputMaps[n].weightedAddMap(influenceMapOfUnit(n, l));
    }
    //add also each global map, weighted. Here global maps are already computed
    for(quint32 g=0; g < m_globalInfluenceMaps.size(); g++) {
        m_unitOutputMaps[n].weightedAddMap(m_globalInfluenceMaps[g], glbMapWeightForUnit(n, g));
    }

    m_isMapNComputed[n] = true;
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


qint32 MultiInfluenceNetworkModule::indexOfCustomMapWeightOfTypeAndNumberOfUnit(qint32 n, adaenums::iMapType type, qint32 typeNumber) {
    qint32 k = -1;
    for(qint32 l=0; l < m_localMapsPerUnit; l++) {
        if(adaenums::isCustomType(influenceMapOfUnit(n, l).getType()))
            k++;
        if(influenceMapOfUnit(n, l).getType() == type) {
            typeNumber--;
            if(typeNumber == 0) {
                return k;
            }
        }
    }
    return -1;
}


qint32 MultiInfluenceNetworkModule::indexOfStdMapWeightOfTypeAndNumberOfUnit(qint32 n, adaenums::iMapType type, qint32 typeNumber) {
    qint32 s = -1;
    for(qint32 l=0; l < m_localMapsPerUnit; l++) {
        if(!adaenums::isCustomType(influenceMapOfUnit(n, l).getType()))
            s++;
        if(influenceMapOfUnit(n, l).getType() == type) {
            typeNumber--;
            if(typeNumber == 0) {
                return s;
            }
        }
    }
    return -1;
}


qint32 MultiInfluenceNetworkModule::indexOfGlobalMapWeightOfTypeAndNumber(adaenums::iMapType type, qint32 typeNumber) {
    for(qint32 g = 0; g < m_globalMapsAmount; g++) {
        if(m_globalInfluenceMaps[g].getType() == type) {
            typeNumber--;
            if(typeNumber == 0) {
                return g;
            }
        }
    }
    return -1;
}


qint32 MultiInfluenceNetworkModule::indexOfGlobalCustomMapWeightOfTypeAndNumber(adaenums::iMapType type, qint32 typeNumber) {
    qint32 gk = -1;
    for(qint32 g = 0; g < m_globalMapsAmount; g++) {
        if(adaenums::isCustomType(m_globalInfluenceMaps[g].getType()))
            gk++;
        if(m_globalInfluenceMaps[g].getType() == type) {
            typeNumber--;
            if(typeNumber == 0)
                return gk;
        }
    }
    return -1;
}
