#include <Arduboy2.h>
#include <Sprites.h>

Arduboy2 arduboy;
BeepPin1 beep;

// --- Custom Number Bitmaps (PROGMEM) for Score Display ---
const uint8_t PROGMEM num0[] = { 3, 5, 0x0F, 0x09, 0x09, 0x09, 0x0F };
const uint8_t PROGMEM num1[] = { 3, 5, 0x04, 0x06, 0x04, 0x04, 0x0E };
const uint8_t PROGMEM num2[] = { 3, 5, 0x0F, 0x01, 0x0F, 0x08, 0x0F };
const uint8_t PROGMEM num3[] = { 3, 5, 0x0F, 0x01, 0x07, 0x01, 0x0F };
const uint8_t PROGMEM num4[] = { 3, 5, 0x09, 0x09, 0x0F, 0x01, 0x01 };
const uint8_t PROGMEM num5[] = { 3, 5, 0x0F, 0x08, 0x0F, 0x01, 0x0F };
const uint8_t PROGMEM num6[] = { 3, 5, 0x0F, 0x08, 0x0F, 0x09, 0x0F };
const uint8_t PROGMEM num7[] = { 3, 5, 0x0F, 0x01, 0x02, 0x04, 0x04 };
const uint8_t PROGMEM num8[] = { 3, 5, 0x0F, 0x09, 0x0F, 0x09, 0x0F };
const uint8_t PROGMEM num9[] = { 3, 5, 0x0F, 0x09, 0x0F, 0x01, 0x0F };

const uint8_t* const numberTable[] PROGMEM = {
  num0, num1, num2, num3, num4, num5, num6, num7, num8, num9
};

// --- Vertical Display Wrapper ---
class VerticalDisplay {
public:
  void drawPixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= 64 || y < 0 || y >= 128) return;
    int physX = 127 - y;
    int physY = x;
    arduboy.drawPixel(physX, physY, color);
  }

  void drawBitmap(int x, int y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t color) {
    for (uint8_t j = 0; j < h; j++) {
      for (uint8_t i = 0; i < w; i++) {
        uint8_t byteVal = pgm_read_byte(bitmap + (j / 8) * w + i);
        if (byteVal & (1 << (j % 8))) {
          drawPixel(x + i, y + j, color);
        }
      }
    }
  }

  void drawFastVLine(int x, int y, int h, uint8_t color) {
    for (int i = 0; i < h; i++) {
      drawPixel(x, y + i, color);
    }
  }
};

VerticalDisplay vdisplay;

// --- Constants ---
#define MAX_PLAYER_BULLETS 4
#define MAX_ENEMY_BULLETS 6
#define ENEMY_ROWS 4
#define ENEMY_COLS 7
#define MAX_ENEMIES (ENEMY_ROWS * ENEMY_COLS)
#define MAX_PARTICLES 12
#define MAX_STARS 8

// --- Sprites ---
const uint8_t PROGMEM playerSprite[] = {
  0x04, 0x0E, 0x1F, 0x1F, 0x0A
};

const uint8_t PROGMEM beeSprite[] = {
  0x0A, 0x1F, 0x0E, 0x0A, 0x04
};

const uint8_t PROGMEM butterflySprite[] = {
  0x11, 0x0E, 0x1F, 0x0E, 0x11
};

const uint8_t PROGMEM bossSprite[] = {
  0x0E, 0x1F, 0x1B, 0x1F, 0x0E
};

const uint8_t PROGMEM mothershipSprite[] = {
  0x1C, 0x3E, 0x7F, 0x5D, 0x7F, 0x3E, 0x1C
};

// --- Data Structures ---
struct Bullet {
  float x, y;
  float vy;
  bool active;
};

struct Enemy {
  float x, y;
  float homeX, homeY;
  uint8_t type;
  uint8_t hp;
  bool active;
  bool diving;
  bool returning;
  float divePhase;
  uint8_t divePattern; 
  int8_t diveDirection;
};

struct Mothership {
  float x, y;
  bool active;
  bool diving;
  int8_t dir;
  uint8_t hp;
  float phase;
};

struct Particle {
  float x, y, vx, vy;
  uint8_t life;
};

struct Star {
  uint8_t x, y, speed;
};

// --- Game State Variables ---
enum GameState { TITLE, PLAYING, GAMEOVER };
GameState gameState = TITLE;

