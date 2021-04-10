#ifndef EVOLUTIONMANAGER_H
#define EVOLUTIONMANAGER_H

#include "ai/genetic/EvoEnums.h"
#include "ai/genetic/weightvector.h"
#include <QVector>



/**
 * @brief The EvolutionManager class creates and/or manages a population of weights Vectors.
 */
class EvolutionManager
{
public:
    EvolutionManager();

    EvolutionManager(qint32 weightVectorLength, qint32 populationSize, float minWeight, float maxWeight,
                     evoenums::CrossoverType crossoverType = evoenums::CrossoverType::mixRandom, float mutationProbability = 0.05f);


    /**
     * @brief createRandomPopulation create a new population with specified parameters. This remove the old population, if any
     * @param weightVectorLength
     * @param populationSize
     * @param minWeight
     * @param maxWeight
     */
    void createRandomPopulation(qint32 weightVectorLength, qint32 populationSize, float minWeight, float maxWeight);

    /**
     * @brief createRandomPopulation create a new population with specified parameters. Population parameters are assumed
     * to be set beforehand the call to this function. If not use the overloaded version. This remove the old population, if any
     */
    void createRandomPopulation();

    void resetPopulation();
    void setPopulation(QVector<WeightVector> newPopulation);
    QVector<WeightVector>& getPopulation();

    inline qint32 getPopulationSize() {
        return m_populationSize;
    }

    void resetGeneration();
    qint32 getGeneration();

    /**
     * @brief setElitismDegree how many of the best elements, across all evolution, should be kept. Best elements are copied
     * in the next gen, so If this equals the population's size then it's trivial the evolution since it's only a copy of the
     * last population. Default is 1
     * @param bestElementsToKeep
     */
    void setElitismDegree(qint32 bestElementsToKeep);

    /**
     * @brief setRandomismDegree set how many new totally random samples should be generated at each new generaion,
     * ignoring selection and crossover. Default is 0 since is not a general wanted behaviour.
     * It should be a low number wrt the population size
     * @param newRandomSamples
     */
    void setRandomismDegree(qint32 newRandomSamples);

    /**
     * @brief maximizeFitness this indicates to the evo manager that better fitnesses are higher ones. This is the default
     */
    void maximizeFitness();

    /**
     * @brief maximizeFitness this indicates to the evo manager that better fitnesses are lower ones. By default is the opposite
     */
    void minimizeFitness();

    /**
     * @brief setFitnessFunction Set the new fitness function used by this evolution manager.
     * This MUST be provided since
     */
    void setFitnessFunction(float (*fitnessFunction)(WeightVector weightVector));
    /**
     * @brief setCrossoverFunctionType set the type of the crossover among the ones in the enum. If 'custom' is selected, make sure
     * to set the function with the setCrossoverFunction(...) function
     * @param crossoverType
     */
    void setCrossoverFunctionType(evoenums::CrossoverType crossoverType);

    /**
     * @brief setCrossoverFunction set the crossover fct for this evolution manager. The crossover fct works with 2 Vectors of weights and
     * possibly a 3rd used as a mask.
     * By setting a crossover function, this fct will be used by default, except if specified with false on setThisAsCrossoverFunction
     * @param setThisAsCrossoverFunction default is true. If false the set function will be used only if the type of crossover function
     * is set to custom
     */
    void setCrossoverFunction(WeightVector (*crossoverFunction) (WeightVector weightVector_1, WeightVector weightVector_2),
                              bool setThisAsCrossoverFunction = true);

    /**
     * @brief setMutationFunction the mutation func used to mutate a weights Vector
     */
    void setMutationFunction(void (*mutationFunction) (WeightVector& weightVector, float minWeight, float maxWeight, float probability));

    /**
     * @brief canStartEvolutionProcess
     * @return true if this evo manager can start the auto evolving process
     */
    bool canStartEvolution();

    /**
     * @brief startEvolution start the evolving process. This works only if everything is set up (all functions and parameters).
     * Call canStartEvolution() first to check if it can start
     * @param fitnessTreshold
     * @param maximizeFitness
     * @param maxGenerations
     */
    void startEvolution(float fitnessTreshold, bool maximizeFitness, qint32 maxGenerations = infinite);

    /**
     * @brief canPerformOneEvolutionStep see performOneEvolutionStep
     */
    bool canPerformOneEvolutionStep();

    /**
     * @brief performOneEvolutionStep perform only one step of evolution, passing to the new generation.
     * @return true if it performed successfully the evolution. It does if it has set all needed functions.
     * Use canPerformOneEvolutionStep to check it.
     */
    bool performOneEvolutionStep();

    /**
     * @brief getNthBestWeightVector get the Nth best vector according to the fitness function.
     * @param bestWeightVectorIndex The index, from 0 to the size of the population - 1
     * @return
     */
    WeightVector getNthBestWeightVector(qint32 bestWeightVectorIndex);

    QString toQStringPopulation();

private:

    static const qint32 infinite;

    evoenums::CrossoverType m_crossoverType{evoenums::CrossoverType::splitMiddle};

    /**
     * @brief m_population all vectors of weights representing the current population
     */
    QVector<WeightVector> m_population;

    qint32 m_populationSize;
    qint32 m_weightVectorLength;
    qint32 m_bestElementsToKeep{1};
    qint32 m_newRandomSamples{0};
    qint32 m_generationNumber{0};
    float m_minWeight{0};
    float m_maxWeight{1};
    float m_mutationProbability;

    bool m_isPopulationSorted{false};
    bool m_fitnessToMaximize{true};

    float (*m_fitnessFunction)(WeightVector weightVector) {nullptr};
    WeightVector (*m_crossoverFunction) (WeightVector weightVector_1, WeightVector weightVector_2);
    void (*m_mutationFunction) (WeightVector& weightVector, float minWeight, float maxWeight, float probability);
    QPair<WeightVector, WeightVector> (*m_selectionFunction) (QVector<WeightVector>& population);


    void sortPopulationByFitness();
};

#endif // EVOLUTIONMANAGER_H
