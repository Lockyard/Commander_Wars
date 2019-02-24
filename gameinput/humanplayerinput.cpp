#include "humanplayerinput.h"

#include <QSound>

#include "game/gamemap.h"

#include "game/gameaction.h"

#include "game/terrain.h"

#include "game/unitpathfindingsystem.h"

#include "game/building.h"

#include "game/player.h"
#include "game/co.h"

#include "game/gameanimationfactory.h"
#include "resource_management/gamemanager.h"
#include "resource_management/fontmanager.h"

#include "coreengine/mainapp.h"

#include "coreengine/interpreter.h"

#include "gameinput/markedfielddata.h"

HumanPlayerInput::HumanPlayerInput(GameMenue* pMenue)
    : m_pMenue(pMenue)
{
    connect(pMenue, SIGNAL(sigRightClick(qint32,qint32)), this, SLOT(rightClick(qint32,qint32)), Qt::QueuedConnection);
    connect(pMenue, SIGNAL(sigLeftClick(qint32,qint32)), this, SLOT(leftClick(qint32,qint32)), Qt::QueuedConnection);
    connect(pMenue->getCursor(), SIGNAL(sigCursorMoved(qint32,qint32)), this, SLOT(cursorMoved(qint32,qint32)), Qt::QueuedConnection);
    connect(this, SIGNAL(performAction(GameAction*)), pMenue, SLOT(performAction(GameAction*)), Qt::QueuedConnection);
}

HumanPlayerInput::~HumanPlayerInput()
{
    delete m_pGameAction;
    m_pGameAction = nullptr;
    delete m_pUnitPathFindingSystem;
    m_pUnitPathFindingSystem = nullptr;
}


void HumanPlayerInput::rightClick(qint32 x, qint32 y)
{
    if (GameAnimationFactory::getAnimationCount() > 0)
    {
        GameAnimationFactory::finishAllAnimations();
    }
    else if (m_pGameAction != nullptr)
    {
        if (m_pGameAction->getInputStep() > 0)
        {
            // todo implement go back steps
            cleanUpInput();
        }
        else if (m_CurrentMenu.get() == nullptr)
        {
            cleanUpInput();
        }
        else
        {
            Unit* pUnit = m_pGameAction->getTargetUnit();
            if (pUnit != nullptr)
            {
                if (m_pGameAction->getInputStep() == 0)
                {
                    // go one step back :)
                    m_CurrentMenu->getParent()->removeChild(m_CurrentMenu);
                    m_CurrentMenu = nullptr;
                    createMarkedMoveFields();
                }
            }
            else
            {
                cleanUpInput();
            }
        }
    }
}

void HumanPlayerInput::cleanUpInput()
{
    clearMenu();
    // delete action
    delete m_pGameAction;
    m_pGameAction = nullptr;
    delete m_pUnitPathFindingSystem;
    m_pUnitPathFindingSystem = nullptr;
    clearMarkedFields();
    deleteArrow();
    m_pMenue->getCursor()->changeCursor("cursor+default");
}

void HumanPlayerInput::clearMenu()
{
    if (m_CurrentMenu.get() != nullptr)
    {
        m_CurrentMenu->getParent()->removeChild(m_CurrentMenu);
        m_CurrentMenu = nullptr;
    }
}

void HumanPlayerInput::clearMarkedFields()
{
    GameMap* pMap = GameMap::getInstance();
    for (qint32 i = 0; i < m_Fields.size(); i++)
    {
        pMap->getTerrain(m_FieldPoints[i].x(), m_FieldPoints[i].y())->removeChild(m_Fields[i]);
    }
    m_FieldPoints.clear();
    m_Fields.clear();
    if (m_pMarkedFieldData != nullptr)
    {
        delete m_pMarkedFieldData;
        m_pMarkedFieldData = nullptr;
    }
    if (m_ZInformationLabel.get() != nullptr)
    {
        GameMap::getInstance()->removeChild(m_ZInformationLabel);
        m_ZInformationLabel = nullptr;
    }
}

