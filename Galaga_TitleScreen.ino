#include <Sprites.h>

#include "src/Images.h"
#include "src/utils/Arduboy2Ext.h"

int8_t boss_X = 20;
int8_t bees_X[] = { 1, 21, 41 };
int8_t butterfly_X[] = { 11, 31 };

int8_t coords[] = { 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 
29, 28, 27, 26, 25,24 ,23 , 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 12, 11, 11, 11, 10, 10, 10, 10, 10, 11, 11, 12, 11, 11, 10, 10,  };
uint8_t phase = 0;
uint8_t counter = 0;


void titleScreen() {

	arduboy.invert(false);

	switch (phase) {
	
		case 0:
Serial.println(counter);
		Sprites::drawOverwrite(110 + coords[counter], boss_X, Images::Large_Boss, arduboy.frameCount % 96 < 48);
		for (uint8_t i = 0; i < 2; i++) {
			Sprites::drawOverwrite(94 + coords[counter], butterfly_X[i], Images::Large_Butterfly, arduboy.frameCount % 96 < 48);
		}

		for (uint8_t i = 0; i < 3; i++) {
			Sprites::drawOverwrite(78 + coords[counter], bees_X[i], Images::Large_Bee, arduboy.frameCount % 96 < 48);
		}

		counter++;
		if (counter == 30) {
		phase = 1;
		}
		break;

		case 1:
	Sprites::drawOverwrite(110, boss_X, Images::Large_Boss, arduboy.frameCount % 96 < 48);
		for (uint8_t i = 0; i < 2; i++) {
			Sprites::drawOverwrite(94, butterfly_X[i], Images::Large_Butterfly, arduboy.frameCount % 96 < 48);
		}

		for (uint8_t i = 0; i < 3; i++) {
			Sprites::drawOverwrite(78, bees_X[i], Images::Large_Bee, arduboy.frameCount % 96 < 48);
		}
		break;

	}

	Sprites::drawOverwrite(33, 0, Images::Galaga_Title, 0);
	Sprites::drawOverwrite(24, 8, Images::LargePlayer, arduboy.frameCount % 2);

	if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
		resetGame();
		gameState = GameState::PLAYING;
	} 

}