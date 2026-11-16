#pragma once

struct Mothership {
	int16_t xCoord, yCoord;
	bool active;
	bool diving;
	int16_t dir;
	uint8_t hp;
	int16_t phase;
};