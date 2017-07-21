#pragma once

#include "Common.h"

namespace CasiaBot
{

class AutoObserver
{
    int                         _cameraLastMoved;
    int                         _unitFollowFrames;
    BWAPI::Unit      _observerFollowingUnit;

public:

    AutoObserver();
    void onFrame();
};

}