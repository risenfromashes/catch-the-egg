#include "scratchHeader.h"

Sphere   InitialBall, Ball1, Ball2, Ball3;
Point    previous_center_1, previous_center_2, previous_center_3;
Paddle   CurrentPaddle, InitialPaddle;
Velocity InitialVelocity, Velocity1, Velocity2, Velocity3;
Powerup  AllPowerups[25];

int    score            = 0;
int    life             = 3;
int    isCompleted      = 0;
int    isThruBall       = 0;
int    AreBricksFalling = 0;
char   name[620];
char   shownScore[200];
Brick  AllBricks[total_bricks];
int    shouldRestart      = 0;
int    isSoundOn          = 0;
double speed_change       = 1.025;
double InitialSpeedChange = 1.025;
double reference_x, reference_y; // for setting the bricks
int    page;

void InitializePaddle(void);
void drawPaddle(void);
void InitializeBall(void);
void drawBall(void);
void InitializeBricks(void);
void drawBricks(void);
void BallPositionChange(double);
void reStart(void);
void BallSpeedChange();
void InitializePowerups(void);
void drawPowerups(void);
void ChangePowerupPosition(double);
void ApplyPowerups(Powerup* this_powerup);
void HandlePowerups(void);
void FallBricks(void);
enum Sounds { SOUND_HIT_NORMAL_BRICK, SOUND_DROP_NORMAL };
char* sounds[20];
void  iDraw()
{
    static int f = 1;
    if (f) {
        glutPassiveMotionFunc(mousePassiveMoveHandlerFF);
        f = 0;
    }
    static double t0 = getTime();
    double        dt = getTime() - t0;
    iClear();
    BallPositionChange(dt);
    ChangePowerupPosition(dt);
    // BallSpeedChange();
    drawPaddle();
    drawBall();
    drawBricks();
    drawPowerups();
    t0 = t0 + dt;
    return;
}
void iMouseMove(int mx, int my) { return; }
void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (!Ball1.isMoving) Ball1.isMoving = 1;
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {}
    return;
}
void iPassiveMouseMove(int mx, int my)
{
    CurrentPaddle.left = mx - CurrentPaddle.dx / 2 - CurrentPaddle.side_circle_radius;
    if (CurrentPaddle.left < CurrentPaddle.side_circle_radius) CurrentPaddle.left = CurrentPaddle.side_circle_radius;
    if (CurrentPaddle.left > window_width - CurrentPaddle.dx - CurrentPaddle.side_circle_radius)
        CurrentPaddle.left = window_width - CurrentPaddle.dx - CurrentPaddle.side_circle_radius;
    if (Ball1.isActivated && Ball1.isMoving == 0) { Ball1.center.x = CurrentPaddle.left + CurrentPaddle.dx / 2; }
    return;
}
void iKeyboard(unsigned char key)
{
    switch (key) {
        case ' ':
            if (!Ball1.isMoving) Ball1.isMoving = 1;
            break;
        default: break;
    }
}
void iSpecialKeyboard(unsigned char key)
{
    switch (key) {
        case GLUT_KEY_LEFT:
            CurrentPaddle.left -= 30;
            if (CurrentPaddle.left - CurrentPaddle.side_circle_radius < 0)
                CurrentPaddle.left = CurrentPaddle.side_circle_radius;
            break;
        case GLUT_KEY_RIGHT:
            CurrentPaddle.left += 30;
            if (CurrentPaddle.left + CurrentPaddle.dx + CurrentPaddle.side_circle_radius > window_width)
                CurrentPaddle.left = window_width - CurrentPaddle.side_circle_radius - CurrentPaddle.dx;
            break;
        default: break;
    }
}
void LoadSounds()
{
    readFileToMemory("music\\hitnormalbrick.wav", &sounds[SOUND_HIT_NORMAL_BRICK]);
    readFileToMemory("music\\dropnormal.wav", &sounds[SOUND_DROP_NORMAL]);
}
int main()
{
    LoadSounds();
    InitializePaddle();
    InitializeBall();
    InitializeBricks();
    InitializePowerups();
    if (isSoundOn) PlaySound("music\\Seasons converted.wav", NULL, SND_ASYNC | SND_LOOP);
    iInitialize(1280, 720, "DxBall");
    return 0;
}

