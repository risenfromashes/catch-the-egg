#include "ui.h"
#include "gamestate.h"
#include "leaderboard.h"

int width = 1280, height = 720;

typedef enum { NULL_UI, MAIN_UI, IN_GAME_UI, ENTER_NAME_UI, LEADERBOARD_UI } UIScreen;

Layout*    ui;
UIScreen   currentUI = NULL_UI;
UIScreen   activeUI  = MAIN_UI;
UIStyle    buttonStyle;
UIStyle    buttonLabelStyle;
UIStyle    iconButtonStyle;
SVGObject* bg;
SVGObject* closeIcon;
SVGObject* fullscreenIcon;
SVGObject* pauseIcon;
SVGObject *oneChicken, *twoChicken;
Div*       gameFormatMenu;
GameState* game     = NULL;
GameState* saveGame = NULL;
GameFormat lastFormat;
int        lastScore;
Label*     scoreLabel;
Label*     timeLabel;
int        isFullScreen = 0;

void addMainButtons();
void addGameSelectionButtons(Button* ngButton);
void addLeaderBoardSelectionButtons(Button* lbButton);
void mouseOverButton(UIStyle* style, int enter);
void mouseOverIconButton(UIStyle* style, int enter);
void handleNgButton(Button* b, int button, int state);
void handleLbButton(Button* b, int button, int state);
void handleGameSelection(Button* b, int button, int state);
void handleLeaderBoardSelection(Button* b, int button, int state);
void handleClose(Button* b, int button, int state);
void handleFullScreen(Button* b, int button, int state);
void handlePause(Button* b, int button, int state);
void addTable();
void addEditText();
void addGameOverlay();
void loadUIAssets();
void loadStyles();
void loadUI();
void updateGameUI();
void toggleFullScreen();

void iDraw()
{
    if (game != NULL && game == saveGame) {
        resumeGame(game);
        saveGame = NULL;
    }
    if (!game || !game->paused) {
        iClear();
        if (game) drawFrame(game);
        if (activeUI != currentUI) {
            if (ui) {
                clearLayout(ui);
                if (currentUI == MAIN_UI && gameFormatMenu) gameFormatMenu = NULL;
            }
            loadUI();
            currentUI = activeUI;
        }
        if (currentUI == IN_GAME_UI && game) updateGameUI();
        renderLayout(ui);
        if (game) {
            if (isFinished(game)) {
                lastFormat = game->format;
                lastScore  = game->score;
                freeGameState(game);
                game     = NULL;
                activeUI = ENTER_NAME_UI;
            }
        }
    }
}
void iMouseMove(int mx, int my) {}

void iMouse(int button, int state, int mx, int my)
{
    if (ui) handleMouseClick(ui, button, state, mx, my);
}
void iPassiveMouseMove(int mx, int my)
{
    if (ui) handlePassiveMouse(ui, mx, my);
}
void iResize(int w, int h) {}
void iKeyboard(unsigned char key)
{
    if (tolower(key) == 'f') toggleFullScreen();
    if (game) keyDown(game, key);
    if (ui) handleKeyInput(ui, key);
}

void iSpecialKeyboard(unsigned char key)
{
    if (game) keyDown(game, key);
}

void iKeyboardUp(unsigned char key)
{
    if (game) keyUp(game, key);
}
void iSpecialKeyboardUp(unsigned char key)
{
    if (game) keyUp(game, key);
}

void iExit()
{
    if (game) saveGameState(game);
}

int main()
{
    loadStyles();
    loadUIAssets();
    loadLeaderBoards();
    iSetTransparency(1);
    iInitializeEx(width, height, 0, "Catch the Egg");
    return 0;
}

void loadUI()
{
    ui = createLayout(0, 0, 1280, 720);
    switch (activeUI) {
        case MAIN_UI:
            addBackground(ui, bg);
            addMainButtons();
            break;
        case IN_GAME_UI: addGameOverlay(); break;
        case ENTER_NAME_UI:
            addBackground(ui, bg);
            addEditText();
            break;
        case LEADERBOARD_UI:
            addBackground(ui, bg);
            addTable();
            break;
    }
}

