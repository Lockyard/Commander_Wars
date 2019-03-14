#include "game/GameEnums.h"

#include "coreengine/mainapp.h"

#include "coreengine/interpreter.h"

void GameEnums::registerEnums()
{
    Interpreter* pInterpreter = Mainapp::getInstance()->getInterpreter();
    QJSValue value = pInterpreter->newQObject(new GameEnums());
    value.setProperty("Alliance_Friend", Alliance_Friend);
    value.setProperty("Alliance_Enemy", Alliance_Enemy);

    value.setProperty("PowerMode_Off", PowerMode_Off);
    value.setProperty("PowerMode_Power", PowerMode_Power);
    value.setProperty("PowerMode_Superpower", PowerMode_Superpower);

    value.setProperty("RocketTarget_Money", RocketTarget_Money);
    value.setProperty("RocketTarget_HpHighMoney", RocketTarget_HpHighMoney);
    value.setProperty("RocketTarget_HpLowMoney", RocketTarget_HpLowMoney);

    value.setProperty("UnitRank_None", UnitRank_None);
    value.setProperty("UnitRank_Lieutenant", UnitRank_Lieutenant);
    value.setProperty("UnitRank_General", UnitRank_General);
    value.setProperty("UnitRank_Veteran", UnitRank_Veteran);
    value.setProperty("UnitRank_CO0", UnitRank_CO0);
    value.setProperty("UnitRank_CO1", UnitRank_CO1);

    value.setProperty("Directions_None", Directions_None);
    value.setProperty("Directions_North", Directions_North);
    value.setProperty("Directions_NorthEast", Directions_NorthEast);
    value.setProperty("Directions_East", Directions_East);
    value.setProperty("Directions_SouthEast", Directions_SouthEast);
    value.setProperty("Directions_South", Directions_South);
    value.setProperty("Directions_SouthWest", Directions_SouthWest);
    value.setProperty("Directions_West", Directions_West);
    value.setProperty("Directions_NorthWest", Directions_NorthWest);
    value.setProperty("Directions_All", Directions_All);
    value.setProperty("Directions_Direct", Directions_Direct);
    value.setProperty("Directions_Diagnonal", Directions_Diagnonal);

    pInterpreter->setGlobal("GameEnums", value);
}