// template for an cow automation script of any sort not all menu's and objects are supported here for now

//parametrized variables to choose some stuff on automation of game
	
	//if true automate, if false don't
	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	var goAuto = true;//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< AUTOMATE MATCHES?
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	//load custom rules? If yes, which file?
	var loadCustomRules = true;
	var customRules = "C:/Users/Loren/CLionProjects/Commander_Wars_Fork/install/Debug/data/gamerules/Training1Rules.grl";
	
	//which map should be selected?
	//new2players is a folder I made because for whatever reason it doesn't build/see the 2_players folder anymore
	var init_mapFolderPath = "maps/New2Players/";
	var init_mapFilename = "Basin Forest.map";
	
	//matches to complete before stopping automation. < 0 to leave the training manager to do this
	var matchCounterTarget = -1;
	
	
	//other variables not to parametrize for automation
	var matchCounter = 0;

var Init =
{

	//function called when in the menu
	main = function(menu)
	{
		if(goAuto) {
			menu.enterSingleplayer();
		}
	},
	
	
	//function called when in map selection
	mapsSelection = function(menu)
	{
		if(!goAuto) {
			return;
		}
		
		
		menu.selectMap(init_mapFolderPath, init_mapFilename);
		
		menu.slotButtonNext();
		
		if(loadCustomRules)
			menu.loadRules(customRules);
		
		menu.slotButtonNext();
		
		var selection = menu.getPlayerSelection();
		//normal ai vs test first ai
		selection.selectPlayerAi(0, 3);
		selection.selectPlayerAi(1, 5);
		menu.startGame();
	},
	
	
	//function called on victory of a player. increases the match counter
	onVictory = function(menu)
	{
		
		if(goAuto) {
			menu.exitMenue();
			matchCounter++;
			
			//stop automating the game after  a target amount of matches, if set
			if(matchCounterTarget > 0 && matchCounter >= matchCounterTarget) {
				goAuto = false;
			}
		}
	},
	
	stopTraining = function(trainingManager) {
		goAuto = false;
	}
	
	
}
