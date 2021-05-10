#ifndef INFLUENCEMAP_H
#define INFLUENCEMAP_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <qtextstream.h>
#include "ai/islandmap.h"
#include "3rd_party/oxygine-framework/oxygine-framework.h"
#include "AdaptaEnums.h"

#include "coreengine/qmlvector.h"
#include <QtMath>

class Player;
class Unit;
class UnitPathFindingSystem;
class GameMap;
class QmlVectorPoint;

/**
 * @brief The InfluenceMap class implements a simple and hopefully efficient version of a classic influence map
 * Any type of influence can be added and propagated.
 * Some stuff is implemented a bit less clean to be faster, since is crucial for training (e.g. the 2D map is a 1D vector and
 * not a class on its own which cover this thing).
 */
class InfluenceMap : public QObject
{
    Q_OBJECT
public:
    explicit InfluenceMap();
    InfluenceMap(adaenums::iMapType type);
    InfluenceMap(const InfluenceMap &other);
    InfluenceMap(InfluenceMap &&other);
    virtual ~InfluenceMap();
    void reset();

    /**
     * @brief Add the influence that a unit exercise on the map, based on where it can attack.
     * This gives a weight = attackWeight on tiles it can attack in 1 turn, and lower weights for tile it can attack in N
     * turns. max turn range is given by steps. (if steps == 1 for instance, all tiles it can attack in 1 turn have
     * attackWeight and all others 0, while if steps == 2 also the tiles reachable in 2 turns where this unit can attack
     * have a weight, which is attackWeight * stepMultiplier. At distance 3 turns is atkWeight * stepMultiplier^2, etc
     * this DOES NOT CHECK IF THE UNIT HAS AMMO since the given influence is custom anyway. So check it before calling it if
     * is important
     * set steps = -2 to add an influence value for all tiles reachable by that unit
     * steps for indirect can be set to differentiate indirect units, since they are heavier to compute. the default set it equal to steps
     */
    void addUnitAtkInfluence(Unit* pUnit,float attackWeight, float stepMultiplier, qint32 steps, qint32 stepsForIndirect = -3);

    void addUnitValueInfluence(Unit* pUnit, QPoint startingPoint, bool ignoreEnemies, float unitWeight);

    /**
     * @brief propagate the dmgValue, interpreted as the positions where the pAttackerTypeUnit should do damage, targeting
     * the poin enemyTargetPoint. This works only for direct units
     */
    void addUnitDirectDmgValueInfluence(Unit* pAttackerTypeUnit, QPoint enemyTargetPoint, float dmgValue);

    /**
     * @brief propagate the dmgValue, interpreted as the positions where the pAttackerTypeUnit should do damage, targeting
     * the poin enemyTargetPoint. This works only for indirect units
     */
    void addUnitIndirectDmgValueInfluence(Unit* pAttackerTypeUnit, QPoint enemyTargetPoint, float dmgValue);


    /**
     * @brief a faster version of the omonym - "fast" method. the damage decrease with distance from target point
     * since the unit is indirect can be not precise since will not consider how near the unit is from a point where it can attack
     * the unit, rather its distance with the target unit
     */
    void addUnitIndirectDmgValueInfluenceFast(Unit* pAttackerTypeUnit, QPoint enemyTargetPoint, float dmgValue, float negExp = -1);

    /**
     * @brief add the dmgValue that an indirect unit would do to an enemy in the target point. Fast version to be easier to
     * compute, althought it's not precise this way
     * dmgValue is added to all cells which can reach the target point in 1 turn (which is without moving the unit)
     * then this value is decreased for each further cell from these ones by x^negExp depending on pure tile distance.
     * negExp must be negative or funny stuff will happen
     */
    void addUnitIndirectDmgValueInfluenceFaster(Unit* pAttackerTypeUnit, QPoint enemyTargetPoint, float dmgValue, float negExp = -1);



    /**
     * @brief add an influence on all map, of defense based on each tile's number of stars relative to the unit,
     * an additional mutliplier if there's a friendly building and a multiplier to eventually disincentivize going
     * on friendly factories pPlayer is used to identify friendly buildings
     */
    void addMapDefenseInfluence(Player* pPlayer, Unit* pUnit, float weightPerStar, float friendlyBuildingMultiplier, float friendlyFactoryMultiplier);

    /**
     * @brief sum 1:1 the values of sumMap into this map, weighted wrt sumMap's weight
     */
    void weightedAddMap(InfluenceMap &sumMap);

    /**
     * @brief sum 1:1 the values of sumMap into this map, but weighted accordingly to the given customWeight
     * instead of the sumMap's one
     */
    void weightedAddMap(InfluenceMap &sumMap, float customWeight);

    float getCurrMaxAbsInfluence();

    /**
     * @brief sortNodePointsByInfluence sort the points in order from highest to lowest influence in this map
     */
    void sortNodePointsByInfluence(QVector<QPoint> & nodePoints);

    /**
     * @brief show shows graphically the influence map by coloring tiles
     */
    void showColoredTiles();
    /**
     * @brief showValues show on each tile the influence value
     */
    void showValues();
    /**
     * @brief showAll shows both color and value for each tile depending on the influence
     */
    void showAllInfo();
    /**
     * @brief hide hides the influence map shown with any show method
     */
    void hide();


