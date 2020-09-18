#include "t_server.h"

void putfile(char *hostname,char *path,int connectfd,int dataconnectfd){
	char buffer[BUF]={"\0"};
	int filecontent;
	int readinbytes=0;
	char *bname;

	printf("Recieved Put command from Client: %s\n",hostname);
	printf("Transferring File\n");
	
	strtok(path,"\n");//parse newline		
	bname=basename(path);
	filecontent=open(bname,O_WRONLY | O_CREAT | O_EXCL,S_IRWXU);//open file
	if(filecontent<0){
		write(connectfd,"E\n",strlen("E\n"));
		close(filecontent);
		close(dataconnectfd);
		printf("***Error Creating file***\n");
		perror("filecontent");
		return;
	}
	else{
		write(connectfd,"A\n",strlen("A\n"));	
		while((readinbytes=read(dataconnectfd,buffer,sizeof(buffer)))){
			write(filecontent,buffer,readinbytes);
		}
			close(filecontent);
			close(dataconnectfd);
			printf("File Transfer Complete\n");
			return;
	}
	
	close(dataconnectfd);
	close(filecontent);
	printf("***Error Creating file***\n");
	write(connectfd,"E\n",strlen("E\n"));
	return;
}


void getfile(char *hostname,char *path,int connectfd,int dataconnectfd){
	char buffer[BUF]={"\0"};
	int filecontent;
	int readoutbytes=0;
	
	printf("Recieved Get command from Client: %s\n",hostname);
	printf("Transferring File\n");
	
	strtok(path,"\n");//parse newline
	filecontent=open(path,O_RDONLY);//open file
	if(filecontent<0){
		close(dataconnectfd);
		close(filecontent);
		printf("***Error Creating file***\n");
		write(connectfd,"E\n",strlen("E\n"));
		return;
	}
	else{	
		while((readoutbytes=read(filecontent,buffer,sizeof(buffer)))){
			write(dataconnectfd,buffer,readoutbytes);
		}
		close(filecontent);
		close(dataconnectfd);
		printf("File Transfer Complete\n");
		write(connectfd,"A\n",strlen("A\n"));
		return;
	}
	close(dataconnectfd);
	close(filecontent);
	printf("***Error Creating file***\n");
	write(connectfd,"E\n",strlen("E\n"));
	return;
}


void listdirectory(char *hostname,int connectfd,int dataconnectfd){
	int firstfork;

	printf("Received list directory command from Client: %s\n",hostname);
	if((firstfork=fork())){
		wait(&firstfork);
		printf("List directory completed\n");
		close(dataconnectfd);
		write(connectfd,"A\n",strlen("A\n"));
		return;
	}
	else{	
		dup2(dataconnectfd,1);
		if(execlp("ls","ls","-la",NULL)){
			printf("***Execlp Error***\n");
			printf("***Cound not run ls***\n");
			write(connectfd,"E\n",strlen("E\n"));
			return;

		}
			

		

	}

	return;
}


void changedirectory(char *hostname,char *path,int connectfd){
	strtok(path,"\n");
	printf("Received Change directory command from Client: %s\n",hostname);
	if(chdir(path)!=0){//changing directory to read files
		printf("***ERROR: Could not change directory***\n");
		perror("Chdir");
		write(connectfd,"E\n",strlen("E\n"));	
		return;
	}
	
	printf("Change Directory Successful\n");
	write(connectfd,"A\n",strlen("A\n"));	
	
	return;

}


