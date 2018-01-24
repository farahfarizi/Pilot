#include "Room.h"
#include "Ship.h"
#include "Bullet.h"

#include <iostream>
#include <vector>

using namespace std;

Ship::Ship (Controller & cntrller, int initmode, std::string name) : Actor (), controller (cntrller), playmode (initmode), name (name)
{
	posx = 0;
	posy = 0;

	radius = 14.0;
	speed = 50.0;

	direction = 0.0;

	type = SHIP;
	mode = playmode;

	bulletinterval = 0.3;
	bulletstimeout = 0.0;

	target = NULL;

	score = 0;
}


Ship::~Ship(void)
{

}

bool Ship::update (Model & model, double deltat)

{
	double newposx = posx;
	double newposy = posy;

	double bulletposx = posx;
	double bulletposy = posy;
	bool bullet = false;

	double accx = 0.0;
	double accy = 0.0;
	double thrust = 50.0;
	double rotrate = 2.5;

	double controlthrust = 0.0;
	double controlleft = 0.0;
	double controlright = 0.0;
	double controlfire = 0.0;

	if (mode == INPLAY) // human controlled
	{
		//char c = controller.lastKey ();
		//switch (c)
		//{
		//case 'W': controlthrust = 1.0; break;
		//case 'A': controlleft = 1.0; break;
		//case 'D': controlright = 1.0; break;
		//case VK_SPACE: controlfire = 1.0; break;
		//default:
		//	// unknown key.
		//	;
		//}
		if (controller.isActive('W'))
		{
			controlthrust = 1.0;
		}
		if (controller.isActive('A'))
		{
			controlleft = 1.0;
		}
		if (controller.isActive('D'))
		{
			controlright = 1.0;
		}
		if (controller.isActive(VK_SPACE))
		{
			controlfire = 1.0;
		}
	}
	if (mode == AUTO) // AI! controlled.
	{
		doAI (model, controlthrust, controlleft, controlright, controlfire);
	}
	// may be another mode here that controls nodes from information
	// received over the network?

	accx += controlthrust * thrust * -sin (direction); 
	accy += controlthrust * thrust * cos (direction); 
	direction -= controlleft * rotrate * deltat;
	direction += controlright * rotrate * deltat; 

	if (controlthrust > 0.0)
	{
		thruston = true;
	}
	else
	{
		thruston = false;
	}

	double grav = 5.01;
	if (mode == RECOVERY)
	{
		grav = 0.0;
	}
	vx = vx + accx * deltat;
	vy = vy + (accy + grav) * deltat;
	newposx = posx + vx * deltat;
	newposy = posy + vy * deltat;

	if (model.canMove (posx, posy, newposx, newposy))
	{
		posx = newposx;
		posy = newposy;

		if (mode == RECOVERY)
		{
			recoverytimer -= deltat;
			if (recoverytimer < 0.0f)
			{
				mode = playmode;
			}
		}
	}
	else
	{
    	// hit something.
		triggerKill ();
	}

	if ((controlfire > 0.0) && (bulletstimeout < 0.0))
	{
		double fx = (1.4 * radius * -sin (direction));
		double fy = (1.4 * radius * cos (direction));
		double bulletposx = newposx + fx;
		double bulletposy = newposy + fy;

		double bulletspeed = 70.0;
		double flen = sqrt (fx * fx + fy * fy);
		double bvx = bulletspeed * fx / flen + vx;
		double bvy = bulletspeed * fy / flen + vy;

		model.addActor (new Bullet (bulletposx, bulletposy, bvx, bvy, this));
		bulletstimeout = bulletinterval;
	}

	bulletstimeout -= deltat;

	return true;
}

void Ship::triggerKill ()

{
	if (mode != RECOVERY)
	{
    	score -= 1.0;
	}

	mode = RECOVERY;
	recoverytimer = 5.0;
	vx = (rand () % 50) - 25;
	vy = (rand () % 50) - 25;
	target = NULL;
}

void Ship::display (View & view, double offsetx, double offsety, double scale)

{
	// Find center of screen.
	int cx, cy;
	view.screenSize (cx, cy);
	cx = cx / 2;
	cy = cy / 2;

	int x = (int) ((posx - (offsetx - cx)) * scale);
	int y = (int) ((posy - (offsety - cy)) * scale);

	double base = radius / 2.0f;
	double height = radius * 2.0f;

	int bx = (int) (base * cos (direction) * scale);
	int by = (int) (base * sin (direction) * scale);

	int hx = (int) (height * -sin (direction) * scale);
	int hy = (int) (height * cos (direction) * scale);

	int r = 23;
	int g = 23;
	int b = 80;

	if (mode == RECOVERY)
	{
		r = 255;
		g = 0;
		b = 45;
		view.drawCircle (x, y, (int) (height * scale), r, g, b);
	}
	if( playmode == Ship::INPLAY )
		view.drawText (x, y, name, 48, 192, 192);
	else
		view.drawText(x, y, name);

	view.drawLine (x - hx / 2, y - hy / 2, x + bx - hx / 2, y + by - hy / 2, r, g, b);
	view.drawLine (x + hx - hx / 2, y + hy - hy / 2, x + bx - hx / 2, y + by - hy / 2, r, g, b);
	view.drawLine (x + hx - hx / 2, y + hy - hy / 2, x - bx - hx / 2, y - by - hy / 2, r, g, b);
	view.drawLine (x - hx / 2, y - hy / 2, x - bx - hx / 2, y - by - hy / 2, r, g, b);

	if (thruston)
	{
    	view.drawLine (x - hx / 2, y - hy / 2, x - hx, y - hy, 255, 0, 0);
    	view.drawLine (x - hx / 2, y - hy / 2, x - hx - bx, y - hy - by, 255, 0, 0);
    	view.drawLine (x - hx / 2, y - hy / 2, x - hx + bx, y - hy + by, 255, 0, 0);
	}
}

void Ship::doAI (Model & model, double & controlthrust, double & controlleft, double & controlright, double & controlfire)

{
	if (target == NULL)
	{
		// find someone to shoot at.
		vector <Actor *> actors = model.getActors ();
        for (std::vector <Actor *>::iterator i = actors.begin (); i != actors.end (); i++)
		{
			if (((*i)->getType () == SHIP) && (*i != this) /*&& ((Ship *)*i)->isFairGame()*/ && (rand () % 5 == 1))
			{
				target = (Ship *) (*i);
				break;
			}
		}
	}

	if (target != NULL)
	{
		double fx = -sin (direction);
		double fy = cos (direction);
		double vx;
		double vy;
		target->getPosition (vx, vy);
		vx = vx - posx;
		vy = vy - posy;

		// use the sign of the cross product of the forward vector and the vector to the target to tell which way to turn.
		double cp = vx * fy - fx * vy;
		if (cp > 0)
		{
			controlleft = 1.0;
		}
		else
		{
			controlright = 1.0;
		}

		double dist = sqrt (vx * vx + vy * vy);
		if (dist > 200.0)
		{
			controlthrust = 1.0;
		}
		if ((dist > 50.0) && (dist < 200.0))
		{
			controlfire = 1.0;
		}

		//if (!target->isFairGame())	// is the target active? If not, choose another
		//{
		//	target = NULL;
		//}
	}
}

double Ship::getScore ()

{
	return score;
}

void Ship::addHit ()

{
	score += 1.5;
}

bool Ship::isFairGame ()

{
	return (mode != RECOVERY);
}