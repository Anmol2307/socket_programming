#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <openssl/md5.h>
#include "nodeData.h"
using namespace std;

int reqType;
int contactedNode;
string configFile;
int backlog = 5; // TODO : check?
string myIp = "127.0.0.1";
int myPort = 27381;
int myTCPPort = 27382;
int tcpSockfd;
int N;
int mySocket;
char request[200];

vector<nodeData> cloudNodesData; //data for all the nodes of cloud

void readNodeData(string filePath){
	string line;
 ifstream myfile (filePath.c_str());
 for(int i = 0; i<N; i++){
  getline (myfile,line);
  nodeData newNode;
  char ip[30];
  int port;
  char foldPath[100];
  sscanf(line.c_str(), "%s %d %s", ip, &port, foldPath);
  newNode.setNode(ip, port, foldPath);
  cloudNodesData.push_back(newNode);
}
myfile.close();
return;
}

void setup(){

	//set the sockAddr object for own address.
	struct sockaddr_in myAddr;
	memset((char *)&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = inet_addr(myIp.c_str());
	myAddr.sin_port = htons(myPort);

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
}

void setup_TCP(){

  //set the sockAddr object for own address.
  struct sockaddr_in myAddr;
  memset((char *)&myAddr, 0, sizeof(myAddr)); // TODO : is this right?
  myAddr.sin_family = AF_INET;
  myAddr.sin_addr.s_addr = inet_addr(myIp.c_str());
  myAddr.sin_port = htons(myTCPPort); // TODO will same port be used??
  


  //int sockfd;
  //create a socket
  if ((tcpSockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1){
    printf("Could not create TCP socket. Exiting!!\n");
    exit(0);
  } 

  //bind the socket with the port and ip
  if (bind(tcpSockfd, (struct sockaddr *)&myAddr, sizeof(struct sockaddr)) == -1) {
    printf("Could not bind TCP socket. Exiting!!\n");
    exit(0);
  } 

  if (listen(tcpSockfd, backlog) == -1 ) {
    perror("Could not establish listen!! Exiting!\n");
    exit(0);
  }
  else {
    printf("Listen stage successful.\n");
  }
  cout<<"THE TCP SOCKET CREATED IS "<<tcpSockfd<<endl;
  //return sockfd;
}

string getMd5sum(string filePath){

	unsigned char c[MD5_DIGEST_LENGTH];
    //char filename[]=filePath.c_str();
  int i;
  FILE *inFile = fopen (filePath.c_str(), "rb");
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];

  if (inFile == NULL) {
    printf ("%s can't be opened.\n", filePath.c_str());
    exit(0);
  }

  MD5_Init (&mdContext);
  while ((bytes = fread (data, 1, 1024, inFile)) != 0)
    MD5_Update (&mdContext, data, bytes);
  MD5_Final (c,&mdContext);
  char ch[5];
  string s;
  for(i = 0; i < MD5_DIGEST_LENGTH; i++) {sprintf(ch, "%02x", c[i]); s+=ch;}
    fclose (inFile);
  return 	s;
}

void sendRequest(){
	struct sockaddr_in remoteAddr;
	memset((char *)&remoteAddr, 0, sizeof(remoteAddr));
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_addr.s_addr = inet_addr(cloudNodesData[contactedNode].ipAddress.c_str());
	remoteAddr.sin_port = htons(cloudNodesData[contactedNode].portNo);

	if (sendto(mySocket, request, strlen(request), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0){
		perror("Could not forward request!! Exiting!\n");
		exit(0);
	}
	else{
    
    int sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in nodeAddr;
    int new_fd ;

    cout<<"Waiting for Connection Established!!"<<endl;
    while( (new_fd = accept(tcpSockfd, (struct sockaddr *)&nodeAddr, (socklen_t *) &sin_size)) == -1 ){
      perror("Error in accept!! Exiting!\n");
      exit(0);     
    }
    
    cout<<"Connection Established!!"<<endl;
    
    char msg[100] = "hello!";
    if(send(new_fd, msg, strlen(msg), 0) == -1){
       perror("Could not send msg!! Exiting!\n");
      exit(0);
    }
    else {
      printf("Send stage successful.\n");
    }
   
    // char * recvBuffer = new char[2048];
    // int recvd = recv(new_fd, recvBuffer, 2048, 0);
    // if (recvd == -1){
    //   perror("Could not receive msg!! Exiting!\n");
    //   exit(0);
    // }
    // else if (recvd == 0){
    //   printf("Remote size has closed connection on you!\n");
    //   exit(0);
    // }
    // else {
    //   printf("Receive stage successful. recvd value = %d\n",recvd);
    // }

    
		// cout<<"Request Sent!!"<<endl;
  }
}



int main(){
	printf("Enter the number of nodes: ");
	scanf("%d", &N);
	cout<<"Enter the config file"<<endl;
	cin>>configFile;
	readNodeData(configFile);
	cout<<"Enter 1 for store request and 2 for get request"<<endl;	
	cin>>reqType;
	cout<<"Which node do you want to send the request to. Select between 0 and "<<N<<": ";	
	cin>>contactedNode;
	setup();
  setup_TCP();
	if(reqType == 1){
		string storeFilePath;
		//store request
		cout<<"Enter the file path to be stored: ";
		cin>>storeFilePath;
		sprintf(request, "%s %d %s %s", myIp.c_str(), myTCPPort,"store", getMd5sum(storeFilePath).c_str());
    // printf("%s", getMd5sum(storeFilePath).c_str());
		sendRequest();
	}
	else{
		string md5;
		cout<<"Enter the md5 sum of the file you want to retrieve: ";
		cin>>md5;
		sprintf(request, "%s %d %s %s", myIp.c_str(), myTCPPort,"get", md5.c_str());
		sendRequest();
	}
}
