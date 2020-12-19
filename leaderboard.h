#pragma once

#include "gamestate.h"

typedef struct {
    char name[32];
    int  score;
} Entry;

typedef struct {
    Entry entries[10];
} LeaderBoard;

LeaderBoard leaderBoards[N_GAME_FORMAT];

void loadLeaderBoards()
{
    FILE* fp = fopen("leaderboard.dat", "rb");
    if (fp) {
        for (int i = 0; i < N_GAME_FORMAT; i++) {
            for (int j = 0; j < 10; j++) {
                fread(leaderBoards[i].entries[j].name, 1, 32, fp);
                fread(&leaderBoards[i].entries[j].score, sizeof(int), 1, fp);
            }
        }
        fclose(fp);
    }
    else {
        for (int i = 0; i < N_GAME_FORMAT; i++) {
            for (int j = 0; j < 10; j++) {
                strcpy(leaderBoards[i].entries[j].name, "-");
                leaderBoards[i].entries[j].score = 0;
            }
        }
    }
}

void saveLeaderBoards()
{
    FILE* fp = fopen("leaderboard.dat", "wb");
    for (int i = 0; i < N_GAME_FORMAT; i++) {
        for (int j = 0; j < 10; j++) {
            fwrite(leaderBoards[i].entries[j].name, 1, 32, fp);
            fwrite(&leaderBoards[i].entries[j].score, sizeof(int), 1, fp);
        }
    }
    fclose(fp);
}

void addToLeaderBoard(GameFormat format, const char* name, int score)
{
    int i = 9;
    if (score > leaderBoards[format].entries[i].score) {
        strcpy(leaderBoards[format].entries[i].name, name);
        leaderBoards[format].entries[i].score = score;
        while (--i >= 0) {
            if (score > leaderBoards[format].entries[i].score)
                swap(&leaderBoards[format].entries[i], &leaderBoards[format].entries[i + 1], sizeof(Entry));
            else
                break;
        }
        saveLeaderBoards();
    }
}