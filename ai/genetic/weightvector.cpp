#include "weightvector.h"
#include <QTime>

QRandomGenerator WeightVector::random = QRandomGenerator(QTime::currentTime().msecsSinceStartOfDay());

WeightVector::WeightVector(qint32 size)
{
    m_weights.reserve(size);
}

WeightVector::WeightVector(QVector<float> weights) {
    m_weights = QVector(weights);
}

float WeightVector::operator[](qint32 index) {
    return m_weights[index];
}

qint32 WeightVector::size() {
    return m_weights.size();
}

void WeightVector::overwriteWeight(qint32 index, float newWeight) {
    m_weights[index] = newWeight;
}

void WeightVector::overwrite(QVector<float> newWeightVector) {
    m_weights.clear();
    m_weights.append(newWeightVector);
}


void WeightVector::setFitness(float fitness) {
    m_fitness = fitness;
}


QVector<float> WeightVector::getQVector() {
    return m_weights;
}


QString WeightVector::toQString() {
    QString ret = "[";
    for(qint32 i = 0; i < m_weights.size()-1; i++) {
        ret += QString::number(m_weights[i]) + ", ";
    }
    ret += QString::number(m_weights[m_weights.size()-1]) + "]";
    return ret;
}

WeightVector WeightVector::generateRandomWeightVector(qint32 size, float minWeight, float maxWeight) {
    QVector<float> weights;
    weights.reserve(size);

    if(minWeight > maxWeight) {
        float tmp = minWeight;
        minWeight = maxWeight;
        maxWeight = tmp;
    }

    float randBound = maxWeight - minWeight;

    //generate N floats from minWeight to maxWeight
    for(qint32 i = 0; i < size; i++) {
        weights.append(random.bounded(randBound) + minWeight);
    }

    return WeightVector(weights);
}
