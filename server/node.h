#ifndef NODE_H
#define NODE_H
#include "nodeData.h"
#include <sys/socket.h>
#include <cstdio>
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#define LENGTH 512 

using namespace std;


/*
 * Class to handle the node functionalities, and store the data for one particular node
 * And store data corresponding to all nodes
 */
class node{
private:
	static int N;                              // Total number of nodes
	static vector<nodeData> cloudNodesData;    // Store IP, port for all nodes
	int id;                                    // ID of this particular node
	int hexToInt(char c);                      // Convert hex character to decimal
	int bigModulo(string str, int N);          // Convert a given md5 sum in hex to decimal and get the modulo with number of nodes

public:
	node();                                    // Constructor function
	node (int n, vector<nodeData> cldNodesData); // Constructor to store data for all nodes
	node(int i);                               // Constructor for this particular node, all the functionality is handled here
};

#endif