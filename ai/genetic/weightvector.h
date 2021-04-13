#ifndef WEIGHTVECTOR_H
#define WEIGHTVECTOR_H

#include <QVector>
#include <QRandomGenerator>
#include <QJsonObject>

/**
 * @brief The WeightVector class is just a vector of weights with a couple of extra functions for evolutionary algorithm
 */
class WeightVector
{
public:
    WeightVector() = delete;
    WeightVector(qint32 size);
    WeightVector(QVector<float> weights);

    WeightVector(const WeightVector &other) {
        m_fitness = other.m_fitness;
        m_weights = QVector(other.m_weights);
    }

    WeightVector(WeightVector &&other){
        m_fitness = other.m_fitness;
        other.m_fitness = 0.0f;
        m_weights = other.m_weights;
        other.m_weights.clear();
    }

    WeightVector& operator=(const WeightVector &other) {
        m_fitness = other.m_fitness;
        m_weights = QVector(other.m_weights);
        return *this;
    }

    ~WeightVector() = default;

    float operator[](qint32 index);

    inline qint32 size() const {
        return m_weights.size();
    }

    void overwriteWeight(qint32 index, float newWeight);

    void overwrite(QVector<float> newWeightVector);

    inline float at(qint32 index) const {
        return m_weights.at(index);
    }

    void setFitness(float fitness);

    inline float getFitness() const {
        return m_fitness;
    }

    QVector<float> getQVector() const;

    inline static bool isLessFitThan(WeightVector wv1, WeightVector wv2) {
        return wv1.m_fitness < wv2.m_fitness;
    }

    inline static bool isMoreFitThan(WeightVector wv1, WeightVector wv2) {
        return wv1.m_fitness > wv2.m_fitness;
    }

    QString toQString();

    void writeToJson(QJsonObject &json) const;
    void readFromJson(const QJsonObject &json);

    static WeightVector generateFromJson(const QJsonObject &json);
    static WeightVector generateRandomWeightVector(qint32 size, float minWeight, float maxWeight);

private:
    QVector<float> m_weights;
    float m_fitness{0};

    static QRandomGenerator random;
};

#endif // WEIGHTVECTOR_H