#include "evofunctions.h"
#include <QRandomGenerator>
#include <QTime>
#include <QtMath>
#include "coreengine/console.h"
#include "weightvector.h"



namespace evofunc {

    WeightVector splitMiddleCrossoverFct(WeightVector weightVector_1, WeightVector weightVector_2) {
        WeightVector res = WeightVector(weightVector_1);
        qint32 minSize = weightVector_1.size() < weightVector_2.size() ? weightVector_1.size() : weightVector_2.size();

        for(int i = weightVector_1.size()/2; i < minSize; i++) {
            res.overwriteWeight(i, weightVector_2.at(i));
        }

        return res;
    }


    WeightVector splitRandomCrossoverFct(WeightVector weightVector_1, WeightVector weightVector_2) {
        WeightVector res = WeightVector(weightVector_1);
        QRandomGenerator random = QRandomGenerator(QTime::currentTime().msecsSinceStartOfDay());
        qint32 minSize = weightVector_1.size() < weightVector_2.size() ? weightVector_1.size() : weightVector_2.size();
        qint32 randIndex = random.bounded(0, minSize);

        for(int i = randIndex; i < minSize; i++) {
            res.overwriteWeight(i, weightVector_2.at(i));
        }

        return res;
    }


    WeightVector mixRandomCrossoverFct(WeightVector weightVector_1, WeightVector weightVector_2) {

        QRandomGenerator random = QRandomGenerator(QTime::currentTime().msecsSinceStartOfDay());
        qint32 minSize = weightVector_1.size() < weightVector_2.size() ? weightVector_1.size() : weightVector_2.size();
        WeightVector res = WeightVector(std::vector<float>(minSize));

        //on each position get randomly an item from 1 or 2
        for(qint32 i = 0; i < minSize; i++) {
            res.overwriteWeight(i, random.generateDouble() >= .5 ? weightVector_1.at(i) : weightVector_2.at(i));
        }

        return res;
    }

    mutationFuncPtr getMutationFunctionFromType(evoenums::MutationType type, mutationFuncPtr defaultMutationFuncPtr) {
        switch(type) {
        case evoenums::MutationType::individualRandom:
            return individualRandomMutation;
        default:
            return defaultMutationFuncPtr;
        }
    }


    //default mutation
    void individualRandomMutation(WeightVector& weightVector, float minWeight, float maxWeight, float probability) {
        QRandomGenerator random = QRandomGenerator(QTime::currentTime().msecsSinceStartOfDay());
        for(qint32 i = 0; i < weightVector.size(); i++) {
            if(random.generateDouble() < probability) {
                weightVector.overwriteWeight(i, minWeight + static_cast<float>(random.generateDouble()*(maxWeight - minWeight)));
            }
        }
    }


    QVector<QPair<WeightVector*, WeightVector*>> adaptaSelection(QVector<WeightVector>& population, qint32 couplesToReturn, float minFitness, float maxFitness) {
        QVector<float> customFitnesses(population.size());
        float totalFitness(0.0f);
        QVector<QPair<WeightVector*, WeightVector*>> selectedCouples;
        selectedCouples.reserve(couplesToReturn);

        customFitnesses = generateCustomWeightedFitnesses(population, minFitness, maxFitness);
        totalFitness = std::accumulate(customFitnesses.begin(), customFitnesses.end(), 0.0f);

        //log chances of being selected
        if(Console::getLogLevel() <= Console::eDEBUG && totalFitness != 0.0f) {
            for(qint32 i=0; i<customFitnesses.size(); i++) {
                Console::print("Vector " + QString::number(i) + " has " + QString::number(customFitnesses[i]/totalFitness*100.0f, 'f', 3) +
                               "% chance of being chosen", Console::eDEBUG);
            }
        }

        QRandomGenerator random = QRandomGenerator(QTime::currentTime().msecsSinceStartOfDay());
        double randomTarget = 0;
        qint32 firstParentIndex = -1;
        qint32 secondParentIndex = -1;
        //generate N couples of parents to return;
        for(qint32 i = 0; i < couplesToReturn; i++) {
            QPair<WeightVector*, WeightVector*> parentsPair{};

            //take the 1st parent randomly weighted on custom fitnesses
            randomTarget = random.bounded(totalFitness);

            for(qint32 j = 0; j < customFitnesses.size(); j++) {
                randomTarget -= customFitnesses.at(j);
                if(randomTarget < 0 || j == customFitnesses.size()-1) {
                    firstParentIndex = j;
                    break;
                }
            }

            //take the 2nd parent, weighted as well but excluding the first parent
            randomTarget = random.bounded(totalFitness - customFitnesses.at(firstParentIndex));

            for(qint32 j = 0; j < customFitnesses.size(); j++) {
                if(j == firstParentIndex) //skip the check if is the first parent
                    continue;
                randomTarget -= customFitnesses.at(j);
                if(randomTarget < 0 || j == customFitnesses.size()-1 ||
                        (j == customFitnesses.size() - 2 && firstParentIndex == customFitnesses.size() - 1)) {
                    secondParentIndex = j;
                    break;
                }
            }

            //Console::print("Adapta selection of parents " + QString::number(firstParentIndex) + " and " + QString::number(secondParentIndex), Console::eDEBUG);
            parentsPair.first = &(population[firstParentIndex]);
            parentsPair.second = &(population[secondParentIndex]);

            selectedCouples.append(parentsPair);
        }

        return selectedCouples;
    }


    QVector<float> generateCustomWeightedFitnesses(QVector<WeightVector>& population, float minFitness, float maxFitness) {
        if(population.isEmpty())
            return QVector<float>();

        QVector<float> customFitnesses(population.size(), 1.0f);
        float fitnessMultiplier;
        //if min and max fitnesses are the same doesn't make sense, but return all equal custom fitnesses to have equal chance of being selected
        if(maxFitness == minFitness) {
            Console::print("While generating custom weights, min and max fitnesses are the same! Can't select properly!", Console::eWARNING);
            return customFitnesses;
        }
        //normal case
        else
            fitnessMultiplier = 1.0f/(maxFitness - minFitness);

        //generate custom positive fitnesses from the original fitnesses, which go from minFitness to MaxFitness
        for(qint32 i=0; i < population.size(); i++) {
            float fitness = population.at(i).getFitness();
            fitness = qMin(maxFitness - minFitness, qMax(0.0f, fitness - minFitness))*fitnessMultiplier;
            fitness = static_cast<float>(qPow(fitness, 4));
            customFitnesses[i] = fitness;
        }

        return customFitnesses;
    }
}
