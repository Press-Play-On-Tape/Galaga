#pragma once

struct Mothership {

	bool active;
	bool diving;
	
	int16_t xCoord, yCoord;
	int16_t dir;
	uint8_t hp;
	int16_t phase;
	uint16_t spawnTimer = 0;

};