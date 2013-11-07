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
using namespace std;

class node{
private:
	static int N;
	static vector<nodeData> cloudNodesData;
	int id;
	int hexToInt(char c);
	int bigModulo(string str, int N);

public:
	node();
	node (int n, vector<nodeData> cldNodesData);
	node(int i);
};

#endif