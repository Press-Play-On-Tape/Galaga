#pragma once

struct Enemy {
	int16_t xCoord, yCoord;
	int16_t homeX, homeY;
	int16_t targetX, targetY;
	uint8_t type;
	uint8_t hp;
	bool active;
	bool queued;
	bool arriving;
	bool diving;
	bool returning;
	int16_t divePhase;
	uint8_t divePattern;
	int16_t diveDirection;

	int16_t entryX;
	int16_t entryY;
	int16_t entryPhase;

	uint8_t spawnSide;
};
