#include "t_server.h"


int socketmaker(char *portnum,char *address){
	struct sockaddr_in servAddr;
	struct in_addr **pptr;
	struct hostent* hostEntry;

	int datasocket;
	int portnumber=atoi(portnum);

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(portnumber);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	datasocket = socket(AF_INET, SOCK_STREAM, 0);
	
	hostEntry = gethostbyname(address);
	
	pptr = (struct in_addr **) hostEntry->h_addr_list;
	memcpy(&servAddr.sin_addr, *pptr, sizeof(struct in_addr));

	if(connect(datasocket,(struct sockaddr *) &servAddr,sizeof(servAddr))<0){
		perror("Error Data Connection: ");
		return(-1);
		
	}
	else{
		
		return(datasocket);

	}


}

void help(){
	printf("Function - usage\n");
	printf("To exit program type - exit\n");
	printf("To change local directory type - cd <pathname>\n");
	printf("To change server directory type - rcd <pathname\n");
	printf("To print local directory type - ls\n");
	printf("To print server directory type - rls\n");
	printf("To get a file from the server type - get <pathname>\n");
	printf("To show contents of a file from server type - show <pathname>\n");
	printf("To put a file on the server type - put <pathname>\n");
	return;
}


bool quit(int socketfd){
	char buffer[BUF];
	write(socketfd,"Q\n",strlen("Q\n"));
	while(read(socketfd,buffer,sizeof(buffer))>0){
		if(buffer[0]=='A'){						
			return(true);
		}
	}
	return(false);

}


bool changedirectory(char *path,int socketfd){
	
	if(chdir(path)!=0){//changing directory to read files
		return(false);
	}

	return(true);
}



bool listdirectory(){
	int firstfork;
		
	if((firstfork=fork())){
		wait(&firstfork);
		return(true);
	}
	else{
		int secondfork;
		int pipefd[2];

		if(pipe(pipefd)==-1){
		printf("Pipe Error in Listing Directory\n");
		printf("Please try again\n");
		return(false);
		}

		if((secondfork=fork())){
			close(pipefd[1]);
			dup2(pipefd[0],0);
			close(pipefd[0]);
			wait(&secondfork);
			if(execlp("more", "more", "-20", NULL)){
				printf("Execlp Error\n");
				printf("Cound not run ls\n");
				return(false);
			}
		}
		else{
			close(pipefd[0]);
			dup2(pipefd[1],1);
			close(pipefd[1]);
			if(execlp("ls","ls","-la",NULL)){
				printf("Execlp Error\n");
				printf("Cound not run ls\n");
				return(false);

			}
			

		}

	}
        return(false);
}


bool listserverdirectory(int socketfd,char *address){
	char buffer[BUF];
	char portnumber[BUF];
	int datasocket;
	
	write(socketfd,"D\n",strlen("D\n"));
	read(socketfd,portnumber,sizeof(portnumber));
	datasocket=socketmaker(portnumber+1,address);

	if(portnumber[0]=='A'){
		write(socketfd,"L\n",strlen("L\n"));
		read(socketfd,buffer,sizeof(buffer));
		if(buffer[0]=='A'){
			int childfork;
			if((childfork=fork())){
				wait(&childfork);
				close(datasocket);
				return(true);
			}
			else{
				dup2(datasocket,0);
				execlp("more", "more", "-20", NULL);

			}
		}

	}
	close(datasocket);					
	return(false);
}

bool serverchangedirectory(char *path,int socketfd){
	char buffer[BUF]={"\0"};
	char serverres[32];
	buffer[0]='C';
	for(int i=0;i<=strlen(path);i++){
		buffer[i+1]=path[i];
	}
	buffer[strlen(buffer)]='\n';
	write(socketfd,buffer,strlen(buffer));
	read(socketfd,serverres,sizeof(serverres));
	if(serverres[0]=='A'){
		return(true);
	}

	return(false);
}


bool serverget(char *path, int socketfd,char *address){
	char buffer[BUF]={"\0"};
	char serverres[32];
	char portnumber[BUF];
	int datasocket;
	char *bname;

	write(socketfd,"D\n",strlen("D\n"));
	read(socketfd,portnumber,sizeof(portnumber));
	datasocket=socketmaker(portnumber+1,address);

	if(portnumber[0]=='A'){
		buffer[0]='G';
		for(int i=0;i<=strlen(path);i++){
			buffer[i+1]=path[i];
		}
		buffer[strlen(buffer)]='\n';
		write(socketfd,buffer,strlen(buffer));
		read(socketfd,serverres,sizeof(serverres));
		if(serverres[0]=='A'){
			int filecontent;
			int readinbytes=0;
			bname=basename(path);
			filecontent=open(bname,O_WRONLY | O_CREAT | O_EXCL,S_IRWXU);
			if(filecontent<0){
				close(filecontent);
				close(datasocket);
				return(false);
			}
			
			while((readinbytes=read(datasocket,buffer,sizeof(buffer)))){

				write(filecontent,buffer,readinbytes);
			}
			close(filecontent);
			close(datasocket);
			return(true);
		}
	}
	close(datasocket);
	return(false);
}


