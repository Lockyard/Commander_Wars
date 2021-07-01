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
#include "genetic/evofunctions.h"
#include "ai/adapta/trainingmanager.h"


////////////////////////////////////////////////////////////////
#include <chrono> // for std::chrono functions

class A {
public:
    A(int a) : a_int(a) {};
    int getA() {return a_int;};
    virtual int getB() = 0;
private:
    int a_int;
};

class B1 : public A {
public:
    B1(int a, int b) : A(a), b_int(b){};
    virtual int getB() override {return b_int;};
private:
    int b_int;
};

class B2 : public A {
public:
    B2(int a, int b) : A(a), b_int(b){};
    virtual int getB() override {return b_int*2;};
private:
    int b_int;
};

void testClasses() {
    std::vector<A*> avec;
    B1 b1(1, 2);
    B2 b2(3, 4);
    avec.push_back(&b1);
    avec.push_back(&b2);
    Console::print("b1's getB: " + QString::number(b1.getB()), Console::eDEBUG);
    Console::print("b1's getB from avec: " + QString::number(avec[0]->getB()), Console::eDEBUG);
    Console::print("b2's getB: " + QString::number(b2.getB()), Console::eDEBUG);
    Console::print("b2's getB from avec: " + QString::number(avec[1]->getB()), Console::eDEBUG);
};

class MyTimer
{
private:
    // Type aliases to make accessing nested type easier
    using clock_t = std::chrono::high_resolution_clock;
    using second_t = std::chrono::duration<double, std::ratio<1>>;

    std::chrono::time_point<clock_t> m_beg;

public:
    MyTimer() : m_beg(clock_t::now()){
    }

    void reset()
    {
        m_beg = clock_t::now();
    }

    double elapsed() const
    {
        return std::chrono::duration_cast<second_t>(clock_t::now() - m_beg).count();
    }
};

quint32 indexOfWithStdVector(std::vector<QString> &stringVec, QString searchedString) {
    for(quint32 j=0; j < stringVec.size(); j++){
        if(stringVec[j] == searchedString)
            return j;
    }
    return 0;
}

//result is that QStringList is the fastest, around 2/3 of time required to std::map
void testMapSpeed() {
    std::map<QString, quint32> unitIDToMIndexMap;
    std::vector<QString> unitListVector = {"INFANTRY", "MECH", "RECON", "ARTILLERY", "LIGHT_TANK", "FLAK", "HEAVY_TANK", "NEOTANK", "ROCKETTHROWER", "MEGATANK", //10
                                    "SUPERMACHINE1", "ROBO1", "ROBO2", "ROBO3", "ROBO4", "ROBO5", "ROBO6", "ROBO7", "ROBO8", "ROBO85", //10
                                    "ROBO9", "ROBO10", "ROBO11", "ROBO12", "ROBO13", "ROBO14", "ROBO15", "ROBO16", "ROBO17", "ROBO18", //10
                                    "ROBO19", "ROBO20", "ROBO21", "ROBOASD", "ROBORJEGKR", "ROBO_MECH", "ROBO_INFANTRY", "ROBO_ART", "ROBO_TANK", "ROBO_NEO"}; //10
    QStringList unitList;

    unitList.reserve(unitListVector.size());
    for(quint32 i=0; i < unitListVector.size(); i++) {
        unitIDToMIndexMap[unitListVector[i]] = i;
        unitList.append(unitListVector[i]);
    }

    qint32 a = 0;
    qint32 listIndex = 0;
    qint32 size = unitListVector.size(); //40
    qint32 bigN = 10'000'000;
    double elapsed;


    //map
    MyTimer t1;
    for(qint32 i=0; i<bigN; i++) {
        a += unitIDToMIndexMap.at(unitListVector[listIndex]);
        listIndex++;
        if(listIndex == size)
            listIndex = 0;
    }
    elapsed = t1.elapsed();
    Console::print("Time elapsed using std::map for " + QString::number(bigN) + " reads is: " + QString::number(elapsed) + "\n(a = " + QString::number(a) + ")", Console::eDEBUG);

    a = 0;
    listIndex = 0;
    //QSList
    MyTimer t2;
    for(qint32 i=0; i<bigN; i++) {
        a += unitList.indexOf(unitListVector[listIndex]);
        listIndex++;
        if(listIndex == size)
            listIndex = 0;
    }
    elapsed = t2.elapsed();
    Console::print("Time elapsed using QStringList.indexOf for " + QString::number(bigN) + " reads is: " + QString::number(elapsed) + "\n(a = " + QString::number(a) + ")", Console::eDEBUG);

    a = 0;
    listIndex = 0;
    //map
    MyTimer t3;
    for(qint32 i=0; i<bigN; i++) {
        a += indexOfWithStdVector(unitListVector, unitListVector[listIndex]);
        listIndex++;
        if(listIndex == size)
            listIndex = 0;
    }
    elapsed = t3.elapsed();
    Console::print("Time elapsed using cycling over a vector (of size " + QString::number(size) +  ") for " + QString::number(bigN) + " read is: " + QString::number(elapsed) + "\n(a = " + QString::number(a) + ")", Console::eDEBUG);
}

/**
 * @brief testSpeedOfVectors
 * From what resulted, on average for 30M length vectors, QVector did around 1.5 secs, QVector of QVector 3 secs,
 * std vector 0.15 (ten times less!) and a standard array even faster (0.1)
 */
