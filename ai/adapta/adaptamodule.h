#ifndef ADAPTAMODULE_H
#define ADAPTAMODULE_H

#include <QObject>
#include "game/player.h"
#include "ai/genetic/weightvector.h"

/**
 * @brief The AdaptaModule class represents a module for the adapta ai. A module is any component of the adapta which does
 * its computation and then generates each turn bids for some units currently in game (possibly none, possibly all).
 * The bid (a value in [0,1]) should represent how much a unit is considered important for that module in that turn.
 * For instance an attack module will probably bid less than a capture module for an infantry unit, if both actions are
 * possible, since an infantry attacking is in general less impactful than conquering.
 * The adapta will give control of a unit to the module who bid the most for that unit.
 * Modules can be implemented in any way, behavioral or trained, and can focus on any numbers of units. For instance
 * a module could generate bids only for ground units, or only for indirect units, etc, depending on specialization.
 * Modules should generate a bid also for building units, and modules can as well bid or not for requesting units to be
 * built.
 * If a bid should not be done, then the default bid is 0.
 * A module can also be weighted by the AI itself. The module should process its stuff without accounting this, since
 * it's an information only used by the ai to try to change strategies.
 */
class AdaptaModule : public QObject
{
    Q_OBJECT
public:
    AdaptaModule();
    AdaptaModule(Player* pPlayer);
    AdaptaModule(const AdaptaModule &other);
    AdaptaModule(AdaptaModule &&other);
    AdaptaModule &operator=(const AdaptaModule &other);
    virtual ~AdaptaModule() = default;

    //Methods to be overridden. I wanted to make this class abstract, but it breaks QVectors, so this is it
    virtual void readIni(QString name){};
    /**
     * @brief processStartOfTurn calculate bids for this turn and process stuff for the start of turn
     */
    virtual void processStartOfTurn(){};

    /**
     * @brief processHighestBidUnit process the action for the highest bid unit of this module
     * @param weighted
     */
    virtual void processHighestBidUnit(){};
    virtual void processUnit(Unit* pUnit){};
    virtual void notifyUnitUsed(Unit* pUnit){};
    virtual bool assignWeightVector(WeightVector weightVector){return false;};

    virtual float getBidFor(Unit* pUnit);
    /**
     * @brief getHighestBid get the value of the highest bid done by this module. By default the bid is weighted by
     * this module's weight (see [set]moduleWeight())
     */
    virtual float getHighestBid(bool weighted = true);
    virtual Unit* getHighestBidUnit();

    //getters, setters
    float moduleWeight() const;
    void setModuleWeight(float moduleWeight);

protected:
    //how much this module's bids are weighted by the adapta AI. default is 1 so that no modifications are done to the bids
    float m_moduleWeight = 1.0f;

    //here are kept all units and then bids for each unit, on a given turn
    //these 2 vectors must be kept on same size. They are not a vector of pairs to be faster, although a bit less clean
    spQmlVectorUnit m_units;
    QVector<float> m_unitsBids;

    Player* m_pPlayer;
};

#endif // ADAPTAMODULE_H
