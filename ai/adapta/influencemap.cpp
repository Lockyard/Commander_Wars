#include "influencemap.h"

#include "coreengine/mainapp.h"
#include "coreengine/globalutils.h"
#include "coreengine/console.h"
#include "ai/coreai.h"

#include "game/gamemap.h"
#include "game/unitpathfindingsystem.h"
#include "resource_management/unitspritemanager.h"
#include "resource_management/fontmanager.h"

#include <qfile.h>


/**
 * @brief InfluenceMap::InfluenceMap create with specified islands. Set to 0 all map
 * @param islands
 */
InfluenceMap::InfluenceMap(const QVector<spIslandMap> & islands) : m_islands(islands)
{
    Mainapp* pApp = Mainapp::getInstance();
    this->moveToThread(pApp->getWorkerthread());
    Interpreter::setCppOwnerShip(this);
    spGameMap pMap = GameMap::getInstance();
    m_mapWidth = pMap->getMapWidth();
    m_mapHeight = pMap->getMapHeight();

    m_influenceMap2D.resize(m_mapWidth * m_mapHeight);

    for (qint32 i=0; i<m_mapWidth*m_mapHeight; i++) {
        m_influenceMap2D.append(0);
    }

}

InfluenceMap::~InfluenceMap(){
    if(isInfoTilesMapInitialized) {
        for(auto & sprite : m_infoTilesMap) {
            sprite->detach();
        }

        for (auto & textField : m_infoTextMap) {
            textField->detach();
        }
    }
}

void InfluenceMap::reset() {
    for(qint32 i = 0; i < m_influenceMap2D.size(); i++) {
        m_influenceMap2D.replace(i, 0);
    }
}


void InfluenceMap::addUnitInfluence(Unit* pUnit, spQmlVectorUnit pEnemyUnits, float unitWeight) {

    float hpValue = pUnit->getHpRounded()*.1f;
    qint32 movePoints = pUnit->getMovementpoints(pUnit->getPosition());

    UnitPathFindingSystem* pPfs = new UnitPathFindingSystem(pUnit);
    pPfs->setIgnoreEnemies(pEnemyUnits);
    //double the move points of the unit. used to calculate the influence on 2 steps
    pPfs->setMovepoints(movePoints*2);
    pPfs->explore();


    auto points = pPfs->getAllNodePoints();
    //for each reachable point by this unit
    for (const auto & point : points) {

        float multiplier = 1.0f;
        qint32 fieldCost = pPfs->getTargetCosts(point.x(), point.y());
        if (movePoints > 0 && fieldCost > 0) {
            if(fieldCost <= movePoints) { //field reachable in 1 turn
                multiplier = 1.0f;
            } else if (fieldCost <= movePoints*2) { //field reachable in 2 turns
                multiplier = 0.5f;
            } else {
                multiplier = 0.0f;
            }
        }

        //increment the influence on this specific point, according to the multiplier, the given weight and unit's hp
        addValueAt( unitWeight * hpValue * multiplier, point.x(), point.y());
        Console::print("influence value at (" + QString::number(point.x()) + ", " +
                       QString::number(point.y()) + ") is " +
                       QString::number(getInfluenceValueAt(point.x(), point.y())), Console::eDEBUG);
    }

}


void InfluenceMap::showColoredTiles() {
    //if the map is not initialized, initialize it
    if(!isInfoTilesMapInitialized)
        initializeInfoTilesMap();

    spGameMap pMap = GameMap::getInstance();
    qint32 currInfluenceValue = 0;
    for (qint32 x = 0; x < m_mapWidth; ++x)
    {
        for (qint32 y = 0; y < m_mapHeight; ++y)
        {
            currInfluenceValue = getInfluenceValueAt(x, y);
            //set color and transparency according to the value on tile (positive/negative or zero)
            getInfoTileAt(x, y)->setColor(currInfluenceValue >= 0 ? M_POSITIVE_COLOR : M_NEGATIVE_COLOR);
            getInfoTileAt(x, y)->setAlpha(currInfluenceValue == 0 ? M_ALPHA_HIDE : M_ALPHA_SHOW);
        }
    }
}

