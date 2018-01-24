#ifndef __RATS_ACTOR
#define __RATS_ACTOR

#include "Model.h"
#include "QuickDraw.h"


// A parent class for all active content in the game: players, rats, etc.
class Actor
{
protected:
	// Position of the actor.
	double posx;
	double posy;

	// Magnitude of velocity.
	double speed;
	// Unit velocity vector.
	double vx;
	double vy;

	// Size of bounding circle around the object.
	double radius;

	// Type of actor.
	int type;

	// Move the actor by adding speed * deltat to position. Returns false if the move
	// if blocked.
	bool basicUpdate (Model & model, double deltat);

public:
	enum ActorTypes { UNDEFINED, PLAYER, RAT, BULLET, RATFACTORY, SHIP };

	Actor(void);
	virtual ~Actor(void);

	// Identify which type of actor this is.
	virtual int getType ();

	// Retrieve the player's position.
	virtual void getPosition (double & x, double & y);
	virtual void setPosition (double x, double y);

	virtual double getRadius ();
	virtual void setRadius (double r);

	// Read input and update state accordingly. Returns false if update failed (hit a wall) and
	// the actor must be destroyed.
	virtual bool update (Model & model, double deltat) = 0;

	// Show the actor.
	virtual void display (View & view, double offsetx, double offsety, double scale) = 0;

};

#endif // __RATS_ACTOR