void loadUIAssets()
{
    bg             = SVGParse("assets/scene/background.svg");
    closeIcon      = SVGParse("assets/icons/close.svg");
    fullscreenIcon = SVGParse("assets/icons/fullscreen.svg");
    pauseIcon      = SVGParse("assets/icons/pause.svg");
    oneChicken     = SVGParse("assets/icons/one_chicken.svg");
    twoChicken     = SVGParse("assets/icons/two_chicken.svg");
}
void loadStyles()
{
    buttonStyle                     = defaultStyle();
    buttonLabelStyle                = defaultStyle();
    buttonLabelStyle.textFill.color = {255, 255, 255};
    buttonLabelStyle.fontSize       = FONT_MEDIUM;
    buttonStyle.flowDirection       = FLOW_ROW;
    buttonStyle.alignment.h         = ALIGN_HCENTER;
    buttonStyle.stroke.color        = {255, 255, 255};
    buttonStyle.margin.t            = 20;
    buttonStyle.padding             = {10, 12, 15, 15};
    buttonStyle.stroke.width        = 4;
    buttonStyle.onMouse             = mouseOverButton;

    iconButtonStyle              = defaultStyle();
    iconButtonStyle.fill.fill    = 1;
    iconButtonStyle.fill.opacity = 0.1;
    iconButtonStyle.margin       = {10, 10, 10, 10};
    iconButtonStyle.padding      = {5, 5, 5, 5};
    iconButtonStyle.onMouse      = mouseOverIconButton;
}
void toggleFullScreen()
{
    isFullScreen = !isFullScreen;
    if (isFullScreen) {
        glutDestroyWindow(glutGetWindow());
        glutGameModeString("1280x720");
        glutEnterGameMode();
        iInit();
    }
    else {
        glutLeaveGameMode();
        glutCreateWindow("Catch The Egg");
        iInit();
    };
}
void handleResumeSaveGame(Button* b, int button, int state)
{
    if (state == GLUT_DOWN) {
        assert(saveGame);
        game     = saveGame;
        activeUI = IN_GAME_UI;
    }
}

void addMainButtons()
{
    saveGame = loadGameState();
    Button* button[5];
    button[0]          = createButton("NEW GAME", NULL);
    button[0]->onClick = handleNgButton;
    if (saveGame) {
        button[1]          = createButton("RESUME GAME", NULL);
        button[1]->onClick = handleResumeSaveGame;
    }
    button[2]          = createButton("LEADERBOARD", NULL);
    button[2]->onClick = handleLbButton;
    button[3]          = createButton("HELP", NULL);
    button[4]          = createButton("EXIT", NULL);
    button[4]->onClick = handleClose;
    for (int i = 0; i < 5; i++) {
        if (button[i]) {
            button[i]->base.style        = buttonStyle;
            button[i]->label->base.style = buttonLabelStyle;
            if (i == 0) button[i]->base.style.margin.t = 200;
            addToLayout(ui, (Element*)button[i]);
        }
    }
}

void handleGameSelection(Button* b, int button, int state)
{
    if (state == GLUT_DOWN) {
        assert(!game);
        game     = createGame((GameFormat)b->base.data.iData);
        activeUI = IN_GAME_UI;
    }
}
void handleLeaderBoardSelection(Button* b, int button, int state)
{
    if (state == GLUT_DOWN) {
        lastFormat = (GameFormat)b->base.data.iData;
        activeUI   = LEADERBOARD_UI;
    }
}

