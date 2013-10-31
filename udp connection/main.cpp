#include<cstdio>
#include<iostream>
#include<string>
#include<cstdlib>
#include<fstream>
#include<vector>
#include "node.h"
#include "nodeData.h"
using namespace std;


int N; //Number of nodes
vector<nodeData> cloudNodesData; //data for all the nodes of cloud
vector<node> cloudNodes; //actual node objects in the cloud



void readNodeData(string filePath){
	string line;
  	ifstream myfile (filePath.c_str());
  	for(int i = 0; i<N; i++){
  		getline (myfile,line);
  		nodeData newNode;
  		char ip[30];
  		int port;
  		char foldPath[100];
  		sscanf(line.c_str(), "%s:%d %s", ip, &port, foldPath);
  		newNode.setNode(ip, port, foldPath);
  		cloudNodesData.push_back(newNode);
  	}
  	myfile.close();
  	return;
}

void initialiazeNodes(){
	for(int i = 0 ; i<N; i++){
		node newNode(cloudNodesData[i], i);
		cloudNodes.push_back(newNode);
	}
	return;
}


int main(){
	printf("Enter the number of nodes: ");
	scanf("%d", &N);
	printf("Enter the configuration file path: ");
	string configFile;
	cin>>configFile;
	readNodeData(configFile);
	vector<nodeData> node::cloudNodesData = cloudNodesData;
	int node::N = N;

	initialiazeNodes();

	return 0;
}