void HumanPlayerInput::leftClick(qint32 x, qint32 y)
{

    if (GameAnimationFactory::getAnimationCount() > 0)
    {
        // do nothing
    }
    else if (m_CurrentMenu.get() != nullptr)
    {
        // do nothing
    }
    else if (m_pMarkedFieldData != nullptr)
    {
        // did we select a marked field?
        if (m_pMarkedFieldData->getAllFields())
        {
            markedFieldSelected(QPoint(x, y));
        }
        else
        {
            QVector<QPoint>* pFields = m_pMarkedFieldData->getPoints();
            for (qint32 i = 0; i < pFields->size(); i++)
            {
                if ((pFields->at(i).x() == x) &&
                    (pFields->at(i).y() == y))
                {
                    markedFieldSelected(QPoint(x, y));
                    break;
                }
            }
        }
    }
    // no action selected
    else if (m_pGameAction == nullptr)
    {
        // prepare action
        m_pGameAction = new GameAction();
        m_pGameAction->setTarget(QPoint(x, y));
        GameMap* pMap = GameMap::getInstance();
        Unit* pUnit = pMap->getTerrain(x, y)->getUnit();
        if (pUnit != nullptr)
        {
            selectUnit(x, y);
        }
        else
        {
            Building* pBuilding = pMap->getTerrain(x, y)->getBuilding();
            QStringList actions;
            QStringList possibleActions;
            if (pBuilding != nullptr)
            {
                actions = pBuilding->getActionList();
                for (qint32 i = 0; i < actions.size(); i++)
                {
                    if (m_pGameAction->canBePerformed(actions[i]))
                    {
                        possibleActions.append(actions[i]);
                    }
                }
            }
            if (possibleActions.size() > 0)
            {
                if ((possibleActions.size() == 1) &&
                    (!m_pGameAction->isFinalStep(actions[0])))
                {
                    // skip show select action menu
                    m_pGameAction->setActionID(actions[0]);
                    getNextStepData();
                }
                else
                {
                    if (possibleActions.size() > 0)
                    {
                        createActionMenu(possibleActions, x, y);
                    }
                }
            }
            else
            {
                actions = getEmptyActionList();
                possibleActions.clear();
                for (qint32 i = 0; i < actions.size(); i++)
                {
                    if (m_pGameAction->canBePerformed(actions[i]))
                    {
                        possibleActions.append(actions[i]);
                    }
                }
                if (possibleActions.size() > 0)
                {
                    createActionMenu(possibleActions, x, y);
                }
                else
                {
                    cleanUpInput();
                }
            }
        }

    }
    // we want to select an action
    else if (m_pGameAction->getActionID() == "")
    {
        if ((m_pUnitPathFindingSystem != nullptr) &&
            (m_pUnitPathFindingSystem->isReachable(x, y)) &&
            (m_CurrentMenu.get() == nullptr))
        {
            Unit* pUnit = m_pGameAction->getTargetUnit();
            m_pGameAction->setMovepath(m_ArrowPoints);
            if (pUnit != nullptr)
            {
                // we want to do something with this unit :)
                QStringList actions = pUnit->getActionList();
                QStringList possibleActions;
                for (qint32 i = 0; i < actions.size(); i++)
                {
                    if (m_pGameAction->canBePerformed(actions[i]))
                    {
                        possibleActions.append(actions[i]);
                    }
                }
                if (possibleActions.size() > 0)
                {
                    createActionMenu(possibleActions, x, y);
                }
            }
        }
        else
        {
            cleanUpInput();
        }
    }
    else
    {
        //cleanUpInput();
    }
}

void HumanPlayerInput::markedFieldSelected(QPoint point)
{
    m_pGameAction->writeDataInt32(point.x());
    m_pGameAction->writeDataInt32(point.y());
    clearMarkedFields();
    m_pGameAction->setInputStep(m_pGameAction->getInputStep() + 1);
    if (m_pGameAction->isFinalStep())
    {
        // if not perform action
        finishAction();
    }
    else
    {
        // else introduce next step
        getNextStepData();
    }
}

void HumanPlayerInput::menuItemSelected(QString itemID, qint32 cost)
{
    // we're currently selecting the action for this action
    if (m_pGameAction->getActionID() == "")
    {
        // set the id
        m_pGameAction->setActionID(itemID);
        m_pGameAction->setCosts(m_pGameAction->getCosts() + cost);
        // check if the action needs further information
    }
    // we want to append some data to the action
    else
    {
        m_pGameAction->writeDataString(itemID);
        // increase costs and input step
        m_pGameAction->setCosts(m_pGameAction->getCosts() + cost);
        m_pGameAction->setInputStep(m_pGameAction->getInputStep() + 1);
    }
    if (m_pGameAction->isFinalStep())
    {
        // if not perform action
        finishAction();
    }
    else
    {
        // else introduce next step
        getNextStepData();
    }
}

