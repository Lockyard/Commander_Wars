var BATTLEANIMATION =
        {
    defaultFrameDelay : 75,

    getMaxUnitCount : function()
    {
        return 5;
    },

    loadStandingAnimation : function(sprite, unit, defender, weapon)
    {

    },

    hasMoveInAnimation : function(sprite, unit, defender, weapon)
    {
        // return true if the unit has an implementation for loadMoveInAnimation
        return false;
    },

    hasDyingAnimation : function()
    {
        // return true if the unit has an implementation for loadDyingAnimation
        return false;
    },

    loadMoveInAnimation : function(sprite, unit, defender, weapon)
    {
        sprite.loadAnimation("loadStandingAnimation", unit, defender, weapon);
    },

    loadStopAnimation : function(sprite, unit, defender, weapon)
    {
        sprite.loadAnimation("loadStandingAnimation", unit, defender, weapon);
    },

    loadDyingAnimation : function(sprite, unit, defender, weapon)
    {
        sprite.loadAnimation("loadStandingAnimation", unit, defender, weapon);
    },

    loadDyingAnimationSound : function(sprite, unit, defender, weapon)
    {
        var maxUnitCount = sprite.getMaxUnitCount()
        var startCount = sprite.getUnitCount(maxUnitCount, globals.roundUp(sprite.getDyingStartHp()));
        var endCount = sprite.getUnitCount(maxUnitCount, globals.roundUp(sprite.getDyingEndHp()));
        for (var i = 0; i < startCount - endCount; i++)
        {
            //sprite.loadSound("tank_explode.wav", 1, "resources/sounds/", i * BATTLEANIMATION.defaultFrameDelay);
        }
    },

    loadStandingFiredAnimation : function(sprite, unit, defender, weapon)
    {
        sprite.loadAnimation("loadStandingAnimation", unit, defender, weapon);
    },

    loadFireAnimation : function(sprite, unit, defender, weapon)
    {
        sprite.loadAnimation("loadStandingAnimation", unit, defender, weapon);
    },

    getPositionOffset : function(sprite, unit, terrain, unitIdx)
    {
        return Qt.point(0, 0);
    },


    loadImpactUnitOverlayAnimation : function(sprite, unit, defender, weapon)
    {
        if (weapon === 0)
        {
            sprite.loadColorOverlayForLastLoadedFrame("#969696", 1000, 1, 0);
        }
        else
        {
            sprite.loadColorOverlayForLastLoadedFrame("#969696", 300, 3, 0);
        }
    },

    loadImpactAnimation : function(sprite, unit, defender, weapon)
    {
        var count = sprite.getUnitCount(BATTLEANIMATION.getMaxUnitCount());
        var i = 0;
        if (weapon === 0)
        {
            sprite.loadSprite("unit_explosion",  false, sprite.getMaxUnitCount(), Qt.point(0, 20),
                              1, 1.0, 0, 0);
            sprite.addSpriteScreenshake(8, 0.98, 800, 200);
            for (i = 0; i < count; i++)
            {
                sprite.loadSound("impact_explosion.wav", 1, "resources/sounds/", i * BATTLEANIMATION.defaultFrameDelay);
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
    },

    getImpactDurationMS : function(sprite, unit, defender, weapon)
    {
        // should be a second or longer.
        // the time will be scaled with animation speed inside the engine
        return 1000;
    },

    getFireDurationMS : function(sprite, unit, defender, weapon)
    {
        // the time will be scaled with animation speed inside the engine
        return 100;
    },

    getMoveInDurationMS : function(sprite, unit, defender, weapon)
    {
        // the time will be scaled with animation speed inside the engine
        return 100;
    },

    getStopDurationMS : function(sprite, unit, defender, weapon)
    {
        // the time will be scaled with animation speed inside the engine
        return 100;
    },

    getDyingDurationMS : function()
    {
        // the time will be scaled with animation speed inside the engine
        return 0;
    },

    getRelativePosition : function(attacker, defender)
    {
        var defHigh = defender.getTerrain().getVisionHigh();
        var atkHigh = attacker.getTerrain().getVisionHigh();
        if (defender.getUnitType() === GameEnums.UnitType_Air)
        {
            return 1;
        }
        else if ((defHigh > 1) || (atkHigh > 1))
        {
            if (defHigh > atkHigh)
            {
                return 1;
            }
            else if (defHigh < atkHigh)
            {
                return -1;
            }
        }
        return 0;
    },


};
