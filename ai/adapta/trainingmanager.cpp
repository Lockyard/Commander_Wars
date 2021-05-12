#include "trainingmanager.h"
#include "coreengine/console.h"
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "ai/genetic/evofunctions.h"

TrainingManager::TrainingManager(QObject *parent) : QObject(parent), m_continueTraining(true), m_currGenNumberCount(0),
    m_totalGenCount(0), m_isEvoManagerInitialized(false)
    {
}

bool TrainingManager::s_isInstanceInitialized = false;

TrainingManager TrainingManager::s_instance(nullptr);


void TrainingManager::loadIni(QString filename) {
    if (QFile::exists(filename))
    {
        m_iniFileName = filename;
        QSettings settings(filename, QSettings::IniFormat);

        //load stuff for the training manager
        settings.beginGroup("TrainingInfo");
        bool ok = false;
        bool loadPopulation = false;
        m_matchNumberTarget = settings.value("MatchNumberTarget", 10).toInt(&ok);
        if(!ok)
            m_matchNumberTarget = 10;
        loadPopulation = settings.value("LoadPopulation", false).toBool();
        m_partialFitnesses.reserve(m_matchNumberTarget);
        m_populationFileName = settings.value("PopulationFileName", "resources/aidata/adapta/population.json").toString();
        m_maxGenerationCount = settings.value("MaxGenerationCount", 1000).toInt(&ok);
        if(!ok)
            m_maxGenerationCount = 1000;
        m_evaluationType = adaenums::getEvalTypeFromString(settings.value("EvaluationType", "VICTORY_COUNT_ONLY").toString());
        m_trainingPlayerIndex = settings.value("TrainingPlayer", 0).toInt(&ok);
        if(!ok) {
            Console::print("Training Manager didn't found an index to follow a player in INI file! Defaulted to 0!", Console::eWARNING);
            m_trainingPlayerIndex = 0;
        }

        settings.endGroup();

        settings.beginGroup("SaveInfo");
        m_saveOnNewBestFit = settings.value("SaveOnNewBestFit", true).toBool();
        m_genNumberTargetSave = settings.value("GenerationNumberTargetSave", -1).toInt(&ok);
        if(!ok)
            m_genNumberTargetSave = 10;
        m_bestRecordsToSave = settings.value("BestRecordsToSave", 3).toInt(&ok);
        if(!ok)
            m_bestRecordsToSave = 3;
        m_saveNameBestRecords = settings.value("SaveNameBestRecords", "resources/aidata/adapta/best_records_alltime.json").toString();
        settings.endGroup();

        settings.beginGroup("CurrentState");
        m_currWVIndex = settings.value("CurrWVIndex", 0).toInt(&ok);
        if(!ok)
            m_currWVIndex = 0;
        m_currWVMatchNumber = settings.value("CurrWVMatchNumber", 0).toInt(&ok);
        if(!ok)
            m_currWVMatchNumber = 0;
        m_currBestSavedFitness = static_cast<float>(settings.value("CurrBestSavedFitness", 0.0f).toDouble(&ok));
        if(!ok)
            m_currBestSavedFitness = 0.0f;
        settings.endGroup();

        //load stuff for the evolution manager
        settings.beginGroup("EvolutionManager");
        qint32 populationSize, weightVectorLength, elitismDegree, randomismDegree, generation, crossoverType, mutationType;
        float minWeight, maxWeight;
        populationSize = settings.value("PopulationSize", 10).toInt(&ok);
        if(!ok)
            populationSize = 10;
        //set WV length only if not requested differently
        if(m_requestedWVLength <= 0) {
            weightVectorLength = settings.value("WeightVectorLength", 10).toInt(&ok);
            if(!ok)
                weightVectorLength = 10;
        } else {
            weightVectorLength = m_requestedWVLength;
        }
        elitismDegree = settings.value("ElitismDegree", 2).toInt(&ok);
        if(!ok)
            elitismDegree = 2;
        randomismDegree = settings.value("RandomismDegree", 1).toInt(&ok);
        if(!ok)
            randomismDegree = 1;
        generation = settings.value("Generation", 0).toInt(&ok);
        if(!ok)
            generation = 0;
        minWeight = static_cast<float>(settings.value("MinWeight", -10.0).toDouble(&ok));
        if(!ok)
            minWeight = -10.0f;
        maxWeight = static_cast<float>(settings.value("MaxWeight", 10.0).toDouble(&ok));
        if(!ok)
            maxWeight = 10.0f;
        settings.endGroup();
        crossoverType = settings.value("CrossoverFunctionType", 2).toInt(&ok);
        if(!ok)
            crossoverType = 2;
        mutationType = settings.value("MutationFunctionType", 0).toInt(&ok);
        if(!ok)
            mutationType = 0;


        m_evolutionManager.initialize(populationSize, weightVectorLength, minWeight, maxWeight, elitismDegree,
                                      randomismDegree, evoenums::CrossoverType(crossoverType));
        m_evolutionManager.setMutationFunction(evofunc::getMutationFunctionFromType(evoenums::MutationType(mutationType)));
        m_evolutionManager.setGeneration(generation);

        m_evolutionManager.setEliteRecordsNumber(m_bestRecordsToSave);

        bool loadOk = false;
        //load population if requested
        if(loadPopulation) {
            loadOk = m_evolutionManager.loadPopulationFromJsonFile(m_populationFileName);
        }
        //create random if requested or if load failed
        if(!loadPopulation || !loadOk) {
            Console::print("Creating random population", Console::eDEBUG);
            m_evolutionManager.createRandomPopulation();
        }
        m_evolutionManager.setMutationFunction(evofunc::individualRandomMutation);

    } else {
        Console::print("Training manager couldn't load ini file (" + filename + ")! Default values applied", Console::eWARNING);
        m_matchNumberTarget = 10;
        m_populationFileName = "resources/aidata/adapta/population.json";
        m_currWVIndex = 0;
        m_currWVMatchNumber = 0;

        m_evolutionManager.initialize(10, 10, -10.0f, 10.0f, 2, 1, evoenums::CrossoverType::mixRandom);
        m_evolutionManager.setGeneration(0);
        m_evolutionManager.setMutationFunction(evofunc::individualRandomMutation);

        Console::print("Creating random population", Console::eDEBUG);
        m_evolutionManager.createRandomPopulation();

    }
}


