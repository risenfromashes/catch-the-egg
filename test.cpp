#include "gamestate.h"

int width = 1280, height = 720;

GameState* game;

void iDraw()
{
    iClear();
    drawFrame(game);
}
void iMouseMove(int mx, int my) {}

void iMouse(int button, int state, int mx, int my) {}
void iPassiveMouseMove(int, int) {}
void iResize(int w, int h) {}
void iKeyboard(unsigned char key)
{
    if (key == 'Q') exit(0);
    flyChicken(game->chicken[0]);
    keyDown(game, key);
}

void iSpecialKeyboard(unsigned char key) { keyDown(game, key); }

void iKeyboardUp(unsigned char key) { keyUp(game, key); }
void iSpecialKeyboardUp(unsigned char key) { keyUp(game, key); }

int main()
{
    loadAssets();
    game = createGame();
    iSetTransparency(1);
    iInitializeEx(width, height, 0, "Catch the Egg");
    return 0;
}
