var Constructor = function()
{
    this.init = function(unit)
    {
        unit.setAmmo1(6);
        unit.setMaxAmmo1(6);
        unit.setWeapon1ID("WEAPON_BOMB");
        unit.setAmmo2(0);
        unit.setMaxAmmo2(0);
        unit.setWeapon2ID("");
        unit.setFuel(100);
        unit.setMaxFuel(100);
        unit.setBaseMovementPoints(7);
        unit.setMinRange(1);
        unit.setMaxRange(1);
        unit.setVision(2);
    };
    // called for loading the main sprite
    this.loadSprites = function(unit)
    {
        // load sprites
        unit.loadSprite("bomber", false);
        unit.loadSprite("bomber+mask", true);
    };
    this.getMovementType = function()
    {
        return "MOVE_AIR";
    };
    this.getBaseCost = function()
    {
        return 20000;
    };
    this.getName = function()
    {
        return qsTr("Bomber");
    };
    this.startOfTurn = function(unit)
    {
        // pay unit upkeep
        unit.setFuel(unit.getFuel() - 5);
    };
    this.createExplosionAnimation = function(x, y)
    {
        var animation = GameAnimationFactory.createAnimation(x, y);
        animation.addSprite("explosion+air", 0, -map.getImageSize(), 0, 1.5);
        return animation;
    };
    this.canMoveAndFire = function()
    {
        return true;
    };
}

Constructor.prototype = UNIT;
var BOMBER = new Constructor();