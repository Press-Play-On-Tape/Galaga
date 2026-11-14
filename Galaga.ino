#include <Arduboy2.h>
#include <Sprites.h>

Arduboy2 arduboy;
BeepPin1 beep;

// --- Custom 5x8 Font for Numbers (Width: 5, Height: 8, 10 frames for digits 0-9) ---
const uint8_t PROGMEM Numbers[] = {
  5, 8,
  0x07, 0x05, 0x05, 0x05, 0x07, 
  0x07, 0x02, 0x02, 0x03, 0x02, 
  0x07, 0x01, 0x07, 0x04, 0x07, 
  0x07, 0x04, 0x06, 0x04, 0x07, 
  0x04, 0x04, 0x07, 0x05, 0x05, 
  0x07, 0x04, 0x07, 0x01, 0x07, 
  0x07, 0x05, 0x07, 0x01, 0x07, 
  0x04, 0x04, 0x04, 0x04, 0x07, 
  0x07, 0x05, 0x07, 0x05, 0x07, 
  0x07, 0x04, 0x07, 0x05, 0x07, 
};

// --- Constants ---
#define MAX_PLAYER_BULLETS 4
#define MAX_ENEMY_BULLETS 4
#define ENEMY_COLS 4
#define ENEMY_ROWS 6
#define MAX_ENEMIES (ENEMY_ROWS * ENEMY_COLS)
#define MAX_PARTICLES 12
#define MAX_STARS 16

// --- Sprites ---
const uint8_t PROGMEM playerSprite[] = {
  7, 7,
  0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10
};

const uint8_t PROGMEM beeSprite[] = {
  6, 6,
  0x14, 0x3F, 0x1C, 0x1C, 0x3F, 0x14
};

const uint8_t PROGMEM butterflySprite[] = {
  6, 6,
  0x22, 0x1C, 0x3F, 0x3F, 0x1C, 0x22
};

const uint8_t PROGMEM bossSprite[] = {
  7, 7,
  0x1C, 0x3E, 0x7F, 0x6B, 0x7F, 0x3E, 0x1C
};

const uint8_t PROGMEM mothershipSprite[] = {
  7, 7,
  0x1C, 0x3E, 0x7F, 0x5D, 0x7F, 0x3E, 0x1C
};

const uint8_t PROGMEM healthPackSprite[] = {
  6, 6,
  0x1C, 0x1C, 0xFF, 0xFF, 0x1C, 0x1C
};

// --- Data Structures ---
struct Bullet {
  int16_t xCoord, yCoord;
  int16_t vx;
  bool active;
};

struct Enemy {
  int16_t xCoord, yCoord;
  int16_t homeX, homeY;
  int16_t targetX, targetY;
  uint8_t type;
  uint8_t hp;
  bool active;
  bool queued;
  bool arriving;
  bool diving;
  bool returning;
  int16_t divePhase;
  uint8_t divePattern; 
  int16_t diveDirection;
};

struct Mothership {
  int16_t xCoord, yCoord;
  bool active;
  bool diving;
  int16_t dir;
  uint8_t hp;
  int16_t phase;
};

struct HealthPack {
  int16_t xCoord, yCoord;
  int16_t vy;
  bool active;
};

struct Particle {
  int16_t xCoord, yCoord;
  int16_t vx, vy;
  uint8_t life;
};

struct Star {
  int16_t xCoord, yCoord, speed;
};

// --- Game State Variables ---
enum GameState : uint8_t { TITLE, PLAYING, GAMEOVER };
GameState gameState = TITLE;

int16_t playerX = 12 << 4;
int16_t playerY = 32 << 4;
int16_t playerVx = 0;
int16_t playerVy = 0;
uint8_t lives = 3;
int16_t playerHealth = 100;
uint16_t score = 0;
uint8_t wave = 1;
uint16_t invulnTimer = 0;
uint8_t screenInvertTimer = 0;
uint16_t dualShooterTimer = 0;
uint16_t survivalTimer = 0;

