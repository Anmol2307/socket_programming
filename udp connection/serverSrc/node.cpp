#include "node.h"

int node::N;
vector<nodeData> node::cloudNodesData;

node::node(){

}

node::node(int n, vector<nodeData> cldNodesData){
	N = n;
	cloudNodesData = cldNodesData;
}

int node::hexToInt(char c){
	if(c >= '0' && c <= '9') return c-'0';
	else return c-'a'+10;
}

int node::bigModulo(string str, int N){
	int	l = str.length();
	if(l == 1) return hexToInt(str[0])%N;
	return (((bigModulo(str.substr(0,l-1), N) % N) * 16%N) + hexToInt(str[l-1])%N)%N;
}

node::node(int i){
	cout<<i<<endl;
	//exit(0);

	id = i;
	nodeData nData = cloudNodesData[i];

	//set the sockAddr object for own address.
	struct sockaddr_in myAddr;
	memset((char *)&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = inet_addr(nData.ipAddress.c_str());
	myAddr.sin_port = htons(nData.portNo);

	int mySocket;
	//create a socket
	if ((mySocket=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		printf("Could not create socket. Exiting!!\n");
		exit(0);
	}	

	//bind the socket with the port and ip
	if (bind(mySocket, (struct sockaddr *)&myAddr, sizeof(myAddr)) == -1) {
		printf("Could not bind socket %d. Exiting!!\n", id);
		exit(0);
	} 

	//start listening

	int recvlen; //length of data received;
	char receivedData[2048]; //buffer in which the received data is stored
	struct sockaddr_in remoteAddr;
	socklen_t remoteLen = sizeof(remoteAddr);

	char ip[30];
	int port;
	char reqType[10];
	char mf[40];

	cout<<"Now listening "<<id<<endl;

	for (;;){
		recvlen = recvfrom(mySocket, receivedData, 2048, 0, (struct sockaddr *)&remoteAddr, &remoteLen);
		if(recvlen > 0){
			receivedData[recvlen] = 0;
			//request received is of type ipaddr port requestType(store/get) mf(c-string)
			sscanf(receivedData, "%s %d %s %s", ip, &port, reqType, mf);
			cout<<"Request Received: "<<receivedData<<endl;
			//check if this request belongs to this server
			if(bigModulo(mf,N) == id){
				//start a tcp connection and process the request depending upon its type i,e store or get
				//TODO
				cout<<"Start TCP conn"<<endl;
				// #define DEST_IP "10.2.44.57" 
				// #define DEST_PORT 5000 
				
				int sockfd; 
					struct sockaddr_in remoteClient_addr; // will hold the destination addr 
					sockfd = socket(PF_INET, SOCK_STREAM, 0); 
					
					//port = 27381;
					// cout<<"IP of client is "<<ip<<" and port is "<<port<<endl;
					// exit(0);

					remoteClient_addr.sin_family = AF_INET; // host byte order 
					remoteClient_addr.sin_port = htons(port); // network byte order 
					remoteClient_addr.sin_addr.s_addr = inet_addr(ip); 
					memset(&(remoteClient_addr.sin_zero), '\0', 8); // zero the rest of the struct 
					// printf ("HERE %s, %d \n", ip, port);
					// TODO : bind???

					// cout<<"IP of client is "<<ip<<" and port is "<<port<<endl;
					// exit(0);


					if (connect(sockfd, (struct sockaddr *)&remoteClient_addr, sizeof(struct sockaddr)) == -1){
						printf("Could not connect to client. Exiting!!\n");
						//exit(0);
					} 	
					else {
						printf("Connect stage successful.\n");
					}
					//char * msg = "hi!";
					if (strcmp(reqType,  "store") == 0){
						int success = 0;
						while(success == 0)
						{

							/*Receive File from Client */
							string path = nData.folderPath.append("/");
							string finalPath = path.append(mf);
							char* fr_name = (char *)finalPath.c_str();
							char receive_file[LENGTH];
							FILE *fr = fopen(fr_name, "a");
							if(fr == NULL)
								printf("File %s Cannot be opened file on server.\n", fr_name);
							else
							{
								bzero(receive_file, LENGTH); 
								int fr_block_sz = 0;
								while((fr_block_sz = recv(sockfd, receive_file, LENGTH, 0)) > 0) 
								{
									int write_sz = fwrite(receive_file, sizeof(char), fr_block_sz, fr);
									if(write_sz < fr_block_sz)
									{
										perror("File write failed on server.\n");
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
								printf("Ok received from client!\n");
								fclose(fr); 
							}
							success =1;
							close(sockfd);
							printf("[Server] Connection with Client closed. Server will wait now...\n");
						}

						
					}
					else if (strcmp(reqType,  "get") == 0){
						
							/*Receive File from Client */
						string path = nData.folderPath.append("/");
						string finalPath = path.append(mf);
						char* fr_name = (char *)finalPath.c_str();
						char send_file[LENGTH];
						FILE *fileOpen = fopen(send_file, "r");
						if(fileOpen == NULL)
						{
							printf("ERROR: File %s not found.\n", send_file);
							exit(0);
						}

						bzero(send_file, LENGTH); 
						int block_size; 
						while((block_size = fread(send_file, sizeof(char), LENGTH, fileOpen)) > 0)
						{
							if(send(sockfd, send_file, block_size, 0) < 0)
							{
								fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", send_file, errno);
								break;
							}
							bzero(send_file, LENGTH);
						}
						printf("Ok File %s from Server was Sent!\n", send_file);
						
						close(sockfd);
						printf("[Server] Connection with Client closed. Server will wait now...\n");
						
					}
				}
				else{
				//forward the packet to correct destination
					memset((char *)&remoteAddr, 0, sizeof(remoteAddr));
					remoteAddr.sin_family = AF_INET;
					remoteAddr.sin_addr.s_addr = inet_addr(cloudNodesData[bigModulo(mf,N)].ipAddress.c_str());
					remoteAddr.sin_port = htons(cloudNodesData[bigModulo(mf,N)].portNo);
					cout<<"Request Received: "<<receivedData<<endl;
					cout<<"FORWARDING TO CORRECT NODE"<<endl;
					if (sendto(mySocket, receivedData, strlen(receivedData), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0){
						perror("Could not forward request!! Exiting!\n");
						exit(0);
					}
				}
			}	
		}
	}