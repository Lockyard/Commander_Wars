#ifndef CONVOLUTIONALNNMODULE_H
#define CONVOLUTIONALNNMODULE_H

#include "adaptamodule.h"
#include "ai/adaptaai.h"

class ConvolutionalNNModule : public AdaptaModule
{
public:
    struct asd{
        Unit* pUnit;
    };

    ConvolutionalNNModule() = default;
    /*//
    ConvolutionalNNModule(Player* pPlayer, AdaptaAI* ai);

    //
    virtual void readIni(QString name) override;

    virtual void processStartOfTurn() override;

    virtual spQmlVectorUnit processHighestBidUnit() override;
    virtual spQmlVectorUnit processUnit(Unit* pUnit) override;
    virtual void notifyUnitUsed(Unit* pUnit) override;

    virtual float getBidFor(Unit* pUnit) override;


    virtual float getHighestBid(bool weighted = true) override;
    virtual Unit* getHighestBidUnit() override;


    virtual bool assignWeightVector(WeightVector weightVector) override;
    //*/
};

#endif // CONVOLUTIONALNNMODULE_H
