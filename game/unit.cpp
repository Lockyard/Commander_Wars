#include "unit.h"

#include <QColor>

#include "game/player.h"

#include "resource_management/unitspritemanager.h"

#include "game/gamemap.h"

#include "coreengine/console.h"

#include "game/gameaction.h"

#include "game/co.h"

#include "coreengine/tweentogglevisibility.h"

Unit::Unit(QString unitID, spPlayer pOwner)
    : QObject(),
      m_UnitID(unitID),
      m_Owner(pOwner)
{
    if (!m_UnitID.isEmpty())
    {
        initUnit();
        updateSprites();
    }
}

Unit::~Unit()
{

}

void Unit::setOwner(spPlayer pOwner)
{
    // change ownership
    m_Owner = pOwner;
    // update sprites :)
    updateSprites();
}

 spPlayer Unit::getSpOwner()
 {
    return m_Owner;
 }

 void Unit::setTerrain(spTerrain pTerrain)
 {
     m_Terrain = pTerrain;
 }

void Unit::loadSprite(QString spriteID, bool addPlayerColor)
{
    UnitSpriteManager* pUnitSpriteManager = UnitSpriteManager::getInstance();
    oxygine::ResAnim* pAnim = pUnitSpriteManager->getResAnim(spriteID.toStdString());
    if (pAnim != nullptr)
    {
        oxygine::spSprite pSprite = new oxygine::Sprite();
        oxygine::spSprite pWaitSprite = new oxygine::Sprite();
        if (pAnim->getTotalFrames() > 1)
        {
            oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), pAnim->getTotalFrames() * GameMap::frameTime, -1);
            pSprite->addTween(tween);

            oxygine::spTween tweenWait = oxygine::createTween(oxygine::TweenAnim(pAnim), pAnim->getTotalFrames() * GameMap::frameTime, -1);
            pWaitSprite->addTween(tweenWait);
        }
        else
        {
            pSprite->setResAnim(pAnim);
            pWaitSprite->setResAnim(pAnim);
        }
        // repaint the unit?
        if (addPlayerColor)
        {
            QColor color = m_Owner->getColor();
            oxygine::Sprite::TweenColor tweenColor(oxygine::Color(color.red(), color.green(), color.blue(), 255));
            oxygine::spTween tween = oxygine::createTween(tweenColor, 1);
            pSprite->addTween(tween);
        }
        pSprite->setScale(GameMap::Imagesize / pAnim->getWidth());
        pSprite->setPosition(-(pSprite->getScaledWidth() - GameMap::Imagesize) / 2, -(pSprite->getScaledHeight() - GameMap::Imagesize));
        this->addChild(pSprite);
        m_pUnitSprites.append(pSprite);

        oxygine::Sprite::TweenColor tweenColor(oxygine::Color(100, 100, 100, 100));
        oxygine::spTween tweenWait = oxygine::createTween(tweenColor, 1);
        pWaitSprite->addTween(tweenWait);
        pWaitSprite->setScale(GameMap::Imagesize / pAnim->getWidth());
        pWaitSprite->setPosition(-(pSprite->getScaledWidth() - GameMap::Imagesize) / 2, -(pSprite->getScaledHeight() - GameMap::Imagesize));
        pWaitSprite->setPriority(static_cast<short>(Priorities::Waiting));
        this->addChild(pWaitSprite);
        pWaitSprite->setVisible(false);
        m_pUnitWaitSprites.append(pWaitSprite);
    }
    else
    {
        Console::print("Unable to load unit sprite: " + spriteID, Console::eERROR);
    }
}

Player* Unit::getOwner()
{
    return m_Owner.get();
}

void Unit::updateSprites()
{
    Mainapp* pApp = Mainapp::getInstance();
    for (qint32 i = 0; i < m_pUnitSprites.size(); i++)
    {
        this->removeChild(m_pUnitSprites[i]);
    }
    // call the js loader function to do the rest
    m_pUnitSprites.clear();
    QString function1 = "loadSprites";
    QJSValueList args1;
    QJSValue obj1 = pApp->getInterpreter()->newQObject(this);
    args1 << obj1;
    pApp->getInterpreter()->doFunction(m_UnitID, function1, args1);
}

