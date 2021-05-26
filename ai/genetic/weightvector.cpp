#include "weightvector.h"
#include <QTime>
#include <QJsonArray>

QRandomGenerator WeightVector::random = QRandomGenerator(QTime::currentTime().msecsSinceStartOfDay());

WeightVector::WeightVector():m_weights() {}

WeightVector::WeightVector(qint32 size)
{
    m_weights.reserve(size);
}

WeightVector::WeightVector(std::vector<float> weights) {
    m_weights = weights;
}




bool WeightVector::operator==(const WeightVector &other) const {
    if(m_weights.size() != other.m_weights.size())
        return false;
    for(quint32 i = 0; i < m_weights.size(); i++) {
        if(m_weights[i] != other.m_weights[i])
            return false;
    }
    return true;
}



void WeightVector::overwrite(QVector<float> newWeightVector) {
    m_weights.clear();
    m_weights = std::vector<float>(newWeightVector.begin(), newWeightVector.end());
}


void WeightVector::setFitness(float fitness) {
    m_fitness = fitness;
}


std::vector<float> WeightVector::getVector() const {
    return m_weights;
}


QString WeightVector::toQString() {
    QString ret = "[";
    for(quint32 i = 0; i < m_weights.size()-1; i++) {
        ret += QString::number(m_weights[i], 'f', 3) + ", ";
    }
    ret += QString::number(m_weights[m_weights.size()-1], 'f', 3) + "]";
    return ret;
}


QString WeightVector::toQStringFull() {
    QString ret = "(fitness: " + QString::number(m_fitness) +") [";
    for(quint32 i = 0; i < m_weights.size()-1; i++) {
        ret += QString::number(m_weights[i], 'f', 3) + ", ";
    }
    ret += QString::number(m_weights[m_weights.size()-1], 'f', 3) + "]";
    return ret;
}


void WeightVector::writeToJson(QJsonObject &json) const {
    json["fitness"] = m_fitness;
    QJsonArray weightsArray;
    for(quint32 i = 0; i < m_weights.size(); i++) {
        weightsArray.append(m_weights.at(i));
    }
    json["weights"] = weightsArray;
}


void WeightVector::readFromJson(const QJsonObject &json) {
    if (json.contains("fitness") && json["fitness"].isDouble())
            m_fitness = static_cast<float>(json["fitness"].toDouble());

    if(json.contains("weights") && json["weights"].isArray()) {
        m_weights.clear();
        QJsonArray weightsArray = json["weights"].toArray();
        for(qint32 weightIndex=0; weightIndex<weightsArray.size(); weightIndex++) {
            m_weights.push_back(static_cast<float>(weightsArray[weightIndex].toDouble()));
        }
    }
}

WeightVector WeightVector::generateFromJson(const QJsonObject &json) {
    float fitness = 0;
    std::vector<float> weights;

    if (json.contains("fitness") && json["fitness"].isDouble())
            fitness = static_cast<float>(json["fitness"].toDouble());

    if(json.contains("weights") && json["weights"].isArray()) {
        QJsonArray weightsArray = json["weights"].toArray();
        for(qint32 weightIndex=0; weightIndex<weightsArray.size(); weightIndex++) {
            weights.push_back(static_cast<float>(weightsArray[weightIndex].toDouble()));
        }
    }

    WeightVector res(weights);
    res.setFitness(fitness);

    return res;
}



WeightVector WeightVector::generateRandomWeightVector(qint32 size, float minWeight, float maxWeight) {
    std::vector<float> weights;
    weights.reserve(size);

    if(minWeight > maxWeight) {
        float tmp = minWeight;
        minWeight = maxWeight;
        maxWeight = tmp;
    }

    float randBound = maxWeight - minWeight;

    //generate N floats from minWeight to maxWeight
    for(qint32 i = 0; i < size; i++) {
        weights.push_back(random.bounded(randBound) + minWeight);
    }

    return WeightVector(weights);
}