Bullet pBullets[MAX_PLAYER_BULLETS];
Bullet eBullets[MAX_ENEMY_BULLETS];
Enemy enemies[MAX_ENEMIES];
Mothership mothership;
HealthPack healthPack;
Particle particles[MAX_PARTICLES];
Star stars[MAX_STARS];

int16_t formationOffsetX = 0;
int16_t formationOffsetY = 0;
int16_t formationOffsetY_drop = 0;
int16_t lastSineOffsetY = 0;
int8_t lastFormationDir = 0;
uint16_t diveTimer = 0;
uint16_t mothershipSpawnTimer = 0;
uint16_t healthPackSpawnTimer = 0;
uint8_t arrivalReleaseTimer = 0;
uint16_t replenishTimer = 0;

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
  lastSineOffsetY = 0;
  lastFormationDir = 0;
  mothership.active = false;
  mothershipSpawnTimer = 0;
  healthPack.active = false;
  healthPackSpawnTimer = 0;
  arrivalReleaseTimer = 0;
  replenishTimer = 0;
  uint8_t idx = 0;
  for (uint8_t c = 0; c < ENEMY_COLS; c++) {
    for (uint8_t r = 0; r < ENEMY_ROWS; r++) {
      enemies[idx].homeX = (79 + (c * 8)) << 4;
      enemies[idx].homeY = (9 + (r * 8)) << 4;
      enemies[idx].active = true;
      enemies[idx].diving = false;
      enemies[idx].returning = false;
      enemies[idx].queued = true;
      enemies[idx].arriving = false;

      if (idx % 2 == 0) {
        enemies[idx].xCoord = -32 << 4;
        enemies[idx].yCoord = -32 << 4;
      } else {
        enemies[idx].xCoord = 144 << 4;
        enemies[idx].yCoord = -32 << 4;
      }
      
      if (c >= 2) {
        enemies[idx].type = 2; // Boss / Tough (2 hits)
        enemies[idx].hp = 2;
      } else {
        enemies[idx].type = 0; // Bee / Standard (1 hit)
        enemies[idx].hp = 1;
      }
      idx++;
    }
  }
}

void resetGame() {
  playerX = 12 << 4;
  playerY = 32 << 4;
  playerVx = 0;
  playerVy = 0;
  lives = 3;
  playerHealth = 100;
  score = 0;
  wave = 1;
  formationOffsetX = 0;
  formationOffsetY_drop = 0;
  invulnTimer = 90;
  screenInvertTimer = 0;
  dualShooterTimer = 0;
  survivalTimer = 0;
  for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) pBullets[i].active = false;
  for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) eBullets[i].active = false;
  initWave();
}

void firePlayerBullet() {
  if (dualShooterTimer > 0) {
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS && i + 1 < MAX_PLAYER_BULLETS; i += 2) {
      if (!pBullets[i].active && !pBullets[i+1].active) {
        pBullets[i].xCoord = playerX + (4 << 4);
        pBullets[i].yCoord = playerY + (1 << 4);
        pBullets[i].vx = 64;
        pBullets[i].active = true;

        pBullets[i+1].xCoord = playerX + (4 << 4);
        pBullets[i+1].yCoord = playerY + (5 << 4);
        pBullets[i+1].vx = 64;
        pBullets[i+1].active = true;

        beep.tone(beep.freq(880), 2);
        break;
      }
    }
  } else {
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
      if (!pBullets[i].active) {
        pBullets[i].xCoord = playerX + (4 << 4);
        pBullets[i].yCoord = playerY + (3 << 4);
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
      if (enemies[idx].active && !enemies[idx].diving && !enemies[idx].arriving && !enemies[idx].queued) {
        bool rowOccupied = false;
        for (uint8_t d = 0; d < MAX_ENEMIES; d++) {
          if (enemies[d].diving && abs(enemies[d].homeY - enemies[idx].homeY) < (8 << 4)) {
            rowOccupied = true;
            break;
          }
        }
        if (!rowOccupied) {
          target = idx;
          break;
        }
      }
    }

    if (target != -1) {
      enemies[target].diving = true;
      enemies[target].returning = false;
      enemies[target].divePhase = 0;
      enemies[target].divePattern = pattern;
      enemies[target].diveDirection = (random(0, 2) == 0) ? 1 : -1;
    }
  }

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
    Sprites::drawSelfMasked(121, startY, Numbers, digits[i]);
    startY -= 4;
  }
}

