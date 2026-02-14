#include <nds.h>
#include <stdio.h>
#include <string.h>

#define SCREEN_W 256
#define SCREEN_H 192
#define WORLD_W 1600
#define MAX_PLATFORMS 14
#define MAX_COINS 16
#define MAX_ENEMIES 8

#define COL_SKY RGB15(18, 25, 31)
#define COL_GROUND RGB15(12, 20, 8)
#define COL_DIRT RGB15(16, 9, 5)
#define COL_GRASS RGB15(8, 24, 7)
#define COL_POU RGB15(20, 11, 6)
#define COL_WHITE RGB15(31, 31, 31)
#define COL_BLACK RGB15(0, 0, 0)
#define COL_ENEMY RGB15(24, 9, 5)
#define COL_COIN RGB15(31, 25, 5)
#define COL_FLAG RGB15(31, 8, 8)
#define COL_POLE RGB15(25, 25, 25)

typedef struct {
    int x, y, w, h;
} Rect;

typedef struct {
    int x, y;
    int vx, vy;
    int w, h;
    int onGround;
    int invuln;
} Player;

typedef struct {
    int x, y, r;
    int taken;
} Coin;

typedef struct {
    int x, y, w, h;
    int vx;
    int minX, maxX;
    int alive;
} Enemy;

static Player gPlayer;
static Rect gPlatforms[MAX_PLATFORMS];
static Coin gCoins[MAX_COINS];
static Enemy gEnemies[MAX_ENEMIES];

static int gLives = 3;
static int gCoinCount = 0;
static int gCameraX = 0;
static int gStatus = 0; // 0 playing, 1 won, 2 game over

static const int gPlatformCount = 9;
static const int gCoinTotal = 8;
static const int gEnemyTotal = 5;

static volatile u16 *fb = NULL;

static int intersects(const Rect *a, const Rect *b) {
    return (a->x < b->x + b->w) && (a->x + a->w > b->x) && (a->y < b->y + b->h) && (a->y + a->h > b->y);
}

static void putPixel(int x, int y, u16 c) {
    if ((unsigned)x >= SCREEN_W || (unsigned)y >= SCREEN_H) return;
    fb[y * SCREEN_W + x] = c;
}

static void fillRectScreen(int x, int y, int w, int h, u16 c) {
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w > SCREEN_W ? SCREEN_W : x + w;
    int y1 = y + h > SCREEN_H ? SCREEN_H : y + h;

    for (int py = y0; py < y1; py++) {
        int row = py * SCREEN_W;
        for (int px = x0; px < x1; px++) {
            fb[row + px] = c;
        }
    }
}

static void clearScreen(u16 c) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        fb[i] = c;
    }
}

static void initLevel(void) {
    gPlatforms[0] = (Rect){0, 152, 420, 40};
    gPlatforms[1] = (Rect){470, 152, 240, 40};
    gPlatforms[2] = (Rect){760, 152, 260, 40};
    gPlatforms[3] = (Rect){1070, 152, 250, 40};
    gPlatforms[4] = (Rect){1370, 152, 230, 40};
    gPlatforms[5] = (Rect){200, 122, 84, 14};
    gPlatforms[6] = (Rect){640, 110, 92, 14};
    gPlatforms[7] = (Rect){970, 102, 92, 14};
    gPlatforms[8] = (Rect){1230, 112, 120, 14};

    gCoins[0] = (Coin){224, 104, 5, 0};
    gCoins[1] = (Coin){502, 130, 5, 0};
    gCoins[2] = (Coin){668, 92, 5, 0};
    gCoins[3] = (Coin){814, 130, 5, 0};
    gCoins[4] = (Coin){998, 84, 5, 0};
    gCoins[5] = (Coin){1162, 130, 5, 0};
    gCoins[6] = (Coin){1268, 94, 5, 0};
    gCoins[7] = (Coin){1450, 130, 5, 0};

    gEnemies[0] = (Enemy){300, 132, 16, 20, 1, 170, 400, 1};
    gEnemies[1] = (Enemy){570, 132, 16, 20, -1, 470, 680, 1};
    gEnemies[2] = (Enemy){915, 132, 16, 20, 1, 760, 1000, 1};
    gEnemies[3] = (Enemy){1120, 132, 16, 20, -1, 1070, 1290, 1};
    gEnemies[4] = (Enemy){1420, 132, 16, 20, 1, 1370, 1570, 1};
}

static void resetPlayer(void) {
    gPlayer.x = 34;
    gPlayer.y = 80;
    gPlayer.w = 20;
    gPlayer.h = 24;
    gPlayer.vx = 0;
    gPlayer.vy = 0;
    gPlayer.onGround = 0;
    gPlayer.invuln = 60;
    gCameraX = 0;
}

