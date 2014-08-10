/***client****/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/********************HTTP CLient Server Model author @Dipanshu Gupta***********/
char * gethost(char * input)
{
    char *token;
    char * pch;
    pch = strstr (input,":");
    if(pch==NULL)
    {
    	char s[2] = "/";
    	token = strtok(input, s);
    }
    else
    {
    	char s[2] = ":";
        token = strtok(input, s);
    }
    return token;
}

char* getport(char *input)
{
    char *token;
    char s[2] = ":";
    token = strtok(input, s);
   // fprintf(stderr,"input %s\n",token);
    char s2[2]="/";
    token=strtok(NULL, s2);
    if(token==NULL)
    {
        token="80";
    }
    return token;
}

char * getfilepath(char *input)
{
    //fprintf(stderr,"input %s\n",input);
    char * token;
    char s[2] = "/";
    token= strtok(input,s);
    token= strtok(NULL,"");
    return token;
}

int main(int argc, char *argv[])
{
    //general input hostname:port/file
    if (argc != 2)
    {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    char *input_string;
    input_string=argv[1];
    char input[4096];
    strcpy(input,input_string);
    char hostname[4096];
    strcpy(hostname,gethost(input));
    strcpy(input,input_string);
    char *port=getport(input);
    strcpy(input,input_string);
    char *file_path=getfilepath(input);
    //fprintf(stderr,"hostname %s port %s path %s\n",hostname,port,file_path);
		
/***************************Setup sockets***************************************************/

	int socket_fd=socket(AF_INET,SOCK_STREAM,0);//AF_INET = for IPv4, SOCK_STREAM=TCP, Proctol=0
	struct addrinfo hints,*result;
	memset(&hints,0x00,sizeof(hints));//make sure struct is empty
	hints.ai_family= AF_INET; //IPv4
	hints.ai_socktype=SOCK_STREAM;//TCP
	getaddrinfo(hostname,port,&hints,&result);
	connect(socket_fd,result->ai_addr,result->ai_addrlen);
	char request[4096];
	strcpy(request,"GET /");
	strcat(request,file_path);
	strcat(request," ");
	strcat(request,"HTTP/1.0\r\n\r\n");
	write(socket_fd,request,strlen(request));
	//fprintf(stderr,"request %s\n",request);
	char resp[2001];
	int len;
	FILE *fp;
	fp = fopen("output", "w");
//This will print the file requested 
	do{ 
	
		bzero(resp, sizeof(resp));
		len=read(socket_fd,resp,2000);
		if(len>0)
		{
			//resp[len]='\0';
			char * pch= strstr(resp,"\r\n\r\n");
			
			fprintf(stderr,"%s\n",resp);
			if(pch!=NULL)
			{
				pch=pch+4;
				fprintf(fp,"%s",pch);
				
				len=read(socket_fd,resp,2000);
				while(len>0)
				{
					fprintf(fp,"%s",resp);
					bzero(resp, sizeof(resp));
					len=read(socket_fd,resp,2000);	
				}
				fclose(fp);
				exit(1);	
			}
			//fprintf(fp,"%s",resp);

		}
	}while(len>0);
	fclose(fp);

}
