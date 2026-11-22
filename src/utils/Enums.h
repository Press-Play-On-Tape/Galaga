#pragma once

// ----------------------------------------------------------------------------
//  Screen orientation
//
//  The game is played with the Arduboy rotated 90 degrees counter-clockwise.
//  All positions and velocities (xCoord, yCoord, vx, vy, homeX, ...) are in
//  screen-buffer coordinates. Words like top / bottom / left / right / row /
//  column describe what the player sees:
//
//    Buffer x increasing (0 -> 127)  =  up the screen (forward, towards enemies)
//    Buffer y increasing (0 -> 63)   =  left to right across the screen
//
//  So a formation 'row' is a fixed buffer x and a 'column' is a fixed buffer y.
// ----------------------------------------------------------------------------

namespace Constants {

	constexpr uint8_t PlayerMaxBullets = 4;
	constexpr uint8_t EnemiesMaxBullets = 4;
	constexpr uint8_t EnemyRows = 4;
	constexpr uint8_t EnemyColumns = 6;
	constexpr uint8_t EnemiesMax = (EnemyColumns * EnemyRows);
	constexpr uint8_t ParticlesMax = 12;
	constexpr uint8_t StarsMax = 16;
	constexpr uint8_t GameOverDiverCount = 3;
	constexpr uint8_t NumberHighScores = 4;

	constexpr int EEPROM_HS_Magic_Address = 200;
	constexpr int EEPROM_HS_Data_Address  = 201;
	constexpr uint8_t EEPROM_HS_Magic_Value = 0x5A;

	constexpr uint8_t Title_Coords[] = {
		60, 58, 56, 54, 52, 50, 48, 46, 44, 42,
		40, 38, 36, 34, 32, 30, 28, 26, 24, 22,
		20, 18, 16, 14, 12, 10,  9,  8,  7,  6,
		6,  5,  5,  4,  4,  4,  3,  3,  3,  3,
		2,  2,  2,  2,  2,  1,  1,  1,  1,  1,
		1,  1,
	};

};

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

// The edge of the player's view an enemy enters from. Order matters: values are
// picked with random(0, 3).

enum class Side : uint8_t {

	Right,		// Buffer y > 63
	Top,		// Buffer x > 127
	Left		// Buffer y < 0

};

enum class EnemyType : uint8_t {

	Bee,
	Butterfly,
	Other,
	Boss,
	
};

enum class DivePattern : uint8_t {

	GentleSine,
	WiderCosine,
	BasicLoop,
	LoopTheLoop,
	ZigZag,
	Count,
	None
	
};


// Dive paths: per-step deltas in buffer coordinates. The *_dy (sideways) values
// are multiplied by the enemy's diveDirection to mirror the path. Negative *_dx
// moves the enemy down the player's view, towards the player.

const int8_t loop_dy[125] PROGMEM = {
    // 270-degree loop (41 steps)
     0,  2,  2,  1,  2,  1,  2,  1,  1,  1,  1,  0,  1,  0,  0,  0, -1,  0, -1, -1,
    -1, -2, -1, -2, -1, -2, -2, -2, -1, -2, -2, -1, -2, -1, -1, -1, -1, -1, -1,  0,
     0,

    // Downward wave, R = 15 (84 steps)
     0,  0,  1,  0,  1,  1,  1,  2,  1,  1,  2,  2,  1,  2,  2,  1,  2,  2,  1,  1,
     2,  1,  1,  1,  0,  1,  0,  0,
     0,  0, -1,  0, -1, -1, -1, -2, -1, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -1,
    -2, -1, -1, -1,  0, -1,  0,  0,
     0,  0,  1,  0,  1,  1,  1,  2,  1,  1,  2,  2,  1,  2,  2,  1,  2,  2,  1,  1,
     2,  1,  1,  1,  0,  1,  0,  0
};

const int8_t loop_dx[125] PROGMEM = {
    // 270-degree loop (41 steps)
     0,  0,  0,  1,  1,  1,  1,  1,  1,  2,  1,  2,  2,  1,  2,  2,  2,  1,  2,  1,
     2,  1,  1,  1,  0,  1,  0,  0,  0, -1,  0, -1, -1, -1, -1, -2, -1, -2, -1, -2,
    -2,

    // Downward wave, 140 px drop (84 steps)
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1,
    -2, -2, -1, -2, -2, -1, -2, -2,
    -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -2,
    -2, -1, -2, -2, -1, -2, -2, -1,
    -2, -2, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -2, -2, -1, -2, -2,
    -1, -2, -2, -1, -2, -2, -1, -2
};


const int8_t zigzag_dy[85] PROGMEM = {
     2,  2,  3,  2,  2,  2,  1,  1,  0, -1, -1, -2, -2, -2, -3, -2, -2,
    -2, -2, -3, -2, -2, -2, -1, -1,  0,  1,  1,  2,  2,  2,  3,  2,  2,
     2,  2,  3,  2,  2,  2,  1,  1,  0, -1, -1, -2, -2, -2, -3, -2, -2,
    -2, -2, -3, -2, -2, -2, -1, -1,  0,  1,  1,  2,  2,  2,  3,  2,  2,
     2,  2,  3,  2,  2,  2,  1,  1,  0, -1, -1, -2, -2, -2, -3, -2, -2
};

const int8_t zigzag_dx[85] PROGMEM = {
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -2, -1, -2,
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -2, -1, -2,
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -2, -1, -2,
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -2, -1, -2,
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -2, -1, -2
};