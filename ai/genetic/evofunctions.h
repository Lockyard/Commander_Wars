#ifndef EVOFUNCTIONS_H
#define EVOFUNCTIONS_H

#include <QVector>
#include <QPair>
#include "ai/genetic/weightvector.h"
#include "ai/genetic/EvoEnums.h"

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
     * @brief individualRandomMutation this mutates each weight in the weight vector with given probability, setting a
     * new float uniformly random from min to max weight
     */
    void individualRandomMutation(WeightVector& weightVector, float minWeight, float maxWeight, float probability);


    typedef void (*mutationFuncPtr)(WeightVector& weightVector, float minWeight, float maxWeight, float probability);

    mutationFuncPtr getMutationFunctionFromType(evoenums::MutationType type, mutationFuncPtr defaultMutationFuncPtr = individualRandomMutation);


    //OTHER FUNCTIONS /////////////////
    /**
     * @brief adaptaSelection extract 2 (different) parents from the population, with a custom algorithm
     * @param population
     * @return the pair of selected weight vectors
     */
    QVector<QPair<WeightVector*, WeightVector*>> adaptaSelection(QVector<WeightVector>& population, qint32 couplesToReturn);

    /**
     * @brief generateCustomWeightedFitnesses generate a vector of custom fitnesses based on real fitnesses
     * This weights more higher fitnesses and equalize also negative fitnesses, if they go from -10 to +10
     * @param population
     * @return
     */
    QVector<float> generateCustomWeightedFitnesses(QVector<WeightVector>& population);
}



#endif // EVOFUNCTIONS_H
