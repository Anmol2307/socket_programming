#include "nodeData.h"

void nodeData::setNode(char * ip, cint port, char * foldPath){
	ipAddress = ip;
	portNo = port;
	folderPath = foldPath;
}

nodeData::nodeData(){

}