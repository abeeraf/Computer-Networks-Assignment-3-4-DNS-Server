#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string>
#include <iostream>
using namespace std;

#define MAXFD 10	//Size of fds array

string charPtrToString(char* toConvert,int y) {
	
	string str;
	for (int x = y; toConvert[x]; x++)
	str += toConvert[x];
	return str;
}

string ExtractMessage(char* toConvert) {
	
	string str;
	for (int x = 0; toConvert[x]; x++)
	str += toConvert[x];
	return str;
}

void fds_add(int fds[],int fd)	//Add a file descriptor to the fds array
{
	for(int i=0;i<MAXFD;++i)
	{
		if(fds[i]==-1)
		{
	     		fds[i]=fd;
		  	break;
		}
	}
}


int main()
{
	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	assert(sockfd!=-1);
	string rTable="";	
	string address[10][2];
	int num_addresses=0;

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(3000); // port number
	saddr.sin_addr.s_addr = inet_addr("192.168.0.163");

	int res = bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
	assert(res != -1);
	
	//Create listening queue
	listen(sockfd, 5);
	
	//Define fdset collection
	fd_set fdset;

	//Define fds array
	int fds[MAXFD];
	for (int x = 0; x < MAXFD; x++)
	fds[x]=-1;

	//Add a file descriptor to the fds array
	fds_add(fds, sockfd);

	int servers[3];
	int num_servers_connected=0;

	while (1) 
	{
		FD_ZERO(&fdset); //Clear the fdset array to 0
		int maxfd = -1;

		//For loop finds the maximum subscript for the ready event in the fds array
		for (int x = 0; x < MAXFD; x++) 
		{
			if (fds[x] == -1)
			continue;

			FD_SET(fds[x], &fdset);

			if (fds[x] > maxfd)
			maxfd = fds[x];
		}

		struct timeval tv = {0, 0};	
		int n = select(maxfd + 1, &fdset, NULL, NULL, &tv); //Select system call, where we only focus on read events
		
		if (n == -1)	//fail
		perror("select error");
	
		else 
		{
			//check all file descriptors 
			for(int x = 0; x < MAXFD; x++) 
			{
				if (fds[x] == -1)	//event not ready
				continue;

				//Determine if the event corresponding to the file descriptor is ready
				if (FD_ISSET(fds[x], &fdset)) 
				{
			   
					//There are two kinds of cases for judging file descriptors
			   		//A file descriptor is a socket, meaning accept if a new client requests a connection
					if (fds[x] == sockfd) 
					{
						//accept
						struct sockaddr_in caddr;
						int len = sizeof(caddr);
						int c = accept(sockfd, (struct sockaddr *)&caddr, (socklen_t*)&len);//Accept new client connections
						if (c < 0)
						continue;
						fds_add(fds, c);//Add the connection socket to the array where the file descriptor is stored
					}

					//Receive data recv when an existing client sends data
					else 
					{
						char buff[128] = {0};
						int res = recv(fds[x], buff, 127, 0);

						if(res <= 0) 
						{
							close(fds[x]);
							fds[x] = -1;
					 		printf("\nOne Client Over\n");
						}

						else
						{						//on 0 index we have server 1
							//servers[num_servers_connected]=fds[x];  //on 1 index we have server 2
							//num_servers_connected++;		//on 2 index we have server 4
												
							if(buff[0]=='R'){ //appending to routing table
							rTable=rTable+charPtrToString(buff,2);	// skip first char 'R'
							//for(int y = 0 ; y < 3 ; y++)
							//send(servers[y], rTable.c_str(), sizeof(rTable), 0);//Send Routing Table
							send(fds[1], rTable.c_str(), sizeof(rTable), 0);//Send Routing Table
							send(fds[2], rTable.c_str(), sizeof(rTable), 0);//Send Routing Table
							send(fds[3], rTable.c_str(), sizeof(rTable), 0);//Send Routing Table
							
							}

							else if(buff[0]=='M'){
							
							///Extract message

							int num_spaces=3;
							int x;
							for ( x = 1 ; x < sizeof(buff) ; x++){
							if(buff[x]==' ')
							num_spaces--;
							if (!num_spaces)
							break;
							}
							// routing table from x onwards
							string str;
							for (int y = 0; y < x; y++)
							str += buff[y]; //got Message String Now send it to Server with the Client
							//str = "M C Message"
								
							int ser_num_next;
							for (int y = 0; y < rTable.length(); y++)
							{
								if(rTable[y]==str[2] && rTable[y-1]=='*')
								{ //not the first value
									ser_num_next=rTable[y+2]-48;
									if(ser_num_next==2)
									send(fds[2], str.c_str(), sizeof(str), 0);
									else if (ser_num_next==4)
									send(fds[3], str.c_str(), sizeof(str), 0);
									break;
								}
								else if(rTable[y]==str[2]&& y==0) //its the first value so send to server 1
								{send(fds[1], str.c_str(), sizeof(str), 0);}

							}
							}

							else if(buff[0]=='W'){
							///Extract message
							int num_spaces=3;
							int x;
							for ( x = 1 ; x < sizeof(buff) ; x++){
							if(buff[x]==' ')
							num_spaces--;
							if (!num_spaces)
							break;
							}
							// routing table from x onwards
							string str;
							for (int y = 0; y < x; y++)
							str += buff[y]; //got Message String Now send it to Server4
							//str = "W C Website"
							//get Website name 
							string str3;
							for (int y = 4; y < x; y++)
							str3 += buff[y]; //str3 = "Website"
							//check if string is already in Proxy Server
							bool yes=0;
							int l;
							for ( l = 0; l < num_addresses; l++)
							{
								if (address[l][0]==str3)
								{
									yes=1;
									cout<<"Address found in proxy Server"<<endl;
									//send to client server after adding X
									string str2="X"+address[l][1];// X198.162.1.0
									//find client server
									int ser_num_next;
									for (int y = 0; y < rTable.length(); y++)
									{
										if(rTable[y]==str[2] && rTable[y-1]=='*')
										{ //not the first value
											ser_num_next=rTable[y+2]-48;
											if(ser_num_next==2)
											send(fds[2], str2.c_str(), sizeof(str2), 0);
											else if (ser_num_next==4)
											send(fds[3], str2.c_str(), sizeof(str2), 0);
											break;
										}
										else if(rTable[y]==str[2]&& y==0) //first value to server 1
										send(fds[1], str2.c_str(), sizeof(str2), 0);
									}

								}

							}
							if(!yes)//not in proxy server send to dns server
							//add website name to address array
							address[num_addresses][0]=str3; //str3 = "Website"
							send(fds[3], str.c_str(), sizeof(str), 0);//str="W C Website" 	
							}//////
							else if(buff[0]=='I'){ //Getting IP from DNS through server 4
							//buff-->I C IPADDRESS 
							cout<<"IP GOT THROUGH DNS"<<endl;
							address[num_addresses][1]=charPtrToString(buff,4);
							num_addresses++;
							//add address to proxy server
							//send it to server to send to clients
							string str=charPtrToString(buff,0);
							//find client server
							int ser_num_next;
							for (int y = 0; y < rTable.length(); y++)
							{
								if(rTable[y]==str[2] && rTable[y-1]=='*')
								{ //not the first value
									ser_num_next=rTable[y+2]-48;
									if(ser_num_next==2)
									send(fds[2], str.c_str(), sizeof(str), 0);
									else if (ser_num_next==4)
									send(fds[3], str.c_str(), sizeof(str), 0);
									break;
								}
								else if(rTable[y]==str[2]&& y==0) //first value to server 1
								send(fds[1], str.c_str(), sizeof(str), 0);
							}
							}
								
						}		
						
					}
				}
			}
		}
	}
}
