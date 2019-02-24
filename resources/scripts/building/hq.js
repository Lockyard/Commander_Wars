var Constructor = function()
{
    // called for loading the main sprite
    this.loadSprites = function(building)
    {
        if (building.getOwnerID() >= 0)
        {
            // none neutral player
            var player = building.getOwner();
            // get army name
            var armyName = player.getArmy().toLowerCase();
            // bh and bg have the same sprites
            if (armyName === "bg")
            {
                armyName = "bh"
            }
            building.loadSprite("hq+" + armyName, false);
            building.loadSprite("hq+" + armyName + "+mask", true);
        }
        else
        {
            // neutral player
            building.loadSprite("hq+neutral", false);
        }
    };
    this.addCaptureAnimationBuilding = function(animation, building, startColor, capturedColor)
    {
        var player = building.getOwner();
        // get army name
        var armyName = player.getArmy().toLowerCase();
        if (armyName === "bg")
        {
            armyName = "bh"
        }
        if ((armyName !== "os") &&
            (armyName !== "yc") &&
            (armyName !== "ge") &&
            (armyName !== "bm") &&
            (armyName !== "bh"))
        {
            armyName = "os";
        }
        animation.addBuildingSprite("hq+" + armyName + "+mask", startColor , capturedColor, true);
        animation.addBuildingSprite("hq+" + armyName, startColor , capturedColor, false);
    };
    this.getDefense = function()
    {
        return 4;
    };
    this.getConstructionList = function()
    {
        return ["INFANTRY", "MECH", "MOTORBIKE", "RECON", "APC", "FLARE",
                "ARTILLERY", "LIGHT_TANK", "FLAK", "ANTITANKCANNON",
                "HEAVY_TANK", "MISSILE", "ROCKETTHROWER", "MEGATANK"];
    };
}

Constructor.prototype = BUILDING;
var HQ = new Constructor();