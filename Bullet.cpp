//#include "Maze.h"
#include "Bullet.h"

Bullet::Bullet (double x, double y, double vx, double vy, Actor * owner) : owner (owner)
{
	 posx = x;
	 posy = y;
	 
	 speed = sqrt (vx * vx + vy * vy);
	 this->vx = vx / speed;
	 this->vy = vy / speed;

	 type = BULLET;
}

Bullet::~Bullet(void)
{
}

bool Bullet::update (Model & model, double deltat)

{
	return basicUpdate (model, deltat);
}

void Bullet::display (View & view, double offsetx, double offsety, double scale)

{
	// Find center of screen.
	int cx, cy;
	view.screenSize (cx, cy);
	cx = cx / 2;
	cy = cy / 2;

	int x = (int) ((posx - (offsetx - cx)) * scale);
	int y = (int) ((posy - (offsety - cy)) * scale);

	float radius = 2;
	view.drawSolidCircle (x, y, (int) (scale * radius), 0, 255, 00);
}