static void resetRun(void) {
    gLives = 3;
    gCoinCount = 0;
    gStatus = 0;
    initLevel();
    resetPlayer();
}

static void loseLife(void) {
    if (gPlayer.invuln > 0 || gStatus != 0) return;
    gLives--;
    if (gLives <= 0) {
        gStatus = 2;
        return;
    }
    resetPlayer();
}

static void updatePlayerInput(void) {
    scanKeys();
    int held = keysHeld();
    int down = keysDown();

    if ((held & KEY_LEFT) && !(held & KEY_RIGHT)) gPlayer.vx = -2;
    else if ((held & KEY_RIGHT) && !(held & KEY_LEFT)) gPlayer.vx = 2;
    else gPlayer.vx = 0;

    if ((down & KEY_A) && gPlayer.onGround) {
        gPlayer.vy = -8;
        gPlayer.onGround = 0;
    }

    if ((down & KEY_START) && gStatus != 0) {
        resetRun();
    }
}

static void updatePlayerPhysics(void) {
    if (gStatus != 0) return;

    gPlayer.vy += 1;
    if (gPlayer.vy > 7) gPlayer.vy = 7;

    gPlayer.x += gPlayer.vx;
    gPlayer.y += gPlayer.vy;

    Rect pr = {gPlayer.x, gPlayer.y, gPlayer.w, gPlayer.h};
    gPlayer.onGround = 0;

    for (int i = 0; i < gPlatformCount; i++) {
        Rect p = gPlatforms[i];
        if (!intersects(&pr, &p)) continue;

        if (gPlayer.vy >= 0 && (pr.y + pr.h - gPlayer.vy) <= p.y + 4) {
            gPlayer.y = p.y - gPlayer.h;
            gPlayer.vy = 0;
            gPlayer.onGround = 1;
            pr.y = gPlayer.y;
        } else if (gPlayer.vy < 0 && (pr.y - gPlayer.vy) >= (p.y + p.h - 4)) {
            gPlayer.y = p.y + p.h;
            gPlayer.vy = 0;
            pr.y = gPlayer.y;
        } else if (gPlayer.vx > 0) {
            gPlayer.x = p.x - gPlayer.w;
            gPlayer.vx = 0;
            pr.x = gPlayer.x;
        } else if (gPlayer.vx < 0) {
            gPlayer.x = p.x + p.w;
            gPlayer.vx = 0;
            pr.x = gPlayer.x;
        }
    }

    if (gPlayer.x < 0) gPlayer.x = 0;
    if (gPlayer.x > WORLD_W - gPlayer.w) gPlayer.x = WORLD_W - gPlayer.w;

    if (gPlayer.y > SCREEN_H + 40) loseLife();

    if (gPlayer.invuln > 0) gPlayer.invuln--;
}

static void updateCoins(void) {
    if (gStatus != 0) return;
    Rect pr = {gPlayer.x, gPlayer.y, gPlayer.w, gPlayer.h};

    for (int i = 0; i < gCoinTotal; i++) {
        if (gCoins[i].taken) continue;
        Rect cr = {gCoins[i].x - gCoins[i].r, gCoins[i].y - gCoins[i].r, gCoins[i].r * 2, gCoins[i].r * 2};
        if (intersects(&pr, &cr)) {
            gCoins[i].taken = 1;
            gCoinCount++;
        }
    }
}

static void updateEnemies(void) {
    if (gStatus != 0) return;

    Rect pr = {gPlayer.x, gPlayer.y, gPlayer.w, gPlayer.h};

    for (int i = 0; i < gEnemyTotal; i++) {
        if (!gEnemies[i].alive) continue;

        gEnemies[i].x += gEnemies[i].vx;
        if (gEnemies[i].x < gEnemies[i].minX || (gEnemies[i].x + gEnemies[i].w) > gEnemies[i].maxX) {
            gEnemies[i].vx = -gEnemies[i].vx;
        }

        Rect er = {gEnemies[i].x, gEnemies[i].y, gEnemies[i].w, gEnemies[i].h};
        if (!intersects(&pr, &er)) continue;

        int stomp = (gPlayer.vy > 0) && (gPlayer.y + gPlayer.h - 4 <= gEnemies[i].y);
        if (stomp) {
            gEnemies[i].alive = 0;
            gPlayer.vy = -5;
        } else {
            loseLife();
        }
    }
}

static void updateWin(void) {
    if (gStatus != 0) return;
    Rect pr = {gPlayer.x, gPlayer.y, gPlayer.w, gPlayer.h};
    Rect flag = {1540, 86, 20, 66};

    if (intersects(&pr, &flag)) gStatus = 1;
}

static void updateCamera(void) {
    int target = gPlayer.x - 90;
    if (target < 0) target = 0;
    if (target > WORLD_W - SCREEN_W) target = WORLD_W - SCREEN_W;
    gCameraX = target;
}

