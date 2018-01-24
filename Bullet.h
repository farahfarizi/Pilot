#ifndef __RATS_BULLET
#define __RATS_BULLET

class Bullet;

#include "Actor.h"

class Bullet : public Actor
{
private:
	Actor * owner;

public:
	Bullet (double x, double y, double vx, double vy, Actor * owner);
	virtual ~Bullet (void);

	// Read input and update state accordingly. Returns false if move failed because the 
	// bullet hit a wall.
	virtual bool update (Model & model, double deltat);

	// Draw the Bullet.
	virtual void display (View & view, double offsetx, double offsety, double scale);

	Actor * getOwner () { return owner; }
};

#endif // __RATS_BULLET


