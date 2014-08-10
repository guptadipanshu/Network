#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <map>
#include<vector>
#include<iostream>
#include <queue>  
#include <sys/time.h>  
using namespace std;
/**************Router program Implemented on Distance vector @Author -Dipanshu Gupta*************************/

int neighbours[10];	
int send_sockfd[10];
int main_sender_flag=0;
int send_flag=0;	
int start_routing_flag=0;
int signal_resend_table=0;
char filename[100];
char *message_buff;//the final message to be send to the final receiver.
char final_destination[100];
struct timeval begin, end;
int round_count=0;
int okay_to_print=0;
map<int,vector<int> > adjacency_list;//this is the forwading table data structure


struct addrinfo hints,*result;
struct sockaddr *sa;
struct sockaddr *sa_adjacent[10];//will store the address information of all neighbors
socklen_t sa_adj_len[10];//lenght of sa for all neighbours
socklen_t sa_len;
int my_bit;

void discoverNeighbors(int* retval)
{	
	int i;
	for(i=2; i<10; i++)
	{
		char cmdStr[100];
		char buf[1000];
		sprintf(cmdStr, "ping -c 1 -w 1 192.168.56.1%d", i);
		FILE* readCmd = popen(cmdStr, "r");
		fread(buf, 1, 1000, readCmd);
		pclose(readCmd);

		if(strstr(buf, "bytes from"))
		{
			retval[i]=1;
			
		}
		else
			retval[i]=-1;
	}
	//sleep(1);
}

int getNodeID()
{
	FILE* readCmd = popen("ifconfig | grep 192 | sed 's/^.*addr://' | sed 's/ .*//'", "r");
	char buf[100];
	fread(buf, 1, 100, readCmd);
	pclose(readCmd);
	return *(strstr(buf, "\n")-1)-'0';
}

int get_destination(char *buffer);
int get_hop(char*buffer);
int get_distance(char *buffer);
char* get_final_message(char *buffer);
char * create_packet(int destination,int hop,int distance);
void print_fw();
void * send_thread ( void *ptr );
void * receive_thread ( void *ptr );
void * timer_thread ( void *ptr );
void *check_neighbour_thread(void*ptr);

