#ifndef EVOFUNCTIONS_H
#define EVOFUNCTIONS_H

#include <QVector>
#include <QPair>
#include "ai/genetic/weightvector.h"
#include "ai/genetic/EvoEnums.h"
#include "ai/genetic/evolutionmanager.h"

namespace evofunc {

    //CROSSOVER FUNCTIONS ///////////

    //default crossover functions
    WeightVector splitMiddleCrossoverFct(WeightVector weightVector_1, WeightVector weightVector_2);
    WeightVector splitRandomCrossoverFct(WeightVector weightVector_1, WeightVector weightVector_2);

    /**
     * @brief mixRandomCrossoverFct create a new vector deciding at each postition to get the weight of vector 1 or 2
     */
    WeightVector mixRandomCrossoverFct(WeightVector weightVector_1, WeightVector weightVector_2);

    //MUTATION FUNCTIONS ////////////////////

    /**
     * @brief individualRandomMutation this mutates each weight in the weight vector with given probability if the mask on that
     * position has a false, setting a new float uniformly random from min to max weight specified in that position.
     * weightVector and all masks must have same size
     */
    void individualRandomMutation(WeightVector& weightVector, std::vector<bool> &fixedWeightMask, std::vector<float> &minWeightMask,
                                               std::vector<float> &maxWeightMask, float probability);


    typedef void (*mutationFuncPtr)(WeightVector& weightVector, std::vector<bool> &fixedWeightMask, std::vector<float> &minWeightMask, std::vector<float> &maxWeightMask, float probability);

    mutationFuncPtr getMutationFunctionFromType(evoenums::MutationType type, mutationFuncPtr defaultMutationFuncPtr = individualRandomMutation);


    //OTHER FUNCTIONS /////////////////
    /**
     * @brief adaptaSelection extract 2 (different) parents from the population, with a custom algorithm
     * @param population
     * @return the pair of selected weight vectors
     */
    QVector<QPair<WeightVector*, WeightVector*>> adaptaSelection(QVector<WeightVector>& population, qint32 couplesToReturn, float minFitness, float maxFitness);

    /**
     * @brief generateCustomWeightedFitnesses generate a vector of custom fitnesses based on real fitnesses
     * This weights more higher fitnesses and equalize also negative fitnesses, if they go from -10 to +10
     * @param population
     * @return
     */
    QVector<float> generateCustomWeightedFitnesses(QVector<WeightVector>& population, float minFitness, float maxFitness);

    evoenums::TransferLearningType transferLearningTypeFromString(QString typeString);


    /**
     * @brief applyTransferLearning applies transfer learning based on the type given, using the given files as reference
     * (origin is the one to be read which loads a certain configuration, destination is the one that loads the target configuration).
     * If fixTransferredWeights is true, the assigned weights cannot change and be learned during training
     */
    void applyTransferLearning(EvolutionManager &evoManager, evoenums::TransferLearningType type, QString originFile, QString destinationFile, bool fixTransferredWeights);

    //todo if really needed
    //std::pair<std::vector<float>, std::vector<float>> getMinMaxWeightMasksFromType(evoenums::MinMaxWeightMaskType, QString file, float defaultMinWeight = -10, float defaultMaxWeight = 10);
}



#endif // EVOFUNCTIONS_H
