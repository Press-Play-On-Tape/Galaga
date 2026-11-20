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