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
	
	bool active;
	bool queued;
	bool arriving;
	bool diving;
	bool returning;

};
