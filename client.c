#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAXIMUM 1001
#pragma diag_suppress 177

void error(char *msg)
{
    perror(msg);
    exit(0);
}

/*helper function which implements functionality of police customer and admin*/
void helper(char agent,int sockfd)
{
	if(agent=='c'){ //customer-------------------------------------
		char buff[MAXIMUM];
	    int n;
	    char flag;
	    
	    printf("Continue (y/n): ");
	    scanf("%c",&flag);
	    getchar();
	    
	    while(flag=='y')
	    {
	    	char operation[MAXIMUM];
	    	explicit_bzero(operation,MAXIMUM);
	        explicit_bzero(buff,MAXIMUM);
	        buff[0] = flag;
	        buff[1] = '\0';
	        n = write(sockfd,buff,strlen(buff));
	        if (n < 0) {
		    	printf("ERROR writing to socket\n");
		    	exit(0);
			}
	
	        printf("Operation [balance/mini_statement]: ");
	        fgets (operation, MAXIMUM, stdin);
	        
	        // sending command
	        n = write(sockfd,operation,strlen(operation));
	        if (n < 0){
	        	printf("ERROR writing to socket");
	        	exit(0);
			}
	            
	        
	        operation[strlen(operation)-1] = '\0';
	
	        // true or false
	        explicit_bzero(buff,MAXIMUM);
	        n = read(sockfd,buff,MAXIMUM-1);
	        if (n < 0){
	        	printf("ERROR reading to socket");
	        	exit(0);
			}
	        
	        if(!strcmp(buff,"false")) printf("Invalid Operation.\n\n"); 
	        else if(!strcmp(buff,"true"))
	        {
	            if(!strcmp(operation,"balance"))
	            {
	                // delimiter string
	                explicit_bzero(buff,MAXIMUM);
	                strcpy(buff,"content");
	                n = write(sockfd,buff,strlen(buff));
	                if (n < 0){
			        	printf("ERROR writing to socket");
			        	exit(0);
					}
			                // balance
	                explicit_bzero(buff,MAXIMUM);
	                n = read(sockfd, buff, MAXIMUM-1);
	                if (n < 0){
			        	printf("ERROR reading to socket");
			        	exit(0);
					}
	                printf("Your current BALANCE is: %s\n\n", buff);   
	            }
	            else if(!strcmp(operation,"mini_statement"))
	            {
	                // delimeter string
	                explicit_bzero(buff,MAXIMUM);
	                strcpy(buff,"size");
	                n = write(sockfd,buff,strlen(buff));
	                if (n < 0) {
			        	printf("ERROR writing to socket");
			        	exit(0);
					}
	                
	                // file size 
	                explicit_bzero(buff,MAXIMUM);
	                n = read(sockfd, buff, MAXIMUM-1);
	                if (n < 0) {
			        	printf("ERROR reading to socket");
			        	exit(0);
					}
	                
	                int file_size = atoi(buff);
	                int remain_data = file_size;
	                
	                // delimeter string
	                explicit_bzero(buff,MAXIMUM);
	                strcpy(buff,"content");
	                n = write(sockfd,buff,strlen(buff));
	                if (n < 0) 
	                    error("ERROR writing to socket");
	
	                // mini statement
	                printf("your MINI STATEMENT is : \n");
	                explicit_bzero(buff,MAXIMUM);
	                while ((remain_data > 0) && ((n = read(sockfd, buff, MAXIMUM)) > 0))
	                {
	                    printf("%s", buff);
	                    remain_data -= n;
	                    explicit_bzero(buff,MAXIMUM);
	                }
	                printf("\n\n");
	            }
	        }
	        printf("Continue [y/n]: ");
	        scanf("%c",&flag);
	        getchar();
	    }
	    // sending flag
	    explicit_bzero(buff,MAXIMUM);
	    buff[0] = flag;
	    buff[1] = '\0';
	    n = write(sockfd,buff,strlen(buff));  
	}
	else if(agent=='p'){ //police---------------------------------
		int n;
	    char flag;
	    char buff[MAXIMUM];
	    printf("Continue [y/n]: ");
	    scanf("%c",&flag);
	    getchar();
	    
	    while(flag=='y'){
	    	char user_id[MAXIMUM],operation[MAXIMUM];
	        explicit_bzero(buff,MAXIMUM);
	        explicit_bzero(operation,MAXIMUM);
	        explicit_bzero(user_id,MAXIMUM);

	        
	        buff[0] = flag;
	        buff[1] = '\0';
	        n = write(sockfd,buff,strlen(buff));
	        if (n < 0) {
			    printf("ERROR writing to socket");
			    exit(0);
			}
	            
	
	        /* input for services */
	        printf("Operation [balance/mini_statement]: ");
	        fgets (operation, MAXIMUM, stdin);
	        
	        printf("Customer User ID: ");
	        fgets (user_id, MAXIMUM, stdin);
	        
	        explicit_bzero(buff,MAXIMUM);
	        strcat(buff,operation);
	        strcat(buff,"$$$");
	        strcat(buff,user_id);
	
	        // sending command
	        n = write(sockfd,buff,strlen(buff));
	        if (n < 0){
			    printf("ERROR writing to socket");
			    exit(0);
			} 
	            
	
	        operation[strlen(operation)-1] = '\0';
	
	        // true or false
	        explicit_bzero(buff,MAXIMUM);
	        n = read(sockfd,buff,MAXIMUM-1);
	        if (n < 0) {
			    printf("ERROR reading from socket");
			    exit(0);
			}
	        
	        if(!strcmp(buff,"false")) printf("Invalid Operation.\n\n"); 
	        else if(!strcmp(buff,"true"))
	        {
	            if(!strcmp(operation,"balance"))
	            {
	            	// delimeter string
	                explicit_bzero(buff,MAXIMUM);
	                strcpy(buff,"content");
	                n = write(sockfd,buff,strlen(buff));
	                if (n < 0) {
					    printf("ERROR writing to socket");
					    exit(0);
					}
	
	                // balance
	                explicit_bzero(buff,MAXIMUM);
	                n = read(sockfd, buff, MAXIMUM-1);
	                if (n < 0) {
					    printf("ERROR reading to socket");
					    exit(0);
					}
	                    
	                printf("customer Account BALANCE: %s\n\n", buff);   
	            }
	            else if(!strcmp(operation,"mini_statement"))
	            {
	            	// delimeter string
	                explicit_bzero(buff,MAXIMUM);
	                strcpy(buff,"size");
	                n = write(sockfd,buff,strlen(buff));
	                if (n < 0) {
					    printf("ERROR writing to socket");
					    exit(0);
					}
	                
	                explicit_bzero(buff,MAXIMUM);
	                
	                // file size
	                n = read(sockfd, buff, MAXIMUM-1);
	                if (n < 0) {
					    printf("ERROR reading to socket");
					    exit(0);
					}
	                
	                int file_size = atoi(buff);
	                int remain_data = file_size;
	                
	                // delimeter string
	                explicit_bzero(buff,MAXIMUM);
	                strcpy(buff,"content");
	                n = write(sockfd,buff,strlen(buff));
	                if (n < 0) {
					    printf("ERROR writing to socket");
					    exit(0);
					}
	
	                // mini statement
	                printf("customer MINI STATEMENT is: \n");
	                explicit_bzero(buff,MAXIMUM);
	                while ((remain_data > 0) && ((n = read(sockfd, buff, MAXIMUM)) > 0))
	                {
	                    printf("%s", buff);
	                    remain_data -= n;
	                    explicit_bzero(buff,MAXIMUM);
	                }
	                printf("\n\n");
	            }
	        }
	        printf("Continue [y/n]: ");
	        scanf("%c",&flag);
	        getchar();
	    }
	    // sending flag
	    explicit_bzero(buff,MAXIMUM);
	    buff[0] = flag;
	    buff[1] = '\0';
	    n = write(sockfd,buff,strlen(buff));  
	}
	else{ //admin----------------------------
		char buff[MAXIMUM];
	    
	    int n;
	    char flag;
	    
	    printf("Want to continue [y/n]: ");
	    scanf("%c",&flag);
	    getchar();
	    
	    while(flag=='y'){
	    	char id[MAXIMUM],trans[MAXIMUM],amount[MAXIMUM];
	        explicit_bzero(buff,MAXIMUM);
	        explicit_bzero(id,MAXIMUM);
	        explicit_bzero(trans,MAXIMUM);
	        explicit_bzero(amount,MAXIMUM);
	        
	        buff[0] = flag;
	        buff[1] = '\0';
	        n = write(sockfd,buff,strlen(buff));
	        if (n < 0) 
	            error("ERROR writing to socket");
	
	        printf("Customer User ID: ");
	        fgets (id, MAXIMUM, stdin);
	        
	        printf("Transaction Type: ");
	        fgets (trans, MAXIMUM, stdin);
	
	        printf("Amount: ");
	        fgets (amount, MAXIMUM, stdin);
	
	        explicit_bzero(buff,MAXIMUM);
	        strcat(buff,id);
	        strcat(buff,"$$$");
			strcat(buff,trans);
	        strcat(buff,"$$$");
	        strcat(buff,amount);
	        strcat(buff,"$$$");
	
	        //sending command
	        n = write(sockfd,buff,strlen(buff));
	        if (n < 0){
	        	printf("ERROR writing to socket");
	        	exit(0);
			}
	            
	        // true or false
	        explicit_bzero(buff,MAXIMUM);
	        n = read(sockfd,buff,MAXIMUM-1);
	        if (n < 0){
	        	printf("ERROR reading to socket");
	        	exit(0);
			}
	        
	        if(!strcmp(buff,"false")) printf("Transaction denied.\n\n");
	        if(!strcmp(buff,"true")) printf("Transaction successful.\n\n");
	        if(!strcmp(buff,"deficit")) printf("Insufficient Amount.\n\n");
	
	        printf("Continue [y/n]: ");
	        scanf("%c",&flag);
	        getchar();
	    }
	    // sending flag
	    explicit_bzero(buff,MAXIMUM);
	    buff[0] = flag;
	    buff[1] = '\0';
	    n = write(sockfd,buff,strlen(buff));  
	}
	    
}