static void drawCircle(int cx, int cy, int r, u16 c) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if ((x * x) + (y * y) <= r * r) {
                putPixel(cx + x, cy + y, c);
            }
        }
    }
}

static void drawPou(int sx, int sy) {
    fillRectScreen(sx + 2, sy + 2, 16, 16, COL_POU);
    drawCircle(sx + 10, sy + 8, 8, COL_POU);
    drawCircle(sx + 6, sy + 8, 7, COL_POU);
    drawCircle(sx + 14, sy + 8, 7, COL_POU);

    drawCircle(sx + 7, sy + 9, 2, COL_WHITE);
    drawCircle(sx + 13, sy + 9, 2, COL_WHITE);
    putPixel(sx + 7, sy + 9, COL_BLACK);
    putPixel(sx + 13, sy + 9, COL_BLACK);

    fillRectScreen(sx + 8, sy + 14, 4, 1, COL_BLACK);
}

static void drawCoin(int sx, int sy, int r) {
    drawCircle(sx, sy, r, COL_COIN);
    fillRectScreen(sx - 1, sy - (r - 1), 2, (r * 2) - 1, RGB15(28, 20, 3));
}

static void drawWorld(void) {
    clearScreen(COL_SKY);

    fillRectScreen(0, 152, SCREEN_W, 40, COL_DIRT);
    fillRectScreen(0, 152, SCREEN_W, 6, COL_GRASS);

    for (int i = 0; i < gPlatformCount; i++) {
        int sx = gPlatforms[i].x - gCameraX;
        if (sx + gPlatforms[i].w < 0 || sx > SCREEN_W) continue;
        fillRectScreen(sx, gPlatforms[i].y, gPlatforms[i].w, gPlatforms[i].h, COL_DIRT);
        fillRectScreen(sx, gPlatforms[i].y, gPlatforms[i].w, 4, COL_GRASS);
    }

    for (int i = 0; i < gCoinTotal; i++) {
        if (gCoins[i].taken) continue;
        int sx = gCoins[i].x - gCameraX;
        if (sx < -8 || sx > SCREEN_W + 8) continue;
        drawCoin(sx, gCoins[i].y, gCoins[i].r);
    }

    for (int i = 0; i < gEnemyTotal; i++) {
        if (!gEnemies[i].alive) continue;
        int sx = gEnemies[i].x - gCameraX;
        if (sx + gEnemies[i].w < 0 || sx > SCREEN_W) continue;
        fillRectScreen(sx, gEnemies[i].y, gEnemies[i].w, gEnemies[i].h, COL_ENEMY);
        fillRectScreen(sx + 4, gEnemies[i].y + 6, 3, 3, COL_WHITE);
        fillRectScreen(sx + 9, gEnemies[i].y + 6, 3, 3, COL_WHITE);
        putPixel(sx + 5, gEnemies[i].y + 7, COL_BLACK);
        putPixel(sx + 10, gEnemies[i].y + 7, COL_BLACK);
    }

    int poleX = 1540 - gCameraX;
    fillRectScreen(poleX, 86, 3, 66, COL_POLE);
    fillRectScreen(poleX + 3, 94, 22, 12, COL_FLAG);

    if (!(gPlayer.invuln > 0 && (gPlayer.invuln & 4))) {
        drawPou(gPlayer.x - gCameraX, gPlayer.y);
    }
}

static void drawHUD(void) {
    static int lastLives = -1;
    static int lastCoins = -1;
    static int lastStatus = -1;

    if (lastLives == gLives && lastCoins == gCoinCount && lastStatus == gStatus) return;

    consoleClear();
    iprintf("Super Pou Bros DS\n");
    iprintf("Vidas: %d\n", gLives);
    iprintf("Monedas: %d/%d\n", gCoinCount, gCoinTotal);
    iprintf("A: Saltar  LEFT/RIGHT: Mover\n");

    if (gStatus == 1) {
        iprintf("\nGANASTE!\nSTART para reiniciar");
    } else if (gStatus == 2) {
        iprintf("\nGAME OVER\nSTART para reiniciar");
    } else {
        iprintf("\nLlega a la bandera.");
    }

    lastLives = gLives;
    lastCoins = gCoinCount;
    lastStatus = gStatus;
}

int main(void) {
    videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);
    videoSetModeSub(MODE_0_2D);

    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);

    REG_BG2CNT = BG_BMP16_256x256 | BG_BMP_BASE(0);
    fb = (u16 *)BG_BMP_RAM(0);

    consoleDemoInit();

    resetRun();

    while (1) {
        swiWaitForVBlank();

        updatePlayerInput();
        updatePlayerPhysics();
        updateCoins();
        updateEnemies();
        updateWin();
        updateCamera();

        drawWorld();
        drawHUD();
    }

    return 0;
}
