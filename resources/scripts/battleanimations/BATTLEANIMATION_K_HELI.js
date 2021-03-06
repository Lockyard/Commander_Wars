var Constructor = function()
{
    this.getMaxUnitCount = function()
    {
        return 5;
    };
    this.armyData = [["os", "os"],
                     ["bm", "bm"],
                     ["ge", "ge"],
                     ["yc", "yc"],
                     ["bh", "bh"],
                     ["bg", "bh"],
                     ["ma", "ma"],];
    this.loadStandingAnimation = function(sprite, unit, defender, weapon)
    {
        var player = unit.getOwner();
        // get army name
        var armyName = Global.getArmyNameFromPlayerTable(player, BATTLEANIMATION_K_HELI.armyData);
        var offset = Qt.point(-40, 30);
        if (armyName === "ma")
        {
            offset = Qt.point(-30, 20);
        }
        sprite.loadSpriteV2("k_heli+" + armyName + "+mask", GameEnums.Recoloring_Table,
                            BATTLEANIMATION_K_HELI.getMaxUnitCount(), offset);
        sprite.addMoveTweenToLastLoadedSprites(0, -3, 1200);
        sprite.loadSprite("k_heli+" + armyName,  false,
                          BATTLEANIMATION_K_HELI.getMaxUnitCount(), offset, -1, 1.0, 0, 0,
                          false, false, 30);
        sprite.addMoveTweenToLastLoadedSprites(0, -3, 1200);
    };

    this.loadFireAnimation = function(sprite, unit, defender, weapon)
    {
        BATTLEANIMATION_K_HELI.loadStandingAnimation(sprite, unit, defender, weapon);
        var player = unit.getOwner();
        // get army name
        var armyName = Global.getArmyNameFromPlayerTable(player, BATTLEANIMATION_K_HELI.armyData);
        var offset = Qt.point(0, 0);
        var count = sprite.getUnitCount(BATTLEANIMATION_K_HELI.getMaxUnitCount());
        if (weapon === 0)
        {
            // gun
            offset = Qt.point(-5, 41);
            if (armyName === "yc")
            {
                offset = Qt.point(-5, 45);
            }
            else if (armyName === "ge")
            {
                offset = Qt.point(-5, 45);
            }
            else if (armyName === "bm")
            {
                offset = Qt.point(-5, 45);
            }
            else if (armyName === "bh")
            {
                offset = Qt.point(-5, 45);
            }
            sprite.loadMovingSprite("rocket", false, sprite.getMaxUnitCount(), offset,
                                    Qt.point(127, -100), 400, false,
                                    1, 1, -1);
            for (var i = 0; i < count; i++)
            {
                sprite.loadSound("missile_weapon_fire.wav", 1, "resources/sounds/", i * BATTLEANIMATION.defaultFrameDelay);
            }
        }
        else
        {
            // mg
            offset = Qt.point(35, 40);
            if (armyName === "yc")
            {
                offset = Qt.point(31, 45);
            }
            else if (armyName === "ge")
            {
                offset = Qt.point(40, 39);
            }
            else if (armyName === "bm")
            {
                offset = Qt.point(37, 42);
            }
            else if (armyName === "bh")
            {
                offset = Qt.point(40, 37);
            }
            else if (armyName === "ma")
            {
                offset = Qt.point(41, 39);
            }

            sprite.loadSprite("mg_shot",  false, sprite.getMaxUnitCount(), offset,
                              1, 1, 0, 0);
            sprite.addMoveTweenToLastLoadedSprites(0, -3, 1200);
            for (var i = 0; i < count; i++)
            {
                sprite.loadSound("mg_weapon_fire.wav", 1, "resources/sounds/", i * BATTLEANIMATION.defaultFrameDelay);
                sprite.loadSound("mg_weapon_fire.wav", 1, "resources/sounds/", 200 + i * BATTLEANIMATION.defaultFrameDelay);
                sprite.loadSound("mg_weapon_fire.wav", 1, "resources/sounds/", 400 + i * BATTLEANIMATION.defaultFrameDelay);
            }
        }
    };

    this.loadImpactUnitOverlayAnimation = function(sprite, unit, defender, weapon)
    {
        if (weapon === 0)
        {
            sprite.loadColorOverlayForLastLoadedFrame("#969696", 1000, 1, 300);
        }
        else
        {
            sprite.loadColorOverlayForLastLoadedFrame("#969696", 300, 2, 0);
        }
    };

    this.loadImpactAnimation = function(sprite, unit, defender, weapon)
    {
        var count = sprite.getUnitCount(BATTLEANIMATION_K_HELI.getMaxUnitCount());
        var i = 0;
        if (weapon === 0)
        {
            sprite.loadSprite("unit_explosion",  false, sprite.getMaxUnitCount(), Qt.point(0, 20),
                              1, 1.0, 0, 300);
            sprite.addSpriteScreenshake(8, 0.95, 800, 500);
            sprite.loadMovingSprite("rocket", false, sprite.getMaxUnitCount(), Qt.point(127, 90),
                                    Qt.point(-127, -60), 400, true,
                                    1, 1, 0, 0, true);
            for (i = 0; i < count; i++)
            {
                sprite.loadSound("rocket_flying.wav", 1, "resources/sounds/", 0);
                sprite.loadSound("rockets_explode.wav", 1, "resources/sounds/", 200 + i * BATTLEANIMATION.defaultFrameDelay);
            }
        }
        else
        {
            sprite.loadSprite("mg_hit",  false, sprite.getMaxUnitCount(), Qt.point(0, 22),
                              1, 1.0, 0, 0);
            for (i = 0; i < count; i++)
            {
                sprite.loadSound("mg_impact.wav", 1, "resources/sounds/", i * BATTLEANIMATION.defaultFrameDelay);
            }
        }
    };

    this.getImpactDurationMS = function(sprite, unit, defender, weapon)
    {
        if (weapon === 0)
        {
            return 1500 + BATTLEANIMATION.defaultFrameDelay * BATTLEANIMATION_K_HELI.getMaxUnitCount();
        }
        else
        {
            return 1000;
        }
    };

    this.getFireDurationMS = function(sprite, unit, defender, weapon)
    {
        // the time will be scaled with animation speed inside the engine
        return 500 + BATTLEANIMATION.defaultFrameDelay * BATTLEANIMATION_K_HELI.getMaxUnitCount();
    };
};

Constructor.prototype = BATTLEANIMATION;
var BATTLEANIMATION_K_HELI = new Constructor();