void testSpeedOfVectors() {
    QVector<float> milione3;
    qint32 bigN = 10'000'000;
    milione3.resize(bigN*3);
    QVector<QVector<float>> splitMilione;
    std::vector<float> milione3StdV;
    milione3StdV.resize(bigN*3);
    float* floatArray = new float[bigN*3];
    splitMilione.reserve(3);
    for(qint32 i = 0; i < 3; i++) {
        splitMilione.append(QVector<float>(bigN));
    }
    std::vector<std::vector<float>> splitMilioneStd;
    splitMilioneStd.reserve(3);
    for(qint32 i = 0; i < 3; i++) {
        splitMilioneStd.push_back(std::vector<float>(bigN));
    }

    MyTimer t1;
    for(qint32 i=0; i < 3; i++) {
        for(qint32 j=0; j < bigN; j++) {
            milione3[i*bigN + j]++;
        }
    }
    double elapsed = t1.elapsed();
    Console::print("using Qvector, " + QString::number(elapsed) + " seconds passed", Console::eINFO);

    MyTimer t2;
    for(qint32 i=0; i < 3; i++) {
        for(qint32 j=0; j < bigN; j++) {
            splitMilione[i][j]++;
        }
    }
    double elapsed2 = t2.elapsed();
    Console::print("using Qvector of Qvector, " + QString::number(elapsed2) + " seconds passed", Console::eINFO);

    MyTimer t3;
    for(qint32 i=0; i < 3; i++) {
        for(qint32 j=0; j < bigN; j++) {
            milione3StdV[i*bigN + j]++;
        }
    }
    double elapsed3 = t3.elapsed();
    Console::print("using std vector, " + QString::number(elapsed3) + " seconds passed", Console::eINFO);

    MyTimer t4;
    for(qint32 i=0; i < 3; i++) {
        for(qint32 j=0; j < bigN; j++) {
            floatArray[i*bigN + j]++;
        }
    }
    double elapsed4 = t4.elapsed();
    Console::print("using an array, " + QString::number(elapsed4) + " seconds passed", Console::eINFO);

    MyTimer t5;
    for(qint32 i=0; i < 3; i++) {
        for(qint32 j=0; j < bigN; j++) {
            splitMilioneStd[i][j]++;
        }
    }
    double elapsed5 = t5.elapsed();
    Console::print("using std vector of std vector, " + QString::number(elapsed5) + " seconds passed", Console::eINFO);

    delete[] floatArray;
}
/////////////////////////////////////////////////

TestFirstAI::TestFirstAI() : CoreAI(GameEnums::AiTypes_Adapta), m_adaAI(), m_MINmodule(&m_adaAI),
    m_influenceMap(), m_inffMap(m_IslandMaps)
{
    rebuildIslandMaps = true;
    TrainingManager::instance().getAssignedWeightVector();
}


void TestFirstAI::readIni(QString name) {
    QString a = name;
}

//todo remove this
void TestFirstAI::testPrint() {
    Console::print("TESTPRINT it's working!" , Console::eDEBUG);
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

        /*
        for (qint32 i= 0; i < pUnits->size(); i++)
        {
            Unit* pUnit = pUnits->at(i);
            m_influenceMap.addUnitInfluence(pUnit, pEnemyUnits, pUnit->getCosts()*.001f, adaenums::ATTACK1);
        }//*/

        for (qint32 i= 0; i < pEnemyUnits->size(); i++)
        {
            Unit* pUnit = pEnemyUnits->at(i);

            if(pUnit->getUnitID() == "ARTILLERY" && pUnits->size() > 0) {
                //negative weight since are enemies, and this ai's units as enemies of the considered unit
                //m_influenceMap.addUnitAtkInfluence(pUnit, 100, .5f, 2);
                //m_influenceMap.addUnitInfluence(pUnit, pUnits, pUnit->getCosts()*-.001f, adaenums::ATTACK1);
                //m_influenceMap.addMapDefenseInfluence(m_pPlayer, pUnit, 1, 1.5, 0.1);
                //m_influenceMap.addUnitIndirectDmgValueInfluenceFaster(pUnit, pUnit->getPosition(), 80);
                //m_influenceMap.addUnitIndirectDmgValueInfluenceFast(pUnit, pUnit->getPosition(), 80);
                //m_influenceMap.addUnitIndirectDmgValueInfluence(pUnit, pUnit->getPosition(), 80);
            }
        }

        m_influenceMap.showAllInfo();
        Console::print("InfluenceMap:\n" + m_influenceMap.toQString(8), Console::eDEBUG);

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
        //qint32 movePoints = pUnit->getMovementpoints(pUnit->getPosition());

        if(pUnit->hasAction(ACTION_FIRE) && !pUnit->getHasMoved()) {
            //just for test, move 2 tiles above and attack the unit 3 tiles above if any
            QPoint fieldTarget(pUnit->getX(), qMax(0, pUnit->getY() - 2));
            QPoint attackTarget(pUnit->getX(), qMax(0, pUnit->getY() - 3));
            GameMap* pMap = GameMap::getInstance();
            if(pMap->getTerrain(attackTarget.x(), attackTarget.y())->getUnit() != nullptr) {
                spGameAction pAction = new GameAction(ACTION_FIRE);
                pAction->setTarget(QPoint(pUnit->getX(),pUnit->getY()));
                UnitPathFindingSystem upfs(pUnit);
                upfs.explore();
                QVector<QPoint> targets = upfs.getAllNodePoints();

                QVector<QPoint> path = upfs.getPath(static_cast<qint32>(fieldTarget.x()),
                                                   static_cast<qint32>(fieldTarget.y()));
                pAction->setMovepath(path, upfs.getCosts(path));

                CoreAI::addSelectedFieldData(pAction, attackTarget);

                if (pAction->isFinalStep() && pAction->canBePerformed())
                {
                    emit performAction(pAction);
                    return true;
                }
            }
        }

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



