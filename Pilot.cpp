// 00Pilot code - main() function
// 
// Initialise system and create playing arena with all actors.
//
// This code has been modified to improve the keyboard handling, allowing 
// multiple keys to be proceesed at once (ie., FIRE & THRUST)

#include <iostream>
#include <vector>
#include <conio.h>
#include <sstream>


#include <WinSock2.h>
#include <ws2tcpip.h>
#include "QuickDraw.h"
#include "Timer.h"

#include "Room.h"
#include "Ship.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int gamePort = 33303;

enum MESSAGECODES { ADDLINK, REMOVELINKS, WORLDUPDATE };

// Message objects. All must have a message code as the first integer and playerID as the second.

// Send a player update to the server: add a link between two nodes
class AddLink
{
public:
	int action;
	int player;
	int sourcex;
	int sourcey;
	int destx;
	int desty;

	AddLink(int p, int sx, int sy, int dx, int dy) : action(ADDLINK), player(p), sourcex(sx), sourcey(sy), destx(dx), desty(dy) {}
};

// A player update to the server: clear the links for a node.
class RemoveLinks
{
public:
	int action;
	int player;
	int x;
	int y;

	RemoveLinks(int p, int px, int py) : action(REMOVELINKS), player(p), x(px), y(py) {}
};


// Run as a server. Keep an environment, updated regularly.
void server()

{
	// Set up the game environment.
/*	Parameters gameparameters;

	BattleEnvironment model(gameparameters);
	*/

	Room model(-400, 400, 500, -500);
	Ship *ship;
	//Actor gameactors;
	//Ship * ship = new Ship( Ship::INPLAY, "You");
	//model.addActor(ship);
	
	// Create a timer to measure the real time since the previous game cycle.
	Timer timer;
	timer.mark(); // zero the timer.
	double lasttime = timer.interval();
	double avgdeltat = 0.0;

	// Set up the socket.
	// Argument is an IP address to send packets to. 
	int socket_d;
	socket_d = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in my_addr;	// my address information

	my_addr.sin_family = AF_INET;		 // host byte order
	my_addr.sin_port = htons(gamePort); // short, network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	if (bind(socket_d, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1)
	{
		cerr << "Bind error: " << WSAGetLastError() << "\n";
		return;
	}

	u_long iMode = 1;
	ioctlsocket(socket_d, FIONBIO, &iMode); // put the socket into non-blocking mode.

											// A list of the IP addresses of all clients. Clients must all be on separate machines because a single port is
											// used. This could be fixed (how?)
	struct ClientAddr
	{
		int ip;
		int port;

		ClientAddr(int IP, int PORT) : ip(IP), port(PORT) {}
	};
	vector <ClientAddr> clients;

	while (true)
	{
		int n;

		// Calculate the time since the last iteration.
		double currtime = timer.interval();
		double deltat = currtime - lasttime;

		// Run a smoothing step on the time change, to overcome some of the
		// limitations with timer accuracy.
		avgdeltat = 0.2 * deltat + 0.8 * avgdeltat;
		deltat = avgdeltat;
		lasttime = lasttime + deltat;

		// Update the model - the representation of the game environment.
		model.update(deltat);

		struct sockaddr_in their_addr; // connector's address information
		int addr_len = sizeof(struct sockaddr);
		char buf[50000];
		int numbytes = 50000;

		// Receive requests from clients.
		if ((n = recvfrom(socket_d, buf, numbytes, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK) // A real problem - not just avoiding blocking.
			{
				cerr << "Recv error: " << WSAGetLastError() << "\n";
			}
		}
		else
		{
			//		cout << "Received: " << n << "\n";
			// Add any new clients provided they have not been added previously.
			bool found = false;
			for (unsigned int i = 0; i < clients.size(); i++)
			{
				if ((clients[i].ip == their_addr.sin_addr.s_addr) && (clients[i].port == their_addr.sin_port))
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				clients.push_back(ClientAddr(their_addr.sin_addr.s_addr, their_addr.sin_port));
				char their_source_addr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &(their_addr.sin_addr), their_source_addr, sizeof(their_source_addr));
				cout << "Adding client: " << their_source_addr << " " << ntohs(their_addr.sin_port) << "\n";
			}

			// Process any player to server messages.
			switch (*(int *)buf) // First int of each packet is the message code.
			{
			case ADDLINK:
			{
				AddLink * al = (AddLink *)buf;
				QuickDraw window;
				View & view = (View &)window;
				Controller & controller = (Controller &)window; 
				Ship * ship = new Ship(controller, Ship::INPLAY, "You");
				//model.addActor(al->player, al->sourcex, al->sourcey, al->destx, al->desty);
				model.addActor(ship);
			}
			break;
			case REMOVELINKS:
			{
				RemoveLinks * rl = (RemoveLinks *)buf;
				//model.removeLinks(rl->player, rl->x, rl->y);
				//cout << "Removing links for player: " << rl->player << "\n";
			}
			break;
			default:
			{
				//				cout << "Unknown message code: " << * (int *) buf << "\n";
			}
			break;
			}
		}

		// Allow the user to type in the IP address of a new client to include.
		// Required because of the firewalls in the lab. New clients are added
		// by typing in their IP address in the console window and pressing enter.
		// Not nice, but a workaround of the restrictions imposed. Use this recipe
		// for your own UDP based applications.
		// Removed, since firewalls opened.
		//if (_kbhit ())
		//{
		//	char ipnum [80];
		//	cin >> ipnum;
		//	clients.push_back (inet_addr (ipnum));
		//	cout << "Adding: " << ipnum << "(" << inet_addr (ipnum) << ")\n";
		//}

		// Server to player - send environment updates. Very clunky - try to do better.
		for (unsigned int i = 0; i < clients.size(); i++)
		{
			// Send environment updates to clients.
			int addr = clients[i].ip;
			int port = clients[i].port;
			int modelsize;
			
			model.update(deltat);
			
			char * modeldata = model.serialize(WORLDUPDATE, modelsize);
			//char * modeldata = model.serialize(WORLDUPDATE, modelsize);
			struct sockaddr_in dest_addr;
			dest_addr.sin_family = AF_INET;
			dest_addr.sin_addr.s_addr = addr;
			dest_addr.sin_port = port;
			do
			{
				n = sendto(socket_d, modeldata, modelsize, 0, (const sockaddr *) &(dest_addr), sizeof(dest_addr));
				if (n <= 0)
				{
					cout << "Send failed: " << n << "\n";
				}
			} while (n <= 0); // retransmit if send fails. This server is capable of producing data faster than the machine can send it.
							  //			cout << "Sending world: " << n << " " << modelsize << " to " << hex << addr << dec << "\n";
			delete[] modeldata;
		}

		cout << "Server running: " << deltat << "             \r";
	}
}