qint32 Unit::getVision() const
{
    return vision;
}

void Unit::setVision(const qint32 &value)
{
    vision = value;
}

qint32 Unit::getMaxRange()
{
    return getMaxRange(QPoint(getX(), getY()));
}

qint32 Unit::getMaxRange(QPoint position)
{
    qint32 rangeModifier = 0;
    CO* pCO = m_Owner->getCO(0);
    if (pCO != nullptr)
    {
        rangeModifier += pCO->getFirerangeModifier(this, position);
    }
    pCO = m_Owner->getCO(1);
    if (pCO != nullptr)
    {
        rangeModifier += pCO->getFirerangeModifier(this, position);
    }
    qint32 points = maxRange + rangeModifier;
    if (points < minRange)
    {
        points = minRange;
    }
    return points;
}

void Unit::setMaxRange(const qint32 &value)
{
    maxRange = value;
}

qint32 Unit::getMinRange() const
{
    return minRange;
}

void Unit::setMinRange(const qint32 &value)
{
    minRange = value;
}

qint32 Unit::getCosts() const
{
    Mainapp* pApp = Mainapp::getInstance();
    QString function1 = "getBaseCost";
    QJSValueList args1;
    QJSValue erg = pApp->getInterpreter()->doFunction(m_UnitID, function1, args1);
    if (erg.isNumber())
    {
        return erg.toInt();
    }
    else
    {
        return -1;
    }
}

Terrain* Unit::getTerrain()
{
    return m_Terrain.get();
}

bool Unit::canMoveAndFire(QPoint position)
{
    Mainapp* pApp = Mainapp::getInstance();
    QString function1 = "canMoveAndFire";
    QJSValueList args1;
    QJSValue erg = pApp->getInterpreter()->doFunction(m_UnitID, function1, args1);
    if (erg.isBool() && erg.toBool())
    {
        return true;
    }
    CO* pCO = m_Owner->getCO(0);
    if (pCO != nullptr)
    {
        if (pCO->getCanMoveAndFire(this, position))
        {
            return true;
        }
    }
    pCO = m_Owner->getCO(1);
    if (pCO != nullptr)
    {
        if (pCO->getCanMoveAndFire(this, position))
        {
            return true;
        }
    }
    return false;
}

void Unit::loadUnit(Unit* pUnit)
{
    m_TransportUnits.append(pUnit->getTerrain()->getSpUnit());
    // todo check which icon we need to load
    loadIcon("transport", GameMap::Imagesize / 2, GameMap::Imagesize / 2);
}

Unit* Unit::getLoadedUnit(qint32 index)
{
    if ((index >= 0) && (index < m_TransportUnits.size()))
    {
        return m_TransportUnits[index].get();
    }
    return nullptr;
}

void Unit::unloadUnit(Unit* pUnit, QPoint position)
{
    for (qint32 i = 0; i < m_TransportUnits.size(); i++)
    {
        if (m_TransportUnits[i] == pUnit)
        {
            GameMap::getInstance()->getTerrain(position.x(), position.y())->setUnit(m_TransportUnits[i]);
            m_TransportUnits.removeAt(i);
            break;
        }
    }
    if (m_TransportUnits.size() <= 0)
    {
        unloadIcon("transport");
        unloadIcon("transport+hidden");
    }
}

qint32 Unit::getLoadedUnitCount()
{
    return m_TransportUnits.size();
}

qint32 Unit::getBonusOffensive(QPoint position, Unit* pDefender, QPoint defPosition)
{
    qint32 bonus = 0;
    CO* pCO = m_Owner->getCO(0);
    if (pCO != nullptr)
    {
        bonus += pCO->getOffensiveBonus(this, position, pDefender, defPosition);
    }
    pCO = m_Owner->getCO(1);
    if (pCO != nullptr)
    {
        bonus += pCO->getOffensiveBonus(this, position, pDefender, defPosition);
    }
    return bonus;
}