float playerX = 30;
float playerY = 118;
float playerVx = 0;
float playerVy = 0;
uint8_t lives = 3;
uint16_t score = 0;
uint8_t wave = 1;
uint8_t invulnTimer = 0;
uint16_t dualShooterTimer = 0;

Bullet pBullets[MAX_PLAYER_BULLETS];
Bullet eBullets[MAX_ENEMY_BULLETS];
Enemy enemies[MAX_ENEMIES];
Mothership mothership;
Particle particles[MAX_PARTICLES];
Star stars[MAX_STARS];

float formationOffsetX = 0;
float formationOffsetY = 0;
int8_t formationDir = 1;
uint16_t diveTimer = 0;
uint16_t mothershipSpawnTimer = 0;

// --- Helper Functions ---
void spawnParticles(float x, float y, uint8_t count) {
  for (uint8_t i = 0; i < MAX_PARTICLES && count > 0; i++) {
    if (particles[i].life == 0) {
      particles[i].x = x;
      particles[i].y = y;
      particles[i].vx = (random(0, 200) - 100) / 50.0;
      particles[i].vy = (random(0, 200) - 100) / 50.0;
      particles[i].life = random(8, 15);
      count--;
    }
  }
}

void initWave() {
  formationOffsetX = 0;
  formationOffsetY = 0;
  formationDir = 1;
  mothership.active = false;
  mothershipSpawnTimer = 0;
  uint8_t idx = 0;
  for (uint8_t r = 0; r < ENEMY_ROWS; r++) {
    for (uint8_t c = 0; c < ENEMY_COLS; c++) {
      enemies[idx].homeX = 4 + c * 8;
      enemies[idx].homeY = 12 + r * 7;
      enemies[idx].x = enemies[idx].homeX;
      enemies[idx].y = enemies[idx].homeY;
      enemies[idx].active = true;
      enemies[idx].diving = false;
      enemies[idx].returning = false;
      
      if (r == 0) {
        enemies[idx].type = 2; // Boss
        enemies[idx].hp = 2;
      } else if (r == 1 || r == 2) {
        enemies[idx].type = 1; // Butterfly
        enemies[idx].hp = 1;
      } else {
        enemies[idx].type = 0; // Bee
        enemies[idx].hp = 1;
      }
      idx++;
    }
  }
}

void resetGame() {
  playerX = 30;
  playerY = 118;
  playerVx = 0;
  playerVy = 0;
  lives = 3;
  score = 0;
  wave = 1;
  invulnTimer = 0;
  dualShooterTimer = 0;
  for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) pBullets[i].active = false;
  for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) eBullets[i].active = false;
  initWave();
}

void firePlayerBullet() {
  if (dualShooterTimer > 0) {
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS && i + 1 < MAX_PLAYER_BULLETS; i += 2) {
      if (!pBullets[i].active && !pBullets[i+1].active) {
        pBullets[i].x = playerX;
        pBullets[i].y = playerY - 2;
        pBullets[i].vy = -4.0;
        pBullets[i].active = true;

        pBullets[i+1].x = playerX + 4;
        pBullets[i+1].y = playerY - 2;
        pBullets[i+1].vy = -4.0;
        pBullets[i+1].active = true;

        beep.tone(beep.freq(880), 2);
        break;
      }
    }
  } else {
    for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
      if (!pBullets[i].active) {
        pBullets[i].x = playerX + 2;
        pBullets[i].y = playerY - 2;
        pBullets[i].vy = -4.0;
        pBullets[i].active = true;
        beep.tone(beep.freq(880), 2);
        break;
      }
    }
  }
}

void fireEnemyBullet(float x, float y) {
  for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (!eBullets[i].active) {
      eBullets[i].x = x + 2;
      eBullets[i].y = y + 5;
      float bulletSpeed = 1.0 + (wave * 0.12);
      if (bulletSpeed > 2.8) bulletSpeed = 2.8;
      eBullets[i].vy = bulletSpeed;
      eBullets[i].active = true;
      break;
    }
  }
}

