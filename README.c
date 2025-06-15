
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PLAYERS 4
#define MAX_HISTORY 100
#define MAX_FAN_NAME 32
#define MAX_FAN_TYPES 32

typedef struct {
    char name[16];
    int score;
} Player;

typedef struct {
    char id[16];
    char name[32];
    int fan;
    char group[16];
} FanType;

typedef struct {
    int winner;
    int loser; // -1 表示自摸
    int dealer;
    int totalFan;
    int totalMoney;
    int selfDraw;
    int consecutiveWins;
    char timestamp[32];
    char fanTypes[10][MAX_FAN_NAME];
} RoundRecord;

typedef struct {
    Player players[MAX_PLAYERS];
    int dealer;
    int consecutiveWins;
    int baseFan;
    int moneyPerFan;
    RoundRecord history[MAX_HISTORY];
    int historyCount;
} GameState;

GameState game;

FanType fanList[MAX_FAN_TYPES] = {
    {"menqing", "門清", 1, ""},
    {"buqiuren", "不求人", 1, ""},
    {"fengpai", "風牌", 1, ""},
    {"sanyuan", "三元牌", 1, ""},
    {"huapai", "花牌", 1, ""},
    {"gangshang", "槓上開花", 1, ""},
    {"bianzhang", "邊張", 1, "waiting"},
    {"zhongdong", "中洞", 1, "waiting"},
    {"dandiao", "單吊", 1, "waiting"},
    {"sananke", "三暗刻", 2, "anke"},
    {"pinghu", "平胡", 2, ""},
    {"quanqiren", "全求人", 2, ""},
    {"hunyise", "混一色", 4, "color"},
    {"pengpenghu", "碰碰胡", 4, ""},
    {"diting", "地聽", 4, ""},
    {"xiaosanyuan", "小三元", 4, ""},
    {"sianke", "四暗刻", 5, "anke"},
    {"wuanke", "五暗刻", 8, "anke"},
    {"qingyise", "清一色", 8, "color"},
    {"ziyise", "字一色", 8, "color"},
    {"dasanyuan", "大三元", 8, ""},
    {"xiaosxi", "小四喜", 8, ""},
    {"baxian", "八仙過海", 8, ""},
    {"tianting", "天聽", 8, ""},
    {"tianhu", "天胡", 16, "special"},
    {"dihu", "地胡", 16, "special"},
    {"renhu", "人胡", 16, "special"},
    {"dasixi", "大四喜", 16, ""}
};
int fanListSize = 28;

void getTimestamp(char* buf, size_t len) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buf, len, "%Y-%m-%d %H:%M:%S", t);
}

void initGame() {
    const char* names[] = { "東", "南", "西", "北" };
    for (int i = 0; i < MAX_PLAYERS; i++) {
        strcpy(game.players[i].name, names[i]);
        game.players[i].score = 0;
    }
    game.dealer = 0;
    game.consecutiveWins = 0;
    game.baseFan = 3;
    game.moneyPerFan = 10;
    game.historyCount = 0;
}

int computeFanScore(char fanNames[][MAX_FAN_NAME], int count) {
    int total = 0;
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < fanListSize; j++) {
            if (strcmp(fanNames[i], fanList[j].name) == 0) {
                total += fanList[j].fan;
            }
        }
    }
    return total;
}

void applyRound(int winner, int selfDraw, int loser, char fanNames[][MAX_FAN_NAME], int fanCount) {
    RoundRecord* r = &game.history[game.historyCount++];
    r->winner = winner;
    r->selfDraw = selfDraw;
    r->loser = selfDraw ? -1 : loser;
    r->dealer = game.dealer;
    r->consecutiveWins = game.consecutiveWins;
    r->totalFan = game.baseFan + computeFanScore(fanNames, fanCount) + (selfDraw ? 1 : 0) + (winner == game.dealer ? (1 + game.consecutiveWins) : 0);
    r->totalMoney = r->totalFan * game.moneyPerFan;
    getTimestamp(r->timestamp, sizeof(r->timestamp));

    for (int i = 0; i < fanCount; i++) {
        strncpy(r->fanTypes[i], fanNames[i], MAX_FAN_NAME);
    }

    if (selfDraw) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (i == winner)
                game.players[i].score += r->totalMoney * 3;
            else
                game.players[i].score -= r->totalMoney;
        }
    } else {
        game.players[winner].score += r->totalMoney;
        game.players[loser].score -= r->totalMoney;
    }

    if (winner == game.dealer) {
        game.consecutiveWins++;
    } else {
        game.dealer = winner;
        game.consecutiveWins = 0;
    }
}

void printStats() {
    printf("\n=== 玩家統計 ===\n");
    for (int i = 0; i < MAX_PLAYERS; i++) {
        printf("%s：%d 元\n", game.players[i].name, game.players[i].score);
    }
}

void printHistory() {
    printf("\n=== 歷史紀錄 ===\n");
    for (int i = 0; i < game.historyCount; i++) {
        RoundRecord* r = &game.history[i];
        printf("第%d局：%s %s %d台 %d元 %s %s\n", i + 1,
            game.players[r->winner].name,
            r->selfDraw ? "自摸" : "放槍",
            r->totalFan,
            r->totalMoney,
            r->selfDraw ? "" : game.players[r->loser].name,
            r->timestamp
        );
    }
}

int main() {
    initGame();
    char fans1[][MAX_FAN_NAME] = {"清一色", "三暗刻"};
    applyRound(0, 1, -1, fans1, 2); // 東自摸

    char fans2[][MAX_FAN_NAME] = {"碰碰胡"};
    applyRound(2, 0, 1, fans2, 1); // 西放槍南

    printHistory();
    printStats();
    return 0;
}