void InfluenceMap::showValues() {
    //if the map is not initialized, initialize it
    if(!isInfoTilesMapInitialized)
        initializeInfoTilesMap();

    spGameMap pMap = GameMap::getInstance();
    for (qint32 x = 0; x < m_mapWidth; ++x)
    {
        for (qint32 y = 0; y < m_mapHeight; ++y)
        {
            getInfoTextAt(x, y)->setAlpha(255);
            getInfoTextAt(x,y)->setHtmlText(QString::number(getInfluenceValueAt(x, y)));
        }
    }
}


void InfluenceMap::showAllInfo() {
    //first call show so it initializes the map if not initialized
    showColoredTiles();
    showValues();
}





void InfluenceMap::hide() {
    if(!isInfoTilesMapInitialized)
        return;

    for (qint32 y = 0; y < m_mapHeight; ++y)
    {
        for (qint32 x = 0; x < m_mapWidth; ++x)
        {
            getInfoTileAt(x, y)->setAlpha(M_ALPHA_HIDE);
            getInfoTextAt(x, y)->setAlpha(M_ALPHA_HIDE);
        }
    }
}

inline float InfluenceMap::getInfluenceValueAt(qint32 x, qint32 y) {
    return m_influenceMap2D[y*m_mapWidth + x];
}

inline void InfluenceMap::addValueAt(float value, qint32 x, qint32 y) {
    m_influenceMap2D[y*m_mapWidth + x] += value;
}



void InfluenceMap::initializeInfoTilesMap() {
    if(isInfoTilesMapInitialized)
        return;

    //create font style
    oxygine::TextStyle style = FontManager::getMainFont24();
    style.color = FontManager::getFontColor();
    style.vAlign = oxygine::TextStyle::VALIGN_MIDDLE;
    style.hAlign = oxygine::TextStyle::HALIGN_MIDDLE;

    spGameMap pMap = GameMap::getInstance();
    m_infoTilesMap.reserve(m_mapWidth * m_mapHeight);
    m_infoTextMap.reserve(m_mapWidth * m_mapHeight);

    //for each tile on map
    for (qint32 y = 0; y < m_mapHeight; ++y)
    {
        for (qint32 x = 0; x < m_mapWidth; ++x)
        {
            //add the invisible sprite to the grid of sprites and to the map
            oxygine::spColorRectSprite sprite = new oxygine::ColorRectSprite();
            sprite->setSize(GameMap::getImageSize(), GameMap::getImageSize());
            QColor color{M_POSITIVE_COLOR};
            color.setAlphaF(0.0f);
            sprite->setColor(color);
            sprite->setPosition(x * GameMap::getImageSize(), y * GameMap::getImageSize());
            sprite->setPriority(static_cast<qint32>(Mainapp::ZOrder::MarkedFields));
            pMap->addChild(sprite);
            m_infoTilesMap.append(sprite);

            //add a textfield on this tile
            oxygine::spTextField pTextField = new oxygine::TextField();
            pTextField->setStyle(style);
            pTextField->setHtmlText(QString::number(0));
            pTextField->setColor(QColorConstants::White);
            pTextField->setPosition(x * GameMap::getImageSize(), y * GameMap::getImageSize());
            pTextField->setAlpha(0);
            pTextField->setSize(GameMap::getImageSize(), GameMap::getImageSize());
            pTextField->setFontSize(12);
            pTextField->setPriority(static_cast<qint32>(Mainapp::ZOrder::Animation));
            pMap->addChild(pTextField);
            m_infoTextMap.append(pTextField);

        }
    }

    isInfoTilesMapInitialized = true;
}

inline oxygine::spColorRectSprite InfluenceMap::getInfoTileAt(qint32 x, qint32 y) {
    return m_infoTilesMap[y*m_mapWidth + x];
}

inline oxygine::spTextField InfluenceMap::getInfoTextAt(qint32 x, qint32 y) {
    return m_infoTextMap[y*m_mapWidth + x];
}