bool servershow(char *path, int socketfd,char *address){
	char buffer[BUF]={"\0"};
	char serverres[32];
	char portnumber[BUF];
	int datasocket;

	write(socketfd,"D\n",strlen("D\n"));
	read(socketfd,portnumber,sizeof(portnumber));
	datasocket=socketmaker(portnumber+1,address);

	if(portnumber[0]=='A'){
		buffer[0]='G';
		for(int i=0;i<=strlen(path);i++){
			buffer[i+1]=path[i];
		}
		buffer[strlen(buffer)]='\n';
		write(socketfd,buffer,strlen(buffer));
		read(socketfd,serverres,sizeof(serverres));
		if(serverres[0]=='A'){
			int childfork;
			if((childfork=fork())){
				wait(&childfork);
				close(datasocket);
				return(true);
			}
			else{
				dup2(datasocket,0);
				execlp("more", "more", "-20", NULL);

			}
		}
	}
	close(datasocket);
	return(false);

}

bool serverput(char *path, int socketfd,char *address){
	char buffer[BUF]={"\0"};
	char serverres[32];
	char portnumber[BUF];
	int datasocket;
	char pathname[BUF]={"\0"};
	char *bname;

	write(socketfd,"D\n",strlen("D\n"));
	read(socketfd,portnumber,sizeof(portnumber));
	datasocket=socketmaker(portnumber+1,address);

	if(portnumber[0]=='A'){
		pathname[0]='P';
		bname=basename(path);
		for(int i=0;i<=strlen(bname);i++){
			pathname[i+1]=bname[i];
		}
		pathname[strlen(pathname)]='\n';
		write(socketfd,pathname,strlen(pathname));
		read(socketfd,serverres,sizeof(serverres));
		if(serverres[0]=='A'){
			int filecontent;
			int readoutbytes=0;
			filecontent=open(path,O_RDONLY);
			if(filecontent<0){
				close(datasocket);
				close(filecontent);
				return(false);
			}
		
			while((readoutbytes=read(filecontent,buffer,sizeof(buffer)))){

				write(datasocket,buffer,readoutbytes);
			}
			close(filecontent);
			close(datasocket);
			return(true);
		}
	}
	close(datasocket);
	return(false);
}	



void startcommands(int socketfd,char *address){
	char entirecommand[BUF];
	char first[BUF];
	char second[BUF];
	int commandflag=0;
	int testcommand=0;	
	
	while(1){
		printf("MFTP>");
		fgets(entirecommand,BUF,stdin);
		if(sscanf(entirecommand,"%s %s",first,second)==2){
			commandflag=1;

		}
		

		if(commandflag==0){
			if(strcmp(first,"exit")==0){
				if(quit(socketfd)==true){
					printf("***EXITING***\n");
					return;
				}
				else{
					printf("***Error Exiting***\n");
				}
				

			}


			if(strcmp(first,"ls")==0){
				testcommand=1;                                
				if(listdirectory()==true){
					printf("***Done Listing***\n");
			
				}
				else{
					printf("***Error Listing***\n");
				}
				
            }        
			
			
			if(strcmp(first,"rls")==0){
				testcommand=1;
				if(listserverdirectory(socketfd,address)==true){
					printf("***DONE LISTING***\n");

				}
				else{
					printf("***Error Listing***\n");
				}
			}

			if(strcmp(first,"help")==0){
				testcommand=1;
				help();

			}

			
		
			
		}
		else if(commandflag==1){
			commandflag=0;
			if(strcmp(first,"cd")==0){
				testcommand=1;
				if(changedirectory(second,socketfd)==true){
					printf("***Directory Changed***\n");
				}
				else{
					printf("***Error: Pathname invalid could not change directory***\n");
				}
			}

			if(strcmp(first,"rcd")==0){
				testcommand=1;
				if(serverchangedirectory(second,socketfd)==true){
					printf("***Server Directory Changed***\n");
				}
				else{
					printf("***Error: Pathname invlaid could not change server directory***\n");
				}
			}

			if(strcmp(first,"get")==0){
				testcommand=1;
				if(serverget(second,socketfd,address)==true){
					printf("***SEVER GET COMPLETED***\n");
				}
				else{
					printf("***Error: Either invalid path or file already in directory***\n");
				}

			}

			if(strcmp(first,"show")==0){
				testcommand=1;
				if(servershow(second,socketfd,address)==true){
					printf("***SERVER SHOW COMPLETED***\n");
				}
				else{
					printf("***Error: Either invalid path or could not read file***\n");
				}
			}

			if(strcmp(first,"put")==0){
				testcommand=1;
				if(serverput(second,socketfd,address)==true){
					printf("***SEVER PUT COMPLETED***\n");
				}
				else{
					printf("***Error: Either invalid path or cound not read file***\n");
				}
			}

		}
		

		if(testcommand==0){
			printf("***No command Recognized***\n");
			
		}
		else{
			testcommand=0;
		}
		
		memset(&first,'\0',sizeof(first));
		memset(&second,'\0',sizeof(second));
		memset(&entirecommand,'\0',sizeof(entirecommand));

	}

 return;
}



int main(int argc,char** argv){
	
	if(argc!=2){
		printf("Need to have ip address\n");
		return(0);

	}

	struct sockaddr_in servAddr;
	struct in_addr **pptr;
	struct hostent* hostEntry;

	int socketfd;

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(MY_PORT_NUMBER);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	
	hostEntry = gethostbyname(argv[1]);
	
	pptr = (struct in_addr **) hostEntry->h_addr_list;
	memcpy(&servAddr.sin_addr, *pptr, sizeof(struct in_addr));
	
	

	if(connect(socketfd,(struct sockaddr *) &servAddr,sizeof(servAddr))<0){
		perror("Error Connect: ");
		return(-1);
		
	}
	else{
		printf("\n");
		printf("**********CONNECTION MADE*************\n");
		
		startcommands(socketfd,argv[1]);
		close(socketfd);

	}
	
	return(0);

}
