#include <Sprites.h>

#include "src/Images.h"
#include "src/utils/Arduboy2Ext.h"
#include "src/entities/Entities.h"
#include <EEPROM.h>

Arduboy2Ext arduboy;
BeepPin1 beep;


// --- Game State Variables ---

GameState gameState = GameState::SplahScreen;

Bullet pBullets[Constants::PlayerMaxBullets];
Bullet eBullets[Constants::EnemiesMaxBullets];
Enemy enemies[Constants::EnemiesMax];
Mothership mothership;
HealthPack healthPack;
Particle particles[Constants::ParticlesMax];
Star stars[Constants::StarsMax];
Player player;

uint16_t score = 0;
uint8_t wave = 1;
uint8_t screenInvertTimer = 0;
uint16_t survivalTimer = 0;

int16_t formationOffsetY = 0;
uint16_t diveTimer = 0;

uint8_t arrivalReleaseTimer = 0;
uint16_t replenishTimer = 0;
bool waveEnemiesCleared = false;	// All enemies destroyed, only the mothership is left this wave
uint16_t formationFrame = 0;
uint8_t gameOverDelayTimer = 0;
uint8_t goDiversInitialized = 0;







bool enteringInitials = false;
uint8_t initialCursor = 0;
char currentInitials[3] = { 'A', 'A', 'A' };

HighScoreEntry highScores[Constants::NumberHighScores];


#include "Galaga_Render.h"
#include "Galaga_Utils.h"


bool formationComplete() {

	bool complete = true;

	for (uint8_t i = 0; i < Constants::EnemiesMax; i++) {

		if (enemies[i].active && (enemies[i].queued || enemies[i].arriving)) {
			complete = false;
			break;
		}

	}

	return complete;

}

void initWave() {

	mothership.active = false;
	mothership.spawnTimer = 0;
	healthPack.active = false;
	healthPack.spawnTimer = 0;

	arrivalReleaseTimer = 0;
	replenishTimer = 0;
	waveEnemiesCleared = false;
	formationOffsetY = 0;
	formationFrame = 0;

	uint8_t idx = 0;

	// Rows run across the player's view (fixed buffer x), columns run up it (fixed buffer y) ..

	for (uint8_t row = 0; row < Constants::EnemyRows; row++) {

		for (uint8_t col = 0; col < Constants::EnemyColumns; col++) {

			const uint8_t ROW_SPACING = 10;
			const uint8_t COL_SPACING = 9;

			enemies[idx].homeX = (72 + (row * ROW_SPACING)) << 4;
			enemies[idx].homeY = (6 + (col * COL_SPACING)) << 4;

			if (enemies[idx].homeY < (2 << 4)) enemies[idx].homeY = 2 << 4;

			enemies[idx].active = true;
			enemies[idx].diving = false;
			enemies[idx].queued = true;
			enemies[idx].arriving = false;
			enemies[idx].heading = 0;
			enemies[idx].turnTimer = 0;

			// Pick a random edge of the view to enter from ..
			Side side = static_cast<Side>(random(0, 3));

			enemies[idx].spawnSide = side;

			switch (side) {

			case Side::Right:

				enemies[idx].xCoord = random(40, 100) << 4;
				enemies[idx].yCoord = (64 + 32) << 4;
				break;

			case Side::Top:

				enemies[idx].xCoord = 168 << 4;
				enemies[idx].yCoord = random(4, 56) << 4;
				break;

			default:	// Side::Left

				enemies[idx].xCoord = random(40, 100) << 4;
				enemies[idx].yCoord = -32 << 4;
				break;

			}

			switch (row) {

			case 1:
				enemies[idx].type = EnemyType::Butterfly;
				enemies[idx].hp = 1;
				break;

			case 2:
				enemies[idx].type = EnemyType::Boss;
				enemies[idx].hp = 2;
				break;

			case 3:
				enemies[idx].type = EnemyType::Other;
				enemies[idx].hp = 2;
				break;

			default:
				enemies[idx].type = EnemyType::Boss;
				enemies[idx].hp = 1;
				break;

			}

			idx++;

		}

	}

}

void resetGame() {

	player.reset();
	gameOverDelayTimer = 0;
	score = 0;
	wave = 1;
	screenInvertTimer = 0;
	survivalTimer = 0;
	goDiversInitialized = 0;
	enteringInitials = false;

	for (uint8_t i = 0; i < Constants::PlayerMaxBullets; i++) pBullets[i].active = false;
	for (uint8_t i = 0; i < Constants::EnemiesMaxBullets; i++) eBullets[i].active = false;

	initWave();

}

