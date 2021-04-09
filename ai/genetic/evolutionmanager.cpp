#include "evolutionmanager.h"

#include <QRandomGenerator>
#include <QTime>
#include "ai/genetic/evofunctions.h"


const qint32 EvolutionManager::infinite = std::numeric_limits<qint32>::max();


EvolutionManager::EvolutionManager()
{

}

EvolutionManager::EvolutionManager(qint32 weightVectorLength, qint32 populationSize, float minWeight, float maxWeight,
                                   evoenums::CrossoverType crossoverType, float mutationProbability) :
    m_crossoverType(crossoverType),
    m_populationSize(populationSize),
    m_weightVectorLength(weightVectorLength),
    m_minWeight(minWeight),
    m_maxWeight(maxWeight),
    m_mutationProbability(mutationProbability)
{
    m_population.reserve(m_populationSize);
    if(m_minWeight > m_maxWeight) {
        float tmp = m_minWeight;
        m_minWeight = m_maxWeight;
        m_maxWeight = tmp;
    }

    switch(m_crossoverType) {
    case evoenums::splitMiddle:
        m_crossoverFunction = evofunc::splitMiddleCrossoverFct;
        break;
    case evoenums::splitRandom:
        m_crossoverFunction = evofunc::splitRandomCrossoverFct;
        break;
    case evoenums::mixRandom:
        m_crossoverFunction = evofunc::mixRandomCrossoverFct;
        break;
    case evoenums::custom:
    default:
        break;
    }
}


void EvolutionManager::createRandomPopulation(qint32 weightVectorLength, qint32 populationSize, float minWeight,
                                        float maxWeight) {
    m_isPopulationSorted = false;
    m_weightVectorLength = weightVectorLength;
    m_populationSize = populationSize;
    m_minWeight = minWeight < maxWeight ? minWeight : maxWeight;
    m_maxWeight = minWeight < maxWeight ? maxWeight : minWeight;

    m_population.reserve(populationSize);

    //create population with random weights
    for(qint32 i = 0; i < populationSize; i++) {
        //new Vector of weights
        WeightVector newWeightVector = WeightVector::generateRandomWeightVector(weightVectorLength, minWeight, maxWeight);
        //add it.
        //TODO check if this will go out of scope, since is created here
        m_population.append(newWeightVector);
    }
}

void EvolutionManager::createRandomPopulation() {
    createRandomPopulation(m_weightVectorLength, m_populationSize, m_minWeight, m_maxWeight);
}


void EvolutionManager::resetPopulation() {
    m_isPopulationSorted = false;
    m_population.clear();
}


void EvolutionManager::setPopulation(QVector<WeightVector> newPopulation) {
    m_isPopulationSorted = false;
    m_population.clear();
    m_population.append(newPopulation);
    m_populationSize = m_population.size();
}


QVector<WeightVector>& EvolutionManager::getPopulation() {
    return m_population;
}


void EvolutionManager::resetGeneration() {
    m_generationNumber = 0;
}


qint32 EvolutionManager::getGeneration() {
    return m_generationNumber;
}


void EvolutionManager::setElitismDegree(qint32 bestElementsToKeep) {
    //set the best elements to keep at most as population size
    m_bestElementsToKeep = bestElementsToKeep < m_populationSize ? bestElementsToKeep : m_populationSize;
}

void EvolutionManager::setRandomismDegree(qint32 newRandomSamples) {
    m_newRandomSamples = newRandomSamples < m_populationSize ? newRandomSamples : m_populationSize;
}


void EvolutionManager::maximizeFitness() {
    m_fitnessToMaximize = true;
}


void EvolutionManager::setFitnessFunction(float (*fitnessFunction)(WeightVector weightVector)) {
    m_fitnessFunction = fitnessFunction;
}


