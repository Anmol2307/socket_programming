#include "node.h"

int node::N;
vector<nodeData> node::cloudNodesData;

node::node(){

}

/*
 * Constructor Function
 */
 node::node(int n, vector<nodeData> cldNodesData){
 	N = n;
 	cloudNodesData = cldNodesData;
 }

/*
 * Convert a hex char to decimal int
 */
 int node::hexToInt(char c){
 	if(c >= '0' && c <= '9') return c-'0';
 	else return c-'a'+10;
 }

/*
 * Function to convert a given md5 sum in hex to decimal and get the modulo with number of nodes
 */
 int node::bigModulo(string str, int N){
 	int	l = str.length();
 	if(l == 1) return hexToInt(str[0])%N;
 	return (((bigModulo(str.substr(0,l-1), N) % N) * 16%N) + hexToInt(str[l-1])%N)%N;
 }

/*
 * All functionality of nodes is handled here.
 * Creates a socket for listening to UDP connections and appropriately forwards the requests.
 * If needed, creates a TCP connection with client and sends/receives files.
 */
 node::node(int i){

 	id = i;
 	nodeData nData = cloudNodesData[i];

	//set the sockAddr object for own address.
 	struct sockaddr_in myAddr;
 	memset((char *)&myAddr, 0, sizeof(myAddr));
 	myAddr.sin_family = AF_INET;
 	myAddr.sin_addr.s_addr = inet_addr(nData.ipAddress.c_str());
 	myAddr.sin_port = htons(nData.portNo);

  int mySocket;  // UDP socket to listen to incoming requests
	//create a socket
  if ((mySocket=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
  	printf("[ERROR] Could not create socket. Exiting!!\n");
  	exit(0);
  }	

	//bind the socket with the port and ip
  if (bind(mySocket, (struct sockaddr *)&myAddr, sizeof(myAddr)) == -1) {
  	printf("[ERROR] Could not bind socket %d. Exiting!!\n", id);
  	exit(0);
  } 

  printf("[Server] Socket created successfully.\n");
	//start listening

  int recvlen; 							//length of data received;
  char receivedData[2048]; 	//buffer in which the received data is stored
  struct sockaddr_in remoteAddr; // sockaddr object to store the information for incoming request 
  socklen_t remoteLen = sizeof(remoteAddr);

  char ip[30];
  int port;
  char requestType[10];
  char mf[40];

  printf ("[Server] Listening for incoming requests...\n");

  // Continue listening to UDP socket
  for (;;){
  // Get a request from remote node to process
  	recvlen = recvfrom(mySocket, receivedData, 2048, 0, (struct sockaddr *)&remoteAddr, &remoteLen);
  	if(recvlen > 0){
  		receivedData[recvlen] = 0;

			//Request received is of type ipaddr port requestType(store/get) mf(c-string)
  		sscanf(receivedData, "%s %d %s %s", ip, &port, requestType, mf);
  		printf("[Server] Request Received from %s, port %d of type %s! \n",ip, port, requestType);

			//Check if this request belongs to this server
  		if(bigModulo(mf,N) == id){
				//start a tcp connection and process the request depending upon its type i,e store or get
  			printf("[Server] Establishing TCP connection with client...\n");

  			int sockfd; 
				struct sockaddr_in remoteClient_addr; // will hold the destination address 
				if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1){ // TCP connection
					printf("[ERROR] Could not create TCP socket. Exiting!!\n");
				exit(0);
				} 
				remoteClient_addr.sin_family = AF_INET; // host byte order 
				remoteClient_addr.sin_port = htons(port); // network byte order 
				remoteClient_addr.sin_addr.s_addr = inet_addr(ip); // Destination IP address
				memset(&(remoteClient_addr.sin_zero), '\0', 8); // zero the rest of the struct 

				// Establish connection
				if (connect(sockfd, (struct sockaddr *)&remoteClient_addr, sizeof(struct sockaddr)) == -1){
					printf("[ERROR] Could not connect to client. Please check connections!!\n");
					//exit(0);
				} 	
				else {
					printf("[Server] Connected successfully!\n");
				}

				// Process a store request
				if (strcmp(requestType,  "store") == 0){
					int success = 0;
					while(success == 0)
					{
						// Start receiving file, continue recieving till success is 1
						string path = nData.folderPath.append("/");
						string finalPath = path.append(mf);
						char* write_file = (char *)finalPath.c_str();
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
							while((recv_block_size = recv(sockfd, receive_buffer, LENGTH, 0)) > 0) {
								int write_sz = fwrite(receive_buffer, sizeof(char), recv_block_size, fileOpen);
								if(write_sz < recv_block_size){
									printf("[ERROR] File write failed on server.\n");
									exit(0);
								}
								bzero(receive_buffer, LENGTH);
							}
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
							printf("[Server] File received at Server!\n");
							fclose(fileOpen); 
						}
						success = 1;
						// Close connection with Client
						close(sockfd);
						printf("[Server] Connection with Client closed. Server will wait now...\n");
					}


				}
				else if (strcmp(requestType,  "get") == 0){
					string path = nData.folderPath.append("/");
					string finalPath = path.append(mf);
					char* write_file = (char *)finalPath.c_str();
					char send_buffer[LENGTH];
					// Open file to read
					FILE *fileOpen = fopen(write_file, "r");

					// Give error if file cannot be opened
					if(fileOpen == NULL)
					{
						printf("ERROR: File %s not found.\n", write_file);
						exit(0);
					}

					printf("[Server] Sending %s to the Client... ", write_file);
					bzero(send_buffer, LENGTH); 
					int block_size; 
					int flag = true;
					// Create chunks of the file, and send them sequentially to server (node)

					while((block_size = fread(send_buffer, sizeof(char), LENGTH, fileOpen)) > 0)
					{
						if(send(sockfd, send_buffer, block_size, 0) < 0)
						{
							printf( "[ERROR] Failed to send file %s. (errno = %d)\n", write_file, errno);
							flag = false;
							break;
						}
						bzero(send_buffer, LENGTH);
					}
					// File has been sent successfully.
					if (flag){
						printf("[Server] File %s from is Sent!\n", write_file);
					}
					 // Close the connection
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
				printf ("[Server] Forwarding request to correct node... \n");
				if (sendto(mySocket, receivedData, strlen(receivedData), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0){
					printf("[Error] Could not forward request!! Exiting!\n");
					exit(0);
				}
			}
		}	
	}
}