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

enum class DivePattern : uint8_t {

	GentleSine,
	WiderCosine,
	BasicLoop,
	LoopTheLoop,
	ZigZag,
	Count,
	None
	
};





// 24 step deltas (dx, dy) for a 270-degree counter-clockwise loop, R = 15
// Cartesian coords: +X = right, +Y = up
// const int8_t loop_x[24] = {
//      0,  3,  3,  3,  2,  2,  1,  1,
//      0, -1, -1, -1, -3, -2, -3, -3,
//     -3, -3, -3, -2, -2, -2, -1,  0
// };

// const int8_t loop_y[24] = {
//      0,  0,  1,  2,  2,  2,  3,  3,
//      3,  3,  3,  2,  3,  1,  1,  1,
//      0, -1, -1, -2, -2, -3, -3, -3
// };

// #include <stdint.h>

// 72 step deltas (dx, dy):
//   [0..23]  = 270-degree CCW loop (R = 15), ends at (-15, 15) heading down
//   [24..71] = 1.5-cycle downward sine wave (amplitude R = 15, drop = 140 px), ends at (+15, -125)
// Cartesian coords: +X = right, +Y = up
// const int8_t loop_x[180] PROGMEM = {
//     // 270-degree loop (60 steps)
//      0,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  0,  1,  0,  1,  0,  0,
//      0,  0,  0, -1,  0,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2,
//     -1, -1, -1, -1, -1, -2, -1, -1, -1,  0, -1, -1, -1,  0, -1,  0, -1,  0,  0,  0,

//     // Downward wave, R = 15 (120 steps)
//      0,  0,  0,  1,  0,  1,  0,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,
//      1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  0,  1,  0,  1,  0,  0,  0,
//      0,  0,  0, -1,  0, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, -2, -1, -1,
//     -1, -1, -2, -1, -1, -1, -1, -1, -1, -1,  0, -1, -1,  0, -1,  0, -1,  0,  0,  0,
//      0,  0,  0,  1,  0,  1,  0,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  2,  1,  1,
//      1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  0,  1,  0,  1,  0,  0,  0
// };
#include <stdint.h>

// 125 step deltas (dx, dy):
//   [0..40]   = 270-degree CCW loop (R = 15), ends at (-15, 15) heading down
//   [41..124] = 1.5-cycle downward sine wave (amplitude R = 15, drop = 140 px), ends at (+15, -125)
// Cartesian coords: +X = right, +Y = up
#include <stdint.h>

// 125 step deltas (dx, dy):
//   [0..40]   = 270-degree CCW loop (R = 15), ends at (-15, 15) heading down
//   [41..124] = 1.5-cycle downward sine wave (amplitude R = 15, drop = 140 px), ends at (+15, -125)
// Cartesian coords: +X = right, +Y = up
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