void addSelectionButtons(Button* bfButton, int ng)
{
    UIStyle style                 = buttonStyle;
    UIStyle labelStyle            = buttonLabelStyle;
    style.margin.l                = 2;
    style.alignment.h             = ALIGN_LEFT;
    labelStyle.alignment.v        = ALIGN_VCENTER;
    Div* div                      = createDiv();
    div->base.style               = defaultStyle();
    div->base.style.alignment.h   = ALIGN_HCENTER;
    div->base.style.width         = 0;
    div->base.style.height        = 0;
    div->base.style.flowDirection = FLOW_ROW;
    Button* buttons[6];
    buttons[0]                  = createButton("1:30", oneChicken);
    buttons[0]->base.data.iData = ONE_THIRTY_X1;
    buttons[1]                  = createButton("1:30", twoChicken);
    buttons[1]->base.data.iData = ONE_THIRTY_X2;
    buttons[2]                  = createButton("2:00", oneChicken);
    buttons[2]->base.data.iData = TWO_ZERO_X1;
    buttons[3]                  = createButton("2:00", twoChicken);
    buttons[3]->base.data.iData = TWO_ZERO_X2;
    buttons[4]                  = createButton("2:30", oneChicken);
    buttons[4]->base.data.iData = TWO_THIRTY_X1;
    buttons[5]                  = createButton("2:30", twoChicken);
    buttons[5]->base.data.iData = TWO_THIRTY_X2;
    for (int i = 0; i < 6; i++) {
        buttons[i]->icon->base.style.width = 15 + (i % 2) * 15;
        buttons[i]->base.style             = style;
        buttons[i]->base.style.height      = 50;
        buttons[i]->label->base.style      = labelStyle;
        if (ng)
            buttons[i]->onClick = handleGameSelection;
        else
            buttons[i]->onClick = handleLeaderBoardSelection;
        addChild((Element*)div, (Element*)buttons[i]);
    }
    addAfter((Element*)bfButton, (Element*)div);
    gameFormatMenu = div;
}

void addGameSelectionButtons(Button* bfButton) { addSelectionButtons(bfButton, 1); }

void addLeaderBoardSelectionButtons(Button* bfButton) { addSelectionButtons(bfButton, 0); }

void mouseOverButton(UIStyle* style, int enter)
{
    if (enter) {
        style->fill.fill    = 1;
        style->fill.opacity = 0.25;
        style->fill.color   = {255, 255, 255};
        style->stroke.color = {255, 220, 212};
    }
    else {
        style->fill.fill    = 0;
        style->stroke.color = {255, 255, 255};
    }
}

void mouseOverTableRow(UIStyle* style, int enter)
{
    if (enter) {
        style->fill.fill    = 1;
        style->fill.opacity = 0.25;
        style->fill.color   = {255, 255, 255};
        style->stroke.color = {255, 255, 255};
    }
    else {
        style->fill.fill = 0;
    }
}

void handleNgButton(Button* b, int button, int state)
{
    if (state == GLUT_DOWN) {
        if (!gameFormatMenu) { addGameSelectionButtons(b); }
        else {
            removeElement((Element*)gameFormatMenu);
            gameFormatMenu = NULL;
        }
    }
}
void handleLbButton(Button* b, int button, int state)
{
    if (state == GLUT_DOWN) {
        if (!gameFormatMenu) { addLeaderBoardSelectionButtons(b); }
        else {
            removeElement((Element*)gameFormatMenu);
            gameFormatMenu = NULL;
        }
    }
}

void mouseOverIconButton(UIStyle* style, int enter)
{
    if (enter) { style->fill.opacity = 0.25; }
    else {
        style->fill.opacity = 0.1;
    }
}

void handleClose(Button* b, int button, int state)
{
    if (state == GLUT_DOWN) {
        switch (activeUI) {
            case MAIN_UI: exit(0); break;
            case IN_GAME_UI:
                if (game) {
                    saveGameState(game);
                    freeGameState(game);
                    game = NULL;
                }
                activeUI = MAIN_UI;
                break;
            case LEADERBOARD_UI: activeUI = MAIN_UI; break;
        }
    }
}
void handleFullScreen(Button* b, int button, int state)
{
    if (state == GLUT_DOWN) { toggleFullScreen(); }
}
void handlePause(Button* b, int button, int state)
{
    if (state == GLUT_DOWN) {
        assert(game && activeUI == IN_GAME_UI);
        if (game->paused)
            resumeGame(game);
        else
            pauseGame(game);
    }
}

