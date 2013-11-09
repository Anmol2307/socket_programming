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
#include <unistd.h>

#define LENGTH 512 
using namespace std;

int requestType;              // To store the type of request of client
int contactedNode;            // The node to contact (id)
string configFile;            // Configuration file path
int backlog = 5;              // Variable to store backlog
string myIp = "127.0.0.1";    // Client's IP Address
int myUDPPort = 27381;        // The UDP port number of client
int myTCPPort = 27382;        // The TCP port number of client 
int N;                        // Total number of nodes
int tcpSockfd;                // The TCP socket file descriptor   
int udpSockfd;                // The UDP socket file descriptor 
char request[200];            // To store request of client

vector<nodeData> cloudNodesData; //Vector to store data for all the nodes of cloud

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
      char ip[30];
      int port;
      char foldPath[100];
      sscanf(line.c_str(), "%s %d %s", ip, &port, foldPath);
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

/*
 * To setup a UDP socket and bind it.
 * This function basically creates a socket corresponding to the client's IP and port 
 * returns relevant errors when socket could not be created/binded
 */

void setup_UDP(){

  //set the sockAddr object for own address.
  struct sockaddr_in myAddr;
  memset((char *)&myAddr, 0, sizeof(myAddr));
  myAddr.sin_family = AF_INET;
  myAddr.sin_addr.s_addr = inet_addr(myIp.c_str());
  myAddr.sin_port = htons(myUDPPort);

	//create a socket
  if ((udpSockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
    printf("[ERROR] Could not create socket. Exiting!!\n");
    exit(0);
  }	

	//bind the socket with the port and ip
  if (bind(udpSockfd, (struct sockaddr *)&myAddr, sizeof(myAddr)) == -1) {
    printf("[ERROR] Could not bind socket. Exiting!!\n");
    exit(0);
  } 

  printf("[Client] Socket created successfully.\n");
}

/*
 * To setup a TCP socket and bind it.
 * This function basically creates a socket corresponding to the client's IP and port 
 * returns relevant errors when socket could not be created/binded
 */
void setup_TCP(){

  //set the sockAddr object for own address.
  struct sockaddr_in myAddr;
  memset((char *)&myAddr, 0, sizeof(myAddr)); 
  myAddr.sin_family = AF_INET;
  myAddr.sin_addr.s_addr = inet_addr(myIp.c_str());
  myAddr.sin_port = htons(myTCPPort); 
  
  //create a socket
  if ((tcpSockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1){
    printf("[ERROR] Could not create TCP socket. Exiting!!\n");
    exit(0);
  } 

  //bind the socket with the port and ip
  if (bind(tcpSockfd, (struct sockaddr *)&myAddr, sizeof(struct sockaddr)) == -1) {
    printf("[ERROR] Could not bind TCP socket. Exiting!!\n");
    exit(0);
  } 

  printf("[Client] Socket created successfully.\n");

  // start listening on the socket, and grant new sockets when a new connection is made
  if (listen(tcpSockfd, backlog) == -1 ) {
    printf("[ERROR] Could not establish listen!! Exiting!\n");
    exit(0);
  }
  else {
    printf("[Client] Listening to TCP socket on IP = %s and Port = %d", myIp.c_str(), myTCPPort);
  }
}


/*
* Function to return md5 sum corresponding to a file
* Returns the md5 sum as a string of hex numbers
*/

string getMd5sum(string filePath){

	unsigned char c[MD5_DIGEST_LENGTH];
  int i;
  FILE *inFile = fopen (filePath.c_str(), "rb");
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];

  if (inFile == NULL) {
    printf ("[ERROR] %s can't be opened.\n", filePath.c_str());
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


/*
* Function to handle store request on the client. 
* The client sends the request to node, and then starts listening to a TCP port to hand out new connections to nodes.
*/
void storeRequest(string storeFilePath){
  // create a sockaddr object for connecting with the remote node
  struct sockaddr_in remoteAddr;
  memset((char *)&remoteAddr, 0, sizeof(remoteAddr));
  remoteAddr.sin_family = AF_INET;
  remoteAddr.sin_addr.s_addr = inet_addr(cloudNodesData[contactedNode].ipAddress.c_str());
  remoteAddr.sin_port = htons(cloudNodesData[contactedNode].portNo);

  // sned the file store request to remote node
  if (sendto(udpSockfd, request, strlen(request), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0){
    printf("[ERROR] Could not forward request!! Exiting!\n");
    exit(0);
  }
  else{
    // if request is sent successfully, then wait for some node to start a TCP connection with you
    int sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in nodeAddr;
    int new_fd ;

    printf("[Client] Waiting for Connection Establishment from Remote node!!\n");


    while( (new_fd = accept(tcpSockfd, (struct sockaddr *)&nodeAddr, (socklen_t *) &sin_size)) == -1 ){
      printf("[ERROR] Error in accept!! Exiting!\n");
      exit(0);     
    }
    
    // As soon as an accept is successful, start reading and sending the file.
    printf("[Client] Connection Established!!\n");
    
    char* send_file = (char *)storeFilePath.c_str();
    char file_buffer[LENGTH]; 
    FILE *fileOpen = fopen(send_file, "r");

    // Give error if file cannot be opened
    if(fileOpen == NULL)
    {
      printf("[ERROR] File %s not found.\n", send_file);
      exit(0);
    }
    printf("[Client] Sending %s to the Server... \n", send_file);
    
    bzero(file_buffer, LENGTH); 
    int block_size; 
    int flag = true;
    // Create chunks of the file, and send them sequentially to server (node)
    while((block_size = fread(file_buffer, sizeof(char), LENGTH, fileOpen)) > 0)
    {
      if(send(new_fd, file_buffer, block_size, 0) < 0)
      {
        printf("[ERROR] Failed to send file %s. (errno = %d)\n", send_file, errno);
        flag = false;
        break;
      }
      bzero(file_buffer, LENGTH);
    }

    // File has been sent successfully.
    if (flag){
      printf("[Client] File %s from is Sent!\n", send_file);
    }

    // Close the connection
    close(new_fd);
    printf("[Client] Connection with Server closed. \n");
  }
}


/*
* Function to handle get request on the client. 
* The client sends the request to node, and then starts listening to a TCP port to hand out new connections to nodes.
*/
void getRequest(string retrieveFilemf){
  // create a sockaddr object for connecting with the remote node
  struct sockaddr_in remoteAddr;
  memset((char *)&remoteAddr, 0, sizeof(remoteAddr));
  remoteAddr.sin_family = AF_INET;
  remoteAddr.sin_addr.s_addr = inet_addr(cloudNodesData[contactedNode].ipAddress.c_str());
  remoteAddr.sin_port = htons(cloudNodesData[contactedNode].portNo);

  // sned the file get request to remote node
  if (sendto(udpSockfd, request, strlen(request), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0){
    printf("[ERROR] Could not forward request!! Exiting!\n");
    exit(0);
  }
  else{
     // if request is sent successfully, then wait for some node to start a TCP connection with you
    int sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in nodeAddr;
    int new_fd ;

    printf("[Client] Waiting for Connection Establishment from Remote node!!\n");
    while( (new_fd = accept(tcpSockfd, (struct sockaddr *)&nodeAddr, (socklen_t *) &sin_size)) == -1 ){
      printf("[ERROR] Error in accept!! Exiting!\n");
      exit(0);     
    }
    
    // As soon as an accept is successful, start reading and sending the file.
    printf("[Client] Connection Established!!\n");
    

    // Initialise a variable to check if file was received properly or not
    int success = 0;
    while(success == 0)
    {
      // Start receiving file, continue recieving till success is 1
      char* write_file = (char *)retrieveFilemf.c_str();
      char receive_buffer[LENGTH];
      FILE *fileOpen = fopen(write_file, "w+");
      if(fileOpen == NULL){
        printf("[ERROR] Cannot open the file %s to write.\n", write_file);
        exit(0);        
      }
      else{
        bzero(receive_buffer, LENGTH); 
        int recv_block_size = 0;
        // Try writing to file whenever a new packet is recieved
        while((recv_block_size = recv(new_fd, receive_buffer, LENGTH, 0)) > 0) {
          int write_sz = fwrite(receive_buffer, sizeof(char), recv_block_size, fileOpen);
          if(write_sz < recv_block_size){
            printf("[ERROR] File write failed on client.\n");
            exit(0);
          }
          bzero(receive_buffer, LENGTH);
          // if (recv_block_size == 0 || recv_block_size != 512) 
            // break;
        }

        // Check if receive suffered any errors. 
        if(recv_block_size < 0){
          if (errno == EAGAIN){
            // Timeout Error
            printf("[ERROR] recv() timed out.\n");
            exit(0);
          }
          else{
            // Other errors
            printf("[ERROR] recv() failed due to errno = %d\n", errno);
            exit(0);
          }
        }
        // File received successfully
        printf("[Client] File received at client!\n");
        fclose(fileOpen); 
      }
      success = 1;
      // Close connection with Server
      close(new_fd);
      printf("[Client] Connection with Server closed. \n");
    }
  }
}


/*
 * Main Function
 * To  Handle interaction between program and user
 */
int main(){
  printf("Enter the number of nodes: ");
  scanf("%d", &N);
  printf("Enter the location of config file: ");
  cin>>configFile;
  while ( !readNodeData(configFile) ) {
    cout << "Enter new location of config file: ";
    cin >> configFile;
  }

  printf ("[Client] Creating UDP Socket...\n");
  setup_UDP();
  printf ("[Client] Creating TCP Socket...\n");
  setup_TCP();

  while (1){
    printf("\nSelect action to perform:\n 1 - Store a File\n 2 - Retrieve a file\n 0 - Quit\n");
    printf("Enter your choice: ")	;
    cin>>requestType;
    if (requestType == 0){
      printf("[Client] Thank you for using the FileMesh Program! \n");
      close(udpSockfd);
      close(tcpSockfd);
      return 0;
    }
    printf("Select node to connect, choose between 0 to %d: ", N-1);	
    cin>>contactedNode;

    // Handle store request
    if(requestType == 1){
      string storeFilePath;
      printf("Enter the file path to be stored: ");
      cin>>storeFilePath;

      // Make a request accordingly, to be sent to server
      sprintf(request, "%s %d %s %s", myIp.c_str(), myTCPPort,"store", getMd5sum(storeFilePath).c_str());
      printf("Your md5 sum is: %s\n", getMd5sum(storeFilePath).c_str()); 
      storeRequest(storeFilePath);
    }
    // Handle retrieve request
    else{
      string md5;
      printf("Enter the md5 sum of the file you want to retrieve: ");
      cin>>md5;

      // Make a request accordingly, to be sent to server
      sprintf(request, "%s %d %s %s", myIp.c_str(), myTCPPort,"get", md5.c_str());
      getRequest(md5);
    }
  }
}
