#ifndef EVOENUMS_H
#define EVOENUMS_H

namespace evoenums {

    enum CrossoverType {
        splitMiddle,
        splitRandom,
        mixRandom,
        custom,
    };

    enum MutationType {
        individualRandom,
    };

    enum TransferLearningType {
        noTL,
        minTL,
    };

    enum MinMaxWeightMaskType {
        noMask,
        minModuleMask,
    };
}

#endif // EVOENUMS_H
