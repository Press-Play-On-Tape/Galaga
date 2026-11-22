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

	// Note: cast to int16_t - on the AVR a uint16_t promotes to 'unsigned int', so the subtraction
	// would wrap to a huge number instead of going negative ..

	if (mothership.spawnTimer > max(150, 350 - (int16_t)(survivalTimer / 4))) {
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

// ----------------------------------------------------------------------------
//  Enemy rotation.
//
//  The enemy images have 16 frames, each rotated a further 22.5 degrees. Frame 0
//  is the formation pose, facing -x (down the player's view, towards the player).
//  Frame k faces R(k * 22.5deg) * (-1, 0) in buffer coordinates.
//
constexpr uint8_t EnemyHeadings = 16;
constexpr uint8_t EnemyTurnFrames = 2;		// Frames between each 22.5 degree turn step

uint8_t headingFromMovement(int16_t moveX, int16_t moveY) {

	// The direction 'forward' points, relative to the formation pose ..

	int16_t fx = -moveX;
	int16_t fy = -moveY;

	int32_t ax = abs(fx);
	int32_t ay = abs(fy);


	// Pick the nearest 22.5 degree step within the quadrant using
	// tan(11.25) ~ 1/5, tan(33.75) ~ 2/3, tan(56.25) ~ 3/2, tan(78.75) ~ 5 ..

	uint8_t q;

	if (ay * 5 < ax)           q = 0;
	else if (ay * 3 < ax * 2)  q = 1;
	else if (ay * 2 < ax * 3)  q = 2;
	else if (ay < ax * 5)      q = 3;
	else                       q = 4;

	if (fx >= 0 && fy >= 0) return q;
	if (fx < 0 && fy >= 0)  return 8 - q;
	if (fx < 0)             return 8 + q;
	return (EnemyHeadings - q) % EnemyHeadings;

}

// Turn one step (the shortest way round) towards the target heading ..

void turnEnemyTowards(Enemy &enemy, uint8_t targetHeading) {

	if (enemy.turnTimer > 0) {
		enemy.turnTimer--;
		return;
	}

	uint8_t diff = (targetHeading + EnemyHeadings - enemy.heading) % EnemyHeadings;
	if (diff == 0) return;

	if (diff < EnemyHeadings / 2) {
		enemy.heading = (enemy.heading + 1) % EnemyHeadings;
	}
	else {
		enemy.heading = (enemy.heading + EnemyHeadings - 1) % EnemyHeadings;
	}

	enemy.turnTimer = EnemyTurnFrames - 1;

}

void startPlayerPuff() {

	player.puffX = player.xCoord >> 4;
	player.puffY = player.yCoord >> 4;
	player.deathTimer = 28; // 7 frames * 4 frames per puff

}


// ----------------------------------------------------------------------------
//  Damage the player (unless shielded or invulnerable). After a hit that does
//  not cost a life the player gets a short grace period so that several hits
//  in quick succession (e.g. mothership then a diving enemy) don't stack up.
//
constexpr uint8_t PlayerHitGraceFrames = 45;
constexpr uint8_t PlayerRespawnGraceFrames = 90;

void damagePlayer(int16_t amount, uint8_t particleCount, uint8_t toneDuration) {

	// No more damage once the last life is lost (stops lives-- wrapping 0 -> 255 during the game over delay) ..

	if (player.lives == 0 || player.shieldActive || player.invulnTimer > 0) return;

	spawnParticles(player.xCoord >> 4, player.yCoord >> 4, particleCount);
	player.health -= amount;
	screenInvertTimer = 6;
	beep.tone(beep.freq(80), toneDuration);

	if (player.health > 0) {
		player.invulnTimer = PlayerHitGraceFrames;
		return;
	}

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
		player.invulnTimer = PlayerRespawnGraceFrames;

	}

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
		enemies[enemyIdx].queued = false;

		damagePlayer(30, 8, 10);

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
			damagePlayer(50, 20, 15);
			return;		// Mothership is gone, so skip the bullet checks below ..

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
		damagePlayer(30, 8, 10);

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
