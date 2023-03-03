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
#include <fstream>
using namespace std;
#define MAXFD 10 //Size of fds array

string charPtrToString(char* toConvert,int y) {
	
	string str;
	for (int x = y; toConvert[x]; x++)
	str += toConvert[x];
	return str;
}
void fds_add(int fds[], int fd) //Add a file descriptor to the fds array
{
	int i = 0;
	for (; i < MAXFD; ++i)
	{
		if (fds[i] == -1)
		{
			fds[i] = fd;
			break;
		}
	}
}

int main()
{
	int num_client;
	cout<<"For DNS Server Enter '7' as Client Number."<<endl;
	cout<<"Enter Client Number:"<<endl;
	cin>>num_client;

	int num_server;
	cout<<"Client "<<num_client<<" is Online"<<endl;

	///////////////////////////////////////CONNECTING CLIENTS WITH RESPECTIVE SERVERS////////////////////////////
	int sockfd = socket(AF_INET,SOCK_STREAM,0);	
	assert(sockfd != -1 );

	//Set Address Information
	struct sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	
	if (num_client==1 || num_client==2)
	saddr.sin_port = htons(7678),num_server=1;
	else if(num_client==3 || num_client==4)
	saddr.sin_port = htons(2000),num_server=2;
	else if(num_client==5 || num_client==6 || num_client==7)
	saddr.sin_port = htons(4000),num_server=4;
	saddr.sin_addr.s_addr = inet_addr("192.168.0.163");

	//Link to server
	int res = connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	assert(res != -1);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	fd_set fdset;

	//Define fds array
	int fds[MAXFD], fds1[MAXFD];
	int i = 0;
	for (; i < MAXFD; ++i)
	{
		fds[i] = -1;
	}

	//Add a file descriptor to the fds array
	fds_add(fds, sockfd);

	while(1){
	int maxfd = -1;
		int i = 0;
		for (; i < MAXFD; i++)
		{
			if (fds[i] == -1)
			{
				continue;
			}

			FD_SET(fds[i], &fdset);

			if (fds[i] > maxfd)
			{
				maxfd = fds[i];
			}
		}

		struct timeval tv = {5, 0};
	//if (num_client==7)
	//goto fordns;


	int opt;
	if(num_client!=7){
	cout<<"Choose Option:"<<endl;
	cout<<"(1)Message other Client."<<endl; //Will need full routing table for this
	cout<<"(2)Get Address from DNS server."<<endl;
	cout<<"(3)Just Connect."<<endl;
	cout<<"(4)Exit."<<endl;
	cout<<"(5)Enter Receiving State."<<endl;}
	else {
	cout<<"(3)Just Connect."<<endl;
	cout<<"(4)Exit."<<endl;
	cout<<"(5)Enter Receiving State."<<endl;
	}
	cin>>opt;

	
	string str="";
	string str2="";
	
	//M is to message client format ---> M(ClientNumber)(Message)
	//W is for Website format ---> W(Website Name)
	//For None will send just send R
	char buff[128]={0};
	int client_to_mess;

	
	if (opt==5)
	{
		
		
		int n = select(maxfd + 1, &fdset, NULL, NULL, &tv);
		if (n == -1)
		{
			perror("select error");
		}
		else
		{
			for (i = 0; i < MAXFD; ++i)
			{
				if (fds[i] == -1)
				{
					continue;
				}
				if (FD_ISSET(fds[i], &fdset))
				{
					if (fds[i] == sockfd)
					{
					
						int res = recv(sockfd,buff,127,0);cout<<buff<<endl;
						if(res>0)
						{
							if(buff[0]=='M')
							{
								char reply;
								string str=charPtrToString(buff,3);
								cout<<"Message Recieved from Client"<<buff[2]-48<<" "<<str<<endl;
								cout<<"Reply [Y/N]"<<endl;
								cin>>reply;
								if(reply=='Y'||reply=='y')
								{
									//client_to_mess=buff[2]-48;
									goto here;

								}
								else
								continue;
								
								
							}
							if(buff[0]=='W'&&num_client==7)// Let DNS Server do Its thing
							{
								//IF DNS SERVER
								char buff[128]={0};
								recv(sockfd,buff,127,0); //buff --->W C website
								string domain=charPtrToString(buff,5); //this is just website like google 
								string word;
								string found;       //If the website is found in the DNS Server
								fstream obj("dns.txt", ios::in);

								string temp = "www."+domain+".com";          //Adding "www." at start of the domain name
								while (!obj.eof())
								{
									obj >> word;
									if (word == temp)
									obj >> found;
								}
								obj.close();
								cout<<"IP ADDRESS IS :"<<found<<endl;
								found="I "+to_string(buff[2]-48)+" "+ found;
								send(sockfd,found.c_str(), sizeof(found),0);//--->I C IP
								//break;
							}
							if(buff[0]=='W' && num_client==!7)// Let DNS Server do Its thing
							{
								break;
							}
							if(buff[0]=='X')
							{
								cout<<"Received address from ProxyServer."<<endl;
								string str=charPtrToString(buff,1);
								cout<<"ADDRESS OF WEBSITE: "<<str<<endl;
							}
							if(buff[0]=='I')
							{
								cout<<"Received address from ProxyServer through DNS."<<endl;
								string str=charPtrToString(buff,4);
								cout<<"ADDRESS OF WEBSITE: "<<str<<endl;
							}
						}
					
					}
				}
				else
				{
					cout << "Sorry!! No Notification\n";
				}
			}
		}
		
	
	}
	else if (opt==1)
	{
		here:
		cout<<"Enter the Client Number of the Client you Need to Message"<<endl;
		cin>>client_to_mess;
		//here:
		cout<<"Enter the Message"<<endl;
		cin>>str2;
		str="M "+to_string(client_to_mess)+" "+str2+" "+to_string(num_client)+" "+to_string(num_server);
		send(sockfd,str.c_str(), sizeof(str),0);

	}
	else if (opt==2)
	{ 
		cout<<"Enter the website you need the address of"<<endl;
		cin>>str;
		str="W "+to_string(num_client)+" "+str+" "+to_string(num_client)+" "+to_string(num_server);
		send(sockfd,str.c_str(), sizeof(str),0);
	}
	else if (opt==3)
	{
		//fordns:
		str="R "+to_string(num_client)+" "+to_string(num_server);
		send(sockfd,str.c_str(), sizeof(str),0);
		//if(num_client==7)
		//goto dnsHop;
	}
	else if (opt==4)
	break;

	}
	close(sockfd);

}