void InitializePaddle()
{
    InitialPaddle.left               = window_width / 2;
    InitialPaddle.bottom             = bottom_margin;
    InitialPaddle.dx                 = 90;
    InitialPaddle.dy                 = 15;
    InitialPaddle.side_circle_radius = 5;
    CurrentPaddle                    = InitialPaddle;
    return;
}
void drawPaddle()
{
    iSetColor(255, 255, 255);
    iFilledRectangle(CurrentPaddle.left, CurrentPaddle.bottom, CurrentPaddle.dx, CurrentPaddle.dy);
    iFilledCircle(CurrentPaddle.left, CurrentPaddle.bottom + CurrentPaddle.dy / 2, CurrentPaddle.side_circle_radius);
    iFilledCircle(CurrentPaddle.left + CurrentPaddle.dx,
                  CurrentPaddle.bottom + CurrentPaddle.dy / 2,
                  CurrentPaddle.side_circle_radius,
                  1000);
    return;
}
void InitializeBall()
{
    InitialBall.radius      = 10;
    InitialBall.center.x    = CurrentPaddle.left + CurrentPaddle.dx / 2;
    InitialBall.center.y    = CurrentPaddle.bottom + CurrentPaddle.dy + InitialBall.radius;
    InitialBall.isActivated = 1;
    InitialBall.isMoving    = 0;
    Ball1                   = InitialBall;
    Ball2                   = InitialBall;
    Ball3                   = InitialBall;
    previous_center_1       = InitialBall.center;
    Ball2.isActivated       = 0;
    Ball3.isActivated       = 0;
    InitialVelocity.dx      = 0;
    InitialVelocity.dy      = 5;
    Velocity1               = InitialVelocity;
    Velocity2               = InitialVelocity;
    Velocity3               = InitialVelocity;
    return;
}
void drawBall()
{
    if (Ball1.isActivated) {
        iSetColor(255, 0, 0);
        iFilledCircle(Ball1.center.x, Ball1.center.y, Ball1.radius);
    }
    if (Ball2.isActivated) {
        iSetColor(255, 0, 0);
        iFilledCircle(Ball2.center.x, Ball2.center.y, Ball2.radius);
    }
    if (Ball3.isActivated) {
        iSetColor(255, 0, 0);
        iFilledCircle(Ball3.center.x, Ball3.center.y, Ball3.radius);
    }
    return;
}
void BallPositionChange(double dt)
{
    if (Ball1.isActivated) {
        if (!Ball1.isMoving) return;
        Ball1.center.x += Velocity1.dx * dt / 0.015;
        Ball1.center.y += Velocity1.dy * dt / 0.015;
        int doesHit;
        // brick collision checking
        if (Ball1.center.x + Ball1.radius >= reference_x && Ball1.center.y - Ball1.radius <= reference_y) {
            for (int i = 0; i < total_bricks; i++) {
                Brick* now = &(AllBricks[i]);
                doesHit    = doesBallHitBrick(now, &Ball1, &previous_center_1);
                if (doesHit == -1)
                    continue;
                else if (doesHit == 1 || doesHit == 2) {
                    Velocity1.dx *= -1;
                    if (isThruBall) Velocity1.dx *= -1;
                }
                else if (doesHit == 3 || doesHit == 4) {
                    Velocity1.dy *= -1;
                    if (isThruBall) Velocity1.dy *= -1;
                }
                // if no powerup
                if (now->PowerUpAssociation == -1
                    /*&& isSoundOn*/)
                    PlaySound((LPCSTR)sounds[SOUND_HIT_NORMAL_BRICK], NULL, SND_MEMORY | SND_ASYNC);
                else {
                    if (isSoundOn) PlaySound("music\\withpowerup.wav", NULL, SND_ASYNC);
                    AllPowerups[now->PowerUpAssociation].state = 1;
                }
                now->isShown = 0;
                break;
            }
        }
        // paddle collision
        if (!shouldRestart && Ball1.center.y - Ball1.radius <= CurrentPaddle.bottom + CurrentPaddle.dy) {
            if (Ball1.center.x + Ball1.radius >= CurrentPaddle.left - CurrentPaddle.side_circle_radius &&
                Ball1.center.x - Ball1.radius <=
                    CurrentPaddle.left + CurrentPaddle.dx + CurrentPaddle.side_circle_radius) {
                Velocity1.dy *= -1;
                Velocity1.dx = ((Ball1.center.x - (CurrentPaddle.left + CurrentPaddle.dx / 2)) /
                                (CurrentPaddle.dx / 2 + CurrentPaddle.side_circle_radius)) *
                               Velocity1.dy;
                if (Ball1.radius < 15.9 /*&& isSoundOn */) {
                    PlaySound((LPCSTR)sounds[SOUND_DROP_NORMAL], NULL, SND_MEMORY | SND_ASYNC);
                }
                else {
                    if (isSoundOn) PlaySound("music\\largeballdrop.wav", NULL, SND_ASYNC);
                }
                if (AreBricksFalling) FallBricks();
            }
            else
                shouldRestart = 1;
        }
        // left or right wall
        if (Ball1.center.x >= window_width - Ball1.radius || Ball1.center.x <= Ball1.radius) {
            Velocity1.dx *= -1;
            // if (isSoundOn) PlaySound("music\\wallhit.wav", NULL, SND_ASYNC);
        }
        // top wall
        if (Ball1.center.y >= window_height - Ball1.radius) {
            Velocity1.dy *= -1;
            // if (isSoundOn) PlaySound("music\\wallhit.wav", NULL, SND_ASYNC);
        }
        HandlePowerups();
        if (Ball1.center.x >= window_width - Ball1.radius)
            Ball1.center.x -= (Ball1.center.x - window_width + Ball1.radius);
        if (Ball1.center.x <= Ball1.radius) Ball1.center.x += (Ball1.radius - Ball1.center.x);
        if (Ball1.center.y >= window_height - Ball1.radius)
            Ball1.center.y -= (Ball1.center.y - window_height + Ball1.radius);
        if (Ball1.center.y <= bottom_margin + Ball1.radius + CurrentPaddle.dy) {
            if (!shouldRestart) Ball1.center.y += (bottom_margin + Ball1.radius + CurrentPaddle.dy - Ball1.center.y);
        }
        previous_center_1 = Ball1.center;
        if (Ball1.center.y + Ball1.radius <= 0) {
            reStart();
            if (isSoundOn) PlaySound("music\\balldrop.wav", NULL, SND_ASYNC);
        }
    }
    if (Ball2.isActivated) {}
    if (Ball3.isActivated) {}
    return;
}
void InitializeBricks(void)
{
    reference_x = 50;
    reference_y = window_height - 200;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            int    now_index               = i * col + j;
            Brick* this_brick              = AllBricks + now_index;
            this_brick->dx                 = BrickLength;
            this_brick->dy                 = BrickHeight;
            this_brick->temp_color         = {rand() % 256, rand() % 256, rand() % 256};
            this_brick->left               = reference_x + j * BrickLength;
            this_brick->bottom             = reference_y - i * BrickHeight;
            this_brick->isShown            = 1;
            this_brick->type               = 1;
            this_brick->PowerUpAssociation = -1;
        }
    }
    return;
}
void drawBricks()
{
    for (int i = 0; i < total_bricks; i++) {
        if (AllBricks[i].isShown) {
            Brick* now = &(AllBricks[i]);
            iSetColor(now->temp_color.red, now->temp_color.green, now->temp_color.blue);
            iFilledRectangle(now->left, now->bottom, now->dx, now->dy);
        }
    }
    return;
}
void reStart()
{
    shouldRestart = 0;
    life--;
    if (life < 0) Ball1.isActivated = 0;
    isThruBall       = 0;
    AreBricksFalling = 0;
    CurrentPaddle    = InitialPaddle;
    Ball1            = InitialBall;
    Velocity1        = InitialVelocity;
    speed_change     = InitialSpeedChange;
    for (int i = 0; i < totalpowerups; i++) {
        if (AllPowerups[i].state == 1) {
            AllPowerups[i].state    = 2;
            AllPowerups[i].isCaught = 0;
        }
    }
    return;
}
void BallSpeedChange()
{
    static double t0 = getTime();
    double        t  = getTime();
    if (t - t0 >= 5.0) {
        if (Ball1.isActivated && Ball1.isMoving) {
            Velocity1.dx *= speed_change;
            Velocity1.dy *= speed_change;
        }
        if (Ball2.isActivated && Ball2.isMoving) {
            Velocity2.dx *= speed_change;
            Velocity2.dy *= speed_change;
        }
        if (Ball3.isActivated && Ball3.isMoving) {
            Velocity3.dx *= speed_change;
            Velocity3.dy *= speed_change;
        }
        t0 = t;
    }
    return;
}
void InitializePowerups()
{
    for (int i = 0; i < totalpowerups; i++) {
        Powerup* this_powerup                   = &(AllPowerups[i]);
        this_powerup->index                     = 4 * i + 1; // so that one brick does not have more than one
        AllBricks[4 * i + 1].PowerUpAssociation = i;
        this_powerup->left                      = AllBricks[this_powerup->index].left;
        this_powerup->bottom                    = AllBricks[this_powerup->index].bottom - PowerUpHeight;
        this_powerup->isCaught                  = 0;
        this_powerup->state                     = 0;
        this_powerup->type                      = rand() % 11;
    }
    return;
}
void ApplyPowerups(Powerup* this_powerup)
{
    int type = this_powerup->type;
    if (type == 0) { // extra life
        if (isSoundOn) PlaySound("music\\positivecaught.wav", NULL, SND_ASYNC);
        if (life < 5) life++;
    }
    if (type == 1) { // wider paddle
        if (isSoundOn) PlaySound("music\\largethings.wav", NULL, SND_ASYNC);
        if (10 + CurrentPaddle.dx + 2 * CurrentPaddle.side_circle_radius <= window_width / 2) CurrentPaddle.dx += 10;
    }
    if (type == 2) { // faster ball
        if (isSoundOn) PlaySound("music\\fallbrick.wav", NULL, SND_ASYNC);
        if (speed_change <= 2.25) speed_change *= 1.25;
    }
    if (type == 3) { // thru ball
        if (isSoundOn) PlaySound("music\\positivecaught.wav", NULL, SND_ASYNC);
        isThruBall = 1;
    }
    if (type == 4) { // slower ball
        if (speed_change >= 1.000025) speed_change /= 5;
    }
    if (type == 5) { // shrink ball
        if (isSoundOn) PlaySound("music\\fallbrick.wav", NULL, SND_ASYNC);
        Ball1.radius = 3;
    }
    if (type == 6) { // shrink paddle
        if (CurrentPaddle.dx >= 30) CurrentPaddle.dx -= 15;
    }
    if (type == 7) { // supershrink paddle
        CurrentPaddle.dx = 20;
    }
    if (type == 8) { // mega ball
        if (isSoundOn) PlaySound("music\\largethings.wav", NULL, SND_ASYNC);
        Ball1.radius = 16;
    }
    if (type == 9) { // death
        reStart();
        if (isSoundOn) PlaySound("music\\balldrop.wav", NULL, SND_ASYNC);
        if (life < 0) {}
    }
    if (type == 10) { // falling bricks
        if (isSoundOn) PlaySound("music\\fallbrick.wav", NULL, SND_ASYNC);
        AreBricksFalling = 1;
    }
    return;
}
const char PowerupImageName[][35] = {"powerups\\extralife.bmp",
                                     "powerups\\widepaddle.bmp",
                                     "powerups\\fastball.bmp",
                                     "powerups\\thruball.bmp",
                                     "powerups\\slowball.bmp",
                                     "powerups\\shrinkball.bmp",
                                     "powerups\\shrinkpaddle.bmp",
                                     "powerups\\supershrinkpaddle.bmp",
                                     "powerups\\megaball.bmp",
                                     "powerups\\death.bmp",
                                     "powerups\\fallingbricks.bmp"};
