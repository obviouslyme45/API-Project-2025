# API-Project-2025
Final project of the "Algoritmi e principi dell'informatica" course for the "Ingegneria Informatica" study program at Politecnico di Milano

Grade: 30 e lode/30 e lode

# Description
The program models a route optimization system for a logistics company in order to decrease costs of travel. The map is divided into hexagons, each with its own cost of passage. The hexagons have, in addition to the normal land routes to adjacent hexagons, also the possibility of a flight connection with other hexagons on the map (up to 5 per hexagon).

# Main features:
- Dijkstra pathfinding algorithm in order to find the best route from a hexagon A to a hexagon B
- A function to add/remove a flight connection between two hexagon
- A function to initialize a new map and/or delete the old one
- A function to change the cost of a set of hexagon within a radius of a given center hexagon

# Program structure:
- A structure for each hexagon to hold the progressive id and of its neighbors, plus a pointer to a linked list of flight paths beginning from the hexagon
- A min heap structure to compute the shortest path in Dijkstra's algorithm
- A cache array to hold all the recently calculated paths
- A Breadth First Search algorithm to change the cost of passage from a hexagon within the radius of the given center hexagon (given a particular formula), using an array as a queue
