/*
 * Main File
 * To Handle interaction between program and user
 */
#include <cstdio>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>
#include "node.h"
#include "nodeData.h"
#include <pthread.h>
//#include <thread>
using namespace std;



int N;                              //  Number of nodes
vector<nodeData> cloudNodesData;    //  Data for all the nodes of cloud
int nodeId;                         //  Present node ID


/*
 * To read and store the information data for all nodes on the cloud.
 * The data is read from a configuration file.
 */
bool readNodeData(string filePath){
  string line;
  ifstream myfile (filePath.c_str());
  if (myfile.is_open()){
    for(int i = 0; i<N; i++){
      getline (myfile,line);
      nodeData newNode;
      char* ip;
      int port ;
      char* foldPath;
      ip = strtok((char*)line.c_str(), ":");
      if (ip == NULL){
        printf("[ERROR] Wrong format in config file. Try Again!\n");
        return false;
      }
      char* port1 =  strtok(NULL, " ");
      if (port1 == NULL){
        printf("[ERROR] Wrong format in config file. Try Again!\n");
        return false;
      }
      sscanf(port1, "%d", &port);
      foldPath = strtok(NULL, " ");
      if (foldPath == NULL){
        printf("[ERROR] Wrong format in config file. Try Again!\n");
        return false;
      }

      newNode.setNode(ip, port, foldPath);
      cloudNodesData.push_back(newNode);
    }
    myfile.close();
    return true;
  }
  else {
    printf("[ERROR] Error opening config file. Try again!\n");
    return false;
  }
}


void setStaticVariables(){
	node newNode(N , cloudNodesData);
	return;
}

/*
 * Main Function
 * To  Handle interaction between program and user
 */
int main(){
	//printf("Enter the number of nodes: ");
	scanf("%d", &N);
	//printf("Enter the node id: ");
	scanf("%d", &nodeId);
	//cout<<"Node id is "<<nodeId<<endl;
	//printf("Enter the configuration file path: ");
	string configFile;
	scanf("\n");
	getline(cin,configFile);
	//cout<<"config file is "<<configFile<<endl;
	//exit(0);
	while ( !readNodeData(configFile) ) {
    printf("Enter new location of config file: ");
    cin >> configFile;
  }
  setStaticVariables();
  // Create the node and start functionality
	node myNode(nodeId);
	return 0;
}
