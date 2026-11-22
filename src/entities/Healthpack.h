#pragma once

struct HealthPack {

	int16_t xCoord, yCoord;
	int16_t vy;
	uint16_t spawnTimer = 0;
	bool active;
	
};