Button* createCloseButton()
{
    Button* closeButton                  = createButton(NULL, closeIcon);
    closeButton->base.style              = iconButtonStyle;
    closeButton->base.style.alignment.h  = ALIGN_RIGHT;
    closeButton->onClick                 = handleClose;
    closeButton->icon->base.style.height = 30;
    closeButton->icon->base.style.width  = 30;
    closeButton->icon->opacity           = 0.5;
    return closeButton;
}

Button* createFullScreenButton()
{
    Button* fullscreenButton                  = createButton(NULL, fullscreenIcon);
    fullscreenButton->base.style              = iconButtonStyle;
    fullscreenButton->base.style.alignment.h  = ALIGN_RIGHT;
    fullscreenButton->base.style.alignment.v  = ALIGN_BOTTOM;
    fullscreenButton->onClick                 = handleFullScreen;
    fullscreenButton->icon->base.style.height = 30;
    fullscreenButton->icon->base.style.width  = 30;
    fullscreenButton->icon->opacity           = 0.5;
    return fullscreenButton;
}

Button* createPauseButton()
{
    Button* pauseButton                  = createButton(NULL, pauseIcon);
    pauseButton->base.style              = iconButtonStyle;
    pauseButton->base.style.alignment.h  = ALIGN_LEFT;
    pauseButton->base.style.alignment.v  = ALIGN_BOTTOM;
    pauseButton->base.style.padding      = {5, 5, 10, 10};
    pauseButton->onClick                 = handlePause;
    pauseButton->icon->base.style.height = 30;
    pauseButton->icon->base.style.width  = 20;
    pauseButton->icon->opacity           = 0.5;
    return pauseButton;
}

void addTable()
{
    Table*  table                    = createTable();
    Div*    title                    = createDiv();
    Label*  label                    = createLabel("LEADERBOARD");
    Button* closeButton              = createCloseButton();
    label->base.style.fontSize       = FONT_LARGE;
    label->base.style.textFill.color = {255, 255, 255};
    title->base.style.alignment.h    = ALIGN_HCENTER;
    title->base.style.margin.t       = 150;
    title->base.style.fill.fill      = 1;
    title->base.style.fill.color     = {92, 158, 196};
    title->base.style.fill.opacity   = 0.5;
    title->base.style.padding        = {10, 10, 30, 30};
    table->base.style.alignment.h    = ALIGN_HCENTER;
    table->base.style.margin.t       = 30;
    table->base.style.fill.fill      = 1;
    table->base.style.fill.color     = {191, 214, 217};
    table->base.style.fill.opacity   = 0.5;
    for (int i = 0; i < 11; i++) {
        TableRow* row = addTableRow(table);
        if (i > 0)
            row->base.style.onMouse = mouseOverTableRow;
        else {
            row->base.style.fill.fill      = 1;
            row->base.style.fill.color     = {92, 158, 196};
            table->base.style.fill.opacity = 0.25;
        }
        for (int j = 0; j < 3; j++) {
            char       num[16];
            TableData* data;
            if (i == 0) {
                switch (j) {
                    case 0: data = addTableData(row, "Rank"); break;
                    case 1: data = addTableData(row, "Name"); break;
                    case 2: data = addTableData(row, "Score"); break;
                }
            }
            else {
                switch (j) {
                    case 0:
                        itoa(i, num, 10);
                        data = addTableData(row, num);
                        break;
                    case 1: data = addTableData(row, leaderBoards[lastFormat].entries[i - 1].name); break;
                    case 2:
                        itoa(leaderBoards[lastFormat].entries[i - 1].score, num, 10);
                        data = addTableData(row, num);
                        break;
                }
            }
            data->base.style                      = defaultStyle();
            data->base.style.padding              = {5, 5, 20, 20};
            data->text->base.style                = defaultStyle();
            data->text->base.style.textFill.color = {255, 255, 255};
        }
    }
    addChild((Element*)title, (Element*)label);
    addToLayout(ui, (Element*)title);
    addToLayout(ui, (Element*)table);
    addToLayout(ui, (Element*)closeButton);
}

