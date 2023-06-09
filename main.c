#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <time.h>   // for nanosleep
#endif

typedef enum {
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
} direction_t;

typedef struct {
    int x;
    int y;
} point_t;

typedef struct El {
    direction_t original_dir;
    int x;
    int y;
    struct El *next;
} queue_el_t;

typedef struct {
    queue_el_t *head;
    queue_el_t *tail;
} queue_t;

#define MAX_SIZE (32)

bool grid[MAX_SIZE][MAX_SIZE];
point_t goals[9], players[9];
direction_t blockers[9];
bool finished[9];
size_t goals_n = 0, players_n = 0;
size_t grid_w = 0, grid_h = 0;
bool visited[MAX_SIZE][MAX_SIZE];

void queue_add(queue_t *queue, direction_t original_dir, int x, int y) {
    queue_el_t *el = malloc(sizeof *el);
    el->x = x;
    el->y = y;
    el->original_dir = original_dir;

    if (queue->head == NULL) {
        queue->head = queue->tail = el;
    }
    else {
        queue->tail->next = el;
        queue->tail = el;
    }
}

queue_el_t queue_remove(queue_t *queue) {
    queue_el_t res = *queue->head;
    free(queue->head);

    if (queue->head == queue->tail) {
        queue->head = queue->tail = NULL;
    }
    else {
        queue->head = res.next;
    }

    res.next = NULL;
    return res;
}

bool next_to(int x, int y, int goal_x, int goal_y) {
    if (x == goal_x && y == goal_y) return true;
    if (x + 1 == goal_x && y == goal_y) return true;
    if (x - 1 == goal_x && y == goal_y) return true;
    if (x == goal_x && y + 1 == goal_y) return true;
    if (x == goal_x && y - 1 == goal_y) return true;
    return false;
}

bool next_to_goal(int x, int y, int goal) {
    return next_to(x, y, goals[goal].x, goals[goal].y);
}

bool occupied(int x, int y, bool inc_players) {
    if (x < 0 || x >= grid_w || y < 0 || y > grid_h) return true;
    if (grid[x][y] || visited[x][y]) return true;

    return false;
}

bool try_move(int player, direction_t dir) {
    point_t pos = players[player];

    int x = pos.x;
    int y = pos.y;

    switch (dir) {
        case UP: y--; break;
        case DOWN: y++; break;
        case LEFT: x--; break;
        case RIGHT: x++; break;
        default: return false;
    }

    if (occupied(x, y, true)) return false;
    for (size_t i = 0; i < players_n; i++) {
        if (players[i].x == x && players[i].y == y) {
            blockers[i] = dir;
            return false;
        }
    }
    if (next_to_goal(x, y, player)) finished[player] = true;
    players[player] = (point_t) { x, y };
    return true;
}

direction_t get_direction(int goal_x, int goal_y, int x, int y) {
    queue_t queue = { 0 };
    memset(visited, 0, sizeof visited);
    visited[x][y] = true;

    if (next_to(x, y, goal_x, goal_y)) return NONE;
    queue_add(&queue, RIGHT, x + 1, y);
    queue_add(&queue, LEFT, x - 1, y);
    queue_add(&queue, DOWN, x, y + 1);
    queue_add(&queue, UP, x, y - 1);

    while (queue.head) {
        queue_el_t el = queue_remove(&queue);
        if (occupied(el.x, el.y, false)) continue;
        if (next_to(el.x, el.y, goal_x, goal_y)) return el.original_dir;
        visited[el.x][el.y] = true;

        queue_add(&queue, el.original_dir, el.x + 1, el.y);
        queue_add(&queue, el.original_dir, el.x - 1, el.y);
        queue_add(&queue, el.original_dir, el.x, el.y + 1);
        queue_add(&queue, el.original_dir, el.x, el.y - 1);
    }

    return NONE;
}

void redraw() {
    #ifdef WIN32
    system("cls");
    #else
    system("clear");
    #endif
    for (int y = 0; y < grid_h; y++) {
        for (int x = 0; x < grid_w; x++) {
            for (size_t i = 0; i < players_n; i++) {
                if (players[i].x == x && players[i].y == y) {
                    printf("%zu", i + 1);
                    goto end_loop;
                }
            }
            for (size_t i = 0; i < goals_n; i++) {
                if (goals[i].x == x && goals[i].y == y) {
                    printf("%c", "ABCDEFGHIJ"[i]);
                    goto end_loop;
                }
            }

            if (grid[x][y]) printf("#");
            else printf(" ");
            end_loop: ;
        }
        printf("\n");
    }
}

bool update() {
    bool moved[9] = { 0 };

    bool all_done = true;
    bool deadlock = true;

    bool cont = true;

    while (cont) {
        cont = false;
        for (size_t i = 0; i < players_n; i++) {
            if (next_to(players[i].x, players[i].y, goals[i].x, goals[i].y)) finished[i] = true;
            if (moved[i]) continue;

            if (blockers[i]) {
                direction_t dir = blockers[i];
                direction_t it = dir;

                while (true) {
                    it = (it) % 4 + 1;
                    if (try_move(i, it)) {
                        blockers[i] = NONE;
                        cont = true;
                        break;
                    }
                    if (it == dir) break;
                }
            }
            else if (!finished[i]) {
                all_done = false;
                direction_t dir = get_direction(goals[i].x, goals[i].y, players[i].x, players[i].y);
                memset(visited, 0, sizeof visited);
                if (dir == NONE) continue;

                if (try_move(i, dir)) {
                    deadlock = false;
                    moved[i] = true;
                }
                else cont = true;
            }
        }
    }

    if (all_done) {
        fprintf(stderr, "All agents have found their targets!\n");
        exit(0);
    }
    if (deadlock) {
        fprintf(stderr, "Some agents got stuck :(\n");
        exit(0);
    }
}

void load(FILE *f) {
    grid_w = grid_h = goals_n = players_n = 0;
    int x = 0, y = 0, c = 0;
    memset(grid, 0, sizeof(grid));

    while (true) {
        while (true) {
            char c = getc(f);
            if (c == -1) {
                if (y >= grid_h) grid_h++;
                return;
            }
            if (c == '\r') { x--; continue; }
            if (c == '\n') break;
            if (x >= MAX_SIZE || y >= MAX_SIZE) continue;

            grid[x][y] = false;
            if (c == '#') grid[x][y] = true;
            if (c >= '1' && c <= '9') {
                int i = c - '1';
                players[i] = (point_t) { x, y };
                if (i + 1 > players_n) players_n = i + 1;
            }
            if (c >= 'A' && c <= 'I') {
                int i = c - 'A';
                goals[i] = (point_t) { x, y };
                if (i + 1 > goals_n) goals_n = i + 1;
            }

            if (++x > grid_w) grid_w = x;
        }
        x = 0;
        if (++y > grid_h) grid_h = y;
    }
}

int main(int argc, char **argv) {
    int sleepN = 1000;
    if (argc < 2) {
        fprintf(stderr, "Invalid arguments supplied! Syntax: <map-file> [interval=1000]\n");
        exit(1);
    }

    if (argc >= 3) sleepN = atoi(argv[2]);

    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        fprintf(stderr, "The provided map file doesn't exist.\n");
        exit(1);
    }
    load(f);
    while (true) {
        redraw();
        #ifdef WIN32
            Sleep(sleepN);
        #else
            struct timespec ts;
            ts.tv_sec = sleepN / 1000;
            ts.tv_nsec = (sleepN % 1000) * 1000000;
            nanosleep(&ts, NULL);
        #endif
        update();
    }
}
