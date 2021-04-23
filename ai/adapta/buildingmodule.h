#ifndef BUILDINGMODULE_H
#define BUILDINGMODULE_H

#include <QObject>
#include "game/building.h"
#include "coreengine/qmlvector.h"
/**
 * @brief The BuildingModule class represents a Module of the Adapta AI, dedicated only at choosing which and where units to
 * build. The working is the same of the AdaptaModule, which means each place in the map where the player can build is
 * associated with a bid [0,1] representing the importance of the unit that should be built there. 0 means that no unit should
 * be built there, according to this module
 */
class BuildingModule : public QObject
{
    Q_OBJECT
public:
    BuildingModule() = delete;
    explicit BuildingModule(Player* m_pPlayer);
    BuildingModule(const BuildingModule &other);
    BuildingModule(BuildingModule &&other);
    BuildingModule &operator=(const BuildingModule &other);
    virtual ~BuildingModule() = default;


    //same as adapta module: these 2 methods should be abstract, but since it breaks QVector, they are not
    virtual void processWhatToBuild(){};

    virtual void buildHighestBidUnit(){};

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

    Player* m_pPlayer;

    spQmlVectorBuilding m_buildings;
    QVector<float> m_buildingsBids;

};

#endif // BUILDINGMODULE_H
