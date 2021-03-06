var Constructor = function()
{
    this.getMaxUnitCount = function()
    {
        return 5;
    };

    this.loadStandingAnimation = function(sprite, unit, defender, weapon)
    {
        sprite.loadSprite("artillerycraft",  false,
                          BATTLEANIMATION_ARTILLERYCRAFT.getMaxUnitCount(), Qt.point(-55, 5));
        sprite.loadSpriteV2("artillerycraft+mask", GameEnums.Recoloring_Table,
                            BATTLEANIMATION_ARTILLERYCRAFT.getMaxUnitCount(), Qt.point(-55, 5));
    };

    this.loadFireAnimation = function(sprite, unit, defender, weapon)
    {
        BATTLEANIMATION_ARTILLERYCRAFT.loadStandingAnimation(sprite, unit, defender, weapon);
        var offset = Qt.point(-12, 42);
        var count = sprite.getUnitCount(BATTLEANIMATION_ARTILLERYCRAFT.getMaxUnitCount());
        // gun
        sprite.loadSprite("artillery_shot",  false, sprite.getMaxUnitCount(), offset,
                          1, 1.0, 0, 0);
        for (var i = 0; i < count; i++)
        {
            sprite.loadSound("cannon_weapon_fire.wav", 1, "resources/sounds/", i * BATTLEANIMATION.defaultFrameDelay);
        }
    };

    this.getFireDurationMS = function(sprite, unit, defender, weapon)
    {
        // the time will be scaled with animation speed inside the engine
        return 500 + BATTLEANIMATION.defaultFrameDelay * BATTLEANIMATION_ARTILLERYCRAFT.getMaxUnitCount();
    };
};

Constructor.prototype = BATTLEANIMATION;
var BATTLEANIMATION_ARTILLERYCRAFT = new Constructor();
