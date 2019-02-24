var Constructor = function()
{
    this.init = function(unit)
    {
        unit.setAmmo1(5);
        unit.setMaxAmmo1(5);
        unit.setWeapon2ID("WEAPON_HEAVY_TANK_GUN");
        unit.setAmmo2(10);
        unit.setMaxAmmo2(10);
        unit.setWeapon2ID("WEAPON_HEAVY_TANK_MG");
        unit.setFuel(50);
        unit.setMaxFuel(50);
        unit.setBaseMovementPoints(5);
        unit.setMinRange(1);
        unit.setMaxRange(1);
        unit.setVision(2);
    };
    // called for loading the main sprite
    this.loadSprites = function(unit)
    {
        // load sprites
        unit.loadSprite("heavy_tank", false);
        unit.loadSprite("heavy_tank+mask", true);
    };
    this.getMovementType = function()
    {
        return "MOVE_TANK";
    };
    this.getBaseCost = function()
    {
        return 12000;
    };
    this.getName = function()
    {
        return qsTr("Heavy Tank");
    };
    this.canMoveAndFire = function()
    {
        return true;
    };
}

Constructor.prototype = UNIT;
var HEAVY_TANK = new Constructor();