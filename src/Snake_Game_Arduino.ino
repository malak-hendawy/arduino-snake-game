#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>

MCUFRIEND_kbv tft;

// ======================
// PIN DEFINITIONS
// ======================
#define JOY_X     13
#define JOY_Y     A5
#define JOY_SW    11

#define LED_GREEN 12
#define LED_RED   0
#define BUZZER    1

// ======================
// COLORS
// ======================
#define BLACK   0x0000
#define GREEN   0x07E0
#define RED     0xF800
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// ======================
// SCREEN & GAME SETTINGS
// ======================
const int screenW = 480;
const int screenH = 320;
const int cellSize = 15;

const int gridW = screenW / cellSize;
const int gridH = screenH / cellSize;

const int maxLength = 300;

// ======================
// GAME VARIABLES
// ======================
int snakeX[maxLength];
int snakeY[maxLength];
int snakeLength = 5;

int foodX, foodY;

int dir = 1;       // 0=UP,1=RIGHT,2=DOWN,3=LEFT
int nextDir = 1;

bool gameStarted = false;
bool gameOver = false;

// ======================
// TIMING
// ======================
unsigned long lastMoveTime = 0;
unsigned long lastInputTime = 0;
unsigned long greenLEDTime = 0;

const int frameDelay = 120;
const int inputDelay = 150;
const int greenLEDDuration = 1000;

bool greenLEDOn = false;
bool gameOverSoundPlayed = false;

// ======================
// SOUND FUNCTIONS
// ======================
void playStartSound() {
  tone(BUZZER, 1200, 120);
  delay(150);
  tone(BUZZER, 1600, 120);
  delay(150);
  tone(BUZZER, 2000, 150);
}

void playEatSound() {
  tone(BUZZER, 1000, 80);
}

void playGameOverSound() {
  tone(BUZZER, 900, 150);
  delay(180);
  tone(BUZZER, 600, 200);
  delay(220);
  tone(BUZZER, 400, 300);
}

// ======================
// LED CONTROL
// ======================
void activateGreenLED() {
  digitalWrite(LED_GREEN, HIGH);
  greenLEDTime = millis();
  greenLEDOn = true;
}

void updateGreenLED() {
  if (greenLEDOn && millis() - greenLEDTime > greenLEDDuration) {
    digitalWrite(LED_GREEN, LOW);
    greenLEDOn = false;
  }
}

void resetLEDsAndSound() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  noTone(BUZZER);
  greenLEDOn = false;
  gameOverSoundPlayed = false;
}

// ======================
// DRAWING FUNCTIONS
// ======================
void drawCell(int x, int y, uint16_t color) {
  tft.fillRect(x, y, cellSize, cellSize, color);
}

void drawSnake() {
  drawCell(snakeX[0], snakeY[0], YELLOW);
  for (int i = 1; i < snakeLength; i++) {
    drawCell(snakeX[i], snakeY[i], GREEN);
  }
}

void drawFood() {
  drawCell(foodX, foodY, RED);
}

// ======================
// GAME LOGIC
// ======================
void placeFood() {
  foodX = random(gridW) * cellSize;
  foodY = random(gridH) * cellSize;

  for (int i = 0; i < snakeLength; i++) {
    if (foodX == snakeX[i] && foodY == snakeY[i]) {
      placeFood();
      return;
    }
  }
}

void initSnake() {
  snakeLength = 5;
  dir = nextDir = 1;

  int startX = (gridW / 2) * cellSize;
  int startY = (gridH / 2) * cellSize;

  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = startX - i * cellSize;
    snakeY[i] = startY;
  }
}

void moveSnake() {
  drawCell(snakeX[snakeLength - 1], snakeY[snakeLength - 1], BLACK);

  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  dir = nextDir;

  if (dir == 0) snakeY[0] -= cellSize;
  if (dir == 1) snakeX[0] += cellSize;
  if (dir == 2) snakeY[0] += cellSize;
  if (dir == 3) snakeX[0] -= cellSize;

  drawSnake();
}

bool checkCollision() {
  if (snakeX[0] < 0 || snakeX[0] >= screenW ||
      snakeY[0] < 0 || snakeY[0] >= screenH)
    return true;

  for (int i = 1; i < snakeLength; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i])
      return true;
  }

  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeX[snakeLength] = snakeX[snakeLength - 1];
    snakeY[snakeLength] = snakeY[snakeLength - 1];
    snakeLength++;

    activateGreenLED();
    playEatSound();

    placeFood();
    drawFood();
  }

  return false;
}

// ======================
// INPUT
// ======================
void handleInput() {
  int x = digitalRead(JOY_X);
  int y = analogRead(JOY_Y);
  int btn = digitalRead(JOY_SW);

  static unsigned long lastBtn = 0;

  if (btn == LOW && millis() - lastBtn > 300) {
    lastBtn = millis();

    if (!gameStarted) {
      gameStarted = true;
      gameOver = false;
      resetLEDsAndSound();
      playStartSound();

      tft.fillScreen(BLACK);
      initSnake();
      placeFood();
      drawSnake();
      drawFood();
    } 
    else if (gameOver) {
      gameStarted = false;
      resetLEDsAndSound();
      tft.fillScreen(BLACK);
    }
  }

  if (millis() - lastInputTime > inputDelay) {
    if (y < 300 && dir != 2) nextDir = 0;
    else if (y > 700 && dir != 0) nextDir = 2;
    else if (x == LOW && dir != 1) nextDir = 3;
    else if (x == HIGH && dir != 3) nextDir = 1;

    lastInputTime = millis();
  }
}

// ======================
// SCREENS
// ======================
void drawStartScreen() {
  tft.fillScreen(BLACK);
  tft.setTextSize(2);
  tft.setTextColor(GREEN);
  tft.setCursor(120, 130);
  tft.print("SNAKE GAME");

  tft.setTextSize(3);
  tft.setCursor(110, 200);
  tft.print("For Dr.Khaled <3");
}

void drawGameOver() {
  tft.fillScreen(BLACK);
  tft.setTextSize(3);
  tft.setTextColor(RED);
  tft.setCursor(140, 120);
  tft.print("GAME OVER");

  tft.setTextSize(2);
  tft.setCursor(150, 180);
  tft.print("SCORE: ");
  tft.print(snakeLength - 5);
}

// ======================
// SETUP
// ======================
void setup() {
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(JOY_SW, INPUT_PULLUP);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  resetLEDsAndSound();

  uint16_t id = tft.readID();
  if (id == 0xFFFF || id == 0x0000) id = 0x9486;

  tft.begin(id);
  tft.setRotation(1);

  drawStartScreen();
}

// ======================
// LOOP
// ======================
void loop() {
  handleInput();
  updateGreenLED();

  if (!gameStarted) return;

  if (gameOver) {
    digitalWrite(LED_RED, HIGH);
    if (!gameOverSoundPlayed) {
      playGameOverSound();
      gameOverSoundPlayed = true;
    }
    drawGameOver();
    delay(100);
    return;
  }

  if (millis() - lastMoveTime > frameDelay) {
    moveSnake();
    if (checkCollision()) {
      gameOver = true;
    }
    lastMoveTime = millis();
  }
}