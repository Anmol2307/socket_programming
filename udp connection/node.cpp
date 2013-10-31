#include "node.h"
#include <cstdlib>

node::node(){

}

node::node(nodeData nData, int i){

	id = i;

	//set the sockAddr object for own address.
	memset((char *)&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = htonl(nData.ipAddress);
	myAddr.sin_port = htons(nData.portNo);

	int mySocket;
	//create a socket
	if ((mySocket=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		printf("Could not create socket. Exiting!!\n");
		exit(0);
	}	

	//bind the socket with the port and ip
	if (bind(mySocket, (struct sockaddr *)&myAddr, sizeof(myAddr)) == -1) {
		printf("Could not bind socket. Exiting!!\n");
		exit(0);
	} 

	//start listening

	int recvlen; //length of data received;
	char receivedData[2048]; //buffer in which the received data is stored
	struct sockaddr_in remoteAddr;

	char ip[30];
  	int port;
  	char reqType[10];
  	int mf;

	for (;;){
		recvlen = recvfrom(mySocket, receivedData, 2048, 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
		if(recvlen > 0){
			receivedData[recvlen] = '\0';
			//request received is of type ipaddr:port requestType(store/get) mf(integer)
			sscanf(receivedData, "%s:%d %s %d", ip, &port, reqType, &mf);
			//check if this request belongs to this server
			if(mf%N == id){
				//start a tcp connection and process the request depending upon its type i,e store or get
				//TODO
			}
			else{
				//forward the packet to correct destination
				memset((char *)&remoteAddr, 0, sizeof(remoteAddr));
				remoteAddr.sin_family = AF_INET;
				remoteAddr.sin_addr.s_addr = htonl(cloudNodesData[mf%N].ipAddress);
				remoteAddr.sin_port = htons(cloudNodesData[mf%N].portNo);

				if (sendto(mySocket, receivedData, strlen(receivedData), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0){
					perror("Could not forward request!! Exiting!\n");
					exit(0)
				}
			}
		}
	}
}