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

    void loadIni(QString filename);
    /**
     * @brief saveState create a save of the current population. Info of backup are read on a ini file specified.
     * Also update the state that will be read from the ini file
     * @param iniFilename
     */
    void saveState(QString iniFilename);

    void setupForMatch();
    WeightVector assignWeightVector();

    void stopTraining();

public slots:
    void onVictory();

private:
    explicit TrainingManager(QObject *parent = nullptr);
    EvolutionManager m_evolutionManager;

    bool m_continueTraining;

    QString m_iniFileName;
    QString m_populationFileName;

    //params to know when to save
    //save population each time a new best fit vector appears?
    bool m_saveOnNewBestFit;
    float m_currBestSavedFitness;
    //save population each N generations have passed since the last save (which can occur before if the previous bool is set to true)
    //<= 0 to never save automatically after N generations
    qint32 m_genNumberTargetSave;
    qint32 m_currGenNumberCount;


    //counters for matches and vectors, to know which vector at each match should be used and when it's going to be evaluated
    qint32 m_currWVIndex;
    qint32 m_currWVMatchNumber;
    qint32 m_matchNumberTarget;
    //how many gen at most the training must do. If negative, go infintely
    qint32 m_maxGenerationCount;
    qint32 m_totalGenCount;

    bool m_isEvoManagerInitialized;

    static TrainingManager s_instance;
    static bool s_isInstanceInitialized;

    void initializeEvolutionManager();
    void advanceMatchCount();
    void evaluateFitnessOfCurrentWV();

    void saveIfProgress();
};

#endif // TRAININGMANAGER_H
