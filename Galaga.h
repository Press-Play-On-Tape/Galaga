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

void drawVerticalScore(uint16_t currentScore) {
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

void drawLivesAndHealth() {

	uint8_t displayLives = (player.lives > 9) ? 9 : player.lives;
	Sprites::drawSelfMasked(121, 2, Images::Numbers, displayLives);

	arduboy.drawRect(120, 11, 6, 22, WHITE);
	uint8_t fillHeight = (player.health * 18) / 100;

	if (fillHeight > 18) fillHeight = 18;

	arduboy.fillRect(122, 13, 2, fillHeight, WHITE);
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

	for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
		if (pBullets[i].active) {
			pBullets[i].xCoord += pBullets[i].vx;
			if (pBullets[i].xCoord > 115 << 4) pBullets[i].active = false;
			arduboy.drawFastHLine(pBullets[i].xCoord >> 4, pBullets[i].yCoord >> 4, 3, WHITE);
		}
	}

	// Health Pack logic
	if (!healthPack.active) {
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
	else {
		healthPack.yCoord += healthPack.vy;
		int16_t hpX = healthPack.xCoord >> 4;
		int16_t hpY = healthPack.yCoord >> 4;

		if (hpY < -10 || hpY > 70) {
			healthPack.active = false;
		}
		else {

			Sprites::drawSelfMasked(hpX, hpY, Images::healthPackSprite, 0);


			// Player shoots a health pack ..

			for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++) {

				if (pBullets[b].active && pBullets[b].xCoord >= healthPack.xCoord &&
					pBullets[b].xCoord <= healthPack.xCoord + (6 << 4) && pBullets[b].yCoord >= healthPack.yCoord &&
					pBullets[b].yCoord <= healthPack.yCoord + (6 << 4)) {

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

			if (healthPack.active && healthPack.xCoord + (6 << 4) > player.xCoord && healthPack.xCoord < player.xCoord + (7 << 4) &&
				healthPack.yCoord + (6 << 4) > player.yCoord && healthPack.yCoord < player.yCoord + (7 << 4)) {

				healthPack.active = false;
				int16_t healAmount = random(25, 51);
				player.health += healAmount;
				if (player.health > 100) player.health = 100;
				spawnParticles(player.xCoord >> 4, player.yCoord >> 4, 6);
				beep.tone(beep.freq(1046), 6);

			}
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

	uint8_t deadColumns = 0;

	for (uint8_t c = 0; c < ENEMY_COLS; c++) {

		bool colDead = true;

		for (uint8_t r = 0; r < ENEMY_ROWS; r++) {

			if (enemies[c * ENEMY_ROWS + r].active) {
				colDead = false;
				break;
			}

		}

		if (colDead) deadColumns++;

	}

	float dynamicAmplitude = 4.0f;

	formationOffsetX = 0;

	formationOffsetY = (1 << 4) + (int16_t)(sin((float)formationFrame * 0.025f) * dynamicAmplitude * 16.0f);

	if (!mothership.active && formationComplete) {

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
		
Serial.print("diveTimer ");
Serial.print(diveTimer);
Serial.print(", currentDiveInterval ");
Serial.println(currentDiveInterval);


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

				enemies[i].divePhase += 1;
				int16_t attackX = player.xCoord;
				int16_t attackY = 48 << 4;   // attack zone near player
				int16_t dx = attackX - enemies[i].xCoord;
				int16_t dy = attackY - enemies[i].yCoord;

// Serial.print("DX=");
// Serial.print(dx);
// Serial.print(" DY=");
// Serial.println(dy);
				

// Once we've reached the player area,
// commit to an escape run.
if (enemies[i].yCoord >= (48 << 4)) {

    // Serial.print("ESCAPE ");
    // Serial.print(i);
    // Serial.print(" X=");
    // Serial.print(enemies[i].xCoord >> 4);
    // Serial.print(" Y=");
    // Serial.println(enemies[i].yCoord >> 4);

    enemies[i].returning = true;

    continue;   // <<<<<<<<<< IMPORTANT
}



				int32_t len = max((int32_t)1, (int32_t)sqrt((float)dx * dx + (float)dy * dy));

				enemies[i].xCoord += (dx * diveSpeed) / len;
				enemies[i].yCoord += (dy * diveSpeed) / len;

				int16_t oldY = enemies[i].yCoord;

				if (enemies[i].xCoord > (160 << 4)) {
					enemies[i].diving = false;
					enemies[i].returning = true;
				}

				if (enemies[i].xCoord < (-32 << 4)) {
					enemies[i].diving = false;
					enemies[i].returning = true;
				}
// Serial.print("DIVE ");
// Serial.print(i);
// Serial.print(" X=");
// Serial.print(enemies[i].xCoord >> 4);
// Serial.print(" Y=");
// Serial.println(enemies[i].yCoord >> 4);

				if (enemies[i].divePattern == 0) {
					enemies[i].xCoord +=
						(int16_t)(sin((float)enemies[i].divePhase * 0.06) * 6.0 * enemies[i].diveDirection);
				}
				else if (enemies[i].divePattern == 1) {
					enemies[i].xCoord += (int16_t)(cos((float)enemies[i].divePhase * 0.08) * 8.0);
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

// Serial.print("ESCAPING ");
// Serial.print(i);
// Serial.print(" X=");
// Serial.print(enemies[i].xCoord >> 4);
// Serial.print(" Y=");
// Serial.println(enemies[i].yCoord >> 4);

					if (enemies[i].yCoord > (96 << 4)) {

					
// Serial.print("OFFSCREEN ");
// Serial.println(i);

uint8_t side = random(0, 3);

enemies[i].spawnSide = side;

switch (side) {

    case 0:  // left
        enemies[i].xCoord = -24 << 4;
        enemies[i].yCoord =
            ((enemies[i].homeY >> 4) + random(-6, 7)) << 4;
        break;

    case 1:  // right
        enemies[i].xCoord = 144 << 4;
        enemies[i].yCoord =
            ((enemies[i].homeY >> 4) + random(-6, 7)) << 4;
        break;

    default: // top
        enemies[i].xCoord =
            ((enemies[i].homeX >> 4) + random(-20, 21)) << 4;
        enemies[i].yCoord = -24 << 4;
        break;
}
// Serial.print("REENTER ");
// Serial.print(i);
// Serial.print(" SIDE=");
// Serial.println(side);

enemies[i].diving = false;
enemies[i].returning = false;

enemies[i].arriving = true;
enemies[i].entryPhase = 0;

enemies[i].divePhase = 0;
enemies[i].divePattern = 255;

// Serial.print("REFORM ");
// Serial.print(i);
// Serial.print(" X=");
// Serial.print(enemies[i].xCoord >> 4);
// Serial.print(" Y=");
// Serial.println(enemies[i].yCoord >> 4);

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

		uint8_t enemySz = (enemies[i].type == 2) ? 7 : 6;

		Rect enemyRect {
			(int16_t)(enemies[i].xCoord >> 4),
			(int16_t)(enemies[i].yCoord >> 4),
			enemySz,
			enemySz
		};
		
		for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++) {

			Rect bulletRect {
				(int16_t)(pBullets[b].xCoord >> 4),
				(int16_t)(pBullets[b].yCoord >> 4),
				3,
				1
			};

			if (pBullets[b].active && arduboy.collide(bulletRect, enemyRect)) {

				pBullets[b].active = false;
				enemies[i].hp--;

				if (enemies[i].hp == 0) {

					enemies[i].active = false;
					enemies[i].active = false;
					spawnParticles(enemies[i].xCoord >> 4, enemies[i].yCoord >> 4, 12);
					score += (enemies[i].type + 1) * 25 * (enemies[i].diving ? 2 : 1);
					beep.tone(beep.freq(160), 4);
				}
				else {
					beep.tone(beep.freq(500), 2);
				}
				break;

			}

		}

		
		if (mothership.active) {


			// Did the player and the mothership collide ?

			Rect playerRect {
				(int16_t)(player.xCoord >> 4),
				(int16_t)(player.yCoord >> 4),
				7,
				7
			};

			Rect mothershipRect {
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

				Rect bulletRect {
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
		
		Rect playerRect {
			(int16_t)(player.xCoord >> 4),
			(int16_t)(player.yCoord >> 4),
			7,
			7
		};

		if (!enemies[i].arriving && invulnTimer == 0 && canCollide && arduboy.collide(enemyRect, playerRect)) {

			enemies[i].active = false;
			enemies[i].xCoord = -1000;
			enemies[i].yCoord = -1000;

			enemies[i].arriving = false;
			enemies[i].diving = false;
			enemies[i].returning = false;
			enemies[i].queued = false;

			spawnParticles(enemies[i].xCoord >> 4, enemies[i].yCoord >> 4, 10);
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

		if (enemies[i].active) {

			const uint8_t* sprite = Images::beeSprite;
			if (enemies[i].type == 1) {
				sprite = Images::butterflySprite;
			}
			else if (enemies[i].type == 2) {
				sprite = Images::bossSprite;
			}

			int16_t eX = enemies[i].xCoord >> 4;
			int16_t eY = enemies[i].yCoord >> 4;

			if (eX >= -8 && eX < 128 && eY >= -8 && eY < 64) {
				Sprites::drawSelfMasked(eX, eY, sprite, 0);
			}

		}

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

				if (invulnTimer == 0 && eBullets[i].xCoord >= player.xCoord && eBullets[i].xCoord <= player.xCoord + (7 << 4) &&
					eBullets[i].yCoord >= player.yCoord && eBullets[i].yCoord <= player.yCoord + (7 << 4)) {

					eBullets[i].active = false;
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

		}

	}

	for (uint8_t i = 0; i < MAX_PARTICLES; i++) {

		if (particles[i].life > 0) {
			particles[i].xCoord += particles[i].vx;
			particles[i].yCoord += particles[i].vy;
			particles[i].life--;
			arduboy.drawPixel(particles[i].xCoord >> 4, particles[i].yCoord >> 4, WHITE);
		}

	}

	if (invulnTimer == 0 || (invulnTimer % 12 < 6)) {

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

	drawVerticalScore(score);
	drawLivesAndHealth();
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
		drawVerticalScore(score);
		if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
			resetGame();
			gameState = GameState::PLAYING;
		}

		break;

	}

	arduboy.display(true);
}