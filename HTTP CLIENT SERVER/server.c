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
/********************HTTP CLient Server Model author @Dipanshu Gupta***********/
int count=0;
pthread_t threads[4096];
int client_fd[4096];
struct addrinfo hints,*result;
void handler(int sig)
{
	
	fflush(stdout);
	int i=0;
	for(i=0;i<count;i++)
	{	
		pthread_join(threads[i],NULL);
		
	}	
	freeaddrinfo(result); 
	exit(0);
}
void *thread_work(void *ptr)
{
	int *client_fd=(int*)ptr;
	char buffer[4096];
    int len= read(*client_fd,buffer,4095);
    buffer[len]='\0';
    //fprintf(stderr,"%s",buffer);
    char* filepath=strtok(buffer,"/");
    filepath=strtok(NULL," ");
    fprintf(stderr,"%s\n",filepath);
    int found=0;
    if(access(filepath,F_OK)==0)
    {
    	found=1;
    	//fprintf(stderr,"FILE FOUND\n");	
    }
    int buffer_size=1000;
	char *header=malloc(buffer_size+1);
	//char header[4096];
	strcpy(header,"HTTP/1.0 "); 
	if(found)
		strcat(header,"200 OK\r\n\r\n");
	else
		strcat(header,"404 Not Found\r\n\r\n");
	
	int header_len=strlen(header);
	if(found)//get the contents from file to pipe
	{
		FILE *fp;
		fp = fopen(filepath,"rb");
		int read=0;
		int content_length=0;
		struct stat st;	
		stat(filepath, &st);
        	content_length = st.st_size;
        	char head[content_length+100];
		while((read= fread(head,1, content_length,fp))>0 )
        	{
        	header_len += read;
        	
        	/*if(header_len>=buffer_size)
        	{ 
        		buffer_size *=2;
        		
        		header=realloc(header,buffer_size+1);
        	}
        	*/
        	}
        	header=realloc(header,content_length+1000);
        	//fprintf(stderr,"head %s\n",head);
        	strcat(header,head);
		fclose(fp);
	
	}
	else
	{
		char * not_header="<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1></body></html>";
		strcat(header,not_header);
		header_len=strlen(header);
	}
	//fprintf(stderr,"%s",header);
	send(*client_fd, header,header_len, 0);
	
	close(*client_fd);
	pthread_exit(NULL);
} 
int main(int argc, char *argv[])
{
    //general input hostname:port/file
    if (argc != 2)
    {
        fprintf(stderr,"usage: [portnumber]\n");
        exit(1);
    }
    char *port=argv[1];
    int sock_fd=socket(AF_INET,SOCK_STREAM,0);
   
    memset(&hints,0x00,sizeof(struct addrinfo));
    hints.ai_family= AF_INET;
    hints.ai_flags=AI_PASSIVE;
    hints.ai_socktype=SOCK_STREAM;
    getaddrinfo(NULL,port,&hints,&result);
    int yes=1;
		// lose the pesky "Address already in use" error message
	if (setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
 	{
   		fprintf(stderr,"i setsockopt\n");
   	} 
	else
		fprintf(stderr,"setsockopt ENABLED\n");	
    bind(sock_fd,result->ai_addr,result->ai_addrlen);
    listen(sock_fd,10);
    /*******************************************************************************************/
    signal(SIGINT,handler);
	while((client_fd[count]=accept(sock_fd,NULL,NULL)))
	{
		pthread_create(&threads[count],NULL,thread_work,&client_fd[count]);
		count++;
	}
   
    
			
}     