void triggerRandomDives() {
  uint8_t squadSize = random(1, 4);
  uint8_t pattern = random(0, 3);

  for (uint8_t s = 0; s < squadSize; s++) {
    int target = -1;
    uint8_t startIndex = random(0, MAX_ENEMIES);
    
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
      uint8_t idx = (startIndex + i) % MAX_ENEMIES;
      if (enemies[idx].active && !enemies[idx].diving) {
        bool columnOccupied = false;
        for (uint8_t d = 0; d < MAX_ENEMIES; d++) {
          if (enemies[d].diving && abs((int)enemies[d].homeX - (int)enemies[idx].homeX) < 6) {
            columnOccupied = true;
            break;
          }
        }
        if (!columnOccupied) {
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

  beep.tone(beep.freq(300), 4);
}

void drawScore(int x, int y, uint16_t currentScore) {
  uint8_t digits[5];
  uint8_t count = 0;
  
  if (currentScore == 0) {
    digits[0] = 0;
    count = 1;
  } else {
    while (currentScore > 0 && count < 5) {
      digits[count++] = currentScore % 10;
      currentScore /= 10;
    }
  }

  for (uint8_t i = 0; i < count; i++) {
    const uint8_t* digitBitmap = (const uint8_t*)pgm_read_word(&numberTable[digits[i]]);
    int digitX = x - (i * 4);
    vdisplay.drawBitmap(digitX, y, digitBitmap + 2, 3, 5, WHITE);
  }
}

// --- Setup ---
void setup() {
  arduboy.begin();
  arduboy.setFrameRate(60);
  arduboy.initRandomSeed();
  beep.begin();

  for (uint8_t i = 0; i < MAX_STARS; i++) {
    stars[i].x = random(0, 64);
    stars[i].y = random(0, 128);
    stars[i].speed = random(1, 3);
  }
}

// --- Update & Draw ---
void updateStars() {
  for (uint8_t i = 0; i < MAX_STARS; i++) {
    stars[i].y += stars[i].speed;
    if (stars[i].y >= 128) {
      stars[i].y = 0;
      stars[i].x = random(0, 64);
    }
    if (random(0, 2) == 0) {
      vdisplay.drawPixel(stars[i].x, stars[i].y, WHITE);
    }
  }
}

void updatePlaying() {
  if (invulnTimer > 0) invulnTimer--;
  if (dualShooterTimer > 0) dualShooterTimer--;

  // Player Controls with Inertia
  if (arduboy.pressed(UP_BUTTON)) playerVx -= 0.35;
  if (arduboy.pressed(DOWN_BUTTON)) playerVx += 0.35;
  
  playerVx *= 0.82;
  playerX += playerVx;

  if (playerX < 1) { playerX = 1; playerVx = 0; }
  if (playerX > 58) { playerX = 58; playerVx = 0; }

  if (arduboy.pressed(RIGHT_BUTTON)) playerVy -= 0.35;
  if (arduboy.pressed(LEFT_BUTTON)) playerVy += 0.35;
  
  playerVy *= 0.82;
  playerY += playerVy;

  if (playerY < 90) { playerY = 90; playerVy = 0; }
  if (playerY > 122) { playerY = 122; playerVy = 0; }

  if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
    firePlayerBullet();
  }

  // Player Bullets
  for (uint8_t i = 0; i < MAX_PLAYER_BULLETS; i++) {
    if (pBullets[i].active) {
      pBullets[i].y += pBullets[i].vy;
      if (pBullets[i].y < 0) pBullets[i].active = false;
      vdisplay.drawFastVLine(pBullets[i].x, pBullets[i].y, 2, WHITE);
    }
  }

  // Formation speed buildup
  float formationSpeed = 0.04 + (wave * 0.025);
  if (formationSpeed > 0.4) formationSpeed = 0.4;
  
  formationOffsetX += formationDir * formationSpeed;

  // Dynamic Edge Detection
  float minActiveHomeX = 999;
  float maxActiveHomeX = -999;
  for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active && !enemies[i].diving) {
      if (enemies[i].homeX < minActiveHomeX) minActiveHomeX = enemies[i].homeX;
      if (enemies[i].homeX > maxActiveHomeX) maxActiveHomeX = enemies[i].homeX;
    }
  }

  if (minActiveHomeX != 999) {
    float leftEdgeLimit = -minActiveHomeX + 1;
    float rightEdgeLimit = 59 - maxActiveHomeX;

    if (formationOffsetX < leftEdgeLimit) {
      formationOffsetX = leftEdgeLimit;
      formationDir = 1;
      formationOffsetY += 1.0;
    }
    if (formationOffsetX > rightEdgeLimit) {
      formationOffsetX = rightEdgeLimit;
      formationDir = -1;
      formationOffsetY += 1.0;
    }
  }

  // Mothership Spawn & Behavior Logic
  if (!mothership.active) {
    mothershipSpawnTimer++;
    if (mothershipSpawnTimer > 350) {
      mothershipSpawnTimer = 0;
      mothership.active = true;
      mothership.diving = false;
      mothership.x = 28;
      mothership.y = 4;
      mothership.hp = 3;
      mothership.dir = (random(0, 2) == 0) ? 1 : -1;
      mothership.phase = 0;
    }
  } else {
    if (!mothership.diving) {
      mothership.x += mothership.dir * 0.5;
      if (mothership.x < 2 || mothership.x > 55) {
        mothership.dir *= -1;
      }
      if (random(0, 300) == 0) {
        mothership.diving = true;
        mothership.phase = 0;
      }
    } else {
      mothership.phase += 0.05;
      mothership.y += 1.2;
      mothership.x += sin(mothership.phase) * 1.5;
      if (mothership.x < 2) mothership.x = 2;
      if (mothership.x > 55) mothership.x = 55;

      if (mothership.y > 132) {
        mothership.y = 4;
        mothership.diving = false;
      }
    }

    vdisplay.drawBitmap(mothership.x, mothership.y + formationOffsetY, mothershipSprite, 7, 7, WHITE);
  }

  // Dive Trigger Timer
  diveTimer++;
  uint16_t currentDiveInterval = max(40, 130 - (wave * 8));
  if (diveTimer >= currentDiveInterval) {
    diveTimer = 0;
    triggerRandomDives();
  }

  // Update Enemies
  uint8_t activeEnemies = 0;
  for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].active) continue;
    activeEnemies++;

    if (!enemies[i].diving) {
      enemies[i].x = enemies[i].homeX + formationOffsetX;
      enemies[i].y = enemies[i].homeY + formationOffsetY;
    } else {
      if (!enemies[i].returning) {
        enemies[i].divePhase += 0.06;
        enemies[i].y += 0.9 + (wave * 0.06);

        if (enemies[i].divePattern == 0) {
          enemies[i].x += sin(enemies[i].divePhase) * 1.0 * enemies[i].diveDirection;
        } else if (enemies[i].divePattern == 1) {
          enemies[i].x += cos(enemies[i].divePhase * 1.3) * 1.3;
        } else {
          float targetDir = (playerX > enemies[i].x) ? 1.0 : -1.0;
          enemies[i].x += targetDir * 0.6;
        }

        if (enemies[i].x < 1) enemies[i].x = 1;
        if (enemies[i].x > 58) enemies[i].x = 58;

        if (random(0, 100) < (1 + wave)) {
          fireEnemyBullet(enemies[i].x, enemies[i].y);
        }

        if (enemies[i].y > 132) {
          enemies[i].y = -12;
          enemies[i].x = enemies[i].homeX + formationOffsetX;
          enemies[i].returning = true;
        }
      } else {
        enemies[i].y += 1.5;
        float targetX = enemies[i].homeX + formationOffsetX;
        if (enemies[i].x < targetX) enemies[i].x += 0.5;
        else if (enemies[i].x > targetX) enemies[i].x -= 0.5;

        float targetY = enemies[i].homeY + formationOffsetY;
        if (enemies[i].y >= targetY) {
          enemies[i].y = targetY;
          enemies[i].diving = false;
          enemies[i].returning = false;
        }
      }
    }

    // Bullet Collisions with Enemies (5x5)
    for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++) {
      if (pBullets[b].active &&
          pBullets[b].x >= enemies[i].x && pBullets[b].x <= enemies[i].x + 5 &&
          pBullets[b].y >= enemies[i].y && pBullets[b].y <= enemies[i].y + 5) {
        pBullets[b].active = false;
        enemies[i].hp--;
        if (enemies[i].hp == 0) {
          enemies[i].active = false;
          spawnParticles(enemies[i].x + 2, enemies[i].y + 2, 4);
          score += (enemies[i].type + 1) * 25 * (enemies[i].diving ? 2 : 1);
          beep.tone(beep.freq(160), 4);
        } else {
          beep.tone(beep.freq(500), 2);
        }
        break;
      }
    }

    // Bullet Collisions with Mothership (7x7) - Only awards dual shooter if swooping
    if (mothership.active) {
      for (uint8_t b = 0; b < MAX_PLAYER_BULLETS; b++) {
        if (pBullets[b].active &&
            pBullets[b].x >= mothership.x && pBullets[b].x <= mothership.x + 7 &&
            pBullets[b].y >= mothership.y + formationOffsetY && pBullets[b].y <= mothership.y + formationOffsetY + 7) {
          pBullets[b].active = false;
          mothership.hp--;
          if (mothership.hp == 0) {
            mothership.active = false;
            spawnParticles(mothership.x + 3, mothership.y + formationOffsetY + 3, 8);
            score += 200;
            
            // Only reward dual-shooter if it was swooping when killed
            if (mothership.diving) {
              dualShooterTimer = 600; // 10 seconds power-up at 60fps
            }
            
            beep.tone(beep.freq(700), 8);
          } else {
            beep.tone(beep.freq(600), 3);
          }
          break;
        }
      }
    }

    // Player Collision with lowest enemies in formation + active diving enemies
    bool canCollide = enemies[i].diving;
    if (!canCollide) {
      bool isLowestInColumn = true;
      for (uint8_t other = 0; other < MAX_ENEMIES; other++) {
        if (enemies[other].active && !enemies[other].diving && 
            abs((int)enemies[other].homeX - (int)enemies[i].homeX) < 5) {
          if (enemies[other].homeY > enemies[i].homeY) {
            isLowestInColumn = false;
            break;
          }
        }
      }
      if (isLowestInColumn) canCollide = true;
    }

    if (enemies[i].active && invulnTimer == 0 && canCollide &&
        enemies[i].x + 5 > playerX && enemies[i].x < playerX + 5 &&
        enemies[i].y + 5 > playerY && enemies[i].y < playerY + 5) {
      enemies[i].active = false;
      spawnParticles(playerX + 2, playerY + 2, 8);
      lives--;
      invulnTimer = 80;
      beep.tone(beep.freq(80), 10);
      if (lives == 0) gameState = GAMEOVER;
    }

    // Draw Enemy (5x5)
    if (enemies[i].active) {
      const uint8_t* sprite = beeSprite;
      if (enemies[i].type == 1) sprite = butterflySprite;
      else if (enemies[i].type == 2) sprite = bossSprite;
      vdisplay.drawBitmap(enemies[i].x, enemies[i].y, sprite, 5, 5, WHITE);
    }
  }

  // Next Wave
  if (activeEnemies == 0) {
    wave++;
    initWave();
  }

  // Enemy Bullets
  for (uint8_t i = 0; i < MAX_ENEMY_BULLETS; i++) {
    if (eBullets[i].active) {
      eBullets[i].y += eBullets[i].vy;
      if (eBullets[i].y > 128) {
        eBullets[i].active = false;
      } else {
        vdisplay.drawPixel(eBullets[i].x, eBullets[i].y, WHITE);
        if (invulnTimer == 0 &&
            eBullets[i].x >= playerX && eBullets[i].x <= playerX + 5 &&
            eBullets[i].y >= playerY && eBullets[i].y <= playerY + 5) {
          eBullets[i].active = false;
          spawnParticles(playerX + 2, playerY + 2, 8);
          lives--;
          invulnTimer = 80;
          beep.tone(beep.freq(80), 10);
          if (lives == 0) gameState = GAMEOVER;
        }
      }
    }
  }

  // Particles
  for (uint8_t i = 0; i < MAX_PARTICLES; i++) {
    if (particles[i].life > 0) {
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
      particles[i].life--;
      vdisplay.drawPixel(particles[i].x, particles[i].y, WHITE);
    }
  }

  // Player Ship (5x5)
  if (invulnTimer == 0 || (invulnTimer % 6 < 3)) {
    if (dualShooterTimer > 0) {
      vdisplay.drawBitmap(playerX - 2, playerY, playerSprite, 5, 5, WHITE);
      vdisplay.drawBitmap(playerX + 2, playerY, playerSprite, 5, 5, WHITE);
    } else {
      vdisplay.drawBitmap(playerX, playerY, playerSprite, 5, 5, WHITE);
    }
  }

  // Render Score at top
  drawScore(56, 2, score);
}

void loop() {
  if (!arduboy.nextFrame()) return;
  arduboy.pollButtons();
  beep.timer();
  arduboy.clear();

  updateStars();

  if (gameState == TITLE) {
    arduboy.setCursor(2, 40);
    arduboy.print(F("GALAGA"));
    arduboy.setCursor(2, 60);
    arduboy.print(F("PRESS A"));
    if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
      resetGame();
      gameState = PLAYING;
    }
  } else if (gameState == PLAYING) {
    updatePlaying();
  } else if (gameState == GAMEOVER) {
    arduboy.setCursor(2, 40);
    arduboy.print(F("OVER"));
    arduboy.setCursor(2, 60);
    drawScore(40, 60, score);
    if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {
      resetGame();
      gameState = PLAYING;
    }
  }

  arduboy.display();
}