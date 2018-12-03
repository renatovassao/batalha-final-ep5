#include "../robot_fight.h"
#include <stdlib.h>


static int HAS_OUTPOST = 0;
static int NEAR_OUT_I = 500;
static int NEAR_OUT_J = 500;

// Busca o posto de controle mais perto da posicao p
void prepareGame(Grid *g, Position p, int turnCount) {
    setName("RENATO ROBOT");
    int i, j;
    for (i = 0; i < g->m; i++) {
        for (j = 0; j < g->n; j++) {
            if (g->map[i][j].isControlPoint) {
                if (abs(p.x - i) + abs(p.y - j) < abs(p.x - NEAR_OUT_I) + abs(p.y - NEAR_OUT_J)) {
                    NEAR_OUT_I = i;
                    NEAR_OUT_J = j;
                }
            }
        }
    }

    if (NEAR_OUT_I != 500)
        HAS_OUTPOST = 1;
}

/*Checa se a posicao dada esta dentro do mapa e nao esta sendo ocupada*/
int valid(Position p, int m, int n, Grid *g) {
    return p.x >= 0 && p.x < m && p.y >= 0 && p.y < n;
}

// Retorna a position movendo para a direction d na posicao p
Position fakeWalk(Position p, Direction d) {
    switch (d) {
        case LEFT:
            p.x--;
            break;
        case TOP_LEFT:
            if (p.y % 2 == 0) {
                p.x--;
                p.y--;
            } else
                p.y--;
            break;
        case TOP_RIGHT:
            if (p.y % 2 == 0) {
                p.y--;
            } else {
                p.x++;
                p.y--;
            }
            break;
        case RIGHT:
            p.x++;
            break;
        case BOTTOM_RIGHT:
            if (p.y % 2 == 0)
                p.y++;
            else {
                p.y++;
                p.y++;
            }
            break;
        case BOTTOM_LEFT:
            if (p.y % 2 == 0) {
                p.x--;
                p.y++;
            } else
                p.y++;
            break;
    }
    return p;
}

// Procura inimigo na direction d a partir da position p
int hasEnemyInDir(Grid *g, Position p, Direction d) {
    Position q = fakeWalk(p, d);

    while (valid(q, g->m, g->n, g)) {
        if (g->map[q.x][q.y].type == ROBOT) {
            return 1;
        }

        q = fakeWalk(q, d);
    }
        
    return 0;
}

Action processTurn(Grid *g, Position p, int turnsLeft) {
    // pega RENATO ROBOT
    Robot *r = &g->map[p.x][p.y].object.robot;

    // se esta tomando tiro, anda para frente
    int i;
    for (i = 0; i < 6; i++) {
        Position q = fakeWalk(p, i);
        if (valid(q, g->m, g->n, g) && g->map[q.x][q.y].type == PROJECTILE)
            return WALK;
    }

    // se esta em um obstaculo, anda para a frente ou gira a esquerda
    if (g->map[p.x][p.y].type == BLOCK) {
        if (valid(fakeWalk(p, r->dir), g->m, g->n, g))
            return WALK;

        return TURN_LEFT;
    }

    // Busca inimigo atras, para deixar obstaculo em cima dele
    if (r->obstacles > 0) {
        Position b_left, b_center, b_right;
        b_left = getNeighbor(p, (r->dir - 2) % 6);
        b_center = getNeighbor(p, (r->dir - 3) % 6);
        b_right = getNeighbor(p, (r->dir - 4) % 6);

        if (g->map[b_left.x][b_left.y].type == ROBOT) {
            return OBSTACLE_LEFT;
        }
        if (g->map[b_center.x][b_center.y].type == ROBOT) {
            return OBSTACLE_CENTER;
        }
        if (g->map[b_right.x][b_right.y].type == ROBOT) {
            return OBSTACLE_RIGHT;
        }
    }

    // Procura inimigo no centro, esquerda e direita, e atira se tiver balas e encontrar algum inimigo
    if (r->bullets > 0) {
        if (hasEnemyInDir(g, p, (r->dir - 1) % 6)) {
            return SHOOT_LEFT;
        }
        if (hasEnemyInDir(g, p, r->dir)) {
            return SHOOT_CENTER;
        }
        if (hasEnemyInDir(g, p, (r->dir + 1) % 6)) {
            return SHOOT_RIGHT;
        }
    }
    
    // Procura posto de controle
    if (HAS_OUTPOST) {
        if (NEAR_OUT_I == p.x && NEAR_OUT_J == p.y) {
            return STAND;
        }

        // simula andar para frente
        Position q = fakeWalk(p, r->dir);

        // se andar para frente diminui a distancia para ponto de controle mais proximo, anda para frente
        if (abs(q.x - NEAR_OUT_I) + abs(q.y - NEAR_OUT_J) < abs(p.x - NEAR_OUT_I) + abs(p.y - NEAR_OUT_J)) {
            return WALK;

        }
    }
    
    // Vira para esquerda para ver se na proxima rodada consegue outra acao
    return TURN_LEFT;
}