void HumanPlayerInput::getNextStepData()
{
    clearMenu();
    clearMarkedFields();
    m_pMenue->getCursor()->changeCursor("cursor+default");

    QString stepType = m_pGameAction->getStepInputType();
    if (stepType.toUpper() == "MENU")
    {
        GameMap* pMap = GameMap::getInstance();
        MenuData* pData = m_pGameAction->getMenuStepData();
        m_CurrentMenu = new HumanPlayerInputMenu(pData->getTexts(), pData->getActionIDs(), pData->getIconList(), pData->getCostList(), pData->getEnabledList());
        m_CurrentMenu->setMenuPosition(m_pGameAction->getActionTarget().x() * GameMap::Imagesize, m_pGameAction->getActionTarget().y() * GameMap::Imagesize);
        pMap->addChild(m_CurrentMenu);
        connect(m_CurrentMenu.get(), SIGNAL(sigItemSelected(QString, qint32)), this, SLOT(menuItemSelected(QString, qint32)), Qt::QueuedConnection);
        delete pData;
    }
    else if (stepType.toUpper() == "FIELD")
    {
        MarkedFieldData* pData = m_pGameAction->getMarkedFieldStepData();
        QVector<QPoint>* pFields = pData->getPoints();
        for (qint32 i = 0; i < pFields->size(); i++)
        {
            createMarkedField(pFields->at(i), pData->getColor(), Terrain::DrawPriority::MarkedFieldLow);
        }
        m_pMarkedFieldData = pData;
        m_pMenue->getCursor()->changeCursor(m_pGameAction->getStepCursor());
    }
}

void HumanPlayerInput::finishAction()
{
    emit performAction(m_pGameAction);
    m_pGameAction = nullptr;
    cleanUpInput();
}



void HumanPlayerInput::createActionMenu(QStringList actionIDs, qint32 x, qint32 y)
{
    clearMarkedFields();
    MenuData data;
    GameMap* pMap = GameMap::getInstance();
    for (qint32 i = 0; i < actionIDs.size(); i++)
    {
        data.addData(GameAction::getActionText(actionIDs[i]), actionIDs[i], GameAction::getActionIcon(actionIDs[i]));
    }
    m_CurrentMenu = new HumanPlayerInputMenu(data.getTexts(), actionIDs, data.getIconList());
    m_CurrentMenu->setMenuPosition(x * GameMap::Imagesize, y * GameMap::Imagesize);

    pMap->addChild(m_CurrentMenu);
    connect(m_CurrentMenu.get(), SIGNAL(sigItemSelected(QString, qint32)), this, SLOT(menuItemSelected(QString, qint32)), Qt::QueuedConnection);
}

void HumanPlayerInput::selectUnit(qint32 x, qint32 y)
{
    QSound::play("resources/sounds/selectunit.wav");

    GameMap* pMap = GameMap::getInstance();
    m_pUnitPathFindingSystem = new UnitPathFindingSystem(pMap->getTerrain(x, y)->getSpUnit());
    m_pUnitPathFindingSystem->explore();
    createMarkedMoveFields();
}

void HumanPlayerInput::createMarkedField(QPoint point, QColor color, Terrain::DrawPriority drawPriority)
{
    GameMap* pMap = GameMap::getInstance();
    GameManager* pGameManager = GameManager::getInstance();
    oxygine::spSprite pSprite = new oxygine::Sprite();
    oxygine::ResAnim* pAnim = pGameManager->getResAnim("marked+field");
    if (pAnim->getTotalFrames() > 1)
    {
        oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), pAnim->getTotalFrames() * GameMap::frameTime, -1);
        pSprite->addTween(tween);
    }
    else
    {
        pSprite->setResAnim(pAnim);
    }
    oxygine::Sprite::TweenColor tweenColor(oxygine::Color(color.red(), color.green(), color.blue(), color.alpha()));
    oxygine::spTween tween2 = oxygine::createTween(tweenColor, 1);
    pSprite->addTween(tween2);
    pSprite->setPriority(static_cast<qint8>(drawPriority));
    pSprite->setScale(GameMap::Imagesize / pAnim->getWidth());
    pSprite->setPosition(-(pSprite->getScaledWidth() - GameMap::Imagesize) / 2, -(pSprite->getScaledHeight() - GameMap::Imagesize));
    pMap->getSpTerrain(point.x(), point.y())->addChild(pSprite);
    m_Fields.append(pSprite);
    m_FieldPoints.append(point);
}

void HumanPlayerInput::createMarkedMoveFields()
{

    QVector<QPoint> points = m_pUnitPathFindingSystem->getAllNodePoints();
    for (qint32 i = 0; i < points.size(); i++)
    {
        createMarkedField(points[i], QColor(50, 230, 200, 255), Terrain::DrawPriority::MarkedFieldLow);
    }
}

