#include "mapselectionmapsmenue.h"

#include "coreengine/mainapp.h"
#include "coreengine/console.h"

#include "resource_management/backgroundmanager.h"
#include "resource_management/fontmanager.h"
#include "resource_management/objectmanager.h"
#include "resource_management/buildingspritemanager.h"

#include "game/gamemap.h"

#include "game/player.h"

#include "game/co.h"

#include "menue/mainwindow.h"

#include "QFileInfo"

MapSelectionMapsMenue::MapSelectionMapsMenue()
    : QObject()
{
    Console::print("Entering Map Selection Menue", Console::eDEBUG);
    Mainapp* pApp = Mainapp::getInstance();
    BackgroundManager* pBackgroundManager = BackgroundManager::getInstance();
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    BuildingSpriteManager* pBuildingSpriteManager = BuildingSpriteManager::getInstance();
    pBuildingSpriteManager->loadAll();
    // load background
    oxygine::spSprite sprite = new oxygine::Sprite();
    addChild(sprite);
    oxygine::ResAnim* pBackground = pBackgroundManager->getResAnim("Background+1");
    sprite->setResAnim(pBackground);
    sprite->setPosition(0, 0);
    // background should be last to draw
    sprite->setPriority(static_cast<short>(Mainapp::ZOrder::Background));
    sprite->setScaleX(pApp->getSettings()->getWidth() / pBackground->getWidth());
    sprite->setScaleY(pApp->getSettings()->getHeight() / pBackground->getHeight());

    pApp->getAudioThread()->loadFolder("resources/music/mapselectionmenue");
    pApp->getAudioThread()->playRandom();

    qint32 width = 0;
    if (pApp->getSettings()->getWidth() / 2 > 400)
    {
        width = 400;
    }
    else if (pApp->getSettings()->getWidth() / 2 < 300)
    {
        width = 300;
    }
    else
    {
        width = pApp->getSettings()->getWidth() / 2;
    }

    m_pMapSelection = new MapSelection(pApp->getSettings()->getHeight() - 70, width, "");
    m_pMapSelection->setPosition(10, 10);
    this->addChild(m_pMapSelection);
    m_pMinimap = new Minimap();
    m_pMinimap->setPosition(0, 0);
    m_pMinimap->setScale(2.0f);
    m_MinimapSlider = new oxygine::SlidingActor();

    m_MinimapSlider->setPosition(10, 10);
    m_MinimapSlider->setSize(pApp->getSettings()->getWidth() - width - 100 - 20,
                             pApp->getSettings()->getHeight() - 210 - 20);
    m_MinimapSlider->setContent(m_pMinimap);

    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("panel");
    m_pMiniMapBox = new oxygine::Box9Sprite();
    m_pMiniMapBox->setVerticalMode(oxygine::Box9Sprite::STRETCHING);
    m_pMiniMapBox->setHorizontalMode(oxygine::Box9Sprite::STRETCHING);
    m_pMiniMapBox->setResAnim(pAnim);
    m_pMiniMapBox->setPosition(width + 50, 50);
    m_pMiniMapBox->setSize(pApp->getSettings()->getWidth() - width - 100,
                           pApp->getSettings()->getHeight() - 210);

    m_pMiniMapBox->addChild(m_MinimapSlider);
    addChild(m_pMiniMapBox);

    // building count
    pAnim = pObjectManager->getResAnim("mapSelectionBuildingInfo");
    m_pBuildingBackground = new oxygine::Box9Sprite();
    m_pBuildingBackground->setVerticalMode(oxygine::Box9Sprite::STRETCHING);
    m_pBuildingBackground->setHorizontalMode(oxygine::Box9Sprite::STRETCHING);
    m_pBuildingBackground->setResAnim(pAnim);
    m_pBuildingBackground->setSize(pApp->getSettings()->getWidth() - width - 100, 60);
    m_pBuildingBackground->setPosition(m_pMiniMapBox->getX(),
                                     m_pMiniMapBox->getY() + m_pMiniMapBox->getHeight() + 20);
    oxygine::TextStyle style = FontManager::getTimesFont10();
    style.color = oxygine::Color(255, 255, 255, 255);
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;

    oxygine::spSlidingActor slider = new oxygine::SlidingActor();
    slider->setSize(m_pBuildingBackground->getWidth() - 20, 100);
    oxygine::spActor content = new oxygine::Actor();
    content->setSize(pBuildingSpriteManager->getBuildingCount()* (GameMap::Imagesize + 12), 100);
    for (qint32 i = 0; i < pBuildingSpriteManager->getBuildingCount(); i++)
    {
        spBuilding building = new Building(pBuildingSpriteManager->getBuildingID(i));
        building->updateBuildingSprites();
        building->setPosition(i * (GameMap::Imagesize + 12), 5 + GameMap::Imagesize / 2);
        content->addChild(building);
        oxygine::spTextField pText = new oxygine::TextField();
        pText->setText("0");
        pText->setPosition(2 + i * (GameMap::Imagesize + 12), 10 + GameMap::Imagesize * 1.2f);
        pText->setStyle(style);
        content->addChild(pText);
        m_BuildingCountTexts.push_back(pText);
    }
    slider->setX(10);
    slider->setContent(content);
    m_pBuildingBackground->addChild(slider);
    addChild(m_pBuildingBackground);

    connect(m_pMapSelection.get(), SIGNAL(itemChanged(QString)), this, SLOT(mapSelectionItemChanged(QString)), Qt::QueuedConnection);
    connect(m_pMapSelection.get(), SIGNAL(itemClicked(QString)), this, SLOT(mapSelectionItemClicked(QString)), Qt::QueuedConnection);

    oxygine::spButton pButtonBack = ObjectManager::createButton(tr("Back"));
    pButtonBack->setPosition(10, pApp->getSettings()->getHeight() - 10 - pButtonBack->getHeight());
    pButtonBack->attachTo(this);
    pButtonBack->addEventListener(oxygine::TouchEvent::CLICK, [=](oxygine::Event * )->void
    {
        emit buttonBack();
    });
    connect(this, SIGNAL(buttonBack()), this, SLOT(slotButtonBack()), Qt::QueuedConnection);
    oxygine::spButton pButtonNext = ObjectManager::createButton(tr("Next"));
    pButtonNext->setPosition(pApp->getSettings()->getWidth() - 10 - pButtonBack->getWidth(), pApp->getSettings()->getHeight() - 10 - pButtonBack->getHeight());
    pButtonNext->attachTo(this);
    pButtonNext->addEventListener(oxygine::TouchEvent::CLICK, [=](oxygine::Event * )->void
    {
        emit buttonNext();
    });
    connect(this, SIGNAL(buttonNext()), this, SLOT(slotButtonNext()), Qt::QueuedConnection);



    // co selection
    m_pCOSelection = new COSelection();
    m_pCOSelection->setPosition(10, 10);
    m_pCOSelection->setScale(1.25f);
    addChild(m_pCOSelection);

    qint32 yPos = m_pCOSelection->getY() + m_pCOSelection->getScaledHeight() + 10;
    m_pPlayerSelection = new Panel(true,
                                   QSize(pApp->getSettings()->getWidth() - 20,
                                         pApp->getSettings()->getHeight() - yPos - 20 - pButtonBack->getHeight()),
                                   QSize(pApp->getSettings()->getWidth() - 70, 100));
    m_pPlayerSelection->setPosition(10, yPos);
    this->addChild(m_pPlayerSelection);
    hideCOSelection();
}

