#pragma once

#include "src/utils/Arduboy2Ext.h"


// --- Helper Functions ---
void spawnParticles(int16_t x, int16_t y, uint8_t count) {

	for (uint8_t i = 0; i < MAX_PARTICLES && count > 0; i++) {

		if (particles[i].life == 0) {
			particles[i].xCoord = x << 4;
			particles[i].yCoord = y << 4;
			particles[i].vx = (int16_t)(random(0, 24) - 12);
			particles[i].vy = (int16_t)(random(0, 24) - 12);
			particles[i].life = random(8, 16);
			count--;
		}

	}

}

void spawnMothership() {

	mothershipSpawnTimer++;

	if (mothershipSpawnTimer > max(150, 350 - (survivalTimer / 4))) {
		mothershipSpawnTimer = 0;
		mothership.active = true;
		mothership.diving = false;
		mothership.xCoord = 110 << 4;
		mothership.yCoord = 28 << 4;
		mothership.hp = 3;
		mothership.dir = (random(0, 2) == 0) ? 1 : -1;
		mothership.phase = 0;
	}
	
}

void checkEnemyAndPlayerCollision(uint8_t enemyIdx) {

	uint8_t enemySz = (enemies[enemyIdx].type == 2) ? 7 : 6;
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

	if (!enemies[enemyIdx].arriving && invulnTimer == 0 && canCollide && arduboy.collide(enemyRect, playerRect)) {

		enemies[enemyIdx].active = false;
		enemies[enemyIdx].xCoord = -1000;
		enemies[enemyIdx].yCoord = -1000;

		enemies[enemyIdx].arriving = false;
		enemies[enemyIdx].diving = false;
		enemies[enemyIdx].returning = false;
		enemies[enemyIdx].queued = false;

		spawnParticles(enemies[enemyIdx].xCoord >> 4, enemies[enemyIdx].yCoord >> 4, 10);
		spawnParticles(player.xCoord >> 4, player.yCoord >> 4, 8);

		player.health -= 34;
		screenInvertTimer = 10;
		beep.tone(beep.freq(80), 10);

		if (player.health <= 0) {

			player.lives--;
			spawnParticles(player.xCoord >> 4, player.yCoord >> 4, 20);
			beep.tone(beep.freq(50), 25);

			if (player.lives == 0) {
				gameState = GameState::GAMEOVER;
			}
			else {
				player.health = 100;
				player.xCoord = 4 << 4;
				player.yCoord = 32 << 4;
				invulnTimer = 90;
			}

		}
		else {
			invulnTimer = 80;
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

		bool msCollision = arduboy.collide(playerRect, mothershipRect);

		if (msCollision && invulnTimer == 0) {

			mothership.active = false;
			spawnParticles(mothership.xCoord >> 4, mothership.yCoord >> 4, 20);
			spawnParticles(player.xCoord >> 4, player.yCoord >> 4, 20);

			player.health -= 50;
			screenInvertTimer = 10;
			beep.tone(beep.freq(80), 15);
			invulnTimer = 80;

		}


		// Player bullet hits the mothership ..

		for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++) {

			Rect bulletRect{
				(int16_t)(pBullets[b].xCoord >> 4),
				(int16_t)(pBullets[b].yCoord >> 4),
				3,
				1
			};

			if (pBullets[b].active && arduboy.collide(bulletRect, mothershipRect)) {

				pBullets[b].active = false;
				mothership.hp--;

				if (mothership.hp == 0) {

					mothership.active = false;
					spawnParticles((mothership.xCoord + formationOffsetX) >> 4, mothership.yCoord >> 4, 8);
					score += 200;
					if (mothership.diving) dualShooterTimer = 600;
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

	for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++) {

		Rect bulletRect{	
			(int16_t)(pBullets[b].xCoord >> 4),	
			(int16_t)(pBullets[b].yCoord >> 4),	
			3,
			1
		};

		if (pBullets[b].active &&					arduboy.collide(bulletRect, healthPackRect)) {
			pBullets[b].active = false;
			healthPack.active = false;
			int16_t healAmount = random(25, 51);
			player.health += healAmount;
			if (player.health > 100) player.health = 100;
			spawnParticles(hpX, hpY, 6);
			beep.tone(beep.freq(1046), 6);

		}
		break;

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

	uint8_t enemySz = (enemies[enemyIdx].type == 2) ? 7 : 6;

	Rect enemyRect{
		(int16_t)(enemies[enemyIdx].xCoord >> 4),
		(int16_t)(enemies[enemyIdx].yCoord >> 4),
		enemySz,
		enemySz
	};

	for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++) {

		Rect bulletRect{
			(int16_t)(pBullets[b].xCoord >> 4),
			(int16_t)(pBullets[b].yCoord >> 4),
			3,
			1
		};

		if (pBullets[b].active && arduboy.collide(bulletRect, enemyRect)) {

			pBullets[b].active = false;
			enemies[enemyIdx].hp--;

			if (enemies[enemyIdx].hp == 0) {

				enemies[enemyIdx].active = false;
				enemies[enemyIdx].active = false;
				spawnParticles(enemies[enemyIdx].xCoord >> 4, enemies[enemyIdx].yCoord >> 4, 12);
				score += (enemies[enemyIdx].type + 1) * 25 * (enemies[enemyIdx].diving ? 2 : 1);
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

	if (invulnTimer == 0 && arduboy.collide(bulletRect, playerRect)) {

		eBullets[bulletIdx].active = false;
		spawnParticles(player.xCoord >> 4, player.yCoord >> 4, 8);
		player.health -= 34;
		screenInvertTimer = 10;
		beep.tone(beep.freq(80), 10);

		if (player.health <= 0) {

			player.lives--;
			spawnParticles(player.xCoord >> 4, player.yCoord >> 4, 20);
			beep.tone(beep.freq(50), 25);

			if (player.lives == 0) {
				gameState = GameState::GAMEOVER;
			}
			else {
				player.health = 100;
				player.xCoord = 4 << 4;
				player.yCoord = 32 << 4;
				invulnTimer = 90;
			}

		}
		else {
			invulnTimer = 80;
		}

	}

}

void launchHealthPack() {

	if (score >= 3000) {

		healthPackSpawnTimer++;

		if (healthPackSpawnTimer > 500) {

			healthPackSpawnTimer = 0;
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

	if (dualShooterTimer > 0) {

		for (uint8_t i = 0; i < MAX_PLAYER_BULLETS && i + 1 < MAX_PLAYER_BULLETS; i += 2) {
		
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

		for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {

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

	for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {

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