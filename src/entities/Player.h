#pragma once

struct Player {

	int16_t xCoord = 4 << 4;
	int16_t yCoord = 32 << 4;
	int16_t vX = 0;
	int16_t vY = 0;

	uint8_t lives = 3;
	int16_t health = 100;

	bool shieldActive = false;
	uint8_t shieldDrainTimer = 0;

	void reset() {

		this->xCoord = 4 << 4;
		this->yCoord = 32 << 4;
		this->vX = 0;
		this->vY = 0;
		this->lives = 3;
		this->health = 100;
		this->shieldActive = false;
		this->shieldDrainTimer = 0;

		this->lives = 1;
		this->health = 5;

	}

};
