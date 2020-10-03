var Constructor = function()
{
	this.getEnemyTerrainDefenseModifier = function(co, unit, posX, posY)
    {
		if (CO_PERK.isActive(co))
		{
			return 1;
		}
		return 0;
    };
	// Perk - Intel
    this.getDescription = function()
    {
        return " +1 Enemy Terrain Stars";
    };
    this.getIcon = function()
    {
        return "intel_leak";
    };
    this.getName = function()
    {
        return "Intel Leak";
    };
};

Constructor.prototype = CO_PERK;
var CO_PERK_INTEL_LEAK = new Constructor();