qint32 Unit::getBonusDefensive(QPoint position, Unit* pAttacker, QPoint atkPosition)
{
    qint32 bonus = 0;
    CO* pCO = m_Owner->getCO(0);
    if (pCO != nullptr)
    {
        bonus += pCO->getDeffensiveBonus(pAttacker, atkPosition, this, position);
    }
    pCO = m_Owner->getCO(1);
    if (pCO != nullptr)
    {
        bonus += pCO->getDeffensiveBonus(pAttacker, atkPosition, this, position);
    }

    return bonus;
}

qint32 Unit::getAttackHpBonus(QPoint position)
{
    qint32 bonus = 0;
    CO* pCO = m_Owner->getCO(0);
    if (pCO != nullptr)
    {
        bonus += pCO->getAttackHpBonus(this, position);
    }
    pCO = m_Owner->getCO(1);
    if (pCO != nullptr)
    {
        bonus += pCO->getAttackHpBonus(this, position);
    }
    return bonus;
}

qint32 Unit::getBonusMisfortune(QPoint position)
{
    qint32 bonus = 0;
    CO* pCO = m_Owner->getCO(0);
    if (pCO != nullptr)
    {
        bonus += pCO->getBonusMisfortune(this, position);
    }
    pCO = m_Owner->getCO(1);
    if (pCO != nullptr)
    {
        bonus += pCO->getBonusMisfortune(this, position);
    }
    return bonus;
}

qint32 Unit::getBonusLuck(QPoint position)
{
    qint32 bonus = 0;
    CO* pCO = m_Owner->getCO(0);
    if (pCO != nullptr)
    {
        bonus += pCO->getBonusLuck(this, position);
    }
    pCO = m_Owner->getCO(1);
    if (pCO != nullptr)
    {
        bonus += pCO->getBonusLuck(this, position);
    }
    return bonus;
}

void Unit::startOfTurn()
{
    Mainapp* pApp = Mainapp::getInstance();
    QString function1 = "startOfTurn";
    QJSValueList args1;
    QJSValue obj1 = pApp->getInterpreter()->newQObject(this);
    args1 << obj1;
    pApp->getInterpreter()->doFunction(m_UnitID, function1, args1);
}

qint32 Unit::getCapturePoints() const
{
    return capturePoints;
}

void Unit::setCapturePoints(const qint32 &value)
{
    capturePoints = value;
    if (capturePoints > 0)
    {
        loadIcon("capture", GameMap::Imagesize / 2, GameMap::Imagesize / 2);
    }
    else
    {
        unloadIcon("capture");
    }
}

qint32 Unit::getBaseMovementPoints() const
{
    return baseMovementPoints;
}

void Unit::setBaseMovementPoints(const qint32 &value)
{
    baseMovementPoints = value;
}

void Unit::initUnit()
{
    Mainapp* pApp = Mainapp::getInstance();
    QString function1 = "init";
    QJSValueList args1;
    QJSValue obj1 = pApp->getInterpreter()->newQObject(this);
    args1 << obj1;
    pApp->getInterpreter()->doFunction(m_UnitID, function1, args1);
    setFuel(fuel);
    setAmmo1(ammo1);
    setAmmo2(ammo2);
    setHp(hp);
}

qint32 Unit::getMaxFuel() const
{
    return maxFuel;
}

void Unit::setMaxFuel(const qint32 &value)
{
    maxFuel = value;
}

qint32 Unit::getFuel() const
{
    return fuel;
}

void Unit::setFuel(const qint32 &value)
{
    fuel = value;
    if (static_cast<float>(fuel) / static_cast<float>(maxFuel) <= 1.0f / 3.0f)
    {
        loadIcon("fuel", GameMap::Imagesize / 2, 0);
    }
    else
    {
        unloadIcon("fuel");
    }
}

