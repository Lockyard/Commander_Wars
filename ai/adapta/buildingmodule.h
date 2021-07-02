#ifndef BUILDINGMODULE_H
#define BUILDINGMODULE_H

#include <QObject>
#include "game/building.h"
#include "coreengine/qmlvector.h"

class BuildingModule;
typedef oxygine::intrusive_ptr<BuildingModule> spBuildingModule;

/**
 * @brief The BuildingModule class represents a Module of the Adapta AI, dedicated only at choosing which and where units to
 * build. The working is the same of the AdaptaModule, which means each place in the map where the player can build is
 * associated with a bid [0,1] representing the importance of the unit that should be built there. 0 means that no unit should
 * be built there, according to this module
 */
class BuildingModule : public QObject, public oxygine::ref_counter
{
    Q_OBJECT
public:
    explicit BuildingModule();
    BuildingModule(const BuildingModule &other);
    BuildingModule(BuildingModule &&other);
    BuildingModule &operator=(const BuildingModule &other);
    virtual ~BuildingModule() = default;

    /**
     * @brief read from a ini file.
     * @return true if load was ok, false if not
     */
    virtual bool readIni(QString filename) = 0;

    /**
     * @brief init this module and gives it the playerPtr reference
     */
    virtual void init(Player* pPlayer);

    virtual void processStartOfTurn() = 0;

    virtual void processWhatToBuild() = 0;

    virtual bool buildHighestBidUnit() = 0;

    /**
     * @brief getHighestBid get the value of the highest bid done by this module. By default the bid is weighted by
     * this module's weight (see [set]moduleWeight())
     */
    virtual float getHighestBid(bool weighted = true);
    virtual Building* getHighestBidBuilding();

    //getters, setters
    float moduleWeight() const;
    void setModuleWeight(float moduleWeight);

protected:
    float m_moduleWeight = 1.0f;

    Player* m_pPlayer{nullptr};

    spQmlVectorBuilding m_buildings;
    QVector<float> m_buildingsBids;

private:
    bool buildUnits(spQmlVectorBuilding pBuildings, spQmlVectorUnit pUnits,
                    spQmlVectorUnit pEnemyUnits, spQmlVectorBuilding pEnemyBuildings);
};

#endif // BUILDINGMODULE_H
