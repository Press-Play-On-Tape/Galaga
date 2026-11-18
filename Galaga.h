#include <Sprites.h>

#include "src/Images.h"
#include "src/utils/Arduboy2Ext.h"
#include "src/entities/Entities.h"


Arduboy2Ext arduboy;
BeepPin1 beep;

// --- Custom 5x8 Font for Numbers (Width: 5, Height: 8, 10 frames for digits
// 0-9) ---

// --- Constants ---
#define MAX_PLAYER_BULLETS 4
#define MAX_ENEMY_BULLETS 4
#define ENEMY_COLS 4
#define ENEMY_ROWS 6
#define MAX_ENEMIES (ENEMY_ROWS * ENEMY_COLS)
#define MAX_PARTICLES 12
#define MAX_STARS 16

// --- Data Structures ---




// --- Game State Variables ---
enum class GameState : uint8_t { SplahScreen, Title_Init, Title, PLAYING, GAMEOVER };
GameState gameState = GameState::SplahScreen;

Bullet pBullets[MAX_PLAYER_BULLETS];
Bullet eBullets[MAX_ENEMY_BULLETS];
Enemy enemies[MAX_ENEMIES];
Mothership mothership;
HealthPack healthPack;
Particle particles[MAX_PARTICLES];
Star stars[MAX_STARS];
Player player;

uint16_t score = 0;
uint8_t wave = 1;
uint16_t invulnTimer = 0;
uint8_t screenInvertTimer = 0;
uint16_t dualShooterTimer = 0;
uint16_t survivalTimer = 0;

int16_t formationOffsetX = 0;
int16_t formationOffsetY = 0;
uint16_t diveTimer = 0;
uint16_t mothershipSpawnTimer = 0;
uint16_t healthPackSpawnTimer = 0;
uint8_t arrivalReleaseTimer = 0;
uint16_t replenishTimer = 0;
uint16_t formationFrame = 0;


#include "Galaga_Render.h"
#include "Galaga_Utils.h"


void initWave() {

	mothership.active = false;
	mothershipSpawnTimer = 0;
	healthPack.active = false;
	healthPackSpawnTimer = 0;
	arrivalReleaseTimer = 0;
	replenishTimer = 0;
	formationOffsetX = 0;
	formationOffsetY = 0;
	formationFrame = 0;

	int16_t waveYShift = 0;

	uint8_t idx = 0;
	for (uint8_t c = 0; c < ENEMY_COLS; c++) {
		for (uint8_t r = 0; r < ENEMY_ROWS; r++) {
			const uint8_t COL_SPACING = 10;
			const uint8_t ROW_SPACING = 9;

			// enemies[idx].homeX = (60 + (c * COL_SPACING)) << 4;
			enemies[idx].homeX = (72 + (c * COL_SPACING)) << 4;

			enemies[idx].homeY = ((6 + (r * ROW_SPACING)) + waveYShift) << 4;

			if (enemies[idx].homeY < (2 << 4)) enemies[idx].homeY = 2 << 4;

			enemies[idx].active = true;
			enemies[idx].diving = false;
			enemies[idx].returning = false;
			enemies[idx].queued = true;
			enemies[idx].arriving = false;

			// Enter from sides at least 24 pixels above the bottom (Y between 4
			// and 38)
			uint8_t side = random(0, 3);

			enemies[idx].spawnSide = side;

			switch (side) {
			case 0:  // far left

				enemies[idx].xCoord = -40 << 4;
				enemies[idx].yCoord = random(4, 56) << 4;

				break;

			case 1:  // far right

				enemies[idx].xCoord = 168 << 4;
				enemies[idx].yCoord = random(4, 56) << 4;

				break;

			default:  // top

				enemies[idx].xCoord = random(40, 100) << 4;

				enemies[idx].yCoord = -32 << 4;

				break;
			}

			if (c == 1) {
				enemies[idx].type = 1;  // Butterfly
				enemies[idx].hp = 1;
			}
			else if (c >= 2) {
				enemies[idx].type = 2;  // Boss
				enemies[idx].hp = 2;
			}
			else {
				enemies[idx].type = 0;  // Bee
				enemies[idx].hp = 1;
			}
			idx++;
		}
	}
}



void resetGame() {

	player.reset();
	score = 0;
	wave = 1;
	formationOffsetX = 0;
	invulnTimer = 90;
	screenInvertTimer = 0;
	dualShooterTimer = 0;
	survivalTimer = 0;
	for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) pBullets[i].active = false;
	for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) eBullets[i].active = false;
	initWave();

}

