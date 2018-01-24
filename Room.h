#ifndef __PILOT_ROOM
#define __PILOT_ROOM

#include <vector>

#define NUMOBSTACLES 10
#define	GIVEUP_THRESHOLD	1000

class Room;

#include "QuickDraw.h"
#include "Actor.h"

// A room is a box with a number of obstacles.
class Obstacle
{
private:
	int x1;
	int y1;
	int radius;

public:
	Obstacle (int xa, int ya, int rad);
	~Obstacle ();

	// Draw the obstacle.
	void display (View & view, double offsetx, double offsety, double scale);

	// Check if the obstacle intersects the given line segment.
	bool collides (double xa, double ya, double xb, double yb);
};

class Room : public Model
{
private:
	// Bounds
	int left;
	int right;
	int top;
	int bottom;

	// Size of the current environment.
	int sx;
	int sy;

	// Obstacles.
	std::vector <Obstacle *> obstacles;

	// List of actors inhabiting the maze.
	std::vector <Actor *> actors;

public:
	Room(int l, int r, int t, int b);
	~Room(void);

	// Run a simulation step of the game environment.
	void update (double deltat);

	// Render the room.
	void display (View & view, double offsetx, double offsety, double scale);

	// Check if the move is allowed - i.e. no collisions.
	bool canMove (double x1, double y1, double x2, double y2);

	// Add an actor to the game. 
	void addActor (Actor * actor);

	// Retrieve the list of actors.
	const std::vector <Actor *> getActors ();


	// serialize the environment and return a block that must be delete [] by the receiver. The code
	// is an int stuffed in at the beginning.
	char * serialize(int code, int & size);

	// deserialize the environment from a block.
	void deserialize(char * data, int size);
};

#endif // __PILOT_ROOM
