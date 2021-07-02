#ifndef ADAPTAAI_H
#define ADAPTAAI_H

#include "coreai.h"
#include <QObject>
#include <QVector>
#include "adapta/adaptamodule.h"
#include "adapta/buildingmodule.h"


/**
 * @brief The AdaptaAI is the AI core of the project. Actually in itself is relatively simple, since the complexity is given more
 * by its modules. The AdaptaAI in short has a list of AdaptaModule(s) and BuildingModule(s). The first modules manage actions,
 * the second ones manage the building of units. Each module has to make a bid, a value going from 0 to 1 which tells how much
 * is important the next action they need to do, so they will receive a unit (or factory) control if no other module bids higher.
 * The AdaptaAI can weight each module as it sees fit, so that its bid are increased or reduced.
 * When all modules bid 0 the turn is over
 */
class AdaptaAI : public CoreAI
{
    Q_OBJECT
public:
    static const QString DEFAULT_CONFIG_NAME;
    /**
     * @brief NO_MODULE_ID the ID which marks to have no modules
     */
    static const QString NO_MODULE_ID;
    /**
     * @brief The currProcessInfo struct contains info about the current process, info which can be shared across modules
     * to be calculated at most once if requested
     */
    struct currProcessInfo {
        spQmlVectorUnit spUnits;
        spQmlVectorUnit spEnemies;
        spQmlVectorBuilding spBuildings;
        spQmlVectorBuilding spEnemyBuildings;
        bool validSpUnits = false;
        bool validSpEnemies = false;
        bool validSpBuildings = false;
        bool validSpEnemyBuildings = false;
    };
public:
    AdaptaAI();
    ~AdaptaAI() = default;
    //AdaptaAI(AdaptaAI &other) = default;

    virtual void readIni(QString name) override;

    virtual void init() override;

    virtual void process() override;

    void addSelectedFieldData(spGameAction pGameAction, qint32 pointX, qint32 pointY);

    /**
     * @return spUnits vector obtained at current process of adaptaAI. Since AdaptaAI shares modules, it's an optimization
     * if modules take from here the list of buildings and units to compute their stuff since they may use this info
     * when checking bids
     */
    spQmlVectorUnit getCurrentProcessSpUnits();
    spQmlVectorUnit getCurrentProcessSpEnemies();
    spQmlVectorBuilding getCurrentProcessSpBuildings();
    spQmlVectorBuilding getCurrentProcessSpEnemyBuildings();

    inline Player* getPlayer() {return m_pPlayer;};


protected:
    virtual void finishTurn() override;

private:
    //all loaded modules of the adapta
    QVector<spAdaptaModule> m_modules;
    QVector<spBuildingModule> m_buildingModules;

    QVector<float> m_unitBidVector;

    qint32 m_lastSelectedModuleIndex;

    bool m_isFirstProcessOfTurn{true};
    bool m_ignoreBids;

    currProcessInfo m_currProcessInfo;

    /**
     * @brief thake the stringList of parameters of ini file and load/generate the modules specified.
     * @return true if load was ok, false if not
     */
    bool loadModulesFromConfig(bool requireAllLoadsOk, QStringList adaptaModulesIDs, QStringList buildingModulesIDs,
                               QStringList adaptaModulesIniFiles, QStringList buildingModulesIniFiles);

    void loadDefaultConfig();

    /**
     * @brief setPlayerPtr to all modules
     */
    void initModules();

    /**
     * @brief to be called at start of process. Invalidates all vectors of units and buildings, so that when requested by modules
     * they will be recomputed for this process
     */
    inline void setCurrentProcessInfosNonValid() {
        m_currProcessInfo.validSpUnits = false;
        m_currProcessInfo.validSpEnemies = false;
        m_currProcessInfo.validSpBuildings = false;
        m_currProcessInfo.validSpEnemyBuildings = false;
    }

    inline void updateCurrentProcessInfoUnits() {
        m_currProcessInfo.spUnits = m_pPlayer->getUnits();
        m_currProcessInfo.validSpUnits = true;
    }

    inline void updateCurrentProcessInfoEnemies() {
        m_currProcessInfo.spEnemies = m_pPlayer->getEnemyUnits();
        m_currProcessInfo.validSpEnemies = true;
    }

    inline void updateCurrentProcessInfoBuildings() {
        m_currProcessInfo.spBuildings = m_pPlayer->getBuildings();
        m_currProcessInfo.validSpBuildings = true;
    }

    inline void updateCurrentProcessInfoEnemyBuildings() {
        m_currProcessInfo.spEnemyBuildings = m_pPlayer->getEnemyBuildings();
        m_currProcessInfo.validSpEnemyBuildings = true;
    }


    friend class AdaptaModule;
    friend class MultiInfluenceNetworkModule;
};

#endif // ADAPTAAI_H
