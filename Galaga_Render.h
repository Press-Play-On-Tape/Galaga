#pragma once

#include "src/utils/Arduboy2Ext.h"

void updateAndRenderBullets() {

	for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {

		if (pBullets[i].active) {

			pBullets[i].xCoord += pBullets[i].vx;
			if (pBullets[i].xCoord > 115 << 4) pBullets[i].active = false;
			arduboy.drawFastHLine(pBullets[i].xCoord >> 4, pBullets[i].yCoord >> 4, 3, WHITE);

		}

	}

}


void renderScore(uint16_t currentScore) {

	uint8_t digits[5];
	uint32_t temp = currentScore;

	for (uint8_t i = 0; i < 5; i++) {
		digits[i] = temp % 10;
		temp /= 10;
	}

	int16_t startY = 63 - 8;

	for (uint8_t i = 0; i < 5; i++) {
		Sprites::drawSelfMasked(121, startY, Images::Numbers, digits[i]);
		startY -= 4;
	}

}


void renderLivesAndHealth() {

	uint8_t displayLives = (player.lives > 9) ? 9 : player.lives;
	Sprites::drawSelfMasked(121, 2, Images::Numbers, displayLives);

	arduboy.drawRect(120, 11, 6, 22, WHITE);
	uint8_t fillHeight = (player.health * 18) / 100;

	if (fillHeight > 18) fillHeight = 18;

	arduboy.fillRect(122, 13, 2, fillHeight, WHITE);
}

void renderParticles() {

	for (uint8_t i = 0; i < MAX_PARTICLES; i++) {

		if (particles[i].life > 0) {
			particles[i].xCoord += particles[i].vx;
			particles[i].yCoord += particles[i].vy;
			particles[i].life--;
			arduboy.drawPixel(particles[i].xCoord >> 4, particles[i].yCoord >> 4, WHITE);
		}

	}

}

void renderPlayer() {

	// Render player ..

	if (invulnTimer == 0 || (invulnTimer % 18 < 9)) {

		int16_t pX = player.xCoord >> 4;
		int16_t pY = player.yCoord >> 4;

		if (pX >= -8 && pX < 128 && pY >= -8 && pY < 64) {

			if (dualShooterTimer > 0) {
				Sprites::drawSelfMasked(pX, pY - 3, Images::playerSprite, 0);
				Sprites::drawSelfMasked(pX, pY + 3, Images::playerSprite, 0);
			}
			else {
				Sprites::drawSelfMasked(pX, pY, Images::playerSprite, 0);
			}

		}

	}

}

void renderEnemy(uint8_t enemyIdx) {

	if (enemies[enemyIdx].active) {

		const uint8_t* sprite = Images::beeSprite;
		if (enemies[enemyIdx].type == 1) {
			sprite = Images::butterflySprite;
		}
		else if (enemies[enemyIdx].type == 2) {
			sprite = Images::bossSprite;
		}

		int16_t eX = enemies[enemyIdx].xCoord >> 4;
		int16_t eY = enemies[enemyIdx].yCoord >> 4;

		if (eX >= -8 && eX < 128 && eY >= -8 && eY < 64) {
			Sprites::drawSelfMasked(eX, eY, sprite, 0);
		}

	}
	
}