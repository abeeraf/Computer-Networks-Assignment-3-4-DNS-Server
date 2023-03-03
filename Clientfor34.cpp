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

//make a file to maintain number of clients which can count Client number.
int CountTotalClients()
{
	string str;
	int x=0;
	fstream obj;

	obj.open("cnum.txt");
	while (getline(obj,str))
	x++;
	obj.close();
	
	x=x%7;
	if(x==0)//incase file is not refreshed
	{
		ofstream obj;	//adding a star to make it 8
		obj.open("cnum.txt", std::ios_base::app);
		obj<<"*"<<endl;
		obj.close();
		return 1;
	}
	return x; 
}

int main()
{
	ofstream obj;	//adding a star to count clients,in the file cnum.txt
	obj.open("cnum.txt", std::ios_base::app);
	obj<<"*"<<endl;
	obj.close();

	int num_client=CountTotalClients();
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
	else if(num_client==5 || num_client==6)
	saddr.sin_port = htons(4000),num_server=4;
	saddr.sin_addr.s_addr = inet_addr("192.168.0.163");

	//Link to server
	int res = connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
	assert(res != -1);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	cout<<"HERERERECLIENTTTT"<<endl;	
	while(1){

	int opt;
	cout<<"Choose Option:"<<endl;
	cout<<"(1)Message other Client."<<endl; //Will need full routing table for this
	cout<<"(2)Get Address from DNS server."<<endl;
	cout<<"(3)Just Connect."<<endl;
	cout<<"(4)Exit."<<endl;
	cin>>opt;

	
	string str="";
	string str2="";
	
	//M is to message client format ---> M(ClientNumber)(Message)
	//W is for Website format ---> W(Website Name)
	//For None will send just send R
	char* buff[128]={0};
	int client_to_mess;	
	if (opt==1)
	{
		cout<<"Enter the Client Number of the Client you Need to Message"<<endl;
		cin>>client_to_mess;
		cout<<"Enter the Message"<<endl;
		cin>>str2;
		str="M "+to_string(client_to_mess)+" "+str2+" "+to_string(num_client)+" "+to_string(num_server);
		send(sockfd,str.c_str(), sizeof(str),0);
		memset(buff,0,128);
		recv(sockfd,buff,127,0);
		cout<<"Message Recieved from Client"<<client_to_mess<<" "<<buff<<endl;	
	}
	 if (opt==2)
	{ 
		cout<<"Enter the website you need the address of"<<endl;
		cin>>str;
		str="W "+str+" "+to_string(num_client)+" "+to_string(num_server);
		send(sockfd,str.c_str(), sizeof(str),0);
		memset(buff,0,128);
		recv(sockfd,buff,127,0);
		cout<<"ADDRESS OF WEBSITE: "<<buff<<endl;
	}
	if (opt==3)
	str="R "+to_string(num_client)+" "+to_string(num_server);
	if (opt==4)
	break;
	}
	close(sockfd);

}
