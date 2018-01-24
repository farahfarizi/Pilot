#include <iostream>
#include <list>
#include <time.h>

#include "QuickDraw.h"

#include "Room.h"
#include "Ship.h"
#include "Bullet.h"

Obstacle::Obstacle (int xa, int ya, int rad) : x1(xa), y1(ya), radius(rad)
{
}

Obstacle::~Obstacle ()
{
}

void Obstacle::display (View & view, double offsetx, double offsety, double scale)
{
	// Find center of screen.
	int cx, cy;
	view.screenSize (cx, cy);
	cx = cx / 2;
	cy = cy / 2;

	int xa = (int) ((x1 - (offsetx - cx)) * scale);
	int ya = (int) ((y1 - (offsety - cy)) * scale);
	int rad = (int) (radius * scale);

	view.drawSolidCircle (xa, ya, rad, 64, 64, 64);
}

// Perform line segment/sphere intersection testing.
bool Obstacle::collides (double xa, double ya, double xb, double yb)
{
	// let V be the vector from A to B
	double vx = xb - xa;
	double vy = yb - ya;

	// Let W be the vector from A to the center of the sphere.
	double wx = x1 - xa;
	double wy = y1 - ya;

	// Distance of center from the line is scalar projection of W onto the normal to the line.
	// = dot product of W and unit v normal.
	double vlen = sqrt (vx * vx + vy * vy);
	double vnormalx = -vy / vlen;
	double vnormaly = vx / vlen;

	double distance = abs (wx * vnormalx + wy * vnormaly);

	if (distance <= radius)
	{
		// Sphere overlaps the line somewhere.
		// Now check to see if it touches between A and B.
		// Get the scalar projection of W onto V. If this is between 0 and |V|, then the intersection is between A and B.
		double between = (wx * vx + wy * vy) / vlen;

		// The sphere center could project beyond the endpoints but still overlap between A and B. The extra amount depends on
		// how close the sphere is to the line.
		double extra = sqrt ((radius * radius) - ((radius - distance) * (radius - distance)));
		if ((between > - extra) && (between < vlen + extra))
		{
			return true;
		}
	}

	return false;
}

Room::Room(int l, int r, int t, int b) : left (l), right (r), top (t), bottom (b)
{
	typedef struct { int x; int y; int r; } POINT;

	std::list<POINT> obstaclePoints;

	srand (time(NULL));
	int lr = right - left;
	int tb = top - bottom;
	int pointCount = NUMOBSTACLES;
	
	bool tooClose = false;
	int tryCount = 0;

	do {
		// Choose a random point for an Obtacle and see if this is not
		// overlapping/close to an exsiting obstacle.
		int tmpX = rand() % (lr-160)+left+80;
		int tmpY = rand() % (tb-160)+bottom+80;
		int tmpRadius = rand() % (lr / 10);		// Radius is up to 10% of the screen width

		tooClose = false;	// Start off assuming this point is OK

		for (auto p : obstaclePoints)
		{
			// Find the distance between the randomly chosen point and the given obstacles.
			double dist = sqrt(static_cast<double>((tmpX - p.x)*(tmpX - p.x) + (tmpY - p.y)*(tmpY - p.y)));
			if (dist < ((lr / 10.0) + 80.0) )
			{
				// too close to another point!  
				// Choose again...
				tooClose = true;
				tryCount++;
				continue;
			}
		}

		// If the POINT chosen doesn't overalp (too close) to another obstacle
		// *or* we have tried too many times to find a location, then add this 
		// obstacle at this point to the list!
		if( !tooClose || tryCount >= GIVEUP_THRESHOLD )
		{
			POINT addPoint;
			addPoint.x = tmpX;
			addPoint.y = tmpY;
			addPoint.r = tmpRadius;

			obstaclePoints.push_front(addPoint);
			pointCount--;
			tryCount = 0;
		}
	} while (pointCount != 0);

	for (auto p : obstaclePoints)
	{
		obstacles.push_back(new Obstacle(p.x, p.y, p.r));
	}

}


Room::~Room(void)
{
	for (std::vector <Obstacle *>::iterator i = obstacles.begin (); i != obstacles.end (); i++)
	{
		delete (*i);
	}
	for (std::vector <Actor *>::iterator i = actors.begin (); i != actors.end (); i++)
	{
		delete (*i);
	}
}

void Room::update (double deltat)
{
	// Move the rats.
	double avgratx = 0.0;
	double avgraty = 0.0;

	// Avoid using iterators since the list may grow during updates.
	for (unsigned int i = 0; i < actors.size (); )
	{
		if (!(actors[i])->update (*this, deltat))
		{
			delete (actors[i]);
			actors.erase (actors.begin () + i);
		}
		else
		{
			// Check for collisions between objects.
			bool killi = false;
			for (unsigned int j = 0; j < actors.size (); )
			{
				bool killj = false;
				double rx, ry;
				double bx, by;
				(actors[i])->getPosition (rx, ry);
				(actors[j])->getPosition (bx, by);
				if ((i != j) && (fabs (rx - bx) + fabs (ry - by) < actors[i]->getRadius () + actors[j]->getRadius ()))
				{
					// two objects are colliding.
					if ((actors[j]->getType () == Actor::BULLET) &&
						(actors[i]->getType () == Actor::SHIP))
					{
						if (((Ship*) (actors[i]))->isFairGame ())
						{
							// remove bullet, kill ship.
							killj = true;
							killi = true;
						}
					}
				}

				if (killj)
				{
					((Ship *) (((Bullet*) (actors[j]))->getOwner ()))->addHit ();
					// remove the bullet.
					delete (actors[j]);
					actors.erase (actors.begin () + j);
				}
				else
				{
					j++;
				}
			}

			if (killi)
			{
				((Ship*) (actors[i]))->triggerKill ();
			}
			else
			{
				i++;
			}
		}
	}
}

void Room::display (View & view, double offsetx, double offsety, double scale)

{
	// Find center of screen.
	int cx, cy;
	view.screenSize (cx, cy);
	cx = cx / 2;
	cy = cy / 2;

	int xl = (int) ((left - (offsetx - cx)) * scale);
	int xr = (int) ((right - (offsetx - cx)) * scale);
	int yt = (int) ((top - (offsety - cy)) * scale);
	int yb = (int) ((bottom - (offsety - cy)) * scale);

	view.drawLine (xl, yt, xl, yb, 255, 0, 0);
	view.drawLine (xr, yt, xr, yb, 255, 0, 0);
	view.drawLine (xl, yt, xr, yt, 255, 0, 0);
	view.drawLine (xl, yb, xr, yb, 255, 0, 0);

	for (std::vector <Obstacle *>::iterator i = obstacles.begin (); i != obstacles.end (); i++)
	{
		(*i)->display (view, offsetx, offsety, scale);
	}
	for (std::vector <Actor *>::iterator i = actors.begin (); i != actors.end (); i++)
	{
		(*i)->display (view, offsetx, offsety, scale);
	}
}

bool Room::canMove (double x1, double y1, double x2, double y2)
{
	if ((x2 < left) || (x2 > right) || (y2 > top) || (y2 < bottom))
		return false;

	for (std::vector <Obstacle *>::iterator i = obstacles.begin (); i != obstacles.end (); i++)
	{
		// False if collides with any single Obstacle.
		if ((*i)->collides (x1, y1, x2, y2))
			return false;
	}
	return true;
}

void Room::addActor (Actor * actor)
{
	actors.push_back (actor);
}

const std::vector <Actor *> Room::getActors ()

{
	return actors;
}

