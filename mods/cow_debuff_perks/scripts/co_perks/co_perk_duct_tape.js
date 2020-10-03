var Constructor = function()
{
    this.getRepairBonus = function(co, unit, posX, posY)
    {
		if (CO_PERK.isActive(co))
		{
			return -1;
		}
        return 0;
    };
	// Perk - Intel
    this.getDescription = function()
    {
        return "Decreases the base repair by 1.";
    };
    this.getIcon = function()
    {
        return "duct_tape";
    };
    this.getName = function()
    {
        return "Duct Tape";
    };
};

Constructor.prototype = CO_PERK;
var CO_PERK_DUCT_TAPE = new Constructor();