void       drawPowerups()
{
    for (int i = 0; i < totalpowerups; i++) {
        Powerup* this_powerup = &(AllPowerups[i]);
        if (this_powerup->state == 0 || this_powerup->state == 2) continue;
        iShowBMP2(this_powerup->left, this_powerup->bottom, PowerupImageName[this_powerup->type], 0x00ffffff);
    }
    return;
}
void ChangePowerupPosition(double dt)
{
    for (int i = 0; i < totalpowerups; i++) {
        Powerup* this_powerup = &(AllPowerups[i]);
        if (this_powerup->state == 1) {
            this_powerup->bottom -= 5 * dt / 0.015;
            if (this_powerup->bottom <= 0) {
                this_powerup->state = 2 * dt / 0.015;
                puts("Powerup Missed");
            }
        }
    }
    return;
}
void HandlePowerups()
{
    for (int i = 0; i < totalpowerups; i++) {
        Powerup* this_powerup = &(AllPowerups[i]);
        if (this_powerup->state == 1) {
            // either left or right bounday has to stay inside
            if (this_powerup->bottom > CurrentPaddle.bottom + CurrentPaddle.dy) continue;
            if ((this_powerup->left >= CurrentPaddle.left - CurrentPaddle.side_circle_radius &&
                 this_powerup->left <= CurrentPaddle.left + CurrentPaddle.dx + CurrentPaddle.side_circle_radius) ||
                this_powerup->left + PowerUpWidth >= CurrentPaddle.left - CurrentPaddle.side_circle_radius &&
                    this_powerup->left + PowerUpWidth <=
                        CurrentPaddle.left + CurrentPaddle.dx + CurrentPaddle.side_circle_radius) {
                puts("Powerup Caught");
                this_powerup->isCaught = 1;
                this_powerup->state    = 2;
                ApplyPowerups(this_powerup);
            }
        }
    }
    return;
}
void FallBricks()
{
    if (isSoundOn) PlaySound("music\\fallbrick.wav", NULL, SND_ASYNC);
    if (reference_y - BrickHeight * (row - 1) - (CurrentPaddle.bottom + CurrentPaddle.dy) <= 50) return;
    for (int i = 0; i < total_bricks; i++) {
        AllBricks[i].bottom -= 15;
        if (AllBricks[i].PowerUpAssociation != -1) { AllPowerups[AllBricks[i].PowerUpAssociation].bottom -= 15; }
    }
    return;
}