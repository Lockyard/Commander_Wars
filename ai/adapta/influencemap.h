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
     * @brief addUnitInfluence add to the influence map the influence of a unit, with its weight
     * and given the type of propagation
     */
    void addUnitInfluence(Unit* pUnit, spQmlVectorUnit pEnemyUnits, float unitWeight, adaenums::propagationType propagationType);

    /**
     * @brief sum 1:1 the values of sumMap into this map, weighted wrt sumMap's weight
     */
    void weightedAddMap(InfluenceMap &sumMap);

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

    QString toQString();

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
    /**
     * @brief m_infoTilesVector a vector containing sprites and stuff to show graphical info on each tile, if show is called
     */
    std::vector<oxygine::spColorRectSprite> m_infoTilesMap;
    /**
     * @brief m_infoTextMap a vector containing texts to be shown on each tile, used to display some info, such as the influence
     * value on each tile
     */
    std::vector<oxygine::spTextField> m_infoTextMap;

    inline void addValueAt(float value, qint32 x, qint32 y) {
        m_influenceMap2D[y*m_mapWidth + x] += value;
    }

    void initializeInfoTilesMap();
    inline oxygine::spColorRectSprite getInfoTileAt(qint32 x, qint32 y) {
        return m_infoTilesMap[y*m_mapWidth + x];
    }

    inline oxygine::spTextField getInfoTextAt(qint32 x, qint32 y) {
        return m_infoTextMap[y*m_mapWidth + x];
    }

    adaenums::iMapType m_type;


};

#endif // INFLUENCEMAP_H