MapSelectionMapsMenue::~MapSelectionMapsMenue()
{

}

void MapSelectionMapsMenue::slotButtonBack()
{
    switch (m_MapSelectionStep)
    {
        case MapSelectionStep::selectMap:
        {
            Console::print("Leaving Map Selection Menue", Console::eDEBUG);
            oxygine::getStage()->addChild(new Mainwindow());
            oxygine::Actor::detach();
            break;
        }
        case MapSelectionStep::selectRules:
        {
            showMapSelection();
            m_MapSelectionStep = MapSelectionStep::selectMap;
            break;
        }
        case MapSelectionStep::selectPlayer:
        {
            hideCOSelection();
            m_MapSelectionStep = MapSelectionStep::selectRules;
            break;
        }
    }
}

void MapSelectionMapsMenue::slotButtonNext()
{
    switch (m_MapSelectionStep)
    {
        case MapSelectionStep::selectMap:
        {
            if (m_pCurrentMap != nullptr)
            {
                hideMapSelection();
                m_MapSelectionStep = MapSelectionStep::selectRules;
            }
            break;
        }
        case MapSelectionStep::selectRules:
        {
            showCOSelection();
            m_MapSelectionStep = MapSelectionStep::selectPlayer;
            break;
        }
        case MapSelectionStep::selectPlayer:
        {
            break;
        }
    }
}

