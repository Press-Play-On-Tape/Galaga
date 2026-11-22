#pragma once

#include "src/utils/Arduboy2Ext.h"


// --- Helper Functions ---
void spawnParticles(int16_t x, int16_t y, uint8_t count) {

	for (uint8_t i = 0; i < Constants::ParticlesMax && count > 0; i++) {

		if (particles[i].life == 0) {
			particles[i].xCoord = x << 4;
			particles[i].yCoord = y << 4;
			particles[i].vx = (int16_t)(random(0, 24) - 12);
			particles[i].vy = (int16_t)(random(0, 24) - 12);
			particles[i].life = random(12, 20);
			count--;
		}

	}

}

void spawnMothership() {

	mothership.spawnTimer++;

	if (mothership.spawnTimer > max(150, 350 - (survivalTimer / 4))) {
		mothership.spawnTimer = 0;
		mothership.active = true;
		mothership.diving = false;
		mothership.xCoord = 110 << 4;
		mothership.yCoord = 28 << 4;
		mothership.hp = 3;
		mothership.dir = (random(0, 2) == 0) ? 1 : -1;
		mothership.phase = 0;
	}
	
}

void startPlayerPuff() {

	player.puffX = player.xCoord >> 4;
	player.puffY = player.yCoord >> 4;
	player.deathTimer = 28; // 7 frames * 4 frames per puff

}

void checkEnemyAndPlayerCollision(uint8_t enemyIdx) {

	uint8_t enemySz = (enemies[enemyIdx].type == EnemyType::Boss) ? 7 : 6;
	bool canCollide = enemies[enemyIdx].diving;

	Rect enemyRect{
		(int16_t)(enemies[enemyIdx].xCoord >> 4),
		(int16_t)(enemies[enemyIdx].yCoord >> 4),
		enemySz,
		enemySz
	};


	// Has the player and the enemy collided?

	Rect playerRect{
		(int16_t)(player.xCoord >> 4),
		(int16_t)(player.yCoord >> 4),
		7,
		7
	};

	if (!enemies[enemyIdx].arriving && canCollide && arduboy.collide(enemyRect, playerRect)) {
		
		spawnParticles(enemies[enemyIdx].xCoord >> 4, enemies[enemyIdx].yCoord >> 4, 10);

		enemies[enemyIdx].active = false;
		enemies[enemyIdx].xCoord = -1000;
		enemies[enemyIdx].yCoord = -1000;

		enemies[enemyIdx].arriving = false;
		enemies[enemyIdx].diving = false;
		enemies[enemyIdx].returning = false;
		enemies[enemyIdx].queued = false;

		if (player.shieldActive)  return;

		if (player.invulnTimer == 0) {

			spawnParticles(player.xCoord >> 4, player.yCoord >> 4, 8);
			player.health -= 34;
			screenInvertTimer = 6;
			beep.tone(beep.freq(80), 10);

			if (player.health <= 0) {

				startPlayerPuff();
				player.lives--;
				beep.tone(beep.freq(50), 25);

				if (player.lives == 0) {

					gameOverDelayTimer = 60;
					player.health = 0;

				}
				else {
					player.health = 100;
					player.xCoord = 4 << 4;
					player.yCoord = 32 << 4;
					player.invulnTimer = 90;
				}

			}

		}

	}

}

void checkMothershipCollision() {

	if (mothership.active) {

		// Did the player and the mothership collide ?

		Rect playerRect{
			(int16_t)(player.xCoord >> 4),
			(int16_t)(player.yCoord >> 4),
			7,
			7
		};

		Rect mothershipRect{
			(int16_t)(mothership.xCoord >> 4),
			(int16_t)(mothership.yCoord >> 4),
			7,
			7
		};

		if (arduboy.collide(playerRect, mothershipRect)) {

			mothership.active = false;
			spawnParticles(mothership.xCoord >> 4, mothership.yCoord >> 4, 20);

			if (player.shieldActive)  return;

			if (player.invulnTimer == 0) {

				spawnParticles(player.xCoord >> 4, player.yCoord >> 4, 20);
				player.health -= 50;
				screenInvertTimer = 6;
				beep.tone(beep.freq(80), 15);

				if (player.health <= 0) {

					startPlayerPuff();
					player.lives--;
					beep.tone(beep.freq(50), 25);

					if (player.lives == 0) {

						gameOverDelayTimer = 60;
						player.health = 0;

					}
					else {
						player.health = 100;
						player.xCoord = 4 << 4;
						player.yCoord = 32 << 4;
						player.invulnTimer = 90;
					}

				}

			}

		}


		// Player bullet hits the mothership ..

		for (uint8_t b = 0; b < Constants::PlayerMaxBullets; b++) {

			Rect bulletRect{
				(int16_t)(pBullets[b].xCoord >> 4),
				(int16_t)(pBullets[b].yCoord >> 4),
				3,
				1
			};

			if (pBullets[b].active && arduboy.collide(bulletRect, mothershipRect)) {

				pBullets[b].active = false;
				mothership.hp--;
				spawnParticles(mothership.xCoord >> 4, mothership.yCoord >> 4, 8);

				if (mothership.hp == 0) {

					mothership.active = false;
					score += 40;
					if (mothership.diving) player.dualShooterTimer = 600;
					beep.tone(beep.freq(700), 8);

				}
				else {
					beep.tone(beep.freq(600), 3);
				}

				break;

			}

		}

	}

}


