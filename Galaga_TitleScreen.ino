#include <Sprites.h>

#include "src/Images.h"
#include "src/utils/Arduboy2Ext.h"
#define DEBUG_BREAK    asm volatile("break\n");

uint8_t counter = 0;
uint16_t titleDecisionTimer = 0;
int16_t titleDesiredY = 32 << 4;

void titleScreen_Init() {

	// Mothership

	enemies[0].yCoord = 24 << 4;
	enemies[0].xCoord = 110 << 4;


	// Butterflies ..

	enemies[1].yCoord = 16 << 4;
	enemies[1].xCoord = 96 << 4;

	enemies[2].yCoord = 34 << 4;
	enemies[2].xCoord = 96 << 4;


	// Bees ..

	enemies[3].yCoord = 7 << 4;
	enemies[3].xCoord = 81 << 4;

	enemies[4].yCoord = 25 << 4;
	enemies[4].xCoord = 81 << 4;

	enemies[5].yCoord = 43 << 4;
	enemies[5].xCoord = 81 << 4;


	// Player ..

	player.yCoord = 24 << 4;
	player.xCoord = -18 << 4;

	for (uint8_t i = 0; i < 6; i++) {
		enemies[i].active = true;
		enemies[i].hp = 0;
	}

	gameState = GameState::Title_Entrace;
	arduboy.invert(false);

}