void triggerRandomDives() {

	// Count how many enemies are currently diving right now ?

	uint8_t currentDivers = 0;

	for (uint8_t i = 0; i < Constants::EnemiesMax; i++) {

		if (enemies[i].active && enemies[i].diving) {
			currentDivers++;
		}

	}

	// 2. Determine max squad/diver cap based on score brackets
	uint8_t scoreCap = 4; // Default fallback for 3001+

	switch (score) {

	case 0 ... 200:
		scoreCap = 1;
		break;

	case 201 ... 400:
		scoreCap = 2;
		break;

	case 401 ... 600:
		scoreCap = 3;
		break;

	default:
		scoreCap = 4;
		break;

	}


	// If we have already reached or exceeded the cap for our score bracket, don't trigger new dives ..

	if (score <= 3000 && currentDivers >= scoreCap) {

		diveTimer = 0;
		return;

	}

	// Calculate how many new divers we can launch without breaking the cap ..

	uint8_t maxSquad = min(4, 1 + (wave / 2) + (survivalTimer / 800));

	if (score <= 3000) {
		maxSquad = min(maxSquad, (uint8_t)(scoreCap - currentDivers));
	}

	if (maxSquad == 0) {
		diveTimer = 0;
		return;
	}

	uint8_t squadSize = random(1, maxSquad + 1);
	DivePattern pattern = static_cast<DivePattern>(random(0, static_cast<uint8_t>(DivePattern::Count)));

	for (uint8_t s = 0; s < squadSize; s++) {

		int16_t target = -1;
		uint8_t startIndex = random(0, Constants::EnemiesMax);

		for (uint8_t i = 0; i < Constants::EnemiesMax; i++) {

			uint8_t idx = (startIndex + i) % Constants::EnemiesMax;

			if (enemies[idx].active &&
				!enemies[idx].diving &&
				!enemies[idx].arriving &&
				!enemies[idx].queued) {
				bool rowOccupied = false;

				for (uint8_t d = 0; d < Constants::EnemiesMax; d++) {

					if (enemies[d].diving && abs(enemies[d].homeY - enemies[idx].homeY) < (8 << 4)) {
						rowOccupied = true;
						break;
					}

				}

				if (!rowOccupied) {

					if (abs(enemies[idx].yCoord - (enemies[idx].homeY + formationOffsetY)) > (4 << 4)) {
						continue;
					}

					target = idx;
					break;

				}

			}

		}

		if (target != -1) {

			enemies[target].xCoord = enemies[target].homeX;
			enemies[target].yCoord = enemies[target].homeY + formationOffsetY;
			enemies[target].diving = true;
			enemies[target].divePhase = 0;
			enemies[target].diveIdx = 0;
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
	loadHighScores();

	for (uint8_t i = 0; i < Constants::StarsMax; i++) {
		stars[i].xCoord = random(0, 115);
		stars[i].yCoord = random(0, 64);
		stars[i].speed = random(1, 3);
	}
}


void updatePlaying() {


	// Housekeeping ..

	if (player.invulnTimer > 0) player.invulnTimer--;
	if (screenInvertTimer > 0) screenInvertTimer--;
	if (player.dualShooterTimer > 0) player.dualShooterTimer--;

	survivalTimer++;

	if (screenInvertTimer > 0) {
		arduboy.invert(true);
	}
	else {
		arduboy.invert(false);
	}


	// Player Controls ..

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

	// A fires, B toggles the round shield ..

	if (arduboy.justPressed(A_BUTTON)) {
		firePlayerBullet();
	}

	if (arduboy.pressed(B_BUTTON)) {

		// Only allow activating if player has health to spend ..

		if (!player.shieldActive && player.health > 5) {

			player.shieldActive = true;
			player.shieldDrainTimer = 0;
			beep.tone(beep.freq(600), 4);

		}

	}
	else {

		player.shieldActive = false;

	}

	// Slowly drain health while shield is active (e.g., 1 health every 20 frames at 60fps = 3/sec)
	if (player.shieldActive) {

		player.shieldDrainTimer++;

		if (player.shieldDrainTimer >= 20) {

			player.shieldDrainTimer = 0;

			if (player.health > 0) {
				player.health--;
			}

			if (player.health == 5) {
				player.shieldActive = false;
			}

		}

	}


	// Update Bullets ..

	updateBullets();


	// Handle health packs ..

	if (!healthPack.active) {

		launchHealthPack();

	}
	else {

		healthPack.yCoord += healthPack.vy;
		int16_t hpY = healthPack.yCoord >> 4;

		if (hpY < -10 || hpY > 70) {

			healthPack.active = false;

		}

	}

	if (healthPack.active) 	checkHealthPackCollision();


	arrivalReleaseTimer++;
	uint8_t releaseThreshold = max(10, 25 - (int16_t)(survivalTimer / 300));	// int16_t cast stops an unsigned wrap on the AVR

	if (arrivalReleaseTimer > releaseThreshold) {

		arrivalReleaseTimer = 0;
		uint8_t releaseCount = random(1, 4);
		uint8_t released = 0;

		for (uint8_t i = 0; i < Constants::EnemiesMax && released < releaseCount; i++) {

			if (enemies[i].active && enemies[i].queued) {
				enemies[i].queued = false;
				enemies[i].arriving = true;
				enemies[i].entryPhase = 0;
				released++;
			}

		}

	}

	// Bring back destroyed enemies over time, unless they have all been cleared and only the mothership is left ..

	replenishTimer++;
	if (!waveEnemiesCleared && replenishTimer > 250) {

		replenishTimer = 0;

		for (uint8_t idx = 0; idx < Constants::EnemiesMax; idx++) {

			if (!enemies[idx].active) {

				Side side = static_cast<Side>(random(0, 3));

				enemies[idx].active = true;
				enemies[idx].hp = (enemies[idx].type == EnemyType::Boss) ? 2 : 1;
				enemies[idx].queued = true;
				enemies[idx].arriving = false;
				enemies[idx].spawnSide = side;
				enemies[idx].heading = 0;
				enemies[idx].turnTimer = 0;

				switch (side) {

				case Side::Right:

					enemies[idx].xCoord = ((enemies[idx].homeX >> 4) + random(-20, 21)) << 4;
					enemies[idx].yCoord = (64 + 24) << 4;
					break;

				case Side::Top:

					enemies[idx].xCoord = 144 << 4;
					enemies[idx].yCoord = ((enemies[idx].homeY >> 4) + random(-6, 7)) << 4;
					break;

				default:	// Side::Left

					enemies[idx].xCoord = ((enemies[idx].homeX >> 4) + random(-20, 21)) << 4;
					enemies[idx].yCoord = -24 << 4;
					break;

				}
				break;

			}

		}

	}

	if (formationComplete()) {
		formationFrame++;
	}


	constexpr float dynamicAmplitude = 4.0f;

	formationOffsetY = (1 << 4) + (int16_t)(sin((float)formationFrame * 0.025f) * dynamicAmplitude * 16.0f);

	if (!mothership.active && formationComplete()) {

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

	}

	if (formationComplete()) {

		if (formationFrame > 120) { 

			diveTimer++;
			uint16_t currentDiveInterval = max(40, 150 - (wave * 8) - (int16_t)(survivalTimer / 40));	// int16_t cast stops an unsigned wrap on the AVR

			if (diveTimer >= currentDiveInterval) {
				diveTimer = 0;
				triggerRandomDives();
			}

		}

	}

	uint8_t activeEnemies = 0;

	for (uint8_t i = 0; i < Constants::EnemiesMax; i++) {

		if (!enemies[i].active) continue;

		activeEnemies++;

		if (enemies[i].queued) continue;

		if (enemies[i].arriving) {

			enemies[i].entryPhase++;

			int16_t targetX = enemies[i].homeX;
			int16_t targetY = enemies[i].homeY + formationOffsetY;
			int16_t dx = targetX - enemies[i].xCoord;
			int16_t dy = targetY - enemies[i].yCoord;
			int16_t curveX = 0;
			int16_t curveY = 0;

			switch (enemies[i].spawnSide) {

			case Side::Top:  // spiral in from above

				curveY = (int16_t)(sin(enemies[i].entryPhase * 0.06f) * (12 << 4));
				curveX = -(int16_t)(cos(enemies[i].entryPhase * 0.05f) * (5 << 4));
				break;

			default:  // Side::Left / Side::Right - weave in from the side

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
				enemies[i].xCoord = enemies[i].homeX;
				enemies[i].yCoord = enemies[i].homeY + formationOffsetY;

			}

		}
		else if (!enemies[i].diving) {

			enemies[i].xCoord = enemies[i].homeX;
			int16_t targetY = enemies[i].homeY + formationOffsetY;

			if (targetY < (2 << 4)) targetY = 2 << 4;
			if (targetY > (60 << 4)) targetY = 60 << 4;

			enemies[i].yCoord = targetY;

		}
		else {


			int16_t diveSpeed = 10 + (wave * 1) + (survivalTimer / 300);

			// Has the enemy gone off the bottom of the view or either side? If so, re-enter from a random edge ..
			if (enemies[i].xCoord < (-8 << 4) || enemies[i].yCoord < (-30 << 4) || enemies[i].yCoord >(90 << 4)) {

				Side side = static_cast<Side>(random(0, 3));
				enemies[i].spawnSide = side;

				switch (side) {

				case Side::Right:

					enemies[i].xCoord = ((enemies[i].homeX >> 4) + random(-20, 21)) << 4;
					enemies[i].yCoord = (64 + 24) << 4;
					break;

				case Side::Top:

					enemies[i].xCoord = 144 << 4;
					enemies[i].yCoord = ((enemies[i].homeY >> 4) + random(-6, 7)) << 4;
					break;

				default: // Side::Left re-entry
					enemies[i].xCoord = ((enemies[i].homeX >> 4) + random(-20, 21)) << 4;
					enemies[i].yCoord = -24 << 4;
					break;

				}

				enemies[i].active = true;
				enemies[i].diving = false;
				enemies[i].queued = false;
				enemies[i].arriving = true;
				enemies[i].entryPhase = 0;
				enemies[i].divePhase = 0;
				enemies[i].divePattern = DivePattern::None;

				continue;
			}

			// Remember where the enemy started this frame so it can turn to face the way it moves ..

			int16_t diveStartX = enemies[i].xCoord;
			int16_t diveStartY = enemies[i].yCoord;

			// Force forward dive momentum along xCoord to always move toward the exit (decreasing xCoord)
			enemies[i].divePhase += 1;

			if (enemies[i].divePattern != DivePattern::LoopTheLoop && enemies[i].divePattern != DivePattern::ZigZag) {

				enemies[i].xCoord -= diveSpeed;
				int16_t dy = player.yCoord - enemies[i].yCoord;
				if (dy > 0) enemies[i].yCoord += min((int16_t)4, dy / 4);
				else if (dy < 0) enemies[i].yCoord -= min((int16_t)4, -dy / 4);
			}



			// Different dive patterns based on divePattern value

			switch (enemies[i].divePattern) {

			case DivePattern::GentleSine:
				enemies[i].yCoord += (int16_t)(sin((float)enemies[i].divePhase * 0.06) * 1.5 * enemies[i].diveDirection);
				break;

			case DivePattern::WiderCosine:
				enemies[i].yCoord += (int16_t)(cos((float)enemies[i].divePhase * 0.08f) * 2.0f * enemies[i].diveDirection);
				break;

			case DivePattern::BasicLoop:
				// Looping pattern: combines both sine and cosine with different frequencies to trace a loop/corkscrew
				enemies[i].xCoord += (int16_t)(cos((float)enemies[i].divePhase * 0.12f) * 2.0f); // slight forward/back loop kick
				enemies[i].yCoord += (int16_t)(sin((float)enemies[i].divePhase * 0.12f) * 4.5f * enemies[i].diveDirection);
				break;

			case DivePattern::LoopTheLoop:

				if (enemies[i].diveIdx < 125 && arduboy.isFrameCount(2)) {

					int8_t yStep = pgm_read_byte(&loop_dy[enemies[i].diveIdx]) * enemies[i].diveDirection;
					int8_t xStep = pgm_read_byte(&loop_dx[enemies[i].diveIdx]);// * enemies[i].diveDirection;

					enemies[i].xCoord += xStep << 4;
					enemies[i].yCoord += yStep << 4;
					enemies[i].diveIdx++;

				}

				break;

			case DivePattern::ZigZag:

				if (enemies[i].diveIdx < 85 && arduboy.isFrameCount(2)) {

					int8_t yStep = pgm_read_byte(&zigzag_dy[enemies[i].diveIdx]) * enemies[i].diveDirection;
					int8_t xStep = pgm_read_byte(&zigzag_dx[enemies[i].diveIdx]);// * enemies[i].diveDirection;

					enemies[i].xCoord += xStep << 4;
					enemies[i].yCoord += yStep << 4;
					enemies[i].diveIdx++;

				}

				break;
			}

			// Turn to face the direction of travel (some patterns only move every other frame) ..

			int16_t moveX = enemies[i].xCoord - diveStartX;
			int16_t moveY = enemies[i].yCoord - diveStartY;

			if (moveX != 0 || moveY != 0) {
				turnEnemyTowards(enemies[i], headingFromMovement(moveX, moveY));
			}

			// Firing check during dive
			uint8_t firingThreshold = wave + (survivalTimer / 600);
			int16_t dxPlayer = abs((player.xCoord >> 4) - (enemies[i].xCoord >> 4));

			if (dxPlayer < 40) {
				if (random(0, 150) < firingThreshold) {
					fireEnemyBullet(enemies[i].xCoord, enemies[i].yCoord);
				}
			}

		}

		// Enemies that are not diving turn back to the formation pose ..

		if (!enemies[i].diving) {
			turnEnemyTowards(enemies[i], 0);
		}


		// Player bullet hits an enemy ..

		checkEnemyAndPlayerBulletCollision(i);


		// Has the player and the enemy collided?

		checkEnemyAndPlayerCollision(i);
		renderEnemy_Static(i);


	}


	// Mothership collisions are checked once per frame, outside the enemy loop, so
	// the mothership can still be shot after the last enemy has been destroyed ..

	checkMothershipCollision();


	// The wave is only over once every enemy and the mothership have been destroyed ..

	if (activeEnemies == 0) {
		waveEnemiesCleared = true;
	}

	if (activeEnemies == 0 && !mothership.active) {
		wave++;
		initWave();
	}

	for (uint8_t i = 0; i < Constants::EnemiesMaxBullets; i++) {

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


	// Render elements of screen ..

	renderParticles();
	renderScore(score);
	renderLivesAndHealth();
	renderBullets();
	renderHealthPack();


	for (uint8_t i = 0; i < Constants::EnemiesMax; i++) {
		renderEnemy_Diving(i);
	}

	{
		int16_t msX = (mothership.xCoord) >> 4;
		int16_t msY = mothership.yCoord >> 4;

		if (msX >= -8 && msX < 128 && msY >= -8 && msY < 64) {
			renderMothership(msX, msY);
		}

	}


	// Render the puff frame?

	uint8_t puffFrame = 0;
	bool showPuff = false;
	bool hidePlayerForExplosion = false;

	if (player.deathTimer > 0) {

		uint8_t elapsed = 28 - player.deathTimer; 
		puffFrame = elapsed / 4;                  
	
		if (puffFrame > 6) puffFrame = 6;
		showPuff = true;
	
		if (puffFrame >= 3) {
			hidePlayerForExplosion = true;
		}
	
		player.deathTimer--;
	
	}

	if (!hidePlayerForExplosion) {
		renderPlayer();
	}

	if (showPuff) {
		Sprites::drawSelfMasked(player.puffX - 4, player.puffY - 4, Images::Puff, puffFrame);
	}

	if (player.shieldActive) {
		int16_t px = (player.xCoord >> 4);
		int16_t py = (player.yCoord >> 4);
		Sprites::drawSelfMasked(px - 4, py - 4, Images::Shield, arduboy.getFrameCount(24) / 3);
	}

	if (gameOverDelayTimer > 0) {

		gameOverDelayTimer--;

		if (gameOverDelayTimer == 0) {

			for (uint8_t i = 0; i < Constants::EnemiesMax; i++) {
				enemies[i].active = false;
			}

			if (qualifiesForHighScore(score)) {
				beginInitialsEntry();
			}
			else {
				enteringInitials = false;
			}

			arduboy.invert(false);
			arduboy.frameCount = 0;
			gameState = GameState::GameOver;

		}

	}

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
		[[fallthrough]];

	case GameState::Title_Entrace ... GameState::Title_PressA:
		updateStars();
		titleScreen();
		break;

	case GameState::Playing:
		updateStars();
		updatePlaying();
		break;

	case GameState::GameOver:
		updateStars();
		gameOver();
		break;

	}

	arduboy.display(true);
}

