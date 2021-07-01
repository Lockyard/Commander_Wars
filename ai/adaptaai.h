#ifndef ADAPTAAI_H
#define ADAPTAAI_H

#include "coreai.h"
#include <QObject>
#include <QVector>
#include "adapta/adaptamodule.h"
#include "adapta/buildingmodule.h"


class AdaptaAI : public CoreAI
{
    Q_OBJECT
public:
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
    QVector<BuildingModule*> m_buildingModules;

    QVector<float> m_unitBidVector;

    qint32 m_lastSelectedModuleIndex;

    bool m_isFirstProcessOfTurn{true};
    bool m_ignoreBids;

    currProcessInfo m_currProcessInfo;

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
