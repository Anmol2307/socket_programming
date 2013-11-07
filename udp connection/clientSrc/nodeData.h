#ifndef NODEDATA_H
#define NODEDATA_H
#include<string>
using namespace std;

class nodeData{
public:
	string ipAddress;
	int portNo;
	string folderPath;

	void setNode(char * ip, int port, char * foldPath);
	nodeData();
};

#endif