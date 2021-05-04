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
    WeightVector();
    WeightVector(qint32 size);
    WeightVector(std::vector<float> weights);

    WeightVector(const WeightVector &other) {
        m_fitness = other.m_fitness;
        m_weights = std::vector<float>(other.m_weights);
    }

    WeightVector(WeightVector &&other){
        m_fitness = other.m_fitness;
        other.m_fitness = 0.0f;
        m_weights = other.m_weights;
        other.m_weights.clear();
    }

    WeightVector& operator=(const WeightVector &other) {
        m_fitness = other.m_fitness;
        m_weights = std::vector<float>(other.m_weights);
        return *this;
    }

    ~WeightVector() = default;

    //member access opt
    inline float operator[](qint32 index) {
        return m_weights[index];
    }
    //comparison opts
    bool operator==(const WeightVector &other) const;
    inline bool operator!=(const WeightVector &other) const {
        return !operator==(other);
    }


    inline qint32 size() const {
        return m_weights.size();
    }

    inline void overwriteWeight(qint32 index, float newWeight) {
        m_weights[index] = newWeight;
    }

    void overwrite(QVector<float> newWeightVector);

    inline float at(qint32 index) const {
        return m_weights.at(index);
    }

    inline void setAt(float value, qint32 index) {
        m_weights[index] = value;
    }

    void setFitness(float fitness);

    inline float getFitness() const {
        return m_fitness;
    }

    std::vector<float> getVector() const;

    inline static bool isLessFitThan(WeightVector wv1, WeightVector wv2) {
        return wv1.m_fitness < wv2.m_fitness;
    }

    inline static bool isMoreFitThan(WeightVector wv1, WeightVector wv2) {
        return wv1.m_fitness > wv2.m_fitness;
    }

    inline static bool isLessFitThanFitness(WeightVector wv, float fitness) {
        return wv.m_fitness < fitness;
    }


    inline static bool isMoreFitThanFitness(WeightVector wv, float fitness) {
        return wv.m_fitness > fitness;
    }

    QString toQString();

    void writeToJson(QJsonObject &json) const;
    void readFromJson(const QJsonObject &json);

    static WeightVector generateFromJson(const QJsonObject &json);
    static WeightVector generateRandomWeightVector(qint32 size, float minWeight, float maxWeight);

private:
    std::vector<float> m_weights;
    float m_fitness{0};

    static QRandomGenerator random;
};

#endif // WEIGHTVECTOR_H
