//#include "Maze.h"
#include "Actor.h"

Actor::Actor(void)
{
  posx = 0;
  posy = 0;

  speed = 0.0;
  vx = 0.0;
  vy = 0.0;

  radius = 1.0;
  type = UNDEFINED;
}


Actor::~Actor(void)
{
}

bool Actor::basicUpdate (Model & model, double deltat)
{
	double newposx = posx + speed * vx * deltat;
	double newposy = posy + speed * vy * deltat;

	if (model.canMove (posx, posy, newposx, newposy))
	{
		posx = newposx;
		posy = newposy;
		return true;
	}
	return false;
}

void Actor::getPosition (double & x, double & y)

{
	x = posx;
	y = posy;
}

void Actor::setPosition (double x, double y)

{
	posx = x;
	posy = y;
}

int Actor::getType ()
{
	return type;
}

double Actor::getRadius ()
{
	return radius;
}

void Actor::setRadius (double r)
{
	radius = r;
}