void titleScreen() {

	switch (gameState) {

		case GameState::Title_Entrace:

			Sprites::drawOverwrite(((enemies[0].xCoord + (Constants::Title_Coords[counter] << 4)) >> 4) - 1, (enemies[0].yCoord >> 4) - 1, Images::Large_Boss, arduboy.getFrameCountHalf(96));

			for (uint8_t i = 0; i < 2; i++) {
				Sprites::drawOverwrite(((enemies[i + 1].xCoord + (Constants::Title_Coords[counter] << 4)) >> 4) - 1, (enemies[i + 1].yCoord >> 4) - 1, Images::Large_Butterfly, arduboy.getFrameCountHalf(96));
			}

			for (uint8_t i = 0; i < 3; i++) {
				Sprites::drawOverwrite(((enemies[i + 3].xCoord + (Constants::Title_Coords[counter] << 4)) >> 4) - 1, (enemies[i + 3].yCoord >> 4) - 1, Images::Large_Bee, arduboy.getFrameCountHalf(96));
			}

			Sprites::drawOverwrite(player.xCoord >> 4, player.yCoord >> 4, Images::LargePlayer, arduboy.isFrameCount(2));

			counter++;

			if (arduboy.isFrameCount(2) == 0 && player.xCoord < 0) {
				player.xCoord += (1 << 4);
			}

			if (counter == 51) {

				gameState = GameState::Title_Shooting;
				counter = 0;
				arduboy.frameCount = 0;

			}

			break;

		case GameState::Title_Shooting:
			{
				uint8_t deadCnt = 0;

				titleDecisionTimer++;

				if (titleDecisionTimer > random(40, 120)) {

					titleDecisionTimer = 0;
					titleDesiredY = random(10, 54) << 4;

				}

				int16_t delta = titleDesiredY - player.yCoord;
				player.yCoord += delta / 16;
				player.yCoord += (int16_t)(sin((float)arduboy.frameCount * 0.11f) * (1 << 4));

				int8_t offset = (int8_t)(sin((float)arduboy.frameCount * 0.025f) * 8.0f) << 4;

				if (enemies[0].active && enemies[0].hp < 3) {
					Sprites::drawOverwrite(enemies[0].xCoord >> 4, (enemies[0].yCoord + offset) >> 4, Images::Large_Boss, arduboy.getFrameCountHalf(96));

				}

				for (uint8_t i = 0; i < 2; i++) {
					if (enemies[i + 1].active && enemies[i + 1].hp < 3) {
						Sprites::drawOverwrite(enemies[i + 1].xCoord >> 4, (enemies[i + 1].yCoord + offset) >> 4, Images::Large_Butterfly, arduboy.getFrameCountHalf(96));
					}
				}

				for (uint8_t i = 0; i < 3; i++) {
					if (enemies[i + 3].active && enemies[i + 3].hp < 3) {
						Sprites::drawOverwrite(enemies[i + 3].xCoord >> 4, (enemies[i + 3].yCoord + offset) >> 4, Images::Large_Bee, arduboy.getFrameCountHalf(96));
					}
				}

				if (enemies[0].hp > 0 && enemies[0].hp < 8) {
					Sprites::drawExternalMask((enemies[0].xCoord >> 4) - 12, ((enemies[0].yCoord + offset) >> 4) - 12, Images::Explosion, Images::Explosion_Mask, enemies[0].hp, enemies[0].hp);
				}

				for (uint8_t i = 0; i < 2; i++) {
					if (enemies[i + 1].hp > 0 && enemies[i + 1].hp < 8) {
						Sprites::drawExternalMask((enemies[i + 1].xCoord >> 4) - 14, ((enemies[i + 1].yCoord + offset) >> 4) - 14, Images::Explosion, Images::Explosion_Mask, enemies[i + 1].hp, enemies[i + 1].hp);

					}
				}

				for (uint8_t i = 0; i < 3; i++) {
					if (enemies[i + 3].hp > 0 && enemies[i + 3].hp < 8) {
						Sprites::drawExternalMask((enemies[i + 3].xCoord >> 4) - 14, ((enemies[i + 3].yCoord + offset) >> 4) - 14, Images::Explosion, Images::Explosion_Mask, enemies[i + 3].hp, enemies[i + 3].hp);
					}
				}

				Sprites::drawOverwrite(player.xCoord >> 4, player.yCoord >> 4, Images::LargePlayer, arduboy.isFrameCount(2));


				// Check for dead and dying enemies ..

				bool exploding = false;

				for (uint8_t j = 0; j < 6; j++) {

					if (enemies[j].hp == 8) {
						deadCnt++;
						continue;
					}

					if (enemies[j].hp == 0) {
						continue;
					}

					exploding = true;

					if (arduboy.isFrameCount(2) == 0) {
						enemies[j].hp++;
					}

				}


				// If there are only two or less enemies, make sure the player moves towards them .. (otherwise the title screen can go on forever).

				uint8_t aliveCnt = 6 - deadCnt;

				if (aliveCnt <= 2 && aliveCnt > 0) {

					int16_t targetY = 0;

					for (uint8_t i = 0; i < 6; i++) {
						if (enemies[i].hp == 0) {   // still alive
							targetY += enemies[i].yCoord + offset;
						}
					}

					titleDesiredY = targetY / aliveCnt;
					titleDesiredY += (random(-4, 5) << 4);

				}
				else {

					if (titleDecisionTimer > random(40, 120)) {
						titleDecisionTimer = 0;
						titleDesiredY = random(10, 54) << 4;
					}

				}

				if (!exploding && arduboy.isFrameCount(32)) {
					firePlayerBullet(12);
				}


				// Render bullets ..

				for (uint8_t i = 0; i < Constants::PlayerMaxBullets; i++) {

					if (!pBullets[i].active) continue;

					pBullets[i].xCoord += pBullets[i].vx;

					if (pBullets[i].xCoord > (120 << 4)) {
						pBullets[i].active = false;
						continue;
					}

					arduboy.drawFastHLine(pBullets[i].xCoord >> 4, pBullets[i].yCoord >> 4, 3, WHITE);
				}

				for (uint8_t b = 0; b < Constants::PlayerMaxBullets; b++) {

					if (!pBullets[b].active) continue;

					for (uint8_t e = 0; e < 6; e++) {

						if (enemies[e].hp != 0) continue;

						int16_t ex = enemies[e].xCoord >> 4;
						int16_t ey = (enemies[e].yCoord + offset) >> 4;
						int16_t bx = pBullets[b].xCoord >> 4;
						int16_t by = pBullets[b].yCoord >> 4;

						uint8_t enemySize = (e == 0) ? 16 : 11;

						if (bx >= ex && bx <= ex + enemySize && by >= ey && by <= ey + enemySize) {

							pBullets[b].active = false;
							enemies[e].hp = 1;
							break;

						}

					}

				}

				if (deadCnt == 6) {
					gameState = GameState::Title_Pause;
					counter = 0;
				}

			}

			break;

		case GameState::Title_Pause:
			{
				counter++;

				if (counter > 32) {
					gameState = GameState::Title_Withdraw;
					counter = 0;
				}

				int16_t delta = titleDesiredY - player.yCoord;
				player.yCoord += delta / 16;
				player.yCoord += (int16_t)(sin((float)arduboy.frameCount * 0.11f) * (1 << 4));
				Sprites::drawOverwrite(player.xCoord >> 4, player.yCoord >> 4, Images::LargePlayer, arduboy.isFrameCount(2));

			}
			break;

		case GameState::Title_Withdraw:

			if (counter < 31) {

				counter++;

				if (arduboy.isFrameCount(2)) {
					player.xCoord -= (1 << 4);
				}

			}
			else if (counter == 31) {

				player.xCoord -= (2 << 4);
				gameState = GameState::Title_PressA;

			}

			Sprites::drawOverwrite(66 + Constants::Title_Coords[counter], 0, Images::Galaga_Title, 0);
			Sprites::drawOverwrite(player.xCoord >> 4, player.yCoord >> 4, Images::LargePlayer, arduboy.isFrameCount(2));

			if (gameState == GameState::Title_PressA) {
				counter = 0;
			}

			break;

		case GameState::Title_PressA:

			if (counter < 31) {
				counter++;				
			}	
			else {
				Sprites::drawOverwrite(55, 20, Images::PressA, 0);
				renderHighScores();
			}

			Sprites::drawOverwrite(66, 0, Images::Galaga_Title, 0);
			break;

	}

	// Sprites::drawOverwrite(33, 0, Images::Galaga_Title, 0);

	if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
		resetGame();
		gameState = GameState::Playing;
	}

}