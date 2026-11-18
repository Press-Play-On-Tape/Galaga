#pragma once

enum class GameState : uint8_t { 
	
	SplahScreen, 
	Title_Init, 
	Title_Entrace, 
	Title_Shooting, 
	Title_Pause, 
	Title_Withdraw, 
	Title_PressA, 
	Playing, 
	GameOver 
	
};

enum class Side : uint8_t {

	Left,
	Right,
	Top

};


enum class EnemyType : uint8_t {

	Bee,
	Butterfly,
	Boss,
	
};