void HumanPlayerInput::cursorMoved(qint32 x, qint32 y)
{
    if (m_pMarkedFieldData != nullptr)
    {
        if (m_pMarkedFieldData->getShowZData())
        {
            // marked field?
            if (m_pMarkedFieldData->getPoints()->contains(QPoint(x, y)))
            {
                if (m_ZInformationLabel.get() != nullptr)
                {
                    GameMap::getInstance()->removeChild(m_ZInformationLabel);
                    m_ZInformationLabel = nullptr;
                }
                m_ZInformationLabel = new oxygine::Actor();
                GameManager* pGameManager = GameManager::getInstance();
                oxygine::spSprite pSprite = new oxygine::Sprite();
                oxygine::ResAnim* pAnim = pGameManager->getResAnim("z_information_label");
                if (pAnim->getTotalFrames() > 1)
                {
                    oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), pAnim->getTotalFrames() * GameMap::frameTime, -1);
                    pSprite->addTween(tween);
                }
                else
                {
                    pSprite->setResAnim(pAnim);
                }
                oxygine::spSprite pSprite2 = new oxygine::Sprite();
                oxygine::ResAnim* pAnim2 = pGameManager->getResAnim("z_information_label+mask");
                if (pAnim2->getTotalFrames() > 1)
                {
                    oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim2), pAnim2->getTotalFrames() * GameMap::frameTime, -1);
                    pSprite2->addTween(tween);
                }
                else
                {
                    pSprite2->setResAnim(pAnim2);
                }
                QColor color = m_pMarkedFieldData->getZLabelColor();
                pSprite2->setColor(color.red(), color.green(), color.blue(), color.alpha());
                m_ZInformationLabel->addChild(pSprite);
                m_ZInformationLabel->addChild(pSprite2);
                // add text to the label
                oxygine::spTextField textField = new oxygine::TextField();
                oxygine::TextStyle style = oxygine::TextStyle(FontManager::getTimesFont10()).
                                           withColor(oxygine::Color(0, 0, 0)).
                                           alignLeft().
                                           alignTop();
                textField->setStyle(style);
                textField->setScale(0.6f);
                textField->setY(-2);
                textField->setText(m_pMarkedFieldData->getZLabelText().toStdString().c_str());
                textField->attachTo(m_ZInformationLabel);

                oxygine::spTextField textField2 = new oxygine::TextField();
                textField2->setStyle(style);
                textField2->setScale(0.7f);
                textField2->setY(8);
                textField2->setX(3);
                QString labelText = "";
                QPoint field(x, y);
                for (qint32 i = 0; i < m_pMarkedFieldData->getPoints()->size(); i++)
                {
                    if (m_pMarkedFieldData->getPoints()->at(i) == field)
                    {
                        labelText = QString::number(m_pMarkedFieldData->getZInformation()->at(i)) + " %";
                        break;
                    }
                }
                textField2->setText(labelText.toStdString().c_str());
                textField2->attachTo(m_ZInformationLabel);

                m_ZInformationLabel->setScale(1.5f);
                m_ZInformationLabel->setPosition(x * GameMap::Imagesize - GameMap::Imagesize / 6.0f,
                                                 y * GameMap::Imagesize - GameMap::Imagesize * 1.5f);
                m_ZInformationLabel->setPriority(static_cast<qint8>(Mainapp::ZOrder::Animation));
                GameMap::getInstance()->addChild(m_ZInformationLabel);
            }
            else
            {
                if (m_ZInformationLabel.get() != nullptr)
                {
                    GameMap::getInstance()->removeChild(m_ZInformationLabel);
                    m_ZInformationLabel = nullptr;
                }
            }
        }
    }
    else if (m_pUnitPathFindingSystem != nullptr)
    {
        if ((m_CurrentMenu.get() == nullptr) && m_pGameAction->getActionID() == "")
        {
            createCursorPath(x, y);
        }
    }
}