void EvolutionManager::setCrossoverFunctionType(evoenums::CrossoverType crossoverType) {

    switch(crossoverType) {
    case evoenums::CrossoverType::splitMiddle:
        m_crossoverFunction = evofunc::splitMiddleCrossoverFct;
        break;
    case evoenums::CrossoverType::splitRandom:
        m_crossoverFunction = evofunc::splitRandomCrossoverFct;
        break;
    case evoenums::CrossoverType::mixRandom:
    default:
        m_crossoverFunction = evofunc::mixRandomCrossoverFct;
    }
}


void EvolutionManager::setCrossoverFunction(WeightVector (*crossoverFunction) (WeightVector weightVector_1, WeightVector weightVector_2),
                          bool setThisAsCrossoverFunction) {
    if(setThisAsCrossoverFunction)
        m_crossoverType = evoenums::CrossoverType::custom;

    m_crossoverFunction = crossoverFunction;
}


/**
 * @brief setMutationFunction the mutation func used to mutate a weights Vector
 */
void EvolutionManager::setMutationFunction(void (*mutationFunction) (WeightVector& weightVector, float minWeight, float maxWeight, float probability)) {
    m_mutationFunction = mutationFunction;
}


bool EvolutionManager::canStartEvolution() {
    return (m_mutationFunction != nullptr && m_crossoverFunction != nullptr &&
            m_fitnessFunction != nullptr && m_populationSize >= 2 && m_population.size() >= 2);
}


void EvolutionManager::startEvolution(float fitnessTreshold, bool maximizeFitness, qint32 maxGenerations) {

}


bool EvolutionManager::canPerformOneEvolutionStep() {
    return (m_mutationFunction != nullptr && m_crossoverFunction != nullptr &&
            m_populationSize >= 2 && m_population.size() >= 2);
}


bool EvolutionManager::performOneEvolutionStep() {
    if(!canPerformOneEvolutionStep())
        return false;

    if(!m_isPopulationSorted)
        sortPopulationByFitness();

    QVector<WeightVector> newPopulation;
    newPopulation.reserve(m_populationSize);
    //copy the n best elements in the new population
    for(qint32 i = 0; i < m_bestElementsToKeep; i++) {
        newPopulation.append(m_population.at(i));
    }

    //create m new completely random samples in the population
    for(qint32 i = 0; i < m_newRandomSamples; i++) {
        newPopulation.append(WeightVector::generateRandomWeightVector(m_weightVectorLength, m_minWeight, m_maxWeight));
    }

    //take all other pairs of parents according to selection
    QVector<QPair<WeightVector*, WeightVector*>> parentsCouples =
            evofunc::adaptaSelection(m_population, m_populationSize - m_bestElementsToKeep - m_newRandomSamples);


    //crossover each couple and generate a new child
    for(qint32 i = 0; i < parentsCouples.size(); i++) {
        WeightVector newChild = m_crossoverFunction( *(parentsCouples.at(i).first), *(parentsCouples.at(i).second));
        m_mutationFunction(newChild, m_minWeight, m_maxWeight, m_mutationProbability);
        newPopulation.append(newChild);
    }

    m_population.clear();
    m_population = QVector(newPopulation);

    m_generationNumber++;
    m_isPopulationSorted = false;
    return true;
}



WeightVector EvolutionManager::getNthBestWeightVector(qint32 bestWeightVectorIndex) {
    if(!m_isPopulationSorted) {
        sortPopulationByFitness();
    }
    return m_population.at(bestWeightVectorIndex);
}


QString EvolutionManager::toQStringPopulation() {
    QString ret = "Population at generation " + QString::number(m_generationNumber) + ":\n";
    for(qint32 i=0; i < m_population.size()-1; i++) {
        ret += m_population[i].toQString() + ",\n";
    }
    ret += m_population[m_population.size()-1].toQString();
    return ret;
}



void EvolutionManager::sortPopulationByFitness() {
    std::sort(m_population.begin(), m_population.end(), WeightVector::isMoreFitThan);
    m_isPopulationSorted = true;
}