qint32 Unit::getMaxAmmo2() const
{
    return maxAmmo2;
}

void Unit::setMaxAmmo2(const qint32 &value)
{
    maxAmmo2 = value;
}

qint32 Unit::getAmmo2() const
{
    return ammo2;
}

bool Unit::hasAmmo2() const
{
    if ((maxAmmo2 < 0) || (ammo2 > 0))
    {
        return true;
    }
    else
    {
        return false;
    }
}


QString Unit::getWeapon2ID() const
{
    return weapon2ID;
}

void Unit::setWeapon2ID(const QString &value)
{
    weapon2ID = value;
}

void Unit::setAmmo2(const qint32 &value)
{
    ammo2 = value;
}

qint32 Unit::getMaxAmmo1() const
{
    return maxAmmo1;
}

void Unit::setMaxAmmo1(const qint32 &value)
{
    maxAmmo1 = value;
}

QString Unit::getWeapon1ID() const
{
    return weapon1ID;
}

void Unit::setWeapon1ID(const QString &value)
{
    weapon1ID = value;
}

qint32 Unit::getAmmo1() const
{
    return ammo1;
}

void Unit::setAmmo1(const qint32 &value)
{
    ammo1 = value;
}

bool Unit::hasAmmo1() const
{
    if ((maxAmmo1 < 0) || (ammo1 > 0))
    {
        return true;
    }
    else
    {
        return false;
    }
}

float Unit::getHp() const
{
    return hp;
}

qint32 Unit::getHpRounded() const
{
    return Mainapp::roundUp(hp);
}

void Unit::setHp(const float &value)
{
    hp = value;
    if (hp > 10)
    {
        hp = 10.0f;
    }
    qint32 hpValue = Mainapp::roundUp(hp);
    if (hpValue < 10)
    {
        loadIcon(QString::number(hpValue), 0, GameMap::Imagesize / 2);
    }
    else
    {
        // unload the number icons
        unloadIcon("1");
        unloadIcon("2");
        unloadIcon("3");
        unloadIcon("4");
        unloadIcon("5");
        unloadIcon("6");
        unloadIcon("7");
        unloadIcon("8");
        unloadIcon("9");
    }
}

QString Unit::getMovementType()
{
    Mainapp* pApp = Mainapp::getInstance();
    QString function1 = "getMovementType";
    QJSValueList args1;
    QJSValue ret = pApp->getInterpreter()->doFunction(m_UnitID, function1, args1);
    if (ret.isString())
    {
        return ret.toString();
    }
    else
    {
        return "";
    }
}

QString Unit::getUnitID()
{
    return m_UnitID;
}

qint32 Unit::getX() const
{
    return m_Terrain->getX();
}

qint32 Unit::getY() const
{
    return m_Terrain->getY();
}

QPoint Unit::getPosition() const
{
    return QPoint(getX(), getY());
}

void Unit::refill()
{
    ammo1 = maxAmmo1;
    ammo2 = maxAmmo2;
    setFuel(maxFuel);
}

void Unit::setHasMoved(bool value)
{
    m_Moved = value;
    // change unit color
    if (m_Moved)
    {
        for(qint32 i = 0; i < m_pUnitWaitSprites.size(); i++)
        {
            m_pUnitWaitSprites[i]->setVisible(true);
        }
    }
    else
    {
        for(qint32 i = 0; i < m_pUnitWaitSprites.size(); i++)
        {
            m_pUnitWaitSprites[i]->setVisible(false);
        }
    }
}

bool Unit::getHasMoved()
{
    return m_Moved;
}

qint32 Unit::getTerrainDefenseModifier(QPoint position)
{
    qint32 modifier = 0;
    CO* pCO = m_Owner->getCO(0);
    if (pCO != nullptr)
    {
        modifier += pCO->getTerrainDefenseModifier(this, position);
    }
    pCO = m_Owner->getCO(1);
    if (pCO != nullptr)
    {
        modifier += pCO->getTerrainDefenseModifier(this, position);
    }
    return modifier;
}