void triggerRandomDives() {
	uint8_t maxSquad = min(5, 1 + (wave / 2) + (survivalTimer / 800));
	uint8_t squadSize = random(1, maxSquad + 1);
	uint8_t pattern = random(0, 3);

	for (uint8_t s = 0; s < squadSize; s++) {
		int16_t target = -1;
		uint8_t startIndex = random(0, MAX_ENEMIES);

		for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
			uint8_t idx = (startIndex + i) % MAX_ENEMIES;
			if (enemies[idx].active &&
				!enemies[idx].diving &&
				!enemies[idx].returning &&
				!enemies[idx].arriving &&
				!enemies[idx].queued) {
				bool rowOccupied = false;
				for (uint8_t d = 0; d < MAX_ENEMIES; d++) {
					if (enemies[d].diving && abs(enemies[d].homeY - enemies[idx].homeY) < (8 << 4)) {
						rowOccupied = true;
						break;
					}
				}
				if (!rowOccupied) {
					if (abs(
						enemies[idx].yCoord -
						(enemies[idx].homeY + formationOffsetY)
					) > (4 << 4)) {
						continue;
					}
					target = idx;
					break;
				}
			}
		}

		if (target != -1) {

			Serial.print("SELECT DIVER ");
			Serial.print(target);
			Serial.print(" HX=");
			Serial.print(enemies[target].homeX >> 4);
			Serial.print(" HY=");
			Serial.println(enemies[target].homeY >> 4);

			enemies[target].xCoord = enemies[target].homeX + formationOffsetX;
			enemies[target].yCoord = enemies[target].homeY + formationOffsetY;
			enemies[target].diving = true;
			enemies[target].returning = false;
			enemies[target].divePhase = 0;
			enemies[target].divePattern = pattern;
			enemies[target].diveDirection = (random(0, 2) == 0) ? 1 : -1;

		}

	}

	diveTimer = 0;

	if (squadSize > 0) {
		beep.tone(beep.freq(300), 4);
	}

}

void setup() {

	arduboy.boot();
	arduboy.display();
	arduboy.flashlight();
	arduboy.systemButtons();
	arduboy.audio.begin();
	arduboy.waitNoButtons();
	arduboy.setFrameRate(60);
	arduboy.initRandomSeed();
	beep.begin();

	Serial.begin(9600);

	for (uint8_t i = 0; i < MAX_STARS; i++) {
		stars[i].xCoord = random(0, 115);
		stars[i].yCoord = random(0, 64);
		stars[i].speed = random(1, 3);
	}
}

