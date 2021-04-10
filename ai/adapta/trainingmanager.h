#ifndef TRAININGMANAGER_H
#define TRAININGMANAGER_H

#include <QObject>
#include "game/gamemap.h"
#include "ai/genetic/weightvector.h"
#include "ai/genetic/evolutionmanager.h"

class TrainingManager : public QObject
{
    Q_OBJECT
public:
    TrainingManager() = delete;
    virtual ~TrainingManager() = default;

    static inline TrainingManager& instance() {
        return s_instance;
    }

    void readIni();
    void setupForMatch();
    WeightVector assignWeightVector();

public slots:
    void onVictory();

private:
    explicit TrainingManager(QObject *parent = nullptr);
    EvolutionManager m_evolutionManager;

    qint32 m_currWVIndex;
    qint32 m_currWVMatchNumber;
    qint32 m_matchNumberTarget;
    qint32 m_elitismDegree;
    qint32 m_randomismDegree;

    bool m_isEvoManagerInitialized;

    static TrainingManager s_instance;
    static bool s_isInstanceInitialized;

    void initializeEvolutionManager();
    void advanceMatchCount();
    void evaluateFitnessOfCurrentWV();


signals:

};

#endif // TRAININGMANAGER_H
