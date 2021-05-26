#ifndef ADAPTAMODULE_H
#define ADAPTAMODULE_H

#include <QObject>
#include "game/player.h"
#include "ai/genetic/weightvector.h"
#include "game/unitpathfindingsystem.h"

class AdaptaModule;
typedef oxygine::intrusive_ptr<AdaptaModule> spAdaptaModule;

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
class AdaptaModule : public QObject, public oxygine::ref_counter
{
    Q_OBJECT
public:
    AdaptaModule();
    AdaptaModule(Player* pPlayer);
    AdaptaModule(const AdaptaModule &other);
    AdaptaModule(AdaptaModule &&other);
    void operator=(const AdaptaModule &other);
    virtual ~AdaptaModule() = default;

    //Methods to be overridden. I wanted to make this class abstract, but it breaks QVectors, so this is it
    virtual void readIni(QString name) = 0;
    /**
     * @brief processStartOfTurn calculate bids for this turn and process stuff for the start of turn
     */
    virtual void processStartOfTurn() = 0;

    /**
     * @brief processHighestBidUnit process the action for the highest bid unit of this module
     */
    virtual bool processHighestBidUnit() = 0;
    virtual bool processUnit(Unit* pUnit) = 0;
    virtual void notifyUnitUsed(Unit* pUnit) = 0;

    virtual float getBidFor(Unit* pUnit) = 0;
    /**
     * @brief getHighestBid get the value of the highest bid done by this module. By default the bid is weighted by
     * this module's weight (see [set]moduleWeight())
     */
    virtual float getHighestBid(bool weighted = true) = 0;
    virtual Unit* getHighestBidUnit() = 0;


    virtual bool assignWeightVector(WeightVector weightVector) = 0;

    /**
     * @brief get in QString format a state of the module.
     */
    virtual QString toQString(){return "";};

    //getters, setters
    float moduleWeight() const;
    void setModuleWeight(float moduleWeight);

    Player *getPPlayer() const;
    void setPPlayer(Player *pPlayer);

protected:
    //how much this module's bids are weighted by the adapta AI. default is 1 so that no modifications are done to the bids
    float m_moduleWeight = 1.0f;

    //here are kept all units and bids for each unit
    QVector<QPair<Unit*, float>> m_unitsBids;
    bool m_isUnitBidsSorted{false};

    Player* m_pPlayer;

    void sortUnitBids();
};

#endif // ADAPTAMODULE_H