void checkHealthPackCollision() {

	int16_t hpX = healthPack.xCoord >> 4;
	int16_t hpY = healthPack.yCoord >> 4;

	Rect healthPackRect {
		hpX,
		hpY,
		6,
		6
	};


	// Player shoots a health pack ..

	for (uint8_t b = 0; b < Constants::PlayerMaxBullets; b++) {

		Rect bulletRect{	
			(int16_t)(pBullets[b].xCoord >> 4),	
			(int16_t)(pBullets[b].yCoord >> 4),	
			3,
			1
		};

		if (pBullets[b].active && arduboy.collide(bulletRect, healthPackRect)) {

			pBullets[b].active = false;
			healthPack.active = false;
			int16_t healAmount = random(25, 51);
			player.health += healAmount;
			if (player.health > 100) player.health = 100;
			spawnParticles(hpX, hpY, 6);
			beep.tone(beep.freq(1046), 6);

			break;

		}

	}


	// Player collides with a health pack ..

	Rect playerRect {
		(int16_t)(player.xCoord >> 4),
		(int16_t)(player.yCoord >> 4),
		7,
		7
	};

	if (arduboy.collide(playerRect, healthPackRect)) {

		healthPack.active = false;
		int16_t healAmount = random(25, 51);
		player.health += healAmount;
		if (player.health > 100) player.health = 100;
		spawnParticles(player.xCoord >> 4, player.yCoord >> 4, 6);
		beep.tone(beep.freq(1046), 6);

	}
	
}

void checkEnemyAndPlayerBulletCollision(uint8_t enemyIdx) {

	uint8_t enemySz = (enemies[enemyIdx].type == EnemyType::Boss) ? 7 : 6;

	Rect enemyRect{
		(int16_t)(enemies[enemyIdx].xCoord >> 4),
		(int16_t)(enemies[enemyIdx].yCoord >> 4),
		enemySz,
		enemySz
	};

	for (uint8_t b = 0; b < Constants::PlayerMaxBullets; b++) {

		Rect bulletRect{
			(int16_t)(pBullets[b].xCoord >> 4),
			(int16_t)(pBullets[b].yCoord >> 4),
			3,
			1
		};

		if (pBullets[b].active && arduboy.collide(bulletRect, enemyRect)) {

			pBullets[b].active = false;
			enemies[enemyIdx].hp--;
			spawnParticles(enemies[enemyIdx].xCoord >> 4, enemies[enemyIdx].yCoord >> 4, 12);

			if (enemies[enemyIdx].hp == 0) {

				enemies[enemyIdx].active = false;
				score += (static_cast<uint8_t>(enemies[enemyIdx].type) + 1) * 5 * (enemies[enemyIdx].diving ? 2 : 1);
				beep.tone(beep.freq(160), 4);

			}
			else {

				beep.tone(beep.freq(500), 2);

			}
			break;

		}

	}

}

void checkPlayerAndEnemyBulletCollision(uint8_t bulletIdx) {

	Rect playerRect {
		(int16_t)(player.xCoord >> 4),
		(int16_t)(player.yCoord >> 4),
		7,
		7
	};

	Rect bulletRect {
		(int16_t)(eBullets[bulletIdx].xCoord >> 4),
		(int16_t)(eBullets[bulletIdx].yCoord >> 4),
		3,
		1
	};

	if (arduboy.collide(bulletRect, playerRect)) {

		eBullets[bulletIdx].active = false;

		if (player.shieldActive)  return;

		if (player.invulnTimer == 0) {

			spawnParticles(player.xCoord >> 4, player.yCoord >> 4, 8);

			player.health -= 34;
			screenInvertTimer = 6;
			beep.tone(beep.freq(80), 10);

			if (player.health <= 0) {

				startPlayerPuff();
				player.lives--;
				beep.tone(beep.freq(50), 25);

				if (player.lives == 0) {

					gameOverDelayTimer = 60;
					player.health = 0;

				}
				else {
					player.health = 100;
					player.xCoord = 4 << 4;
					player.yCoord = 32 << 4;
					player.invulnTimer = 90;
				}

			}

		}

	}

}

