# Space Wars
## Description:
SpaceWars is a 8 Players 2D Multiplayer Battleroyale game where the players will have to show who is the best pilot and fight all the other enemies in space.

## Tutorial
 1. Select a valid PlayerName (If you get an error, means that the name might be already taken)
 2. You'll spawn on the map on a random position, move with the key controls and shoot to all the nemies you find
 3. After killing some enemies you'r spaceship will level up, increasing your capabilities as a fighter.
 4. After 4s you will start regenerating life.
 5. Have fun and show who is the best pilot!! 
 
## Controls 
 	W,A,S,D - Movement
 	Space - Fire

## Authors: 
[Èric Canela](https://github.com/knela96) 
[Joan Marin](https://github.com/X0KA)

## Implementations
### Èric:
	- Client/Server Events (Completely Achieved)
		Allows client and Server send Events that will allow to perform different actions on the application. (NO bugs found)
	- World State Replication (Completely Achieved)
		Replicates all the gameobjects and actions in the Server into each each client, to visualize 
		exactly the same.
	- Client side prediction + Server Reconciliation (Completely Achieved)
		Processes the inputs on the client before sending and receiveing the input packets from the server, to 
		avoid laggy movement.
	- Entity Interpolation (Completely Achieved)
		Interpolates the last position received from the server and Interpolates it to the new one, smoothing 
		the movement of all the players.
	- All Game Mechanics (Scores,Spaceship Level Up,Restore Life)

### Joan:
	- Packets Redundancy (Achieved but there are some BUGS present)
		Each client will send the inputs from the players multiple times (if needed) to the server to make sure 
		all of them are read. The server keeps track of the last inputs sent by each client, so no rebundant 
		inputs are sent).
	- Delivery Manager 	(Achieved but there are some BUGS present)
		In order to manage input packet drops, input packets will be grouped in deliveries. When the client 
		sends these deliveries, it will store them in a list. When the server receives them, will notify the 
		client. If the client is not told that those deliveries where succesfully sent, it will send them again.
	
## Known Bugs:
	- Sometimes, when a packet has to be resent (because it was dropped), the server might give a wrong position 
	for the replacement of the object in the array, asuming it's empty, but it's actually occupied in the client. 
	Sometimes, it might be even occupied by the ship. And since the server is autoritharian, the object that was 
	in the array and must be replaced is destroyed, and because of that, sometimes, the ship is destroyed, so the 
	client will exit the app.
	- There might be sync errors with packets losts(when playing Internet and not LAN), where the spaceship of 
	another client is updated in the server and host client with wrong values(slow movement and rotation), but 
	the host spacehip is updated correctly on the other clients. 
