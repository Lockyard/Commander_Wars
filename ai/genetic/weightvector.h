#ifndef WEIGHTVECTOR_H
#define WEIGHTVECTOR_H

#include <QVector>
#include <QRandomGenerator>

/**
 * @brief The WeightVector class is just a vector of weights with a couple of extra functions for evolutionary algorithm
 */
class WeightVector
{
public:
    WeightVector() = delete;
    WeightVector(qint32 size);
    WeightVector(QVector<float> weights);
    ~WeightVector() = default;

    float operator[](qint32 index);

    qint32 size();

    void overwriteWeight(qint32 index, float newWeight);

    void overwrite(QVector<float> newWeightVector);

    inline float at(qint32 index) {
        return m_weights.at(index);
    }

    void setFitness(float fitness);

    inline float getFitness() const {
        return m_fitness;
    }

    QVector<float> getQVector();

    inline static bool isLessFitThan(WeightVector wv1, WeightVector wv2) {
        return wv1.m_fitness < wv2.m_fitness;
    }

    inline static bool isMoreFitThan(WeightVector wv1, WeightVector wv2) {
        return wv1.m_fitness > wv2.m_fitness;
    }

    QString toQString();

    static WeightVector generateRandomWeightVector(qint32 size, float minWeight, float maxWeight);

private:
    QVector<float> m_weights;
    float m_fitness{0};

    static QRandomGenerator random;
};

#endif // WEIGHTVECTOR_H
