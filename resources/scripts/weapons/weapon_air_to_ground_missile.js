var Constructor = function()
{
    this.getName = function()
    {
        return qsTr("Missile");
    };
    this.getBaseDamage = function(unit)
    {
        switch(unit.getUnitID())
        {
			case "APC":
				return 75;
			case "FLARE":
				return 75;
			case "RECON":
				return 75;

			case "FLAK":
				return WEAPON.lowDamage;
			case "HOVERFLAK":
				return WEAPON.lowDamage;
			case "LIGHT_TANK":
				return WEAPON.softDamage;
			case "HOVERTANK":
				return WEAPON.softDamage;

			case "HEAVY_HOVERCRAFT":
				return 45;
			case "HEAVY_TANK":
				return 45;
			case "NEOTANK":
				return 45;

			case "MEGATANK":
				return WEAPON.softCounterDamage;

			case "ARTILLERY":
				return WEAPON.softDamage;
			case "ANITANKCANNON":
				return WEAPON.lowDamage;
			case "MISSILE":
				return WEAPON.mediumDamage;
			case "ROCKETTRHOWER":
				return 75;
				
			// ships
			case "BATTLESHIP":
				return 25;
			case "CANNONBOAT":
				return 85;
			case "CRUISER":
				return 10;
			case "DESTROYER":
				return 10;
			case "SUBMARINE":
				return 25;
			case "LANDER":
				return 25;
			case "AIRCRAFTCARRIER":
				return 25;
			default:
				return -1;
        }
    };
};

Constructor.prototype = WEAPON;
var WEAPON_AIR_TO_GROUND_MISSILE = new Constructor();