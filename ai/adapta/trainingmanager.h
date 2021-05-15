#ifndef TRAININGMANAGER_H
#define TRAININGMANAGER_H

#include <QObject>
#include "game/gamemap.h"
#include "ai/genetic/weightvector.h"
#include "ai/genetic/evolutionmanager.h"
#include "ai/adapta/AdaptaEnums.h"

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

    /**
     * @brief get the vector to be used by the trainee on this round
     */
    WeightVector getAssignedWeightVector();

    void stopTraining();

    /**
      * @brief if set <= 0, use the weight vector length found in ini file as usual (this is the default),
      * if > 0 use this length instead
      */
    void requestWVLength(qint32 requestedWVLength);

public slots:
    /**
     * @brief catch when there's a victory to evaluate current trainee's performance
     */
    void onVictory();

    void setMatchNumberTarget(qint32 targetMatches);

    void setTrainingPlayerIndex(qint32 trainingPlayerIndex);


private:
    explicit TrainingManager(QObject *parent = nullptr);
    EvolutionManager m_evolutionManager;

    /**
     * @brief the index in the game map -> getPlayer(index), which indicates in the automation of the init.js where is the
     * player which will be evaluated
     */
    qint32 m_trainingPlayerIndex;
    std::vector<float> m_partialFitnesses;
    adaenums::evalType m_evaluationType{adaenums::evalType::VICTORY_COUNT_ONLY};
    bool m_continueTraining;

    QString m_iniFileName;
    QString m_populationFileName;
    QString m_saveNameBestRecords;

    //params to know when to save
    //save population each time a new best fit vector appears?
    bool m_saveOnNewBestFit;
    float m_currBestSavedFitness;
    //save population each N generations have passed since the last save (which can occur before if the previous bool is set to true)
    //<= 0 to never save automatically after N generations
    qint32 m_genNumberTargetSave;
    qint32 m_currGenNumberCount;
    //save parameters for best records of all time
    qint32 m_bestRecordsToSave;


    //counters for matches and vectors, to know which vector at each match should be used and when it's going to be evaluated
    qint32 m_currWVIndex;
    qint32 m_currWVMatchNumber;
    qint32 m_matchNumberTarget;
    //how many gen at most the training must do. If negative, go infintely
    qint32 m_maxGenerationCount;
    qint32 m_totalGenCount;

    bool m_isTrainingManagerInitialized;

    /**
      @brief if this is <= 0, use the weight vector length found in ini file, if > 0 use this length instead.
      Can be requested through a method
      */
    qint32 m_requestedWVLength {-1};

    static TrainingManager s_instance;
    static bool s_isInstanceInitialized;

    void initializeEvolutionManager();
    /**
     * @brief set the reference of this training manager to the init.js file
     */
    void initializeInitJsLink();
    void advanceMatchCount();
    /**
     * @brief evaluate and add the partial fitness based on trainee performances for this match
     */
    void evaluatePartialFitnessOfThisMatch();
    /**
     * @brief evaluate the weight vector, once all partial fitnesses are set
     */
    void evaluateFitnessOfCurrentWV();

    void saveIfProgress();
};

#endif // TRAININGMANAGER_H
