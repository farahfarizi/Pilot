#ifndef __MODEL
#define __MODEL

#include <vector>

class Actor;

class Model
{
public:
	// Check if the move is allowed - i.e. no collisions.
	virtual bool canMove (double x1, double y1, double x2, double y2) = 0;

	// Add an actor to the game. 
	virtual void addActor (Actor * actor) = 0;

	virtual const std::vector <Actor *> getActors () = 0;
};

#endif