    inline float getInfluenceValueAt(qint32 x, qint32 y) {
        return m_influenceMap2D[y*m_mapWidth + x];
    }

    inline void setInfluenceValueAt(float value, qint32 x, qint32 y) {
        m_influenceMap2D[y*m_mapWidth + x] = value;
    }


    QString toQString(qint32 precision = 3);
    QString toQStringFull(qint32 precision = 3);

    static InfluenceMap weightedSum(InfluenceMap &infMap1, InfluenceMap &infMap2);

    inline adaenums::iMapType getType() const {
        return m_type;
    }
    inline void setType(const adaenums::iMapType &value) {
        m_type = value;
    }

    inline float getWeight() const {
        return m_weight;
    }

    inline void setWeight(float weight) {
        m_weight = weight;
    }


private:
    /**
     * @brief m_weight the weight of this influence map. Can be used when summing more influence maps
     */
    float m_weight;
    qint32 m_mapWidth{0};
    qint32 m_mapHeight{0};
    std::vector<float> m_influenceMap2D;

    constexpr static const QColor M_POSITIVE_COLOR{QColorConstants::Blue};
    constexpr static const QColor M_NEGATIVE_COLOR{QColorConstants::Red};
    static const char M_ALPHA_SHOW{127}; //127/255 = 0.5 alpha
    static const char M_ALPHA_HIDE{0};
    bool isInfoTilesMapInitialized{false};

    adaenums::iMapType m_type;

    /**
     * @brief m_infoTilesVector a vector containing sprites and stuff to show graphical info on each tile, if show is called
     */
    std::vector<oxygine::spColorRectSprite> m_infoTilesMap;
    /**
     * @brief m_infoTextMap a vector containing texts to be shown on each tile, used to display some info, such as the influence
     * value on each tile
     */
    std::vector<oxygine::spTextField> m_infoTextMap;

    inline void setValueAt(float value, qint32 x, qint32 y) {
        m_influenceMap2D[y*m_mapWidth + x] = value;
    }

    inline void addValueAt(float value, qint32 x, qint32 y) {
        m_influenceMap2D[y*m_mapWidth + x] += value;
    }

    inline void setValueIfGreaterAt(float value, qint32 x, qint32 y) {
        if(value > m_influenceMap2D[y*m_mapWidth + x])
            m_influenceMap2D[y*m_mapWidth + x] = value;
    }

    void initializeInfoTilesMap();
    inline oxygine::spColorRectSprite getInfoTileAt(qint32 x, qint32 y) {
        return m_infoTilesMap[y*m_mapWidth + x];
    }

    inline oxygine::spTextField getInfoTextAt(qint32 x, qint32 y) {
        return m_infoTextMap[y*m_mapWidth + x];
    }

    /**
     * @brief quick manhattan distance between 2 points. It shouldn't be here such a function probably
     */
    inline static qint32 pointDistance(QPoint p1, QPoint p2) {
        return qAbs(p1.x() - p2.x()) + qAbs(p1.y() - p2.y());
    }

    inline static qint32 pointDistance(qint32 x1, qint32 y1, qint32 x2, qint32 y2) {
        return qAbs(x1 - x2) + qAbs(y1 - y2);
    }


    inline void markAttackArea(qint32 minRange, qint32 maxRange, qint32 pointX, qint32 pointY, std::vector<qint32> &atkTilesMap2D, qint32 markValue) {
        //mark with attackRange all reachable tiles
        qint32 minX = qMax(pointX - maxRange, 0);
        qint32 maxX = qMin(pointX + maxRange, m_mapWidth - 1);
        qint32 minY = qMax(pointY - maxRange, 0);
        qint32 maxY = qMin(pointY + maxRange, m_mapHeight - 1);
        //search efficiently for all tiles this indirect unit can attack in range
        for(qint32 x = minX; x <= maxX; x++) {
            for(qint32 y = minY; y <= maxY; y++) {
                qint32 distance = pointDistance(x, y, pointX, pointY);
                if(distance <= maxRange && distance >= minRange) {
                    qint32 index = y*m_mapWidth + x;
                    if(atkTilesMap2D[index] > markValue) {
                        atkTilesMap2D[index] = markValue;
                    }
                }
            }
        }
    }

    inline void markAttackAreaIfCanMoveOver(Unit* pUnit, qint32 minRange, qint32 maxRange, qint32 pointX, qint32 pointY, std::vector<qint32> &atkTilesMap2D, qint32 markValue) {
        qint32 minX = qMax(pointX - maxRange, 0);
        qint32 maxX = qMin(pointX + maxRange, m_mapWidth - 1);
        qint32 minY = qMax(pointY - maxRange, 0);
        qint32 maxY = qMin(pointY + maxRange, m_mapHeight - 1);
        //search efficiently for all tiles this indirect unit can attack in range
        for(qint32 x = minX; x <= maxX; x++) {
            for(qint32 y = minY; y <= maxY; y++) {
                if(pUnit->canMoveOver(x, y)) {
                    qint32 distance = pointDistance(x, y, pointX, pointY);
                    if(distance <= maxRange && distance >= minRange) {
                        qint32 index = y*m_mapWidth + x;
                        if(atkTilesMap2D[index] > markValue) {
                            atkTilesMap2D[index] = markValue;
                        }
                    }
                }
            }
        }
    }
};

#endif // INFLUENCEMAP_H