void enterPlayerName(const char* name)
{
    addToLeaderBoard(lastFormat, name, lastScore);
    activeUI = LEADERBOARD_UI;
}
void onKeyPlayerName(EditText* editText, unsigned char key)
{
    if (key == '\b')
        subChar(editText->text);
    else if (key == '\r') {
        if (editText->text->pos > 0) { enterPlayerName(editText->text->buffer); }
    }
    else if (editText->text->pos < 32)
        addChar(editText->text, key);
}

void addEditText()
{
    Div*      cont                            = createDiv();
    Div*      hLine                           = createDiv();
    EditText* editText                        = createEditText();
    Label*    label                           = createLabel("Enter your name:");
    cont->base.style.alignment.h              = ALIGN_HCENTER;
    cont->base.style.alignment.v              = ALIGN_VCENTER;
    cont->base.style.padding                  = {40, 40, 40, 40};
    cont->base.style.stroke.color             = {255, 255, 255};
    cont->base.style.stroke.width             = 4;
    label->base.style.fontSize                = FONT_LARGE;
    label->base.style.margin.b                = 20;
    label->base.style.alignment.h             = ALIGN_HCENTER;
    label->base.style.textFill.color          = {255, 255, 255};
    editText->base.style.alignment.h          = ALIGN_HCENTER;
    editText->base.style.padding              = {10, 10, 10, 10};
    editText->text->base.style.textFill.color = {255, 255, 255};
    editText->onKey                           = onKeyPlayerName;
    hLine->base.style.width                   = -1;
    hLine->base.style.height                  = 1;
    hLine->base.style.stroke.width            = 2;
    hLine->base.style.stroke.color            = {255, 255, 255};
    hLine->base.style.alignment.h             = ALIGN_HCENTER;
    addChild((Element*)cont, (Element*)label);
    addChild((Element*)cont, (Element*)editText);
    addChild((Element*)cont, (Element*)hLine);
    addToLayout(ui, (Element*)cont);
}

void addGameOverlay()
{
    UIStyle labelStyle        = defaultStyle();
    labelStyle.fontSize       = FONT_LARGE;
    labelStyle.textFill.color = {255, 255, 255};
    labelStyle.margin.t       = 10;
    Button* closeButton       = createCloseButton();
    Button* fullscreenButton  = createFullScreenButton();
    Button* pauseButton       = createPauseButton();
    timeLabel                 = createLabel("Time: 1:30");
    scoreLabel                = createLabel("Score: 100");
    scoreLabel->base.style = timeLabel->base.style = labelStyle;
    scoreLabel->base.style.margin.l                = 20;
    scoreLabel->base.style.alignment.h             = ALIGN_LEFT;
    timeLabel->base.style.alignment.h              = ALIGN_HCENTER;
    addToLayout(ui, (Element*)scoreLabel);
    addToLayout(ui, (Element*)timeLabel);
    addToLayout(ui, (Element*)closeButton);
    addToLayout(ui, (Element*)fullscreenButton);
    addToLayout(ui, (Element*)pauseButton);
}

void updateGameUI()
{
    assert(game);
    char str[32];
    sprintf(str, "Score: %d", game->score);
    updateText(scoreLabel, str);
    int rem = game->duration - floor(game->t - game->start_t);
    sprintf(str, "Time: %02d:%02d", rem / 60, rem % 60);
    updateText(timeLabel, str);
}