// Do client things. Get input from user, send to server. Get updates
// from server, update visuals.
void client(char * serverIP, char * clientID)

{
	QuickDraw window;
	View & view = (View &)window;
	Controller & controller = (Controller &)window;
	Ship * ship;
	//Parameters gameparameters;


	Room model(-400, 400, 500, -500);
	//BattleEnvironment model(gameparameters);

	//Room model(-400, 400, 500, -500);
	//Ship * ship = new Ship(controller, Ship::INPLAY, "You");
	//model.addActor(ship);

	int offsetx = 0;
	int offsety = 0;
	float scale = 1.0f;

	int startx;
	int starty;
	bool startset = false;

	int player = atoi(clientID);

	// Set up the socket.
	// Argument is an IP address to send packets to. Multicast allowed.
	int socket_d;
	socket_d = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in my_addr;	// my address information

	my_addr.sin_family = AF_INET;		 // host byte order
	my_addr.sin_port = htons(gamePort + 1 + player); // short, network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	if (bind(socket_d, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1)
	{
		cerr << "Bind error: " << WSAGetLastError() << "\n";
		return;
	}

	u_long iMode = 1;
	ioctlsocket(socket_d, FIONBIO, &iMode); // put the socket into non-blocking mode.

	struct sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	inet_pton(AF_INET, serverIP, &dest_addr.sin_addr.s_addr);
	dest_addr.sin_port = htons(gamePort);

	// Create a timer to measure the real time since the previous game cycle.
	Timer timer;
	timer.mark(); // zero the timer.
	double lasttime = timer.interval();
	double avgdeltat = 0.0;

	double scale = 1.0;
	// Very similar to the single player version - spot the difference.
	while (true)
	{
		// Calculate the time since the last iteration.
		double currtime = timer.interval();
		double deltat = currtime - lasttime;

		// Run a smoothing step on the time change, to overcome some of the
		// limitations with timer accuracy.
		avgdeltat = 0.2 * deltat + 0.8 * avgdeltat;
		deltat = avgdeltat;
		lasttime = lasttime + deltat;

		// Allow the environment to update.
		model.update(deltat);
		sendto(socket_d, (const char *)&ship, sizeof(ship), 0, (const sockaddr *) &(dest_addr), sizeof(dest_addr));

		// Check for events from the player.
		/*
		int mx, my, mb;
		bool mset;
		controller.lastMouse(mx, my, mb, mset);
		if (mset)
		{
			int x = (int)(((mx / scale) + offsetx) / gameparameters.CellSize);
			int y = (int)(((my / scale) + offsety) / gameparameters.CellSize);

			if (mb == WM_LBUTTONDOWN)
			{
				startx = x;
				starty = y;
				startset = true;
			}
			if ((mb == WM_RBUTTONDOWN) && (startset))
			{
				model.addLink(player, startx, starty, x, y);
				AddLink al(player, startx, starty, x, y);
				sendto(socket_d, (const char *)&al, sizeof(al), 0, (const sockaddr *) &(dest_addr), sizeof(dest_addr));

				startset = false;
			}
			if (mb == WM_MBUTTONDOWN)
			{
				model.removeLinks(player, x, y);
				RemoveLinks rl(player, x, y);
				sendto(socket_d, (const char *)&rl, sizeof(rl), 0, (const sockaddr *) &(dest_addr), sizeof(dest_addr));
			}
		}
		*/
		

		// Check for updates from the server.
		struct sockaddr_in their_addr; // connector's address information
		int addr_len = sizeof(struct sockaddr);
		char buf[50000];
		int numbytes = 50000;
		int n;

		// Receive requests from clients.
		if ((n = recvfrom(socket_d, buf, numbytes, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK) // A real problem - not just avoiding blocking.
			{
				cerr << "Recv error: " << WSAGetLastError() << "\n";
			}
		}
		else
		{
			cout << "Received world dump: " << n << "\n";
			if (*(int *)buf == WORLDUPDATE)
			{
				model.update;
			}
		}

		// Schedule a screen update event.
		view.clearScreen();
		double offsetx = 0.0;
		double offsety = 0.0;
		(*ship).getPosition(offsetx, offsety);
		model.display(view, offsetx, offsety, scale);

		std::ostringstream score;
		score << "Score: " << ship->getScore();
		view.drawText(20, 20, score.str(), 0, 0, 255);
		view.swapBuffer();
		/*
			view.clearScreen();
		model.display(view, offsetx, offsety, scale);
		view.swapBuffer();*/
	
	}
}


int main(int argc, char * argv [])
{
	/*
	QuickDraw window;
	View & view = (View &) window;
	Controller & controller = (Controller &) window;*/
	/*	Room model (-400, 400, 500, -500);
	Ship * ship = new Ship (controller, Ship::INPLAY, "You");
	model.addActor (ship);*/



	/*// Add some opponents. These are computer controlled - for the moment...
	Ship * opponent;
	opponent= new Ship (controller, Ship::AUTO, "Alex");
	model.addActor (opponent);
	opponent = new Ship (controller, Ship::AUTO, "Siva");
	model.addActor (opponent);
	opponent = new Ship (controller, Ship::AUTO, "Penny");
	mo*/
	// Create a timer to measure the real time since the previous game cycle.
	/*Timer timer;
	timer.mark (); // zero the timer.
	double lasttime = timer.interval ();
	double avgdeltat = 0.0;

	double scale = 1.0;

	while (true)
	{
		// Calculate the time since the last iteration.
		double currtime = timer.interval ();
		double deltat = currtime - lasttime;

		// Run a smoothing step on the time change, to overcome some of the
		// limitations with timer accuracy.
		avgdeltat = 0.2 * deltat + 0.8 * avgdeltat;
		deltat = avgdeltat;
		lasttime = lasttime + deltat;

		// Allow the environment to update.
		model.update (deltat);

		// Schedule a screen update event.
		view.clearScreen ();
		double offsetx = 0.0;
		double offsety = 0.0;
		(*ship).getPosition (offsetx, offsety);
		model.display (view, offsetx, offsety, scale);

		std::ostringstream score;
		score << "Score: " << ship->getScore ();
		view.drawText (20, 20, score.str (), 0, 0, 255);
		view.swapBuffer ();
	}

*/	
	

	// Messy process with windows networking - "start" the networking API.
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (argc > 1)
	{
		// client. First argumentL IP address of server, second argument: player number (0, 1, 2, ...)
		client(argv[1], argv[2]);
	}
	else
	{
		// server.
		server();
	}

	WSACleanup();// Messy process with windows networking - "start" the networking API.
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (argc > 1)
	{
		// client. First argumentL IP address of server, second argument: player number (0, 1, 2, ...)
		client(argv[1], argv[2]);
	}
	else
	{
		// server.
		server();
	}

	WSACleanup();

	return 0;
}

