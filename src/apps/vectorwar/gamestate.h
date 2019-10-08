#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_

/*
 * gamestate.h --
 *
 * Encapsulates all the game state for the vector war application inside
 * a single structure.  This makes it trivial to implement our GGPO
 * save and load functions.
 */

#define PI                    ((double)3.1415926)
#define STARTING_HEALTH       100
#define ROTATE_INCREMENT        3
#define SHIP_RADIUS            15
#define SHIP_WIDTH              8
#define SHIP_TUCK               3
#define SHIP_THRUST             0.06
#define SHIP_MAX_THRUST         4.0
#define SHIP_BREAK_SPEED        0.6
#define BULLET_SPEED            5
#define MAX_BULLETS             30
#define BULLET_COOLDOWN         8
#define BULLET_DAMAGE           10

#define MAX_SHIPS               4

struct Position {
   double x, y;
};

struct Velocity {
   double dx, dy;
};

struct Bullet {
   bool     active;
   Position position;
   Velocity velocity;
};

struct Ship {
   Position position;
   Velocity velocity;
   int      radius;
   int      heading;
   int      health;
   int      speed;
   int      cooldown;
   Bullet   bullets[MAX_BULLETS];
   int      score;
};

struct GameState {
   void Init(HWND hwnd, int num_players);
   void GetShipAI(int i, double *heading, double *thrust, int *fire);
   void ParseShipInputs(int inputs, int i, double *heading, double *thrust, int *fire);
   void MoveShip(int i, double heading, double thrust, int fire);
   void Update(int inputs[], int disconnect_flags);

   int         _framenumber;
   RECT        _bounds;
   int         _num_ships;
   Ship        _ships[MAX_SHIPS];
};

#endif
