var Constructor = function()
{
    this.init = function(unit)
    {
        unit.setAmmo1(3);
        unit.setMaxAmmo1(3);
        unit.setWeapon1ID("");

        unit.setAmmo2(10);
        unit.setMaxAmmo2(10);
        unit.setWeapon2ID("WEAPON_FLARE_MG");

        unit.setFuel(60);
        unit.setMaxFuel(60);
        unit.setBaseMovementPoints(5);
        unit.setMinRange(1);
        unit.setMaxRange(1);
        unit.setVision(2);
    };
    // called for loading the main sprite
    this.loadSprites = function(unit)
    {
        // load sprites
        unit.loadSprite("flare", false);
        unit.loadSprite("flare+mask", true);
    };
    this.doWalkingAnimation = function(action)
    {
        var unit = action.getTargetUnit();
        var animation = GameAnimationFactory.createWalkingAnimation(unit, action);
        var unitID = unit.getUnitID().toLowerCase();
        animation.loadSprite(unitID + "+walk+mask", true, 1.5);
        animation.loadSprite(unitID + "+walk", false, 1.5);
        animation.setSound("movetank.wav", -2);
        return animation;
    };
    this.getMovementType = function()
    {
        return "MOVE_TANK";
    };
    this.getBaseCost = function()
    {
        return 5000;
    };
    this.getName = function()
    {
        return qsTr("Flare");
    };
    this.canMoveAndFire = function()
    {
        return true;
    };
    this.getActions = function()
    {
        // returns a string id list of the actions this unit can perform
        return "ACTION_FLARE,ACTION_FIRE,ACTION_JOIN,ACTION_WAIT,ACTION_CO_UNIT_0,ACTION_CO_UNIT_1";
    };

    this.getDescription = function()
    {
        return "<r>" + qsTr("In Fog of War, can fire ") + "</r>" +
               "<div c='#00ff00'>" + qsTr("flare rocket") + "</div>" +
               "<r>" + qsTr(". Illuminates ") + "</r>" +
               "<div c='#00ff00'>" + qsTr("2 square area") + "</div>" +
               "<r>" + qsTr(" where it lands.") + "</r>";
    };
    this.getUnitType = function()
    {
        return GameEnums.UnitType_Ground;
    };
}

Constructor.prototype = UNIT;
var FLARE = new Constructor();
