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
#include <errno.h>
#define LENGTH 512 
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

void sendRequest(string storeFilePath){
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
    
    //char* fs_name = "/home/anmol/lab09.pdf";
    char* send_file = (char *)storeFilePath.c_str();
    char file_buffer[LENGTH]; 
    printf("[Client] Sending %s to the Server... ", send_file);
    FILE *fileOpen = fopen(send_file, "r");
    if(fileOpen == NULL)
    {
      printf("ERROR: File %s not found.\n", send_file);
      exit(0);
    }

    bzero(file_buffer, LENGTH); 
    int block_size; 
    while((block_size = fread(file_buffer, sizeof(char), LENGTH, fileOpen)) > 0)
    {
      if(send(new_fd, file_buffer, block_size, 0) < 0)
      {
        fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", send_file, errno);
        break;
      }
      bzero(file_buffer, LENGTH);
    }
    printf("Ok File %s from Client was Sent!\n", send_file);
    close(tcpSockfd);
    printf("[Client] Connection with Server closed. \n");
  }
}



void sendRetrieve(string retrieveFilemf){
  struct sockaddr_in remoteAddr;
  memset((char *)&remoteAddr, 0, sizeof(remoteAddr));
  remoteAddr.sin_family = AF_INET;
  remoteAddr.sin_addr.s_addr = inet_addr(cloudNodesData[contactedNode].ipAddress.c_str());
  remoteAddr.sin_port = htons(cloudNodesData[contactedNode].portNo);

  if (sendto(mySocket, request, strlen(request), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0){
    perror("Could not forward retrieve request!! Exiting!\n");
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
    
    int success = 0;
    while(success == 0)
    {

              /*Receive File from Client */
      char* fr_name = (char *)retrieveFilemf.c_str();
      char receive_file[LENGTH];
      FILE *fr = fopen(fr_name, "w+");
      if(fr == NULL)
        printf("File %s Cannot be opened file on server.\n", fr_name);
      else
      {
        bzero(receive_file, LENGTH); 
        int fr_block_sz = 0;
        while((fr_block_sz = recv(new_fd, receive_file, LENGTH, 0)) > 0) 
        {
          int write_sz = fwrite(receive_file, sizeof(char), fr_block_sz, fr);
          if(write_sz < fr_block_sz)
          {
            perror("File write failed on client.\n");
            exit(0);
          }
          bzero(receive_file, LENGTH);
          if (fr_block_sz == 0 || fr_block_sz != 512) 
          {
            break;
          }
        }
        if(fr_block_sz < 0)
        {
          if (errno == EAGAIN)
          {
            printf("recv() timed out.\n");
          }
          else
          {
            fprintf(stderr, "recv() failed due to errno = %d\n", errno);
            exit(1);
          }
        }
        printf("Ok received at client!\n");
        fclose(fr); 
      }
      success =1;
      close(new_fd);
      printf("[Client] Connection with Server closed. \n");
    }
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
  printf("Your md5 sum is: %s", getMd5sum(storeFilePath).c_str());
		// TODO : store this in some file 
  sendRequest(storeFilePath);
}
else{
  string md5;
  cout<<"Enter the md5 sum of the file you want to retrieve: ";
  cin>>md5;
  sprintf(request, "%s %d %s %s", myIp.c_str(), myTCPPort,"get", md5.c_str());
  sendRetrieve(md5);
}
}