void updateStars() {
	for (uint8_t i = 0; i < MAX_STARS; i++) {
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

void updatePlaying() {

	if (invulnTimer > 0) invulnTimer--;
	if (screenInvertTimer > 0) screenInvertTimer--;
	if (dualShooterTimer > 0) dualShooterTimer--;
	survivalTimer++;

	if (screenInvertTimer > 0) {
		arduboy.invert(true);
	}
	else {
		arduboy.invert(false);
	}

	// Player Controls
	if (arduboy.pressed(UP_BUTTON)) player.vY -= 6;
	if (arduboy.pressed(DOWN_BUTTON)) player.vY += 6;

	player.vY = (player.vY * 13) >> 4;
	player.yCoord += player.vY;

	if (player.yCoord < (2 << 4)) {
		player.yCoord = 2 << 4;
		player.vY = 0;
	}
	if (player.yCoord > (54 << 4)) {
		player.yCoord = 54 << 4;
		player.vY = 0;
	}

	if (arduboy.pressed(LEFT_BUTTON)) player.vX -= 6;
	if (arduboy.pressed(RIGHT_BUTTON)) player.vX += 6;

	player.vX = (player.vX * 13) >> 4;
	player.xCoord += player.vX;

	if (player.xCoord < (4 << 4)) {
		player.xCoord = 4 << 4;
		player.vX = 0;
	}
	if (player.xCoord > (35 << 4)) {
		player.xCoord = 35 << 4;
		player.vX = 0;
	}

	if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
		firePlayerBullet();
	}


	// Update and render Bullets ..

	updateAndRenderBullets();


	// Handle health packs ..

	if (!healthPack.active) {

		launchHealthPack();

	}
	else {

		healthPack.yCoord += healthPack.vy;
		int16_t hpX = healthPack.xCoord >> 4;
		int16_t hpY = healthPack.yCoord >> 4;

		if (hpY < -10 || hpY > 70) {

			healthPack.active = false;

		}

		if (healthPack.active) {

			checkHealthPackCollision();
			Sprites::drawSelfMasked(hpX, hpY, Images::healthPackSprite, 0);

		}

	}



	arrivalReleaseTimer++;
	uint8_t releaseThreshold = max(10, 25 - (survivalTimer / 300));
	if (arrivalReleaseTimer > releaseThreshold) {

		arrivalReleaseTimer = 0;
		uint8_t releaseCount = random(1, 4);
		uint8_t released = 0;

		for (uint8_t i = 0; i < MAX_ENEMIES && released < releaseCount; i++) {

			if (enemies[i].active && enemies[i].queued) {
				enemies[i].queued = false;
				enemies[i].arriving = true;
				enemies[i].entryPhase = 0;
				released++;
			}

		}

	}

	replenishTimer++;
	if (replenishTimer > 250) {

		replenishTimer = 0;

		for (uint8_t idx = 0; idx < MAX_ENEMIES; idx++) {

			if (!enemies[idx].active) {

				enemies[idx].active = true;
				enemies[idx].hp = (enemies[idx].type == 2) ? 2 : 1;
				enemies[idx].queued = true;
				enemies[idx].arriving = false;
				uint8_t side = random(0, 3);

				enemies[idx].spawnSide = side;

				switch (side) {
				case 0:  // left

					enemies[idx].xCoord = -24 << 4;

					enemies[idx].yCoord = ((enemies[idx].homeY >> 4) + random(-6, 7)) << 4;

					break;

				case 1:  // right

					enemies[idx].xCoord = 144 << 4;

					enemies[idx].yCoord = ((enemies[idx].homeY >> 4) + random(-6, 7)) << 4;

					break;

				default:  // top

					enemies[idx].xCoord = ((enemies[idx].homeX >> 4) + random(-20, 21)) << 4;

					enemies[idx].yCoord = -24 << 4;

					break;
				}
				break;

			}

		}

	}

	bool formationComplete = true;

	for (uint8_t i = 0; i < MAX_ENEMIES; i++) {

		if (enemies[i].active && (enemies[i].queued || enemies[i].arriving)) {
			formationComplete = false;
			break;
		}

	}

	if (formationComplete) {
		formationFrame++;
	}

	// uint8_t deadColumns = 0;

	// for (uint8_t c = 0; c < ENEMY_COLS; c++) {

	// 	bool colDead = true;

	// 	for (uint8_t r = 0; r < ENEMY_ROWS; r++) {

	// 		if (enemies[c * ENEMY_ROWS + r].active) {
	// 			colDead = false;
	// 			break;
	// 		}

	// 	}

	// 	if (colDead) deadColumns++;

	// }

	float dynamicAmplitude = 4.0f;

	formationOffsetX = 0;

	formationOffsetY = (1 << 4) + (int16_t)(sin((float)formationFrame * 0.025f) * dynamicAmplitude * 16.0f);

	if (!mothership.active && formationComplete) {

		spawnMothership();

	}
	else if (mothership.active) {

		if (!mothership.diving) {

			mothership.yCoord += mothership.dir * 8;

			if (mothership.yCoord < (4 << 4) || mothership.yCoord >(54 << 4)) {
				mothership.dir *= -1;
			}
			if (random(0, 250) == 0) {
				mothership.diving = true;
				mothership.phase = 0;
			}

		}
		else {

			mothership.phase += 1;
			mothership.xCoord -= 12;
			mothership.yCoord += (int16_t)(sin((float)mothership.phase * 0.05) * 24.0);

			if (mothership.yCoord < (4 << 4)) mothership.yCoord = 4 << 4;
			if (mothership.yCoord > (54 << 4)) mothership.yCoord = 54 << 4;

			if (mothership.xCoord < (-8 << 4)) {
				mothership.xCoord = 110 << 4;
				mothership.diving = false;
			}

		}

		int16_t msX = (mothership.xCoord + formationOffsetX) >> 4;
		int16_t msY = mothership.yCoord >> 4;

		if (msX >= -8 && msX < 128 && msY >= -8 && msY < 64) {
			Sprites::drawSelfMasked(msX, msY, Images::mothershipSprite, 0);
		}

	}

	if (formationComplete) {

		diveTimer++;
		uint16_t currentDiveInterval = max(25, 130 - (wave * 8) - (survivalTimer / 40));

		if (diveTimer >= currentDiveInterval) {
			diveTimer = 0;
			triggerRandomDives();
		}

	}

	uint8_t activeEnemies = 0;
	for (uint8_t i = 0; i < MAX_ENEMIES; i++) {

		if (!enemies[i].active) continue;

		activeEnemies++;

		if (enemies[i].queued) continue;

		if (enemies[i].arriving) {

			enemies[i].entryPhase++;

			int16_t targetX = enemies[i].homeX + formationOffsetX;
			int16_t targetY = enemies[i].homeY + formationOffsetY;
			int16_t dx = targetX - enemies[i].xCoord;
			int16_t dy = targetY - enemies[i].yCoord;
			int16_t curveX = 0;
			int16_t curveY = 0;

			switch (enemies[i].spawnSide) {

				case 0:  // left spiral

					curveY = (int16_t)(sin(enemies[i].entryPhase * 0.06f) * (12 << 4));
					curveX = (int16_t)(cos(enemies[i].entryPhase * 0.05f) * (5 << 4));
					break;

				case 1:  // right spiral

					curveY = (int16_t)(sin(enemies[i].entryPhase * 0.06f) * (12 << 4));
					curveX = -(int16_t)(cos(enemies[i].entryPhase * 0.05f) * (5 << 4));
					break;

				default:  // top dive

					curveX = (int16_t)(sin(enemies[i].entryPhase * 0.08f) * (10 << 4));
					break;

			}

			if (abs(dx) > (24 << 4) || abs(dy) > (24 << 4)) {

				enemies[i].xCoord += dx / 10 + curveX / 12;
				enemies[i].yCoord += dy / 10 + curveY / 12;

			}
			else {

				enemies[i].xCoord += dx / 6;
				enemies[i].yCoord += dy / 6;

			}

			if (abs(dx) < (8 << 4) && abs(dy) < (8 << 4)) {

				enemies[i].arriving = false;
				enemies[i].xCoord = enemies[i].homeX + formationOffsetX;
				enemies[i].yCoord = enemies[i].homeY + formationOffsetY;

			}

		}
		else if (!enemies[i].diving) {

			enemies[i].xCoord = enemies[i].homeX + formationOffsetX;
			int16_t targetY = enemies[i].homeY + formationOffsetY;

			if (targetY < (2 << 4)) targetY = 2 << 4;
			if (targetY > (60 << 4)) targetY = 60 << 4;

			enemies[i].yCoord = targetY;

		}
		else {

			int16_t diveSpeed = 10 + (wave * 1) + (survivalTimer / 300);

			if (!enemies[i].returning) {
				// Once we've reached the player area,
				// commit to an escape run.
				if (enemies[i].xCoord < (-40 << 4)) {
					Serial.print("BREAKAWAY ");
					Serial.print(i);
					Serial.print(" X=");
					Serial.print(enemies[i].xCoord >> 4);
					Serial.print(" Y=");
					Serial.println(enemies[i].yCoord >> 4);

					enemies[i].returning = true;

					continue;   // <<<<<<<<<< IMPORTANT
				}

				enemies[i].divePhase += 1;
				int16_t attackX = player.xCoord + random(-(24 << 4), (24 << 4));
				int16_t attackY = 48 << 4;   // attack zone near player
				int16_t dx = attackX - enemies[i].xCoord;
				int16_t dy = attackY - enemies[i].yCoord;
				int32_t len = max((int32_t)1, (int32_t)sqrt((float)dx * dx + (float)dy * dy));

				enemies[i].xCoord += (dx * diveSpeed) / len;
				enemies[i].yCoord += (dy * diveSpeed) / len;

				int16_t oldY = enemies[i].yCoord;


				if (enemies[i].divePattern == 0) {
					enemies[i].xCoord += (int16_t)(sin((float)enemies[i].divePhase * 0.06) * 6.0 * enemies[i].diveDirection);
				}
				else if (enemies[i].divePattern == 1) {
					enemies[i].xCoord += (int16_t)(cos((float)enemies[i].divePhase * 0.08f) * 8.0f * enemies[i].diveDirection);
				}

				if (enemies[i].yCoord < (2 << 4)) enemies[i].yCoord = 2 << 4;

				uint8_t firingThreshold = wave + (survivalTimer / 600);
				int16_t dxPlayer = abs((player.xCoord >> 4) - (enemies[i].xCoord >> 4));

				if (dxPlayer < 40) {
					if (random(0, 150) < firingThreshold) {
						fireEnemyBullet(enemies[i].xCoord, enemies[i].yCoord);
					}
				}

			}
			else {

				// Escape phase: fly off the bottom-left
				if (enemies[i].yCoord >= (48 << 4)) {

					enemies[i].xCoord -= diveSpeed;
					enemies[i].yCoord += diveSpeed;

					if (enemies[i].yCoord > (96 << 4)) {

						uint8_t side = random(0, 3);
						enemies[i].spawnSide = side;

						switch (side) {

							case 0:  // left
								enemies[i].xCoord = -24 << 4;
								enemies[i].yCoord = ((enemies[i].homeY >> 4) + random(-6, 7)) << 4;
								break;

							case 1:  // right
								enemies[i].xCoord = 144 << 4;
								enemies[i].yCoord = ((enemies[i].homeY >> 4) + random(-6, 7)) << 4;
								break;

							default: // top
								enemies[i].xCoord = ((enemies[i].homeX >> 4) + random(-20, 21)) << 4;
								enemies[i].yCoord = -24 << 4;
								break;

						}


						enemies[i].returning = false;
						enemies[i].arriving = true;
						enemies[i].entryPhase = 0;
						enemies[i].diving = false;
						enemies[i].divePhase = 0;
						enemies[i].divePattern = 255;

					}

				}
				else {

					int16_t targetX = enemies[i].homeX + formationOffsetX;
					int16_t targetY = enemies[i].homeY;

					int16_t dx = targetX - enemies[i].xCoord;
					int16_t dy = targetY - enemies[i].yCoord;

					enemies[i].xCoord += dx / 8;
					enemies[i].yCoord += dy / 8;

					if (abs(dx) < (3 << 4) && abs(dy) < (3 << 4)) {

						enemies[i].xCoord = targetX;
						enemies[i].yCoord = targetY;
						enemies[i].diving = false;
						enemies[i].returning = false;
						enemies[i].divePhase = 0;
						enemies[i].divePattern = 255;

					}

				}

			}

		}


		// Player bullet hits an enemy ..

		checkEnemyAndPlayerBulletCollision(i);
		checkMothershipCollision();

		bool canCollide = enemies[i].diving;

		if (!canCollide && !enemies[i].arriving) {

			bool isLeftmostInRow = true;

			for (uint8_t other = 0; other < MAX_ENEMIES; other++) {

				if (enemies[other].active && !enemies[other].diving && !enemies[other].arriving &&
					!enemies[other].queued && abs(enemies[other].homeY - enemies[i].homeY) < (8 << 4)) {

					if (enemies[other].homeX < enemies[i].homeX) {
						isLeftmostInRow = false;
						break;
					}

				}

			}

			if (isLeftmostInRow) canCollide = true;

		}


		// Has the player and the enemy collided?

		checkEnemyAndPlayerCollision(i);
		renderEnemy(i);


	}

	if (activeEnemies == 0 && !mothership.active) {
		wave++;
		initWave();
	}

	for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {

		if (eBullets[i].active) {

			eBullets[i].xCoord += eBullets[i].vx;

			if (eBullets[i].xCoord < 0) {

				eBullets[i].active = false;

			}
			else {

				arduboy.drawFastHLine(eBullets[i].xCoord >> 4, eBullets[i].yCoord >> 4, 3, WHITE);
				checkPlayerAndEnemyBulletCollision(i);


			}

		}

	}

	renderParticles();

	renderPlayer();

	renderScore(score);
	renderLivesAndHealth();
}

void loop() {

	if (!arduboy.nextFrame()) return;
	arduboy.pollButtons();
	beep.timer();

	switch (gameState) {

	case GameState::SplahScreen:
		splashScreen();
		break;

	case GameState::Title_Init:
		titleScreen_Init();
		[[fallthrough]]

	case GameState::Title:
		updateStars();
		titleScreen();
		break;

	case GameState::PLAYING:
		updateStars();
		updatePlaying();
		break;

	case GameState::GAMEOVER:

		updateStars();
		arduboy.invert(false);
		arduboy.setCursor(44, 25);
		arduboy.print(F("OVER"));
		arduboy.setCursor(40, 45);
		renderScore(score);
		if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
			resetGame();
			gameState = GameState::PLAYING;
		}

		break;

	}

	arduboy.display(true);
}