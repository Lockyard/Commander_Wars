#ifndef ISLANDMAP_H
#define ISLANDMAP_H

#include <QObject>
#include <QVector>

#include "oxygine-framework.h"

class Player;


class IslandMap;
typedef oxygine::intrusive_ptr<IslandMap> spIslandMap;

/**
 * @brief The IslandMap class stores a map of the size of the game map, representing the separate areas a specific unit can move
 * on, and if it can reach it eventually with its own movement (for instance a plain separated by mountains constitute 2 island
 * maps for a tank. Each map is walkable by it but the tank alone cannot pass on the other island on its own (and maybe need a
 * boat if any).
 */
class IslandMap : public QObject, public oxygine::ref_counter
{
    Q_OBJECT
    static constexpr qint32 UNKNOWN = -1;
public:
    explicit IslandMap(QString unitID, Player* pOwner);
    virtual ~IslandMap() = default;


public slots:
    /**
     * @brief sameIsland
     * @param x1
     * @param y1
     * @param x2
     * @param y2
     * @return
     */
    inline bool sameIsland(qint32 x1, qint32 y1,
                           qint32 x2, qint32 y2)
    {
        return (m_Islands[x1][y1] > UNKNOWN &&
                m_Islands[x1][y1] == m_Islands[x2][y2]);
    }
    /**
     * @brief getIsland
     * @param x
     * @param y
     * @return the island for this point -1 means not reachable
     */
    inline qint32 getIsland(qint32 x, qint32 y)
    {
        return m_Islands[x][y];
    }
    QString getMovementType() const;
private:
    QVector<QVector<qint32>> m_Islands;
    QString m_MovementType;
};

#endif // ISLANDMAP_H
