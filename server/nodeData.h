
/*
 * Class to store the data for every node
 */
#ifndef NODEDATA_H
#define NODEDATA_H

#include<string>

using namespace std;

class nodeData{
public:
  string ipAddress;      // store IP address of node
  int portNo;            // store port number as integer
  string folderPath;     // store folder path relative to the executable of server

  void setNode(char * ip, int port, char * foldPath); // set variables of nodeData object
  nodeData();            // constructor for the class
};

#endif