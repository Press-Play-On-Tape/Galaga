#pragma once

#include "../utils/Enums.h"

struct Enemy {

	EnemyType type;
	int16_t xCoord, yCoord;
	int16_t homeX, homeY;
	uint8_t hp;
	int16_t entryPhase;
	int16_t divePhase;
	DivePattern divePattern;
	uint8_t diveIdx;
	int8_t diveDirection;
	Side spawnSide;

	uint8_t heading;		// Sprite frame 0-15: the direction the enemy faces in 22.5 degree steps (0 = formation pose)
	uint8_t turnTimer;		// Frames until the enemy can turn another step

	bool active;
	bool queued;
	bool arriving;
	bool diving;

};