void TrainingManager::saveState(QString iniFilename) {
    Console::print("Training Manager is saving...", Console::eDEBUG);
    QSettings settings(iniFilename, QSettings::IniFormat);
    bool ok = false;

    //load backup infos
    settings.beginGroup("SaveInfo");
    qint32 saveIndex, maxSaveIndex;
    QString saveNamePrefix;
    QString saveNameExtension;
    QString saveNameMostRecent;
    QString saveNameBestRecords;
    saveIndex = settings.value("NextSaveIndex", 0).toInt(&ok);
    if(!ok)
        saveIndex = 0;
    maxSaveIndex = settings.value("MaxSaveIndex", 10).toInt(&ok);
    if(!ok)
        maxSaveIndex = 10;
    saveNamePrefix = settings.value("SaveNamePrefix", "resources/aidata/adapta/pop_record_").toString();
    saveNameExtension = settings.value("SaveNameExtension", ".json").toString();
    saveNameMostRecent = settings.value("SaveNameMostRecent", "resources/aidata/adapta/pop_record_last.json").toString();



    //save a population file in the correct backup file...
    m_populationFileName = saveNamePrefix + QString::number(saveIndex) + saveNameExtension;
    m_evolutionManager.savePopulationToJsonFile(m_populationFileName);
    settings.setValue("NextSaveIndex", saveIndex+1);
    settings.endGroup();
    //...and save also a copy on the file holding the most recent save
    m_evolutionManager.savePopulationToJsonFile(saveNameMostRecent);
    //Also save


    //update the file name to be load to the last one
    settings.beginGroup("TrainingInfo");
    settings.setValue("PopulationFileName", m_populationFileName);
    settings.endGroup();

    //save the state
    settings.beginGroup("CurrentState");
    settings.setValue("CurrWVIndex", m_currWVIndex);
    settings.setValue("CurrWVMatchNumber", m_currWVMatchNumber);
    settings.setValue("CurrBestSavedFitness", m_currBestSavedFitness);
    settings.endGroup();

    //save the ini file. doc says it not really necessary but just in case
    settings.sync();

    Console::print("Training Manager: save complete.", Console::eDEBUG);
}




void TrainingManager::setupForMatch() {
    if(!m_isEvoManagerInitialized)
        initializeEvolutionManager();

    //don't connect the event but let the init.js file call the method onVictory instead
    //GameMap* pMap = GameMap::getInstance();
    //connect(pMap->getGameRules(), &GameRules::signalVictory, this, &TrainingManager::onVictory, Qt::QueuedConnection);

}

WeightVector TrainingManager::getAssignedWeightVector() {
    Console::print("assigning vector " + m_evolutionManager.getPopulation()[m_currWVIndex].toQString(), Console::eDEBUG);
    return m_evolutionManager.getPopulation()[m_currWVIndex];
}

//relies on the automation of the init.js file (in templates folder in the repo folder, in base folder where the game is built)
void TrainingManager::stopTraining() {
    m_continueTraining = false;
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString object = "Init";
    QString func = "stopTraining";
    if (pInterpreter->exists(object, func))
    {
        QJSValueList args;
        QJSValue value = pInterpreter->newQObject(this);
        args << value;
        pInterpreter->doFunction(object, func, args);
    }
}


void TrainingManager::requestWVLength(qint32 requestedWVLength) {
    m_requestedWVLength = requestedWVLength;
}