qint32 Unit::getMovementPoints()
{
    qint32 movementModifier = 0;
    CO* pCO = m_Owner->getCO(0);
    if (pCO != nullptr)
    {
        movementModifier += pCO->getMovementPointModifier(this);
    }
    pCO = m_Owner->getCO(1);
    if (pCO != nullptr)
    {
        movementModifier += pCO->getMovementPointModifier(this);
    }
    qint32 points = baseMovementPoints + movementModifier;
    if (points < 0)
    {
        points = 0;
    }
    if (fuel < points)
    {
        return fuel;
    }
    return points;
}

QStringList Unit::getActionList()
{
    Mainapp* pApp = Mainapp::getInstance();
    QString function1 = "getActions";
    QJSValueList args1;
    QJSValue ret = pApp->getInterpreter()->doFunction(m_UnitID, function1, args1);
    if (ret.isString())
    {
        return ret.toString().split(",");
    }
    else
    {
        return QStringList();
    }
}

void Unit::moveUnitAction(GameAction* pAction)
{
    // reduce fuel
    setFuel(fuel - pAction->getCosts());
    moveUnit(pAction->getMovePath());
}

void Unit::moveUnit(QVector<QPoint> movePath)
{
    if (movePath.size() > 0)
    {
        // reset capture points when moving  a unit
        setCapturePoints(0);

        GameMap* pMap = GameMap::getInstance();
        spUnit pUnit = m_Terrain->getSpUnit();
        m_Terrain->setUnit(nullptr);
        // teleport unit to target position
        Console::print("Moving Unit from " + QString::number(getX()) + " , " + QString::number(getY()) + " to " + QString::number(movePath[0].x()) + " , " + QString::number(movePath[0].y()), Console::eLogLevels::eDEBUG);
        pMap->getTerrain(movePath[0].x(), movePath[0].y())->setUnit(pUnit);
    }
}

void Unit::removeUnit()
{
    m_Terrain->setUnit(nullptr);
}

void Unit::killUnit()
{
    Mainapp* pApp = Mainapp::getInstance();
    QString function1 = "createExplosionAnimation";
    QJSValueList args1;
    args1 << getX();
    args1 << getY();
    QJSValue ret = pApp->getInterpreter()->doFunction(m_UnitID, function1, args1);
    removeUnit();
}

void Unit::increaseCapturePoints()
{
    // todo add ko modifications
    // todo add animation
    capturePoints += Mainapp::roundUp(hp);
    // update icons
    setCapturePoints(capturePoints);
}

void Unit::loadIcon(QString iconID, qint32 x, qint32 y)
{
    for (qint32 i = 0; i < m_pIconSprites.size(); i++)
    {
        if (m_pIconSprites[i]->getResAnim()->getName() == iconID.toStdString())
        {
            // already loaded icon
            return;
        }
    }
    UnitSpriteManager* pUnitSpriteManager = UnitSpriteManager::getInstance();
    oxygine::ResAnim* pAnim = pUnitSpriteManager->getResAnim(iconID.toStdString());
    if (pAnim != nullptr)
    {
        oxygine::spSprite pSprite = new oxygine::Sprite();
        if (pAnim->getTotalFrames() > 1)
        {
            oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), pAnim->getTotalFrames() * GameMap::frameTime, -1);
            pSprite->addTween(tween);
        }
        else
        {
            pSprite->setResAnim(pAnim);
        }
        pSprite->setScale((GameMap::Imagesize / 2) / pAnim->getWidth() );
        pSprite->setPosition(x, y);
        pSprite->setPriority(static_cast<short>(Priorities::Icons));

        this->addChild(pSprite);
        m_pIconSprites.append(pSprite);

        updateIconTweens();
    }
    else
    {
        Console::print("Unable to load icon sprite: " + iconID, Console::eERROR);
    }
}

