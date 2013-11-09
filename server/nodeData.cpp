#include "nodeData.h"

/*
 * Set node variables.
 */
void nodeData::setNode(char * ip, int port, char * foldPath){
  ipAddress = ip;
  portNo = port;
  folderPath = foldPath;
}

nodeData::nodeData(){

}