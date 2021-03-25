#ifndef INFLUENCEMAP_H
#define INFLUENCEMAP_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <qtextstream.h>
#include "ai/islandmap.h"
#include "3rd_party/oxygine-framework/oxygine-framework.h"

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
    explicit InfluenceMap(const QVector<spIslandMap> & islands);
    virtual ~InfluenceMap();
    void reset();
    /**
     * @brief addUnitInfluence add to the influence map the influence of a unit, with its weight
     * @param pUnit
     * @param pPfs
     * @param unitWeight
     */
    void addUnitInfluence(Unit* pUnit, spQmlVectorUnit pEnemyUnits, float unitWeight);

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

    inline float getInfluenceValueAt(qint32 x, qint32 y);

private:
    qint32 m_mapWidth{0};
    qint32 m_mapHeight{0};
    QVector<float> m_influenceMap2D;
    const QVector<spIslandMap> & m_islands;

    constexpr static const QColor M_POSITIVE_COLOR{QColorConstants::Blue};
    constexpr static const QColor M_NEGATIVE_COLOR{QColorConstants::Red};
    static const char M_ALPHA_SHOW{127}; //127/255 = 0.5 alpha
    static const char M_ALPHA_HIDE{0};
    bool isInfoTilesMapInitialized{false};
    /**
     * @brief m_infoTilesVector a vector containing sprites and stuff to show graphical info on each tile, if show is called
     */
    QVector<oxygine::spColorRectSprite> m_infoTilesMap;
    /**
     * @brief m_infoTextMap a vector containing texts to be shown on each tile, used to display some info, such as the influence
     * value on each tile
     */
    QVector<oxygine::spTextField> m_infoTextMap;

    inline void addValueAt(float value, qint32 x, qint32 y);

    void initializeInfoTilesMap();
    inline oxygine::spColorRectSprite getInfoTileAt(qint32 x, qint32 y);
    inline oxygine::spTextField getInfoTextAt(qint32 x, qint32 y);



};

#endif // INFLUENCEMAP_H
