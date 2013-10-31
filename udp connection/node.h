#ifndef NODE_H
#define NODE_H
#include "nodeData.h"
#include <sys/socket.h>
#include<vector>

class node{
private:
	static int N;
	static vector<nodeData> cloudNodesData;
	int id;
	struct sockaddr_in myAddr;

public:
	node();
	node(nodeData nData, int i);
};

#endif