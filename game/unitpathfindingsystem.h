#ifndef UNITPATHFINDINGSYSTEM_H
#define UNITPATHFINDINGSYSTEM_H

#include "coreengine/pathfindingsystem.h"

#include "game/unit.h"

#include "game/smartpointers.h"


class UnitPathFindingSystem;
typedef oxygine::intrusive_ptr<UnitPathFindingSystem> spUnitPathFindingSystem;

class UnitPathFindingSystem : public PathFindingSystem
{
    Q_OBJECT
public:
    explicit UnitPathFindingSystem(Unit* pUnit);

    /**
     * @brief getRemainingCost
     * @param x
     * @param y
     * @return the remaining costs for this node to reach the target
     */
    virtual qint32 getRemainingCost(qint32 x, qint32 y, qint32 currentCost) override;
    /**
     * @brief finished checks if this would be the target node to reach
     * @param x
     * @param y
     * @param costs
     * @return
     */
    virtual bool finished(qint32 x, qint32 y, qint32)  override;
    /**
     * @brief getCosts
     * @param x
     * @param y
     * @return the exact costs needed to get onto the given field. -1 = unreachable
     */
    virtual qint32 getCosts(qint32 x, qint32 y)  override;
    /**
     * @brief getCosts returns the costs need to go the given path
     * @param path the path the unit should go
     * @return the costs needed when using the given path
     */
    qint32 getCosts(QVector<QPoint> path);
    /**
     * @brief getClosestReachableMovePath
     * @param target
     * @return
     */
    QVector<QPoint> getClosestReachableMovePath(QPoint target);
    /**
     * @brief setMoveCosts
     * @param movepoints how far this pfs explodes
     */
    void setMovepoints(const qint32 &movepoints);

protected:
    Unit* m_pUnit;
    qint32 m_Movepoints{-1};
};

#endif // UNITPATHFINDINGSYSTEM_H
