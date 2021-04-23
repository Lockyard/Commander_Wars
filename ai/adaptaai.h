#ifndef ADAPTAAI_H
#define ADAPTAAI_H

#include "coreai.h"
#include <QObject>
#include <QVector>
#include "adapta/adaptamodule.h"
#include "adapta/buildingmodule.h"


class AdaptaAI : public CoreAI
{
    Q_OBJECT
public:
    AdaptaAI();

    virtual void readIni(QString name) override;

    virtual void process() override;

protected:
    virtual void finishTurn() override;



private:
    //all loaded modules of the adapta
    QVector<AdaptaModule> m_modules;
    QVector<BuildingModule> m_buildingModules;

    QVector<float> m_unitBidVector;

    bool m_isFirstProcessOfTurn;
    bool m_ignoreBids;
};

#endif // ADAPTAAI_H
