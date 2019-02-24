var Constructor = function()
{
    // loader for stuff which needs C++ Support
    this.init = function (terrain)
    {
        terrain.setTerrainName(qsTr("Mountain"));
    };
    this.getDefense = function()
    {
        return 4;
    };
	this.loadBaseTerrain = function(terrain)
    {
		terrain.loadBaseTerrain("PLAINS");
    };
    this.loadBaseSprite = function(terrain)
    {
        var surroundings = terrain.getSurroundings("MOUNTAIN", false, false, TERRAIN.East, false);
        surroundings += terrain.getSurroundings("MOUNTAIN", false, false, TERRAIN.West, false);
        terrain.loadBaseSprite("mountain" + surroundings);
    };
};
Constructor.prototype = TERRAIN;
var MOUNTAIN = new Constructor();