int main(int argc, char** argv)
{
	if(argc<1)
	{
		fprintf(stderr,"usage ./aout port");		
		exit(0);	
	}
	gettimeofday(&begin, NULL);
	if(argc>2)
	{
		memset(filename,'\0',(sizeof(filename)));
		memset(final_destination,'\0',(sizeof(final_destination)));
		main_sender_flag=1;		
		strcpy(filename,argv[1]);
		strcpy(final_destination,argv[2]);
		send_flag=1;	
	}
	fflush(stderr);
			
	my_bit=getNodeID();
		
	
	pthread_t time_thread;	
	pthread_create(&time_thread, NULL,&timer_thread,NULL);
	
		
	int * my_sock_fd=(int*)malloc(sizeof(int));   	
	*my_sock_fd=socket(AF_INET,SOCK_DGRAM,0);
	fprintf(stderr,"my socket fd %d\n",*my_sock_fd);
	memset(&hints,0x00,sizeof(struct addrinfo));
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=IPPROTO_UDP;
	hints.ai_family= AF_INET;
	
	char my_udpPort[100];
	sprintf(my_udpPort,"192.168.56.1%d", my_bit);
	char port[100];
	strcpy(port,"5555");
	getaddrinfo(my_udpPort,port,&hints,&result);//put my_port info into result
	
	int yes=1;
	if (setsockopt(*my_sock_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
 	{
   		fprintf(stderr,"Unable to connect maybe Host not found or wrong port\n");
   	}
	 
	if (bind(*my_sock_fd,result->ai_addr,result->ai_addrlen)) // put your information in the sock_fd
	{
		fprintf(stderr,"binding udp socket\n");
		exit(1);
	}
	fprintf(stderr,"ip is %s\n",my_udpPort);
	sa=(sockaddr*)malloc(result->ai_addrlen);
  	memcpy(sa, result->ai_addr, result->ai_addrlen);
  	sa_len=result->ai_addrlen;
	
		
	int i;	
	pthread_t check_neighbor_thread;
	pthread_t sender_thread;
	pthread_t receiver_thread;	
	pthread_create (&check_neighbor_thread, NULL,&check_neighbour_thread,NULL);	
	pthread_create (&receiver_thread, NULL,&receive_thread,my_sock_fd);	
	pthread_create (&sender_thread, NULL,&send_thread,NULL);
	pthread_join(check_neighbor_thread, NULL);
	pthread_join(sender_thread, NULL);
	pthread_join(receiver_thread,NULL);
	pthread_join(time_thread,NULL);			
	//print_fw();
	fprintf(stderr,"setup over\n");
	return 0;
}
/*******************Timer runs every 15seconds to deliver message to reciever******************************/
void * timer_thread ( void *ptr )
{
	if(main_sender_flag==1)
	{
		struct stat st;	
		stat(filename, &st);
    	int file_length = st.st_size;
    	if(file_length>1400)
   			file_length=1400;
		else
			file_length=file_length;//to avoid reading \n
		char buf[file_length+1];	
		memset(buf,'\0',(sizeof(buf)));
		FILE *fp;
		fp = fopen(filename,"rb");
		fread(buf,file_length,1,fp);
		//fprintf(stderr,"original %s",buf);
		fclose(fp);
		message_buff=(char*)malloc(sizeof(char)*(file_length+100));
		memset(message_buff,'\0',sizeof(char)*(file_length+100));
		char *header=create_packet(atoi(final_destination),0,0);
		char id_bit[3];
		memset(id_bit,'\0',(sizeof(id_bit)));
		strcpy(message_buff,header);
		free(header); 
		strcat(message_buff," ");
		strcat(message_buff,buf);
		sprintf(id_bit,"%d",my_bit);
		//fprintf(stderr,"sending after 15 seconds %s\n",id_bit);
		strcat(message_buff,id_bit);
		//fprintf(stderr,"sending after 15 seconds %s",message_buff);	
	}
	while(1)
	{
		//fprintf(stderr,"reached\n");		
		gettimeofday(&end, NULL);
		double elapsed = (end.tv_sec - begin.tv_sec);
		if(elapsed==15)	
		{
			if(send_flag==1)
			{
				vector<int> temp=adjacency_list[atoi(final_destination)];
				fprintf(stderr,"sending to final destination %s at hop %d\n",message_buff,temp[0]);
				if(temp[0]!=-1 && temp[1]<20)
				{
					sendto(send_sockfd[temp[0]],message_buff,strlen(message_buff),0 ,sa_adjacent[temp[0]],sa_adj_len[temp[0]]);
						
					if(main_sender_flag!=1)// we want the sender to send every 15 seconds
					{
						send_flag=0;			
						free(message_buff);
					}
				}
				else
					fprintf(stderr,"the link is down we will try after 15seconds \n");	
			}
			fprintf(stderr,"difference in time %lf\n",elapsed);
			round_count++;
			gettimeofday(&begin, NULL);
		}
	}
	return NULL;
}
/**************Neighbor checker thread*************************/

void *check_neighbour_thread(void*ptr)
{
	int neighbors[10];
	for(int i=2; i<10; i++)
	{
		vector<int> adjacent_cells(2);
		adjacent_cells[0]=-1;//next hop
		adjacent_cells[1]=-1;//distance
		adjacency_list.insert((make_pair(i, adjacent_cells)));	
		//fprintf(stderr,"created %d\n",i);	
	}
	okay_to_print=1;
	while(1)
	{
		discoverNeighbors(neighbors);
		int allow_multicast=0;
		for(int i=2; i<10; i++)
		{
			//printf("%d\n", neighbors[i]);
			vector<int>temp=adjacency_list[i];
			if(neighbors[i]==1)//this is a new link detected.
			{
				if(i==my_bit)//send packet to itself
				{
					temp[0]=i;//next hop
					temp[1]=0;//distance
					adjacency_list[i]=temp;
				}
				else
				{
					if(temp[1]!=1)
						allow_multicast=1;					
					temp[0]=i;//next direct hop
					temp[1]=1;//distance detween each neighbour
					adjacency_list[i]=temp;	
					//fprintf(stderr,"destination %d hop 1%d\n",i,i);					
				}
				char id[100];
				sprintf(id,"192.168.56.1%d",i);	
				//fprintf(stderr,"adding address of %s\n",id);	
				char port[100];
				strcpy(port,"5555");
				getaddrinfo(id,port,&hints,&result);
				sa_adjacent[i]=(sockaddr*)malloc(result->ai_addrlen);
				memcpy(sa_adjacent[i], result->ai_addr, result->ai_addrlen);
				sa_adj_len[i]=result->ai_addrlen;
				do
    			{			
					send_sockfd[i]=socket(result->ai_family, result->ai_socktype, result->ai_protocol);
					if(send_sockfd[i]>= 0)
    					break; /*success*/    					
				}while ((result=result->ai_next) != NULL);
				//fprintf(stderr,"%s %d\n",neighbour_port,send_sockfd[i]);							
			}
			else if(neighbors[i]==-1 && temp[1]!=-1)// oops link has gone down
			{
				temp[1]=20;//distance
				adjacency_list[i]=temp;
				allow_multicast=1;
				fprintf(stderr,"link %d has gone down\n",i);
			}
		}
		if(allow_multicast==1)
		{
			fprintf(stderr,"flag set for resend\n")	;		
			start_routing_flag=1;
			signal_resend_table=1;
		}
	}
	return NULL;
}
/*****************Sender thread to do multicast**********************/
void* send_thread(void *ptr)
{
	int i=2;	
	while(1)
	{	
		if(i<10 && start_routing_flag==1)
		{
			fprintf(stderr,"starting to send neighbors\n");
			for(i=2;i<10;i++)
			{
				//fprintf(stderr,"checking %d\n",i);				
				vector<int> temp=adjacency_list[i];
				if(temp[0]!=-1 && i!=my_bit)
				{
					char *buff=create_packet(i,my_bit,temp[1]);
					for(int j=2;j<10;j++)
					{
						temp=adjacency_list[j];
						if(signal_resend_table==1)
						{
									signal_resend_table=0;
									i=2;
									fprintf(stderr,"resend acknwledged \n");
									break;
						}
						if(temp[0]!=-1 && j!=my_bit && j!=i)//donot send to destination its own packet 
						{		
					 		sendto(send_sockfd[temp[0]],buff,strlen(buff),0 ,sa_adjacent[temp[0]],sa_adj_len[temp[0]]);				
							fprintf(stderr,"send %s\n",buff);
						}	
					}
					free(buff);
				}
				if(signal_resend_table==1)//our fw table was updated resend to everyone
				{
					signal_resend_table=0;
					i=2;
				}
			}
		}
		if(signal_resend_table==1)//our fw table was updated resend to everyone
		{
				fprintf(stderr,"resend table outer while\n");				
				signal_resend_table=0;
				i=2;
		}
	}
	fprintf(stderr,"while loop terminated\n");
	return NULL;
}
/*********Receiver thread updates forwarding table and asks for multicast if required*************/
void * receive_thread(void *ptr)
{
	int *rec_fd=(int *)ptr;	
	char buffer[1500];	
	fprintf(stderr,"reciever thread %d\n",*rec_fd);					
	while(1)
	{	
		//print_fw();
		memset(buffer,'\0',(sizeof(buffer)));
		int rc=recvfrom(*rec_fd,buffer, 1500, 0, NULL, NULL);
		int round_no=round_count;
		start_routing_flag=1;
		fprintf(stderr,"got %s\n",buffer);
		int next_hop=get_hop(buffer);
		int destination=get_destination(buffer);
		if(next_hop!=0)
		{
			int distance=get_distance(buffer);	
			vector<int> temp=adjacency_list[destination];
			if(( (temp[0]==-1) || (temp[0]!=-1 && temp[1]>distance+1)) && destination!=my_bit)
			{
				fprintf(stderr,"update fw table\n");
				temp[1]=distance+1;//+1 because we need to take one hop to the sender who had endorsed this path
				temp[0]=next_hop;//from where we got the packet
				adjacency_list[destination]=temp;
				signal_resend_table=1;
				//print_fw();
			}
			else if(distance==20 && next_hop==temp[0])
			{
				temp[1]=20;//+1 because we need to take one hop to the sender who had endorsed this path
				temp[0]=next_hop;//from where we got the packet
				adjacency_list[destination]=temp;
				signal_resend_table=1;
				//print_fw();	
			}
			else if(distance==20 && next_hop!=temp[0])
			{
				signal_resend_table=1;
				fprintf(stderr,"BROADCASTING OUR TABLE WE HAVE ALTERNATIVE PATH\n");
			}
		}
		else
		{
			//The message is either for us or for forwarding
			struct timeval final_time;
			gettimeofday(&final_time, NULL);
			double elapsed = (final_time.tv_sec - begin.tv_sec);
			if(elapsed>7)
				round_no++;
			if(destination==my_bit)//packet at final destination reached
			{
				char *final_message=get_final_message(buffer);
				fprintf(stderr,"final message %s\n",final_message);
				char output_file_name[20];
				memset(output_file_name,'\0',sizeof(output_file_name));
				sprintf(output_file_name,"message%d.txt",round_no);
				FILE *fp;
				fp = fopen(output_file_name,"wb+");
				fwrite(final_message,1,strlen(final_message),fp);
				fclose(fp);
				free(final_message);
			} 
			else //it is for forwarding to next hop
			{
				fprintf(stderr,"forwarding message\n");				
				message_buff=(char*)malloc(sizeof(char)*(strlen(buffer)+10));
				memset(message_buff,'\0',sizeof(char)*(strlen(buffer)+10));
				strcpy(message_buff,buffer);
				char id_bit[3];
				memset(id_bit,'\0',(sizeof(id_bit)));
				sprintf(id_bit,"%d",my_bit);
				strcat(message_buff,id_bit);
				vector<int> temp=adjacency_list[destination];
				if(temp[1]>=20)//the hop is unavialable for the destination
				{
					fprintf(stderr,"the link is down try after 15seconds\n");
					send_flag=1;
				}
				else
				{
					sendto(send_sockfd[temp[0]],message_buff,strlen(message_buff),0 ,sa_adjacent[temp[0]],sa_adj_len[temp[0]]);
					free(message_buff);
					fprintf(stderr,"forwarding %s\n",message_buff);
					
				}				
					
			}
		}
	}	
	return NULL;
}
/*************All functions below are to process the packets***********************/
char * create_packet(int destination,int next_hop, int distance)// create a packet given its destination
{
	char dest[10];					
	char nhop[10];					
	char dist[10];					
	char* buf=(char*)malloc(sizeof(char)*100);
	memset(dest,'\0',sizeof(dest));
	memset(nhop,'\0',sizeof(nhop));
	memset(dist,'\0',sizeof(dist));
	memset(buf,'\0',sizeof(char)*100);		 	
	sprintf(dest,"%d",destination);
	sprintf(nhop,"%d",next_hop);
	sprintf(dist,"%d",distance);
	strcpy(buf,dest);
	strcat(buf,".");
	strcat(buf,nhop);
	strcat(buf,".");
	strcat(buf,dist);
	return buf;
}

int get_destination(char *buffer)
{
	char dest[20];
	memset(dest,'\0',sizeof(dest));	
	for(int i=0;i<strlen(buffer);i++)
	{
		dest[i]=buffer[i];		
		if(buffer[i+1]=='.')
			break;	
	}
	return atoi(dest);
}
int get_hop(char*buffer)
{
	char hop[20];
	memset(hop,'\0',sizeof(hop));
	int k=0;	
	for(int i=0;i<strlen(buffer);i++)
	{		
		if(buffer[i+1]=='.')
		{
			for(int j=i+2;j<strlen(buffer);j++,k++)
			{
				hop[k]=buffer[j];	
				if(buffer[j+1]=='.')
					break;
			}	
			break;			
		}
	}
	return atoi(hop);
}
int get_distance(char *buffer)
{
	char dist[20];
	memset(dist,'\0',sizeof(dist));
	int k=0;	
	for(int i=0;i<strlen(buffer);i++)
	{		
		if(buffer[i+1]=='.')
		{
			for(int j=i+2;j<strlen(buffer);j++)
			{	
				if(buffer[j+1]=='.')
				{	for(int l=j+2;l<strlen(buffer);l++,k++)
					{
						dist[k]=buffer[l];
						//fprintf(stderr,"%c",buffer[l]);
					}
					break;				
				}			
			}	
			break;			
		}
	}
	return atoi(dist);
}
char* get_final_message(char *buffer)
{
	char *data=(char*)malloc(sizeof(char)*strlen(buffer));
	memset(data,'\0',sizeof(char)*strlen(buffer));
	int index=0;
	for(int i=0;i<strlen(buffer);i++)
	{
		if(buffer[i]=='.')
		{
			for(int j=i+1;j<strlen(buffer);j++)
			{	if(buffer[j]=='.')
				{
					for(int k=j+1;k<strlen(buffer);k++)
					{
						if(buffer[k]==' ')
						{
							for(int l=k+1;l<strlen(buffer);l++,index++)
							{
								data[index]=buffer[l];
							}
							break;
						}
					}
					break;	
				}	
			}
			break;
		}
	}
	return data;
}
void print_fw()
{
	if(okay_to_print==1)
	{	
		for(int i=2;i<10;i++)
		{
			fprintf(stderr,"destnation%d ",i);
			vector<int> temp=adjacency_list[i];
			fprintf(stderr,"next hop 1%d distance %d \n",temp[0],temp[1]);	
		}
		fprintf(stderr,"\n");	
	}
}