//slots
void TrainingManager::onVictory() {
    evaluatePartialFitnessOfThisMatch();
    Console::print("Training: Finished match " + QString::number(m_currWVMatchNumber+1) + "/" +
                   QString::number(m_matchNumberTarget) + " of Weight vector " + QString::number(m_currWVIndex+1) + "/" +
                   QString::number(m_evolutionManager.getPopulationSize()), Console::eDEBUG);
    advanceMatchCount();
}


//private fncts


void TrainingManager::initializeEvolutionManager() {
    Interpreter::setCppOwnerShip(this);
    Mainapp* pApp = Mainapp::getInstance();
    this->moveToThread(pApp->getWorkerthread());
    //load ini also set some stuff for the evolution manager and loads or create its population
    loadIni("resources/aidata/adapta/training.ini");

    //now set to the init.js the training manager
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString object = "Init";
    QString func = "setupTrainingManager";
    if (pInterpreter->exists(object, func))
    {
        QJSValueList args;
        QJSValue value = pInterpreter->newQObject(this);
        args << value;
        pInterpreter->doFunction(object, func, args);
    }

    m_isEvoManagerInitialized = true;
}



void TrainingManager::advanceMatchCount() {
    //increase counter of match to select the correct weight to be tested
    m_currWVMatchNumber++;
    if(m_currWVMatchNumber == m_matchNumberTarget) {
        //current Weight Vector has finished its testing, calculate its real fitness
        evaluateFitnessOfCurrentWV();
        m_currWVMatchNumber = 0;

        //start testing the new weight vector
        m_currWVIndex++;
        //if there are no new WVs to test for this generation then evolve population
        if(m_currWVIndex == m_evolutionManager.getPopulationSize()) {

            //population are (eventually) saved BEFORE evolving, so here all vectors are evalued and sorted
            m_evolutionManager.sortPopulationByFitness();
            saveIfProgress();

            //now do evolution if training goes on
            if(m_continueTraining) {
                m_evolutionManager.performOneEvolutionStep();
                Console::print("Training manager evolved population (gen " +
                               QString::number(m_evolutionManager.getGeneration()) + "):\n" +
                               m_evolutionManager.toQStringPopulation(), Console::eDEBUG);
                //the first new WV to be examined is the first after the elite vectors, which were already tested
                m_currWVIndex = m_evolutionManager.getElitismDegree();
            }
        }
    }
}


void TrainingManager::evaluatePartialFitnessOfThisMatch() {
    float partialFitness = adaenums::calculatePartialFitnessFromType(m_evaluationType, m_trainingPlayerIndex);
    m_partialFitnesses.push_back(partialFitness);
    Console::print("Current trainee got a partial fitness of " + QString::number(partialFitness) +
                   " for this match", Console::eDEBUG);
}

void TrainingManager::evaluateFitnessOfCurrentWV() {
    float fitness = adaenums::calculateFinalFitnessFromType(m_evaluationType, m_partialFitnesses);
    m_evolutionManager.getPopulation()[m_currWVIndex].setFitness(fitness);
    Console::print("Vector " + QString::number(m_currWVIndex+1) + "/" +
                   QString::number(m_evolutionManager.getPopulationSize()) +
                   " got fitness of " + QString::number(fitness, 'f', 8) + " (gen: " + QString::number(m_evolutionManager.getGeneration()) + ")\n" +
                   m_evolutionManager.getPopulation()[m_currWVIndex].toQString(), Console::eDEBUG);
    m_partialFitnesses.clear();
}


void TrainingManager::saveIfProgress() {
    m_currGenNumberCount++;
    m_totalGenCount++;
    //if we reached the target generations of evolutions, save and stop training
    if(m_totalGenCount >= m_maxGenerationCount) {
        saveState(m_iniFileName);
        stopTraining();
    }
    //else save if there's a new best fitness (if option enabled)
    else if(m_saveOnNewBestFit && (m_evolutionManager.getNthBestWeightVector(0).getFitness() > m_currBestSavedFitness)) {
        m_currBestSavedFitness = m_evolutionManager.getNthBestWeightVector(0).getFitness();
        m_currGenNumberCount = 0;
        saveState(m_iniFileName);
        //else save if the defined number of generations has passed (if option enabled)
    } else if(m_genNumberTargetSave > 0 && m_currGenNumberCount >= m_genNumberTargetSave) {
        m_currGenNumberCount = 0;
        saveState(m_iniFileName);
    }
    //this is separate, but save also the best records if option is enabled and there are new ones
    if(m_bestRecordsToSave > 0) {
        //if the update actually inserts new records, save them
        if(m_evolutionManager.updateEliteRecords()) {
            Console::print("New elite records, saving them in '" + m_saveNameBestRecords + "'", Console::eDEBUG);
            m_evolutionManager.saveEliteRecords(m_saveNameBestRecords);
        }
    }
}
