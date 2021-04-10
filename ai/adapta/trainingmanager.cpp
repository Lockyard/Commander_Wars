#include "trainingmanager.h"
#include "coreengine/console.h"
#include <iostream>
#include "ai/genetic/evofunctions.h"

TrainingManager::TrainingManager(QObject *parent) : QObject(parent), m_isEvoManagerInitialized(false) {
    std::cout << "training manager was created";
}

bool TrainingManager::s_isInstanceInitialized = false;

TrainingManager TrainingManager::s_instance(nullptr);


void TrainingManager::readIni() {
    //todo read from file, this is a mockup
    m_currWVIndex = 0;
    m_currWVMatchNumber = 0;
    m_matchNumberTarget = 3;
    m_elitismDegree = 1;
    m_randomismDegree = 1;
}


void TrainingManager::setupForMatch() {
    if(!m_isEvoManagerInitialized)
        initializeEvolutionManager();

    spGameMap pMap = GameMap::getInstance();
    connect(pMap->getGameRules(), &GameRules::signalVictory, this, &TrainingManager::onVictory, Qt::QueuedConnection);

}

WeightVector TrainingManager::assignWeightVector() {
    Console::print("assigning vector " + m_evolutionManager.getPopulation()[m_currWVIndex].toQString(), Console::eDEBUG);
    return m_evolutionManager.getPopulation()[m_currWVIndex];
}


//slots
void TrainingManager::onVictory() {
    Console::print("Training: Finished match " + QString::number(m_currWVMatchNumber+1) + "/" +
                   QString::number(m_matchNumberTarget) + " of Weight vector " + QString::number(m_currWVIndex+1) + "/" +
                   QString::number(m_evolutionManager.getPopulationSize()), Console::eDEBUG);
    advanceMatchCount();
}


//private fncts
void TrainingManager::initializeEvolutionManager() {
    readIni();
    m_evolutionManager = EvolutionManager(10, 3, -10.0f, 10.0f, evoenums::mixRandom, .05f);
    m_evolutionManager.setMutationFunction(evofunc::individualRandomMutation);
    m_evolutionManager.setElitismDegree(m_elitismDegree);
    m_evolutionManager.setRandomismDegree(m_randomismDegree);
    m_evolutionManager.maximizeFitness();
    m_evolutionManager.createRandomPopulation();

    m_isEvoManagerInitialized = true;
}

void TrainingManager::advanceMatchCount() {
    //increase counter of match to select the correct weight to be tested
    m_currWVMatchNumber++;
    if(m_currWVMatchNumber == m_matchNumberTarget) {
        //current Weight Vector has finished its testing, calculate its real fitness
        evaluateFitnessOfCurrentWV();
        m_currWVMatchNumber = 0;

        //start testing the new weight vector
        m_currWVIndex++;
        //if there are no new WVs to test for this generation then evolve population
        if(m_currWVIndex == m_evolutionManager.getPopulationSize()) {
            m_evolutionManager.performOneEvolutionStep();
            Console::print("Training manager evolved population (gen " +
                           QString::number(m_evolutionManager.getGeneration()) + "):\n" +
                           m_evolutionManager.toQStringPopulation(), Console::eDEBUG);
            //the first new WV to be examined is the first after the elite vectors, which were already tested
            m_currWVIndex = m_elitismDegree;
        }
    }
}


void TrainingManager::evaluateFitnessOfCurrentWV() {
    //TODO put a real fitness calculation and not a dummy one
    QVector<float> wVec = m_evolutionManager.getPopulation()[m_currWVIndex].getQVector();
    float dummyFitness = std::accumulate(wVec.begin(), wVec.end(), 0.0f) * .1f;
    m_evolutionManager.getPopulation()[m_currWVIndex].setFitness(dummyFitness);
    Console::print("Vector " + QString::number(m_currWVIndex+1) + "/" +
                   QString::number(m_evolutionManager.getPopulationSize()) +
                   " got fitness of " + QString::number(dummyFitness) + "\n" +
                   m_evolutionManager.getPopulation()[m_currWVIndex].toQString(), Console::eDEBUG);
}