void MapSelectionMapsMenue::mapSelectionItemClicked(QString item)
{
    QFileInfo info = m_pMapSelection->getCurrentFolder() + item;
    if (info.isFile())
    {
        emit buttonNext();
    }
}

void MapSelectionMapsMenue::mapSelectionItemChanged(QString item)
{
    QFileInfo info = m_pMapSelection->getCurrentFolder() + item;
    if (info.isFile())
    {
        if (m_pCurrentMap != nullptr)
        {
            delete m_pCurrentMap;
            m_pCurrentMap = nullptr;
        }
        m_pCurrentMap = new GameMap(info.absoluteFilePath(), false, true);
        m_pMinimap->updateMinimap(m_pCurrentMap);
        m_MinimapSlider->setContent(m_pMinimap);
        m_MinimapSlider->snap();
        BuildingSpriteManager* pBuildingSpriteManager = BuildingSpriteManager::getInstance();
        for (qint32 i = 0; i < pBuildingSpriteManager->getBuildingCount(); i++)
        {
            qint32 count = m_pCurrentMap->getBuildingCount(pBuildingSpriteManager->getBuildingID(i));
            m_BuildingCountTexts[i]->setText(QString::number(count).toStdString().c_str());
        }
    }
}

void MapSelectionMapsMenue::hideMapSelection()
{
    m_pMapSelection->setVisible(false);
    m_pMiniMapBox->setVisible(false);
    m_pBuildingBackground->setVisible(false);
}
void MapSelectionMapsMenue::showMapSelection()
{
    m_pMapSelection->setVisible(true);
    m_pMiniMapBox->setVisible(true);
    m_pBuildingBackground->setVisible(true);
}

void MapSelectionMapsMenue::hideCOSelection()
{
    m_pCOSelection->setVisible(false);
    m_pPlayerSelection->setVisible(false);
    m_pPlayerSelection->clearContent();
}
void MapSelectionMapsMenue::showCOSelection()
{
    m_pPlayerSelection->setVisible(true);
    m_pCOSelection->setVisible(true);
    m_pCOSelection->colorChanged(m_pCurrentMap->getPlayer(0)->getColor());

    // font style
    oxygine::TextStyle style = FontManager::getMainFont();
    style.color = oxygine::Color(255, 255, 255, 255);
    style.vAlign = oxygine::TextStyle::VALIGN_DEFAULT;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;

    // add player labels at top
    QStringList items = {tr("Username"), tr("CO's"), tr("Color"), tr("AI Strength"), tr("Startfonds"), tr("Income"), tr("Team")};
    qint32 labelminStepSize = 100;
    qint32 curPos = 5;
    for (qint32 i = 0; i < items.size(); i++)
    {
        oxygine::spTextField pLabel = new oxygine::TextField();
        pLabel->setStyle(style);
        pLabel->setText(items[i].toStdString().c_str());
        qint32 width = pLabel->getTextRect().getWidth() + 10;
        pLabel->setPosition(curPos, 5);
        m_pPlayerSelection->addItem(pLabel);
        if (width < labelminStepSize)
        {
            width = labelminStepSize;
        }
        curPos += width;
    }
    // add player selection information
    for (qint32 i = 0; i < m_pCurrentMap->getPlayerCount(); i++)
    {

    }
}
