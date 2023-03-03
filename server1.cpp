#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <pthread.h>
using namespace std;

#define MAXFD 10	//Size of fds array

string charPtrToString(char* toConvert,int y) {
	
	string str;
	for (int x = y; toConvert[x]; x++)
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
	///////////////////////////////////////////////////CONNECT TO PROXY SERVER///////////////////////////////////////////
	int proxy_sockfd = socket(AF_INET,SOCK_STREAM,0);	
	assert(proxy_sockfd != -1 );

	//Set Address Information
	struct sockaddr_in psaddr;
	memset(&psaddr,0,sizeof(psaddr));
	psaddr.sin_family = AF_INET;
	psaddr.sin_port = htons(3000);
	psaddr.sin_addr.s_addr = inet_addr("192.168.0.163");

	//Link to server
	int resp = connect(proxy_sockfd,(struct sockaddr*)&psaddr,sizeof(psaddr));
	assert(resp != -1);
	//////////////////////////////////////////////////CONNECT TO CLIENTS/////////////////////////////////////////////////
	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	assert(sockfd!=-1);	

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(7678); // port number
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
	fds_add(fds, proxy_sockfd);

	string ser1_mess="";
	int clients[2];
	int num_clients_connected=0;	
	string port_number;
	string rTable;

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
						clients[num_clients_connected]=c;
						num_clients_connected++;
						port_number=to_string(caddr.sin_port);
						if (c < 0)
						continue;
						fds_add(fds, c);//Add the connection socket to the array where the file descriptor is stored
					}
					if (fds[x] == proxy_sockfd) //receive from Proxy Server
					{
						char buff[128] = {0};
						int res = recv(fds[x], buff, 127, 0); //recieve from both 1 and 2 append to a string

						if(res <= 0) 
						{
							close(fds[x]);
							fds[x] = -1;
							printf("\nOne Client Over\n");
						}

						else
						{
							if (buff[0]=='M') //Buffer string will start from 
							{
								//str = "M C Message"
								string str=charPtrToString(buff,0);
								//if(buff[2]=='1')
								send(fds[1], str.c_str(), sizeof(str), 0);
								//else if(buff[2]=='2')
								send(fds[2], str.c_str(), sizeof(str), 0);
								
							}
							else if (buff[0]=='X')
							{
								cout<<"Sending Address from Proxy Server"<<endl;
								//str = "XIPADDRESS"
								string str=charPtrToString(buff,0);
								//if(buff[2]=='1')
								send(fds[1], str.c_str(), sizeof(str), 0);
								//else if(buff[2]=='2')
								send(fds[2], str.c_str(), sizeof(str), 0);
							}
							else if (buff[0]=='I')
							{
								cout<<"Sending Address from DNS-->Proxy Server-->Clinet"<<endl;
								//str = "I C IPADDRESS"
								string str=charPtrToString(buff,0);
								//if(buff[2]=='1')
								send(fds[1], str.c_str(), sizeof(str), 0);
								//else if(buff[2]=='2')
								send(fds[2], str.c_str(), sizeof(str), 0);
							}
							else
							{	rTable=charPtrToString(buff,0);
								cout<<"Routing Table Received from PServer: "<<rTable<<endl;
								//buff is the full routing table sent
								//cout confirmation message
								int count=0; 
								for (int x = 0 ; x < sizeof(buff) ; x ++)
								if(buff[x]=='*')
								count++;

								for (int x = 0 ; x < sizeof(buff) ; x ++){
								
								if(count==1){
								cout<<"C"<<buff[x++]<<" is Connected to S";
								x++;
								cout<<buff[x++]<<" and has port number ";

								for ( x=x+1; buff[x]!='*' ; x ++)
								cout<<buff[x];
								cout<<endl;break;
								}

								else if(buff[x]=='*')
								count--;
								}
							}						
						}
					}
					//Receive data recv when an existing client sends data
					else 
					{
						char buff[128] = {0};
						int res = recv(fds[x], buff, 127, 0); //recieve from both 1 and 2 append to a string

						if(res <= 0) 
						{
							close(fds[x]);
							fds[x] = -1;
							printf("\nOne Client Over\n");
						}

						else
						{	
							//add port number to buff and then send
							string str=charPtrToString(buff,0)+" "+port_number+"*"; 
							clients[num_clients_connected]=fds[x]; //on 0 index we have client 1
							num_clients_connected++;
							if(str[0]=='M' && str[2]==1)
							send(fds[2], str.c_str(), sizeof(str), 0);
							else if(str[0]=='M' && str[2]==2)
							send(fds[1], str.c_str(), sizeof(str), 0);
							else
							send(proxy_sockfd, str.c_str(), sizeof(str), 0);
						}
					}
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	close(sockfd);

}




