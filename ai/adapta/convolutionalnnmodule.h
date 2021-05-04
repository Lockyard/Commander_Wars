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

    //Methods to be overridden. I wanted to make this class abstract, but it breaks QVectors, so this is it
    virtual void readIni(QString name) override;

    virtual void processStartOfTurn() override;

    virtual bool processHighestBidUnit() override;
    virtual bool processUnit(Unit* pUnit) override;
    virtual void notifyUnitUsed(Unit* pUnit) override;

    virtual float getBidFor(Unit* pUnit) override;


    virtual float getHighestBid(bool weighted = true) override;
    virtual Unit* getHighestBidUnit() override;


    virtual bool assignWeightVector(WeightVector weightVector) override;
    //*/
};

#endif // CONVOLUTIONALNNMODULE_H
