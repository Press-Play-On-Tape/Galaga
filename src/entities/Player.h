#pragma once

struct Player {

	int16_t xCoord = 4 << 4;
	int16_t yCoord = 32 << 4;
	int16_t vX = 0;
	int16_t vY = 0;

	uint8_t lives = 3;
	int16_t health = 100;
};
