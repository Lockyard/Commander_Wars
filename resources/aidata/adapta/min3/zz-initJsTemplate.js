// template for an cow automation script of any sort not all menu's and objects are supported here for now


var matchConfig = function(folderPath, fileName, gameRules, AIselection, traineeSlot) {
	this.mapFolderPath = folderPath;
	this.mapFileName = fileName;
	this.gameRules = gameRules; //game rules path "data/gamerules/rulesname.grl"
	this.AIselection = AIselection; //an array containing the AI to be selected. if a map has 2 players for instance, [0, 2] will choose human in slot 0 and normalAI in slot 1
	this.traineeSlot = traineeSlot; //the slot of the AI to be evaluated
}
	

var Init =
{
	//parametrized variables to choose some stuff on automation of game
	
	//if true automate, if false don't
	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	goAuto = true,//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< AUTOMATE MATCHES?
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	logLevel = 1,
	
	//load custom rules?
	loadCustomRules = true,
	customRules1 = "data/gamerules/Training1Rules_win1.grl",
	customRules2 = "data/gamerules/Training1Rules_win2.grl",
	
	//other paths: "maps/2_player/" "maps/pre_deployed/" "maps/custom/";
	customFolderPath = "maps/custom/",
	
	//the set of match configurations. 
	matchConfigs = [	
						new matchConfig(customFolderPath, "MINtraining1.map", customRules2, [5, 3], 0), 
						new matchConfig(customFolderPath, "MINtraining1.map", customRules1, [3, 5], 1)
					],
					
	//how many times to repeat the selected maps and AI positions before notifying the training manager to evaluate.
	matchCyclesAmount = 1,
	
	//other map names used: 2P: "Basin Forest.map" "BlackPatchWar.map" "force_capture_4p2.map"
	//pre deployed: "Crosspaths.map"
	//custom: "force_capture.map" "force_capture_4p.map" "force_destroy.map" "crosspathscustom.map"
	
	//for storing the training manager reference
	trainingManager = null,
	
	//total matches to complete before stopping automation. < 0 to leave the training manager to do this
	matchCountTargetGlobal = -1,
	
	//other variables not to parametrize for automation /////////////////////////
	matchCounterGlobal = 0,
	currMatchConfigIndex = 0,
	matchNumberTarget = 0,
	
	//functions/////////////////////////////////////////////////////////////////
	
	setupInitJs = function() {
		GameConsole.setLogLevel(this.logLevel);
		this.matchNumberTarget = this.matchConfigs.length * this.matchCyclesAmount;
		if(this.matchNumberTarget < 1) {
			this.goAuto = false;
		}
	},
	
	//function called when in the menu
	main = function(menu)
	{
		if(this.goAuto) {
			this.setupInitJs();
			menu.enterSingleplayer();
		}
	},
	
	
	//function called when in map selection
	mapsSelection = function(menu)
	{
		if(!this.goAuto) {
			return;
		}
		//select map of current map Index
		menu.selectMap(this.matchConfigs[this.currMatchConfigIndex].mapFolderPath, this.matchConfigs[this.currMatchConfigIndex].mapFileName);
		
		menu.slotButtonNext();
		
		if(this.loadCustomRules)
			menu.loadRules(this.matchConfigs[currMatchConfigIndex].gameRules);
		
		menu.slotButtonNext();
		
		var selection = menu.getPlayerSelection();
		//select AIs based on the map config
		for(let player = 0; player < this.matchConfigs[this.currMatchConfigIndex].AIselection.length; player++) {
			selection.selectPlayerAi(player, this.matchConfigs[this.currMatchConfigIndex].AIselection[player]);
		}
		if(this.trainingManager != null) {
			this.trainingManager.setTrainingPlayerIndex(this.matchConfigs[this.currMatchConfigIndex].traineeSlot);
		}
		this.currMatchConfigIndex += 1;
		if(this.currMatchConfigIndex >= this.matchConfigs.length) {
			this.currMatchConfigIndex = 0;
		}
		
		menu.startGame();
	},
	
	
	//function called on victory of a player. increases the match counter
	onVictory = function(menu)
	{
		
		if(this.goAuto) {
			if(this.trainingManager) {
				this.trainingManager.onVictory();
			}
			
			//stop automating the game after  a target global amount of matches, if set
			if(this.matchCountTargetGlobal > 0){
				this.matchCounterGlobal++;
				if(this.matchCounter >= this.matchCountTargetGlobal) {
					this.goAuto = false;
				}
			}
			
			menu.exitMenue();
		}
	},
	
	setupTrainingManager(tm) {
		this.trainingManager = tm;
		this.trainingManager.setMatchNumberTarget(this.matchNumberTarget);
	},
	
	stopTraining = function(tm) {
		this.goAuto = false;
	}
	
	
}