void HumanPlayerInput::createCursorPath(qint32 x, qint32 y)
{
    QVector<QPoint> points = m_ArrowPoints;
    deleteArrow();
    if (m_pUnitPathFindingSystem->getCosts(x, y) > 0)
    {
        // is it a neighbour field to the last target?
        qint32 fieldCosts = m_pUnitPathFindingSystem->getCosts(m_pGameAction->getTarget().x(), m_pGameAction->getTarget().y());
        if (((points.size() > 0) && ((points[0].x() - x + points[0].y() - y) != 0)))
        {
            if ((points.size() > 0) && ((qAbs(points[0].x() - x) + qAbs(points[0].y() - y)) == 1))
            {
                if (points.contains(QPoint(x, y)))
                {
                    points = m_pUnitPathFindingSystem->getPath(x, y);
                }
                else
                {
                    points.push_front(QPoint(x, y));
                    if (m_pUnitPathFindingSystem->getCosts(points) - fieldCosts > m_pGameAction->getTargetUnit()->getMovementPoints() )
                    {
                        // not reachable this way get the ideal path
                        points = m_pUnitPathFindingSystem->getPath(x, y);
                    }
                }
            }
            else
            {
                points = m_pUnitPathFindingSystem->getPath(x, y);
            }
        }
        else if (points.size() == 0)
        {
            points = m_pUnitPathFindingSystem->getPath(x, y);
        }
        else
        {
            // do nothing
        }
        if ((points.size() == 0) ||
            (points[0].x() != x) ||
            (points[0].y() != y))
        {
            points = m_pUnitPathFindingSystem->getPath(x, y);
        }
        m_pGameAction->setCosts(m_pUnitPathFindingSystem->getCosts(points) - fieldCosts);
        m_ArrowPoints = points;
        GameMap* pMap = GameMap::getInstance();
        GameManager* pGameManager = GameManager::getInstance();
        for (qint32 i = 0; i < points.size() - 1; i++)
        {
            oxygine::spSprite pSprite = new oxygine::Sprite();
            oxygine::ResAnim* pAnim = pGameManager->getResAnim("arrow+unit");
            pSprite->setResAnim(pAnim);
            pSprite->setPriority(static_cast<qint8>(Terrain::DrawPriority::Arrow));
            pSprite->setScale(GameMap::Imagesize / pAnim->getWidth());
            pSprite->setPosition(-(pSprite->getScaledWidth() - GameMap::Imagesize) / 2, -(pSprite->getScaledHeight() - GameMap::Imagesize));
            pMap->getSpTerrain(points[i].x(), points[i].y())->addChild(pSprite);
            m_Arrows.append(pSprite);

            if (i > 0)
            {
                // select arrow
                if (((points[i].x() < points[i + 1].x()) && (points[i].x() > points[i - 1].x())) ||
                    ((points[i].x() < points[i - 1].x()) && (points[i].x() > points[i + 1].x())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::LeftRight));
                }
                else if (((points[i].y() < points[i + 1].y()) && (points[i].y() > points[i - 1].y())) ||
                         ((points[i].y() < points[i - 1].y()) && (points[i].y() > points[i + 1].y())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::UpDown));
                }
                else if (((points[i].y() < points[i + 1].y()) && (points[i].x() < points[i - 1].x())) ||
                         ((points[i].y() < points[i - 1].y()) && (points[i].x() < points[i + 1].x())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::DownRight));
                }
                else if (((points[i].y() < points[i + 1].y()) && (points[i].x() > points[i - 1].x())) ||
                         ((points[i].y() < points[i - 1].y()) && (points[i].x() > points[i + 1].x())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::DownLeft));
                }
                else if (((points[i].y() > points[i + 1].y()) && (points[i].x() < points[i - 1].x())) ||
                         ((points[i].y() > points[i - 1].y()) && (points[i].x() < points[i + 1].x())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::UpRight));
                }
                else if (((points[i].y() > points[i + 1].y()) && (points[i].x() > points[i - 1].x())) ||
                         ((points[i].y() > points[i - 1].y()) && (points[i].x() > points[i + 1].x())))
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::UpLeft));
                }
            }
            else
            {
                // final arrow
                if (points[i].x() < points[i + 1].x())
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::Right));
                }
                else if (points[i].x() > points[i + 1].x())
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::Left));
                }
                else if (points[i].y() < points[i + 1].y())
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::Down));
                }
                else if (points[i].y() > points[i + 1].y())
                {
                    pSprite->setColumn(static_cast<qint32>(Arrows::Up));
                }
            }
        }
    }
}

QStringList HumanPlayerInput::getEmptyActionList()
{
    Interpreter* pInterpreter = Mainapp::getInstance()->getInterpreter();
    QJSValue value = pInterpreter->doFunction("ACTION", "getEmptyFieldActions");
    if (value.isString())
    {
        return value.toString().split(",");
    }
    else
    {
        return QStringList();
    }
}

void HumanPlayerInput::deleteArrow()
{
    GameMap* pMap = GameMap::getInstance();
    for (qint32 i = 0; i < m_Arrows.size(); i++)
    {
        pMap->getTerrain(m_ArrowPoints[i].x(), m_ArrowPoints[i].y())->removeChild(m_Arrows[i]);
    }
    m_ArrowPoints.clear();
    m_Arrows.clear();
}
