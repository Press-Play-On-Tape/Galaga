#pragma once

#include "src/utils/Arduboy2Ext.h"

void renderBullets() {

	for (uint8_t i = 0; i < Constants::PlayerMaxBullets; i++) {

		if (pBullets[i].active) {

			arduboy.drawFastHLine(pBullets[i].xCoord >> 4, pBullets[i].yCoord >> 4, 3, WHITE);

		}

	}

}

void renderScore(uint16_t currentScore) {

	uint8_t digits[5];
	uint16_t temp = currentScore;

	for (uint8_t i = 0; i < 5; i++) {
		digits[i] = temp % 10;
		temp /= 10;
	}

	int16_t startY = 67 - 8;

	for (uint8_t i = 0; i < 5; i++) {
		Sprites::drawSelfMasked(121, startY, Images::Numbers, digits[i]);
		startY -= 4;
	}

}

void renderLivesAndHealth() {


	uint8_t displayLives = (player.lives == 255) ? 0 : player.lives;
	Sprites::drawSelfMasked(121, 0, Images::Player_Score, 0);
	Sprites::drawSelfMasked(121, 7, Images::Numbers, displayLives);

	arduboy.drawRect(121, 15, 6, 22, WHITE);
	uint8_t fillHeight = (player.health * 18) / 100;

	arduboy.fillRect(123, 17, 2, fillHeight, WHITE);

}

void renderParticles() {

	for (uint8_t i = 0; i < Constants::ParticlesMax; i++) {

		if (particles[i].life > 0) {
			particles[i].xCoord += particles[i].vx;
			particles[i].yCoord += particles[i].vy;
			particles[i].life--;
			Sprites::drawExternalMask((particles[i].xCoord >> 4) - 1, (particles[i].yCoord >> 4) - 1, Images::Pixel, Images::Pixel_Mask, 0, 0);
		}

	}

}

void renderPlayer() {



	// Render player ..

	if (player.invulnTimer == 0 || (player.invulnTimer % 24 < 12)) {

		int16_t pX = player.xCoord >> 4;
		int16_t pY = player.yCoord >> 4;

		if (player.health > 0 && pX >= -8 && pX < 128 && pY >= -8 && pY < 64) {

			if (player.dualShooterTimer > 0) {
				Sprites::drawSelfMasked(pX, pY - 3, Images::Player, 0);
				Sprites::drawSelfMasked(pX, pY + 3, Images::Player, 0);
			}
			else {
				Sprites::drawSelfMasked(pX, pY, Images::Player, 0);
			}

		}

	}

}


void renderEnemy_Shared(uint8_t enemyIdx) {

	const uint8_t* sprite = Images::Bee;
	const uint8_t* mask = Images::Bee_Mask;
	
	if (enemies[enemyIdx].type == EnemyType::Butterfly) {
		sprite = Images::Butterfly;
		mask = Images::Butterfly_Mask;
	}

	else if (enemies[enemyIdx].type == EnemyType::Other) {
		sprite = Images::Other;
		mask = Images::Other_Mask;
	}

	else if (enemies[enemyIdx].type == EnemyType::Boss) {
		sprite = Images::Boss;
		mask = Images::Boss_Mask;
	}

	int16_t eX = enemies[enemyIdx].xCoord >> 4;
	int16_t eY = enemies[enemyIdx].yCoord >> 4;

	if (eX >= -8 && eX < 128 && eY >= -8 && eY < 64) {
		uint8_t frame = enemies[enemyIdx].heading;		// Each enemy image has 16 rotated frames ..

		if (enemies[enemyIdx].type == EnemyType::Boss) {
			frame = frame % 8;							// .. except the symmetrical Boss, which only needs 8
		}

		Sprites::drawExternalMask(eX - 1, eY - 1, sprite, mask, frame, frame);
	}

}

void renderEnemy_Static(uint8_t enemyIdx) {

	if (enemies[enemyIdx].active && !enemies[enemyIdx].arriving && !enemies[enemyIdx].diving && !enemies[enemyIdx].queued) {

		renderEnemy_Shared(enemyIdx);

	}
	
}

void renderEnemy_Diving(uint8_t enemyIdx) {

	if (enemies[enemyIdx].active && (enemies[enemyIdx].arriving || enemies[enemyIdx].diving || enemies[enemyIdx].queued)) {

		renderEnemy_Shared(enemyIdx);

	}
	
}

void renderMothership(int8_t x, int8_t y) {

	if (mothership.active) {
		Sprites::drawExternalMask(x - 1, y - 1, Images::Mothership, Images::Mothership_Mask, 0, 0);
	}

}

void renderHealthPack() {
	
	if (healthPack.active) {

		int16_t hpX = healthPack.xCoord >> 4;
		int16_t hpY = healthPack.yCoord >> 4;

		Sprites::drawExternalMask(hpX - 1, hpY - 1, Images::HealthPack, Images::HealthPack_Mask, 0, 0);

	}
	
}

void drawRotatedDigit(int16_t x, int16_t y, uint8_t digit) {

	if (digit > 9) digit = 9;
	Sprites::drawSelfMasked(x, y, Images::Numbers, digit); 

}

void drawRotatedChar3x5(int16_t x, int16_t y, char ch) {

	if (ch < 'A') ch = 'A';
	if (ch > 'Z') ch = 'Z';
	Sprites::drawSelfMasked(x, y, Images::Font_3x5, static_cast<uint8_t>(ch - 'A'));

}

void renderHighScores() {

	const int16_t rowSpacing = 8; 
	const int16_t stepY = 4;       
	int16_t startX = 27;
    int16_t startY = 53;

	Sprites::drawOverwrite(35, 12, Images::HighScores, 0);

	// Each rank is a row in the player's view (fixed buffer x), drawn top to bottom ..

	for (uint8_t i = 0; i < Constants::NumberHighScores; i++) {

		int16_t rowX = startX - (i * rowSpacing);
		int16_t curY = startY;

		curY -= (stepY + 3);

		uint16_t temp = highScores[i].score;
		for (uint8_t d = 0; d < 5; d++) {
			drawRotatedDigit(rowX, curY, temp % 10);
			temp /= 10;
			curY -= 4;
		}


		curY -= 4;
		for (uint8_t c = 0; c < 3; c++) {
			drawRotatedChar3x5(rowX - 1, curY, highScores[i].initials[2 - c]);
			curY -= stepY;
		}

	}

}