void launchHealthPack() {

	if (score >= 600) {

		healthPack.spawnTimer++;

		if (healthPack.spawnTimer > 500) {

			healthPack.spawnTimer = 0;
			healthPack.active = true;
			healthPack.xCoord = random(20, 100) << 4;

			if (random(0, 2) == 0) {
				healthPack.yCoord = -8 << 4;
				healthPack.vy = random(4, 11);
			}
			else {
				healthPack.yCoord = 64 << 4;
				healthPack.vy = random(-11, -3);
			}

		}

	}
	
}

void firePlayerBullet(uint8_t offset = 4) {

	if (player.dualShooterTimer > 0) {

		for (uint8_t i = 0; i < Constants::PlayerMaxBullets && i + 1 < Constants::PlayerMaxBullets; i += 2) {
		
			if (!pBullets[i].active && !pBullets[i + 1].active) {
				pBullets[i].xCoord = player.xCoord + (4 << 4);
				pBullets[i].yCoord = player.yCoord + (1 << 4);
				pBullets[i].vx = 64;
				pBullets[i].active = true;

				pBullets[i + 1].xCoord = player.xCoord + (4 << 4);
				pBullets[i + 1].yCoord = player.yCoord + (5 << 4);
				pBullets[i + 1].vx = 64;
				pBullets[i + 1].active = true;

				beep.tone(beep.freq(880), 2);
				break;
			}

		}

	}
	else {

		for (uint8_t i = 0; i < Constants::PlayerMaxBullets; i++) {

			if (!pBullets[i].active) {
				pBullets[i].xCoord = player.xCoord + (offset << 4);
				pBullets[i].yCoord = player.yCoord + (3 << 4);
				pBullets[i].vx = 64;
				pBullets[i].active = true;
				beep.tone(beep.freq(880), 2);
				break;
			}

		}

	}

}

void fireEnemyBullet(int16_t x, int16_t y) {

	for (uint8_t i = 0; i < Constants::EnemiesMaxBullets; i++) {

		if (!eBullets[i].active) {

			eBullets[i].xCoord = x - (2 << 4);
			eBullets[i].yCoord = y + (2 << 4);
			int16_t bSpeed = 10 + (wave * 1) + (survivalTimer / 600);
			if (bSpeed > 35) bSpeed = 35;
			eBullets[i].vx = -bSpeed;
			eBullets[i].active = true;
			break;

		}

	}

}


void updateStars() {

	for (uint8_t i = 0; i < Constants::StarsMax; i++) {

		stars[i].xCoord -= stars[i].speed;

		if (stars[i].xCoord < 0) {
			stars[i].xCoord = 114;
			stars[i].yCoord = random(0, 64);
		}

		if ((arduboy.frameCount % 4) == (i % 4)) {
			arduboy.drawPixel(stars[i].xCoord, stars[i].yCoord, WHITE);
		}

	}

}


void updateBullets() {

	for (uint8_t i = 0; i < Constants::PlayerMaxBullets; i++) {

		if (pBullets[i].active) {

			pBullets[i].xCoord += pBullets[i].vx;
			if (pBullets[i].xCoord > 115 << 4) pBullets[i].active = false;

		}

	}

}



void saveHighScores() {

	EEPROM.write(Constants::EEPROM_HS_Magic_Address, Constants::EEPROM_HS_Magic_Value);
	EEPROM.put(Constants::EEPROM_HS_Data_Address, highScores);

}

void loadHighScores() {

	if (EEPROM.read(Constants::EEPROM_HS_Magic_Address) == Constants::EEPROM_HS_Magic_Value) {

		EEPROM.get(Constants::EEPROM_HS_Data_Address, highScores);

	}
	else {

		for (uint8_t i = 0; i < Constants::NumberHighScores; i++) {
			highScores[i].initials[0] = 'A';
			highScores[i].initials[1] = 'A';
			highScores[i].initials[2] = 'A';
			highScores[i].score = 0;
		}
		saveHighScores();

	}

}

bool qualifiesForHighScore(uint16_t newScore) {

	return newScore > 0 && newScore > highScores[Constants::NumberHighScores - 1].score;

}

// Returns the rank index (0..3) where newScore was inserted, or -1 if not on the board
int8_t insertHighScore(uint16_t newScore, const char initials[3]) {

	if (!qualifiesForHighScore(newScore)) return -1;

	int8_t rank = Constants::NumberHighScores - 1;
	while (rank > 0 && newScore > highScores[rank - 1].score) {
		rank--;
	}

	// Shift lower scores down
	for (int8_t i = Constants::NumberHighScores - 1; i > rank; i--) {
		highScores[i] = highScores[i - 1];
	}

	highScores[rank].initials[0] = initials[0];
	highScores[rank].initials[1] = initials[1];
	highScores[rank].initials[2] = initials[2];
	highScores[rank].score = newScore;

	saveHighScores();
	return rank;

}