void Unit::unloadIcon(QString iconID)
{
    UnitSpriteManager* pUnitSpriteManager = UnitSpriteManager::getInstance();
    oxygine::ResAnim* pAnim = pUnitSpriteManager->getResAnim(iconID.toStdString());
    if (pAnim != nullptr)
    {
        for (qint32 i = 0; i < m_pIconSprites.size(); i++)
        {
            if (m_pIconSprites[i]->getResAnim() == pAnim)
            {
                this->removeChild(m_pIconSprites[i]);
                m_pIconSprites.removeAt(i);
                break;
            }
        }
        updateIconTweens();
    }
}

void Unit::updateIconTweens()
{
    for (qint32 i = 0; i < m_pIconSprites.size(); i++)
    {
        m_pIconSprites[i]->removeTweens();
    }
    for (qint32 i = 0; i < 4; i++)
    {
        // calculate positions of the icons
        qint32 x = 0;
        qint32 y = 0;
        switch (i)
        {
            case 0:
            {
                x = 0;
                y = 0;
                break;
            }
            case 1:
            {
                x = GameMap::Imagesize / 2;
                y = 0;
                break;
            }
            case 2:
            {
                x = 0;
                y = GameMap::Imagesize / 2;
                break;
            }
            case 3:
            {
                x = GameMap::Imagesize / 2;
                y = GameMap::Imagesize / 2;
                break;
            }
        }
        // check the amount of icons at this position
        qint32 count = 0;
        for (qint32 i2 = 0; i2 < m_pIconSprites.size(); i2++)
        {
            if ((m_pIconSprites[i2]->getPosition().x == x) &&
                (m_pIconSprites[i2]->getPosition().y == y))
            {
                count++;
            }
        }
        // create the toggle visibility sprites
        if (count > 1)
        {
            qint32 step = 0;
            for (qint32 i2 = 0; i2 < m_pIconSprites.size(); i2++)
            {
                if ((m_pIconSprites[i2]->getPosition().x == x) &&
                    (m_pIconSprites[i2]->getPosition().y == y))
                {
                    qint32 visibileTime = 500;
                    float startTime = static_cast<float>(step) / static_cast<float>(count);
                    float endTime = static_cast<float>(step) / static_cast<float>(count) + 1.0f / static_cast<float>(count);
                    oxygine::spTween tween = oxygine::createTween(TweenToggleVisibility(startTime, endTime), visibileTime * count, -1);
                    m_pIconSprites[i2]->addTween(tween);
                    step++;
                }
            }
        }
    }
}

void Unit::serialize(QDataStream& pStream)
{
    pStream << getVersion();
    pStream << m_UnitID.toStdString().c_str();
    pStream << hp;
    pStream << ammo1;
    pStream << ammo2;
    pStream << fuel;
    pStream << m_Rank;
    pStream << m_Owner->getPlayerID();
    pStream << m_Moved;
    qint32 units = m_TransportUnits.size();
    pStream << units;
    for (qint32 i = 0; i < units; i++)
    {
        m_TransportUnits[i]->serialize(pStream);
    }
    pStream << capturePoints;
}

void Unit::deserialize(QDataStream& pStream)
{
    qint32 version = 0;
    pStream >> version;
    char* id;
    pStream >> id;
    m_UnitID = id;
    initUnit();
    pStream >> hp;
    setHp(hp);
    pStream >> ammo1;
    pStream >> ammo2;
    pStream >> fuel;
    setFuel(fuel);
    pStream >> m_Rank;
    quint32 playerID = 0;
    pStream >> playerID;
    m_Owner = GameMap::getInstance()->getspPlayer(playerID);
    if (version > 1)
    {
        pStream >> m_Moved;
        setHasMoved(m_Moved);
        qint32 units;
        pStream >> units;
        for (qint32 i = 0; i < units; i++)
        {
            m_TransportUnits.append(new Unit());
            m_TransportUnits[i]->deserialize(pStream);
        }
    }
    if (version > 2)
    {
        pStream >> capturePoints;
        setCapturePoints(capturePoints);
    }
    updateSprites();
}