int main(int argc, char *argv[]){
    int sockfd, PORT, n;
    char buff[MAXIMUM];
    
    struct sockaddr_in server_addr;
    struct hostent *server;

    if (argc < 3) {
       fprintf(stderr,"Usage: %s hostname PORT\n", argv[0]);
       exit(0);
    }

    PORT = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server = gethostbyname(argv[1]);
    
    if (sockfd < 0 || server == NULL) {
    	printf("ERROR in opening socket OR Host not found\n");
    	exit(0);
	}
    
    explicit_bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(PORT);
    
    // connecting to server
    if (connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
    	printf("ERROR in connecting");
    	exit(0);
	}
        
    char user_type;
    while(1){
    	char username[MAXIMUM], password[MAXIMUM];
        // taking credentials from user   
        printf("Enter Credentails. Username & Password.\n");
        explicit_bzero(username,MAXIMUM);
        explicit_bzero(password,MAXIMUM);
        explicit_bzero(buff,MAXIMUM);
        
        printf("Username: ");
        fgets(username, MAXIMUM, stdin);
        printf("Password: ");
        fgets (password, MAXIMUM, stdin);
        
        
        strcat(buff,username);
        strcat(buff,"$$$");
        strcat(buff,password);

        // sending it to server
        n = write(sockfd,buff,strlen(buff));
        if (n < 0){
        	printf("ERROR reading from socket");
        	exit(0);
		}
        
        // false or exit or success (gives user type)
        explicit_bzero(buff,MAXIMUM);
        n = read(sockfd,buff,MAXIMUM-1);
        if (n < 0) {
        	printf("ERROR reading from socket");
        	exit(0);
		}
            
        if(!strcmp(buff,"exit")){
            printf("You entered the invalid credentials 3 times. Exiting...\n");
            return 0;
        }
        
        if(strcmp(buff,"false")){
            user_type = buff[0];
            break;
        }
    }
    
    /* welcome to the bank */
    switch(user_type){
        case 'A':
            printf("Welcome back Bank Admin.\n");
        	helper('a',sockfd); //admin()
            break;

        case 'C':
            printf("Welcome back Bank Customer.\n");
        	helper('c',sockfd); //customer()
            break;

        case 'P':
            printf("Welcome back Police.\n");
        	helper('p',sockfd); //police()
            break;

        default:
            printf("Invalid choice!!");
    }

    // close the socket
    close(sockfd);  
    return 0;
}