void drawLivesAndHealth() {
  uint8_t displayLives = (lives > 9) ? 9 : lives;
  Sprites::drawSelfMasked(121, 2, Numbers, displayLives);

  arduboy.drawRect(121, 12, 4, 20, WHITE);
  uint8_t fillHeight = (playerHealth * 18) / 100;
  if (fillHeight > 18) fillHeight = 18;
  if (fillHeight > 0) {
    arduboy.fillRect(122, 13 + 18 - fillHeight, 2, fillHeight, WHITE);
  }
}

void setup() {
  arduboy.begin();
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
  } else {
    arduboy.invert(false);
  }

  // Player Controls
  if (arduboy.pressed(UP_BUTTON)) playerVy -= 6;
  if (arduboy.pressed(DOWN_BUTTON)) playerVy += 6;
  
  playerVy = (playerVy * 13) >> 4;
  playerY += playerVy;

  if (playerY < (2 << 4)) { playerY = 2 << 4; playerVy = 0; }
  if (playerY > (54 << 4)) { playerY = 54 << 4; playerVy = 0; }

  if (arduboy.pressed(LEFT_BUTTON)) playerVx -= 6;
  if (arduboy.pressed(RIGHT_BUTTON)) playerVx += 6;
  
  playerVx = (playerVx * 13) >> 4;
  playerX += playerVx;

  if (playerX < (4 << 4)) { playerX = 4 << 4; playerVx = 0; }
  if (playerX > (35 << 4)) { playerX = 35 << 4; playerVx = 0; }

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
    healthPackSpawnTimer++;
    if (healthPackSpawnTimer > 500) {
      healthPackSpawnTimer = 0;
      healthPack.active = true;
      healthPack.xCoord = random(20, 100) << 4;
      if (random(0, 2) == 0) {
        healthPack.yCoord = -8 << 4;
        healthPack.vy = random(4, 11);
      } else {
        healthPack.yCoord = 64 << 4;
        healthPack.vy = random(-11, -3);
      }
    }
  } else {
    healthPack.yCoord += healthPack.vy;
    int16_t hpX = healthPack.xCoord >> 4;
    int16_t hpY = healthPack.yCoord >> 4;

    if (hpY < -10 || hpY > 70) {
      healthPack.active = false;
    } else {
      Sprites::drawSelfMasked(hpX, hpY, healthPackSprite, 0);

      for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++) {
        if (pBullets[b].active &&
            pBullets[b].xCoord >= healthPack.xCoord && pBullets[b].xCoord <= healthPack.xCoord + (6 << 4) &&
            pBullets[b].yCoord >= healthPack.yCoord && pBullets[b].yCoord <= healthPack.yCoord + (6 << 4)) {
          pBullets[b].active = false;
          healthPack.active = false;
          int16_t healAmount = random(25, 51);
          playerHealth += healAmount;
          if (playerHealth > 100) playerHealth = 100;
          spawnParticles(hpX, hpY, 6);
          beep.tone(beep.freq(1046), 6);
          break;
        }
      }

      if (healthPack.active &&
          healthPack.xCoord + (6 << 4) > playerX && healthPack.xCoord < playerX + (7 << 4) &&
          healthPack.yCoord + (6 << 4) > playerY && healthPack.yCoord < playerY + (7 << 4)) {
        healthPack.active = false;
        int16_t healAmount = random(25, 51);
        playerHealth += healAmount;
        if (playerHealth > 100) playerHealth = 100;
        spawnParticles(playerX >> 4, playerY >> 4, 6);
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
        enemies[i].targetX = enemies[i].homeX + formationOffsetX;
        enemies[i].targetY = enemies[i].homeY;
        released++;
      }
    }
  }

  replenishTimer++;
  if (replenishTimer > 250) {
  Serial.println("A");
    replenishTimer = 0;
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
      if (!enemies[i].active) {
        enemies[i].active = true;
        enemies[i].hp = (enemies[i].type == 2) ? 2 : 1;
        enemies[i].queued = true;
        enemies[i].arriving = false;
        if (i % 2 == 0) {
          enemies[i].xCoord = -32 << 4;
          enemies[i].yCoord = -32 << 4;
        } else {
          enemies[i].xCoord = 144 << 4;
          enemies[i].yCoord = -32 << 4;
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
  float dynamicAmplitude = 6.0f + (deadColumns * 2.5f);

  int16_t nextSineOffsetY = (int16_t)(sin((float)arduboy.frameCount * 0.04) * dynamicAmplitude * 16.0);
  int8_t currentFormationDir = (nextSineOffsetY > lastSineOffsetY) ? 1 : ((nextSineOffsetY < lastSineOffsetY) ? -1 : lastFormationDir);

  if (currentFormationDir != lastFormationDir && lastFormationDir != 0) {
    formationOffsetY_drop += 8;
    if (formationOffsetY_drop > (16 << 4)) formationOffsetY_drop = (16 << 4);
    
    formationOffsetX -= 16;
    if (formationOffsetX < -2000) formationOffsetX = -2000;
  }
  lastFormationDir = currentFormationDir;
  lastSineOffsetY = nextSineOffsetY;
  formationOffsetY = nextSineOffsetY + formationOffsetY_drop;

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
  } else if (mothership.active) {
    if (!mothership.diving) {
      mothership.yCoord += mothership.dir * 8;
      if (mothership.yCoord < (4 << 4) || mothership.yCoord > (54 << 4)) {
        mothership.dir *= -1;
      }
      if (random(0, 250) == 0) {
        mothership.diving = true;
        mothership.phase = 0;
      }
    } else {
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
      Sprites::drawSelfMasked(msX, msY, mothershipSprite, 0);
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
      int16_t targetXPos = enemies[i].homeX + formationOffsetX;
      int16_t dx = targetXPos - enemies[i].xCoord;
      int16_t dy = enemies[i].homeY - enemies[i].yCoord;
      int32_t distSq = (int32_t)dx * dx + (int32_t)dy * dy;
      
      if (distSq < (48 * 48)) {
        enemies[i].xCoord = targetXPos;
        enemies[i].yCoord = enemies[i].homeY;
        enemies[i].arriving = false;
      } else {
        int16_t speed = 48;
		if (i ==0 || i == 5) {
			Serial.print("(A) enemies[");
			Serial.print(i);
			Serial.print("], yCoord ");
			Serial.print(enemies[i].yCoord);
		}
        if (abs(dx) > abs(dy)) {
          enemies[i].xCoord += (dx > 0) ? speed : -speed;
          enemies[i].yCoord += (int32_t)dy * speed / abs(dx);
        } else {
          enemies[i].yCoord += (dy > 0) ? speed : -speed;
          enemies[i].xCoord += (int32_t)dx * speed / abs(dy);
        }
		if (i ==0 || i == 5) {
			Serial.print(" after");
			Serial.print(" yCoord ");
			Serial.println(enemies[i].yCoord);
		}
      }
    } 
    else if (!enemies[i].diving) {
      enemies[i].xCoord = enemies[i].homeX + formationOffsetX;
      int16_t targetY = enemies[i].homeY + formationOffsetY;
      if (targetY < (2 << 4)) targetY = 2 << 4;
      if (targetY > (46 << 4)) targetY = 46 << 4;
	  if (i ==0 || i == 5) {
			Serial.print("(B) enemies[");
			Serial.print(i);
			Serial.print("], yCoord ");
			Serial.print(enemies[i].yCoord);
		}
        enemies[i].yCoord = targetY;
		if (i ==0 || i == 5) {
			Serial.print(" after");
			Serial.print(" yCoord ");
			Serial.println(enemies[i].yCoord);
		}

      
    } 
    else {
      if (!enemies[i].returning) {
        enemies[i].divePhase += 1;
        enemies[i].xCoord -= 10 + (wave * 1) + (survivalTimer / 300);

        if (enemies[i].divePattern == 0) {
          enemies[i].yCoord += (int16_t)(sin((float)enemies[i].divePhase * 0.06) * 16.0 * enemies[i].diveDirection);
        } else if (enemies[i].divePattern == 1) {
          enemies[i].yCoord += (int16_t)(cos((float)enemies[i].divePhase * 0.08) * 20.0);
        } else {
          int16_t targetDir = (playerY > enemies[i].yCoord) ? 1 : -1;
          enemies[i].yCoord += targetDir * 10;
        }

        if (enemies[i].yCoord < (2 << 4)) enemies[i].yCoord = 2 << 4;
        if (enemies[i].yCoord > (54 << 4)) enemies[i].yCoord = 54 << 4;

        uint8_t firingThreshold = wave + (survivalTimer / 600);
        if (random(0, 150) < firingThreshold) {
          fireEnemyBullet(enemies[i].xCoord, enemies[i].yCoord);
        }

        if (enemies[i].xCoord < (-16 << 4)) {
          enemies[i].xCoord = 120 << 4;
          enemies[i].yCoord = enemies[i].homeY + formationOffsetY;
          enemies[i].returning = true;
        }
      } else {
        enemies[i].xCoord -= 16;
        int16_t targetY = enemies[i].homeY + formationOffsetY;
        if (enemies[i].yCoord < targetY) enemies[i].yCoord += 8;
        else if (enemies[i].yCoord > targetY) enemies[i].yCoord -= 8;

        int16_t targetX = enemies[i].homeX + formationOffsetX;
        if (enemies[i].xCoord <= targetX) {
          enemies[i].xCoord = targetX;
          enemies[i].diving = false;
          enemies[i].returning = false;
        }
      }
    }

    uint8_t enemySz = (enemies[i].type == 2) ? 7 : 6;
    for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++) {
      if (pBullets[b].active &&
          pBullets[b].xCoord >= enemies[i].xCoord && pBullets[b].xCoord <= enemies[i].xCoord + (enemySz << 4) &&
          pBullets[b].yCoord >= enemies[i].yCoord && pBullets[b].yCoord <= enemies[i].yCoord + (enemySz << 4)) {
        pBullets[b].active = false;
        enemies[i].hp--;
        if (enemies[i].hp == 0) {
          enemies[i].active = false;
          spawnParticles(enemies[i].xCoord >> 4, enemies[i].yCoord >> 4, 4);
          score += (enemies[i].type + 1) * 25 * (enemies[i].diving ? 2 : 1);
          beep.tone(beep.freq(160), 4);
        } else {
          beep.tone(beep.freq(500), 2);
        }
        break;
      }
    }

    if (mothership.active) {
      for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++) {
        if (pBullets[b].active &&
            pBullets[b].xCoord >= mothership.xCoord + formationOffsetX && pBullets[b].xCoord <= mothership.xCoord + formationOffsetX + (7 << 4) &&
            pBullets[b].yCoord >= mothership.yCoord && pBullets[b].yCoord <= mothership.yCoord + (7 << 4)) {
          pBullets[b].active = false;
          mothership.hp--;
          if (mothership.hp == 0) {
            mothership.active = false;
            spawnParticles((mothership.xCoord + formationOffsetX) >> 4, mothership.yCoord >> 4, 8);
            score += 200;
            if (mothership.diving) dualShooterTimer = 600;
            beep.tone(beep.freq(700), 8);
          } else {
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
        if (enemies[other].active && !enemies[other].diving && !enemies[other].arriving && !enemies[other].queued && 
            abs(enemies[other].homeY - enemies[i].homeY) < (8 << 4)) {
          if (enemies[other].homeX < enemies[i].homeX) {
            isLeftmostInRow = false;
            break;
          }
        }
      }
      if (isLeftmostInRow) canCollide = true;
    }

    if (enemies[i].active && !enemies[i].arriving && invulnTimer == 0 && canCollide &&
        enemies[i].xCoord + (enemySz << 4) > playerX && enemies[i].xCoord < playerX + (7 << 4) &&
        enemies[i].yCoord + (enemySz << 4) > playerY && enemies[i].yCoord < playerY + (7 << 4)) {
      enemies[i].active = false;
      spawnParticles(playerX >> 4, playerY >> 4, 8);
      playerHealth -= 34;
      screenInvertTimer = 10;
      beep.tone(beep.freq(80), 10);
      if (playerHealth <= 0) {
        lives--;
        spawnParticles(playerX >> 4, playerY >> 4, 20);
        beep.tone(beep.freq(50), 25);
        if (lives == 0) {
          gameState = GAMEOVER;
        } else {
          playerHealth = 100;
          playerX = 12 << 4;
          playerY = 32 << 4;
          invulnTimer = 90;
        }
      } else {
        invulnTimer = 80;
      }
    }

    if (enemies[i].active) {
      const uint8_t* sprite = beeSprite;
      if (enemies[i].type == 1) sprite = butterflySprite;
      else if (enemies[i].type == 2) sprite = bossSprite;
      
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
      } else {
        arduboy.drawFastHLine(eBullets[i].xCoord >> 4, eBullets[i].yCoord >> 4, 3, WHITE);
        if (invulnTimer == 0 &&
            eBullets[i].xCoord >= playerX && eBullets[i].xCoord <= playerX + (7 << 4) &&
            eBullets[i].yCoord >= playerY && eBullets[i].yCoord <= playerY + (7 << 4)) {
          eBullets[i].active = false;
          spawnParticles(playerX >> 4, playerY >> 4, 8);
          playerHealth -= 34;
          screenInvertTimer = 10;
          beep.tone(beep.freq(80), 10);
          if (playerHealth <= 0) {
            lives--;
            spawnParticles(playerX >> 4, playerY >> 4, 20);
            beep.tone(beep.freq(50), 25);
            if (lives == 0) {
              gameState = GAMEOVER;
            } else {
              playerHealth = 100;
              playerX = 12 << 4;
              playerY = 32 << 4;
              invulnTimer = 90;
            }
          } else {
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

  if (invulnTimer == 0 || (invulnTimer % 6 < 3)) {
    int16_t pX = playerX >> 4;
    int16_t pY = playerY >> 4;
    if (pX >= -8 && pX < 128 && pY >= -8 && pY < 64) {
      if (dualShooterTimer > 0) {
        Sprites::drawSelfMasked(pX, pY - 3, playerSprite, 0);
        Sprites::drawSelfMasked(pX, pY + 3, playerSprite, 0);
      } else {
        Sprites::drawSelfMasked(pX, pY, playerSprite, 0);
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
  arduboy.clear();

  updateStars();

  if (gameState == TITLE) {
    arduboy.invert(false);
    arduboy.setCursor(44, 25);
    arduboy.print(F("GALAGA"));
    arduboy.setCursor(34, 45);
    arduboy.print(F("PRESS A/B"));
    if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
      resetGame();
      gameState = PLAYING;
    }
  } else if (gameState == PLAYING) {
    updatePlaying();
  } else if (gameState == GAMEOVER) {
    arduboy.invert(false);
    arduboy.setCursor(44, 25);
    arduboy.print(F("OVER"));
    arduboy.setCursor(40, 45);
    drawVerticalScore(score);
    if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
      resetGame();
      gameState = PLAYING;
    }
  }

  arduboy.display();
}