int dataconnection(char *hostname,int connectfd){
	printf("Recieved Data Connection Command From Client: %s\n",hostname);
	printf("Connecting Data connection\n");
	

	int listenfd=0;
	int dataconnectfd=0;
	char portnumber[32];
	char clientsend[32]={"\0"};
	struct sockaddr_in servAddr;

	listenfd=socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd<0){
		printf("***Could not create a Dataconnection for Client1: %s***\n",hostname);
		printf("***SENDING ERROR TO CLIENT: %s***\n",hostname);
		write(connectfd,"E\n",strlen("E\n"));
		close(listenfd);
		return(0);
	}
	

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(0);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((bind(listenfd,(struct sockaddr *) &servAddr, sizeof(servAddr))) < 0) {
		printf("***Could not create a Dataconnection for Client2: %s***\n",hostname);
		printf("***SENDING ERROR TO CLIENT: %s***\n",hostname);
		perror("BIND");
		write(connectfd,"E\n",strlen("E\n"));
		close(listenfd);
		return(0);
	}
	

	struct sockaddr_in portsocket;
	unsigned int size = sizeof(portsocket);
	if(getsockname(listenfd, (struct sockaddr *) &portsocket, &size)!=0){
		printf("***Could not create a Dataconnection for Client3: %s***\n",hostname);
		printf("***SENDING ERROR TO CLIENT: %s***\n",hostname);
		write(connectfd,"E\n",strlen("E\n"));
		close(listenfd);
		return(0);
	}
    	
	int port = ntohs(portsocket.sin_port);
	sprintf(portnumber,"%d",port);

	printf("Dataconnection connected to Client: %s\n",hostname);
	clientsend[0]='A';
	for(int i=0;i<=strlen(portnumber);i++){
		clientsend[i+1]=portnumber[i];
	}
	clientsend[strlen(clientsend)]='\n';

	write(connectfd,clientsend,strlen(clientsend));
	listen(listenfd, 1);
	
	int length = sizeof(struct sockaddr_in);
	struct sockaddr_in clientAddr;
	dataconnectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length);
	if(dataconnectfd==-1){
		printf("***Could not create a Dataconnection for Client2: %s***\n",hostname);
		printf("***SENDING ERROR TO CLIENT: %s***\n",hostname);
		perror("accept");
		write(connectfd,"E\n",strlen("E\n"));
		close(listenfd);
		return(0);

	}
	
	return(dataconnectfd);
	
	
}

int startcommands(int connectfd,char *hostname){
	char clientcommand[BUF]={"\0"};
	int dataconnectfd=0;
	
	while(1){
		if(read(connectfd,clientcommand,sizeof(clientcommand))==0){
			close(dataconnectfd);
			return(1);
		}
		
		
		if(clientcommand[0]=='Q'){
			printf("Recieved Exit Command from Client: %s\n",hostname);
			printf("Sending confirmed Acknowledgement\n");
			printf("***Client %s Disconnected***\n",hostname);
			write(connectfd,"A\n",strlen("A\n"));
			return(0);

		}

		if(clientcommand[0]=='D'){
			dataconnectfd=dataconnection(hostname,connectfd);
		}
	
		if(clientcommand[0]=='C'){
			changedirectory(hostname,clientcommand+1,connectfd);
		}
	
		if(clientcommand[0]=='L'){
			listdirectory(hostname,connectfd,dataconnectfd);
		}

		if(clientcommand[0]=='G'){
			getfile(hostname,clientcommand+1,connectfd,dataconnectfd);
		}

		if(clientcommand[0]=='P'){
			putfile(hostname,clientcommand+1,connectfd,dataconnectfd);
		}
		
	}
}


int main(){
	
	struct sockaddr_in servAddr;
	int listenfd;
	int connectfd;
	struct hostent* hostEntry;
	pid_t child;
	int status;	
	char buffer[1024];


	listenfd = socket(AF_INET, SOCK_STREAM,0);



	memset( &servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(MY_PORT_NUMBER);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd,(struct sockaddr *) &servAddr,sizeof(servAddr))<0){
                perror("bind: ");
                return(-1);
 
        }

	int length = sizeof(struct sockaddr_in);
	struct sockaddr_in clientAddr;
	
	listen(listenfd,4);
	
	while(1){
		time_t timevar=time(NULL);

		if((connectfd = accept(listenfd,(struct sockaddr *) &clientAddr,&length))<0){
			perror("Connect: ");
			printf("Could not connect client\n");
			close(connectfd);
			return(-1);
		}
		else{
		hostEntry=gethostbyaddr(&(clientAddr.sin_addr),sizeof(struct in_addr),AF_INET);
		if(hostEntry==NULL){
			perror("Hostentry:  \n");
			return(-1);
		}		

		char* hostName;
		hostName=hostEntry->h_name;

		printf("Recieved connection from Client: %s\n",hostName);
	
		
		if(child=fork()){
			close(connectfd);	

		}else{
			int flag=1;
			flag=startcommands(connectfd,hostName);
			if(flag==1){
				printf("***ERROR CLIENT %s DISCONNECTED***\n",hostName);
			}
			close(connectfd);
			exit(1);	
		}
		
	}

	}





}
