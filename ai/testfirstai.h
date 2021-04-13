#ifndef TESTFIRSTAI_H
#define TESTFIRSTAI_H

#include "coreai.h"
#include "adapta/influencemap.h"
#include "influencefrontmap.h"
#include "ai/genetic/evolutionmanager.h"

/**
 * @brief The TestFirstAI class is just a AI playground where I can understand which methods and stuff I have to do in order
 * to make a AI play. It will be incredibly dumb since making it clever is not its purpose.
 */
class TestFirstAI : public CoreAI
{
    Q_OBJECT
public:
    TestFirstAI();

    virtual void process() override;

    virtual void readIni(QString name) override;

public slots:
    void testPrint();
protected:
    virtual void finishTurn() override;

private:
    bool rebuildIslandMaps;
    InfluenceMap m_influenceMap;
    InfluenceFrontMap m_inffMap;

    /**
     * @brief moveAUnit
     * @param pUnits
     * @return true if a unit was moved
     */
    bool moveAUnit(spQmlVectorUnit pUnits);

    bool buildAUnit(spQmlVectorBuilding pBuildings);



};

#endif // TESTFIRSTAI_H
