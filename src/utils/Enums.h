#pragma once

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

enum class Side : uint8_t {

	Left,
	Right,
	Top

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


const int8_t loop_x[125] PROGMEM = {
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

const int8_t loop_y[125] PROGMEM = {
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


const int8_t zigzag_x[85] PROGMEM = {
     2,  2,  3,  2,  2,  2,  1,  1,  0, -1, -1, -2, -2, -2, -3, -2, -2,
    -2, -2, -3, -2, -2, -2, -1, -1,  0,  1,  1,  2,  2,  2,  3,  2,  2,
     2,  2,  3,  2,  2,  2,  1,  1,  0, -1, -1, -2, -2, -2, -3, -2, -2,
    -2, -2, -3, -2, -2, -2, -1, -1,  0,  1,  1,  2,  2,  2,  3,  2,  2,
     2,  2,  3,  2,  2,  2,  1,  1,  0, -1, -1, -2, -2, -2, -3, -2, -2
};

const int8_t zigzag_y[85] PROGMEM = {
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -2, -1, -2,
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -2, -1, -2,
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -2, -1, -2,
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -2, -1, -2,
    -2, -1, -2, -2, -1, -2, -2, -1, -2, -1, -2, -2, -1, -2, -2, -1, -2
};