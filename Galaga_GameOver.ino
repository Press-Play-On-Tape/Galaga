#pragma once

#include <Sprites.h>

void resetGameOverDiver(Enemy &e, uint8_t idx) {

	e.active = true;
	e.queued = false;
	e.arriving = false;
	e.diving = true;
	e.divePhase = 0;
	e.diveIdx = 0;
	e.diveDirection = (random(0, 2) == 0) ? 1 : -1;
	e.type = static_cast<EnemyType>(idx);

	DivePattern pattern = static_cast<DivePattern>(random(0, static_cast<uint8_t>(DivePattern::Count)));
	e.divePattern = pattern;

	e.xCoord = 128 << 4;
	e.yCoord = random(16, 48) << 4;

}

void beginInitialsEntry() {
	enteringInitials = true;
	initialCursor = 0;
	currentInitials[0] = 'A';
	currentInitials[1] = 'A';
	currentInitials[2] = 'A';
}

void updateAndRenderInitialsEntry() {

	if (arduboy.justPressed(RIGHT_BUTTON)) {
		currentInitials[initialCursor]++;
		if (currentInitials[initialCursor] > 'Z') currentInitials[initialCursor] = 'A';
		beep.tone(beep.freq(660), 2);
	}

	if (arduboy.justPressed(LEFT_BUTTON)) {
		currentInitials[initialCursor]--;
		if (currentInitials[initialCursor] < 'A') currentInitials[initialCursor] = 'Z';
		beep.tone(beep.freq(520), 2);
	}

	if (arduboy.justPressed(DOWN_BUTTON)) {
		initialCursor = (initialCursor + 1) % 3;
	}

	if (arduboy.justPressed(UP_BUTTON)) {
		initialCursor = (initialCursor + 2) % 3;
	}

	if (arduboy.justPressed(A_BUTTON)) {
		if (initialCursor < 2) {
			initialCursor++;
			beep.tone(beep.freq(880), 3);
		}
		else {
			insertHighScore(score, currentInitials);
			enteringInitials = false;
			beep.tone(beep.freq(1046), 8);
		}
	}
	else if (arduboy.justPressed(B_BUTTON)) {
		insertHighScore(score, currentInitials);
		enteringInitials = false;
		beep.tone(beep.freq(1046), 8);
	}

	// Draw 3 rotated initials stacked vertically (adjust x/y spacing to fit your layout)
	const int16_t startX = 26;
	const int16_t startY = 24;
	const int16_t stepY = 6;

	Sprites::drawOverwrite(40, 12, Images::HighScore, 0);

	for (uint8_t i = 0; i < 3; i++) {
		int16_t y = startY + (i * stepY);
		drawRotatedChar3x5(startX, y, currentInitials[i]);

		if (i == initialCursor) {
			arduboy.drawFastVLine(startX - 1, y, 3, WHITE);
		}
	}
}

void updateAndRenderGameOverDivers() {

	if (goDiversInitialized < Constants::GameOverDiverCount) {

		if (arduboy.isFrameCount(64)) {
			resetGameOverDiver(enemies[goDiversInitialized], goDiversInitialized);
			goDiversInitialized++;
		}

	}

	for (uint8_t i = 0; i < Constants::GameOverDiverCount; i++) {

		Enemy &d = enemies[i];
		if (!d.active) continue;

		d.divePhase++;
		d.xCoord -= 4;

		switch (d.divePattern) {

			case DivePattern::GentleSine:
				d.yCoord += (int16_t)(sin((float)d.divePhase * 0.06f) * 1.2f * d.diveDirection);
				break;

			case DivePattern::WiderCosine:
				d.xCoord -= (int16_t)(fabs(cos((float)d.divePhase * 0.08f)) * 1.5f);
				break;

			case DivePattern::BasicLoop:
				d.xCoord += (int16_t)(cos((float)d.divePhase * 0.12f) * 1.5f);
				d.yCoord += (int16_t)(sin((float)d.divePhase * 0.12f) * 2.2f * d.diveDirection);
				break;

			case DivePattern::LoopTheLoop:

				if (d.diveIdx < 125 && arduboy.isFrameCount(2)) {
					int8_t yStep = pgm_read_byte(&loop_dy[d.diveIdx]) * d.diveDirection;
					int8_t xStep = pgm_read_byte(&loop_dx[d.diveIdx]);
					d.xCoord -= (abs(xStep) >> 1) << 4;
					d.yCoord += (yStep >> 1) << 4;
					d.diveIdx++;
				}

				break;

			case DivePattern::ZigZag:

				if (d.diveIdx < 85 && arduboy.isFrameCount(2)) {
					int8_t yStep = pgm_read_byte(&zigzag_dy[d.diveIdx]) * d.diveDirection;
					int8_t xStep = pgm_read_byte(&zigzag_dx[d.diveIdx]);
					d.xCoord -= (abs(xStep) >> 1) << 4;
					d.yCoord += (yStep >> 1) << 4;
					d.diveIdx++;
				}
				
				break;

			default:
				break;

		}

		if (d.yCoord > (52 << 4) || d.yCoord < (-18 << 4)) {

			d.diveDirection = d.diveDirection == 0 ? 1 : 0;

		}

		int16_t sx = d.xCoord >> 4;
		int16_t sy = d.yCoord >> 4;

		// Respawn once off the screen ..

		if (sx < -24 || sx > 132 || sy < -24) {
			resetGameOverDiver(d, i);
			continue;
		}


		// Render enemies ..

		uint8_t frame = arduboy.getFrameCount(16) / 8;

		switch (d.type) {

			case EnemyType::Boss:
				Sprites::drawExternalMask(sx, sy, Images::Large_Boss, Images::Large_Boss_Mask, frame, frame);
				break;

			case EnemyType::Butterfly:
				Sprites::drawExternalMask(sx, sy, Images::Large_Butterfly, Images::Large_Butterfly_Mask, frame, frame);
				break;

			default:
				Sprites::drawExternalMask(sx, sy, Images::Large_Bee, Images::Large_Bee_Mask, frame, frame);
				break;

		}
	
	}

}

void gameOver() {

	arduboy.invert(false);
	Sprites::drawOverwrite(121, 15, Images::Score, 0);
	renderScore(score);
	Sprites::drawExternalMask(67, 0, Images::GameOver, Images::GameOver_Mask, 0, 0);

	if (!enteringInitials && arduboy.frameCount > 48) Sprites::drawOverwrite(54, 19, Images::PressA, 0);

	if (enteringInitials) {

		updateAndRenderInitialsEntry();
		return;

	}
	else {

		renderHighScores();

	}

	updateAndRenderGameOverDivers();

	if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
		resetGame();
		gameState = GameState::Title_Init;
	}

}