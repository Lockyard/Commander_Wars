#include "multiinfluencenetworkmodule.h"
#include <QSettings>

MultiInfluenceNetworkModule::MultiInfluenceNetworkModule()
{

}


void MultiInfluenceNetworkModule::readIni(QString filename) {
    QSettings settings(filename, QSettings::IniFormat);

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

    m_fullUnitAmount = m_unitListFull.size(); //M

    QStringList unitInfMaps = settings.value("UnitInfluenceMaps", "NONE").toStringList();

    m_customMapsPerUnit = 0; //K, incremented now if any
    m_stdMapsPerUnit = 0; //S, incremented now if any
    m_totalMapsPerUnit = 0;

    //initialize all maps for each unit
    if(!unitInfMaps.contains("NONE") && unitInfMaps.size() > 0) {
        for(qint32 i = 0; i < unitInfMaps.size(); i++) {
            if(unitInfMaps[i].startsWith("CUSTOM"))
                m_customMapsPerUnit++;
            else
                m_stdMapsPerUnit++;
        } //here K and S are correct
        m_totalMapsPerUnit = unitInfMaps.size(); //L = S + K

        m_unitInfluenceMaps.reserve(m_unitAmount * m_totalMapsPerUnit); //N * (K + S)
        for(qint32 n=0; n < m_unitAmount; n++) {
            for(qint32 l=0; l < m_totalMapsPerUnit; l++) {
                //create an influence map of the specified type
                m_unitInfluenceMaps.push_back(InfluenceMap(adaenums::getInfluenceMapTypeFromString(unitInfMaps.at(l))));
            }
        }
    }

    //initializate global inf map vector
    QStringList globalInfMaps = settings.value("GlobalInfluenceMaps").toStringList();
    m_globalMapsAmount = 0; //G
    m_globalCustomMapsAmount = 0; //GK
    if(!globalInfMaps.contains("NONE") && globalInfMaps.size() > 0) {
        m_globalMapsAmount = globalInfMaps.size();
        m_globalInfluenceMaps.reserve(m_globalMapsAmount);
        for(qint32 infMapCount=0; infMapCount < unitInfMaps.size(); infMapCount++) {
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
    if(!unitInfMaps.contains("NONE") && unitInfMaps.size() > 0) {
        for(qint32 i=0; i < unitInfMaps.size(); i++) {
            //standard inf maps require only 1 weight per unit, since they are computed in a standard way
            if(unitInfMaps[i].startsWith("STD"))
                weightsPerUnit++;
            //custom maps require a weight for each unit in game, plus 1 since they are weighted as well
            else
                weightsPerUnit += m_fullUnitAmount + 1; //M + 1
        }
    }
    //add 1 weight to each unit for each global influence map
    weightsPerUnit += m_globalInfluenceMaps.size(); //G

    m_requiredVectorLength = m_unitList.size()*weightsPerUnit; //N*(S + K*(M+1) + G)

    settings.endGroup();
}

/**
 * @brief processStartOfTurn calculate bids for this turn and process stuff for the start of turn
 */
void MultiInfluenceNetworkModule::processStartOfTurn() {
    //do unit count at start of turn
    for(qint32 i=0; i < m_unitList.size(); i++) {
        m_unitCount[i] = m_pPlayer->getUnitCount(m_unitList[i]);
    }

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
        //if there is at least 1 unit of a type, compute its influence maps
        if(m_unitCount[n] > 0) {
            k=0; //reset k
            m_unitOutputMaps[n].reset(); //reset output map to 0
            //compute each local map and add it to the output, weighted
            for(qint32 l=0; l < m_totalMapsPerUnit; l++) {
                InfluenceMap &infMap = influenceMapOfUnit(l, n);
                if(adaenums::isCustomType(infMap.getType())) {
                    computeLocalInfluenceMap(infMap, l, n, true, k);
                    k++;
                } else {
                    computeLocalInfluenceMap(infMap, l, n, false);
                }
                m_unitOutputMaps[n].weightedAddMap(infMap);
            }
            //add also each global map, weighted. Here global maps are already computed
            for(quint32 g=0; g < m_globalInfluenceMaps.size(); g++) {
                m_unitOutputMaps[n].weightedAddMap(m_globalInfluenceMaps[g]);
            }
        } //here the output map is done, for unit n
    }
}

/**
 * @brief processHighestBidUnit process the action for the highest bid unit of this module
 * @param weighted
 */
void MultiInfluenceNetworkModule::processHighestBidUnit() {

}

void MultiInfluenceNetworkModule::processUnit(Unit* pUnit) {

}

void MultiInfluenceNetworkModule::notifyUnitUsed(Unit* pUnit) {

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
    qint32 L = m_totalMapsPerUnit; //L = S + K, all local maps amount

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

//private methods///////////////////////////////////////////////////////////////////////////////

void MultiInfluenceNetworkModule::defaultInitializeUnitList(QStringList &unitList) {
    unitList.clear();
    //by default append all units which can be built by all 3 construction buildings
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
    unitList.append(buildList);

    ret = pInterpreter->doFunction("AIRPORT", function1, args1);
    buildList = ret.toVariant().toStringList();
    unitList.append(buildList);
}


void MultiInfluenceNetworkModule::computeGlobalInfluenceMap(InfluenceMap &influenceMap, bool isCustom, quint32 customNum) {
    if(isCustom) {

    }
    //if it's standard
    else {

    }
}

void MultiInfluenceNetworkModule::computeLocalInfluenceMap(InfluenceMap &influenceMap, quint32 localMapNum, quint32 unitNum,
                                                           bool isCustom, quint32 customNum) {
    //todo
}
