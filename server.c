#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <stdlib.h>
#include <sys/sendfile.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <time.h>

#pragma warning(suppress : 4996)
#define MAXIMUM 1200

char *clientIp;
int clientPort;

///////////////////////////////////////////////////////////////////////////// helper function //////////////////////////////////////////////////////////////////////////

void helper(int sockFd,char userType,int cust_id)
{
    if(userType == 'C')
    {
        // if the user is customer
        int n;
        char buffer[MAXIMUM];
        char id[MAXIMUM];
        sprintf(id,"%d",cust_id);

        //Reading flag into buffer
        memset(buffer,0, MAXIMUM);
        n = read(sockFd,buffer,MAXIMUM-1);
        if (n < 0) 
            error("error reading from the socket");
    
        while(buffer[0]=='y')
        {      
            // reading command into buffer
            memset(buffer,0,MAXIMUM);
            n = read(sockFd,buffer,MAXIMUM-1);
            if (n < 0)
            {
                error("error reading from the socket");
            } 
            buffer[strlen(buffer)-1] = '\0';
        
            if(!strcmp(buffer,"balance"))
            {
                // sending signal true
                memset(buffer,0,MAXIMUM);
                strcpy(buffer,"true");
                n = write(sockFd,buffer,strlen(buffer));  
                if (n < 0) 
                {
                    error("error writing to the socket!");
                }
            
                // delimeter string
                memset(buffer,0,MAXIMUM);
                n = read(sockFd,buffer,MAXIMUM-1);
                if (n < 0) 
                {
                    error("error reading from the socket");
                }
            
                balanceAvailable(sockFd,id);
            }
            else if(!strcmp(buffer,"mini_statement"))
            {
                // sending signal true
                memset(buffer,0,MAXIMUM);
                strcpy(buffer,"true");
                n = write(sockFd,buffer,strlen(buffer));  
                if (n < 0) 
                {
                    error("error writing to the socket!");
                }
            
                // delimeter string
                memset(buffer,0,MAXIMUM);
                n = read(sockFd,buffer,MAXIMUM-1);
                if (n < 0) 
                {
                    error("error reading from the socket");  
                }
            
                mini_statement(sockFd,id); 
            }
            else
            {
                fprintf(stdout, "Request from client with ip '%s' declined. \n", clientIp);    
                // sending signal false
                memset(buffer,0,MAXIMUM);
                strcpy(buffer,"false");
                n = write(sockFd,buffer,strlen(buffer));  
                if (n < 0) 
                {
                    error("error writing to the socket!");
                }
            } 
            /* Reading flag */
            memset(buffer,0,MAXIMUM);
            n = read(sockFd,buffer,MAXIMUM-1);
            if (n < 0)
            {
                error("error reading from the socket");
            } 
        }
    }
    else if(userType == 'A')
    {
        char id[MAXIMUM], trans[MAXIMUM], amount[MAXIMUM];
        char buffer[MAXIMUM];
        int n;

        // Reading flag
        memset(buffer,0,MAXIMUM);
        n = read(sockFd,buffer,MAXIMUM-1);
        if (n < 0)
        {
            error("error reading from the socket");
        } 
            
        while(buffer[0]=='y')
        {   
            // reading command from the buffer
            memset(buffer,0,MAXIMUM);
            n = read(sockFd,buffer,MAXIMUM-1);
            if (n < 0) 
            {
                error("error reading from the socket");
            }
        
            memset(amount,0,MAXIMUM);
            memset(id,0,MAXIMUM);
            memset(trans,0,MAXIMUM);


            char *ptr = strtok(buffer,"$$$");
            strcpy(id,ptr);
            ptr = strtok(NULL, "$$$");
            strcpy(trans,ptr);
            ptr = strtok(NULL, "$$$");
            strcpy(amount,ptr);

            id[strlen(id)-1] = '\0';
            trans[strlen(trans)-1] = '\0';
            amount[strlen(amount)-1] = '\0';
        
            // validating the user id
            char *cred = NULL;
            size_t len = 0;
            int check = 0;
            FILE *fp = fopen("login_file.txt","r");
            if(fp == NULL)
            {
                error("Error in opening login_file.");
            }
        
            while(getline(&cred,&len,fp)!=-1)
            {
                char *username = strtok(cred," ");
                strtok(NULL," ");
                char *usertype = strtok(NULL, " ");
            
                if(!strcmp(username,id))
                {
                    check=1;
                    if(usertype[0]=='C')
                    {
                        check=2;
                    }
                    break;
                } 
            }
            free(cred);
            fclose(fp);

            // sending false signal for admin and police accounts
            if(check==0 || check==1 || (strcmp(trans,"credit") && strcmp(trans,"debit")) || !is_valid(amount))
            {
                fprintf(stdout, "Request from client with ip '%s' declined. \n", clientIp);
                memset(buffer,0,MAXIMUM);
                strcpy(buffer,"false");
                n = write(sockFd,buffer,strlen(buffer));  
                if (n < 0) 
                    error("error writing to the socket!");
            }
            //if the account is of the customer
            else
            {
                if(!strcmp(trans,"credit"))
                {
                    credit_amount(id,amount,trans);
                    fprintf(stdout, "Credit request from client with ip '%s' for customer '%s' successfully executed. \n", clientIp, id );
                    // sending signal true 
                    memset(buffer,0,MAXIMUM);
                    strcpy(buffer,"true");
                    n = write(sockFd,buffer,strlen(buffer));  
                    if (n < 0) 
                    {
                        error("error writing to the socket!");
                    }
                }
                else if(!strcmp(trans,"debit"))
                {
                    int f = debit_amount(id, amount, trans);
                    // sending true
                    if(f==1)
                    {
                        fprintf(stdout, "Debit request from client with ip '%s' for customer '%s' successfully executed. \n", clientIp, id );
                        memset(buffer,0,MAXIMUM);
                        strcpy(buffer,"true");
                        n = write(sockFd,buffer,strlen(buffer));  
                        if (n < 0) 
                        {
                            error("error writing to the socket!");
                        }
                    }
                    else
                    {
                        // insufficient amount
                        fprintf(stdout, "Debit request from client with ip '%s' declined. \n", clientIp);
                        memset(buffer,0,MAXIMUM);
                        strcpy(buffer,"deficit");
                        n = write(sockFd,buffer,strlen(buffer));  
                        if (n < 0)
                        {
                            error("error writing to the socket!");
                        } 
                    } 
                }
            }

            // Reading flag 
            memset(buffer,0,MAXIMUM);
            n = read(sockFd,buffer,MAXIMUM-1);
            if (n < 0) 
            {
                error("error reading from the socket");
            }
        }
    }
    else if(userType == 'P')
    {

        char fileName[100],op[MAXIMUM],id[MAXIMUM];
        char buffer[MAXIMUM];
        int n;

        // reading flag from buffer
        memset(buffer,0,MAXIMUM);
        n = read(sockFd,buffer,MAXIMUM-1);
        if (n < 0) 
        {
            error("error reading from the socket");
        }
    
        while(buffer[0]=='y')
        {
            int check = 0;
            // reading command from buffer
            memset(buffer,0,MAXIMUM);
            n = read(sockFd,buffer,MAXIMUM-1);
            if (n < 0) 
            {
                error("error reading from the socket");
            }
        
            // breaking command into operation and user_id
            char *ptr = strtok(buffer,"$$$");
            strcpy(op,ptr);
            ptr = strtok(NULL,"$$$");
            strcpy(id,ptr);

            op[strlen(op)-1] = '\0';
            id[strlen(id)-1] = '\0';
        

            // validating user_id
            char *cred = NULL;
            size_t len = 0;
        
            FILE *fp = fopen("login_file.txt","r");
            if(fp == NULL)
            {
                error("Error in opening login_file.");
            }
        
            while(getline(&cred,&len,fp)!=-1)
            {
                char *username = strtok(cred," ");
                strtok(NULL," ");
                char *usertype = strtok(NULL, " ");
            
                if(!strcmp(username,id))
                {
                    check=1;
                    if(usertype[0]=='C')
                    {
                        check=2;
                    }
                    break;
                } 
            }
            fclose(fp);
            free(cred);
        
            // sending false
            if(check==0 || check==1 || (strcmp(op,"balance") && strcmp(op,"mini_statement")))
            {
                fprintf(stdout, "Request from client with ip '%s' declined. \n", clientIp);    
                memset(buffer,0,MAXIMUM);
                strcpy(buffer,"false");
                n = write(sockFd,buffer,strlen(buffer));  
                if (n < 0) 
                {
                    error("error writing to the socket!");
                }
            }
            else
            {
                if(!strcmp(op,"balance"))
                {
                    // sending true to buffer
                    memset(buffer,0,MAXIMUM);
                    strcpy(buffer,"true");
                    n = write(sockFd,buffer,strlen(buffer));  
                    if (n < 0) 
                    {
                        error("error writing to the socket!");
                    }
                
                    // delimeter string
                    memset(buffer,0,MAXIMUM);
                    n = read(sockFd,buffer,MAXIMUM-1);
                    if (n < 0) 
                    {
                        error("error reading from the socket");  
                    }
                
                    balanceAvailable(sockFd,id);
                }
                else if(!strcmp(op,"mini_statement"))
                {
                    // sending true to buffer
                    memset(buffer,0,MAXIMUM);
                    strcpy(buffer,"true");
                    n = write(sockFd,buffer,strlen(buffer));  
                    if (n < 0) 
                    {
                        error("error writing to the socket!");
                    }
                
                    // delimeter string
                    memset(buffer,0,MAXIMUM);
                    n = read(sockFd,buffer,MAXIMUM-1);
                    if (n < 0) 
                    {
                        error("error reading from the socket");  
                    }
                
                    mini_statement(sockFd,id); 
                }
            }
            //Reading flag
            memset(buffer,0,MAXIMUM);
            n = read(sockFd,buffer,MAXIMUM-1);
            if (n < 0) 
            {
                error("error reading from the socket");
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////// for customer //////////////////////////////////////////////////////////////////////////

void balanceAvailable(int sockFd,char *cust_id)
{
    int n;
    char fileName[MAXIMUM];
    sprintf(fileName,"%s",cust_id);
    strcat(fileName,".txt");
    
    FILE *fp = fopen(fileName,"r");
    if(fp == NULL) 
    {
        error("file not open for balance.");
    }
    
    char *transac = NULL;
    size_t len = 0;
    char *balance;

    while(getline(&transac,&len,fp)!=-1)
    {
        strtok(transac," ");
        strtok(NULL, " ");
        balance = strtok(NULL, " ");
    }
    
    fprintf(stdout, "Sending balance of customer '%s' to client with ip '%s'. \n", cust_id, clientIp);
    // balance
    n = write(sockFd,balance,strlen(balance));
    if (n < 0) 
    {
        error("error writing to the socket!");
    }
    
    free(transac);
    fclose(fp);
}

void mini_statement(int sockFd, char *cust_id)
{
    int n;
    char buffer[MAXIMUM];
    char fileName[MAXIMUM];
    struct stat file_stat;
    sprintf(fileName,"%s",cust_id);
    strcat(fileName,".txt");
    
    
    int fd = open(fileName, O_RDONLY);
    if(fd == -1) 
    {
        error("Error in opening user file for mini_statement");
    }
    
    // finding statistics of file
    if (fstat(fd, &file_stat) < 0) 
    {
        error("Error in getting statistics of file.");
    }

    // writing size
    memset(buffer,0,MAXIMUM);
    sprintf(buffer, "%d", (int)file_stat.st_size);
    n = write(sockFd,buffer,strlen(buffer));
    if (n < 0) 
    {
        error("error writing to the socket!");
    }
    
    // delimeter string
    memset(buffer,0,MAXIMUM);
    n = read(sockFd,buffer,MAXIMUM-1);
    if (n < 0) 
    {
        error("error reading from the socket");
    } 
        
    
    // sending mini statement        
    fprintf(stdout, "Sending mini statement of customer '%s' to client with ip '%s'. \n", cust_id, clientIp);
    while (1) 
    {
        memset(buffer,0,MAXIMUM);
        int bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read == 0) 
        {
            break;
        }
        if (bytes_read < 0)
        {
            error("ERROR reading from file.");
        } 
            
        void *ptr = buffer;
        while (bytes_read > 0)
        {
            int written_bytes = write(sockFd, ptr, bytes_read);
            if (written_bytes <= 0) 
            {
                error("error writing to the socket!");
            }
            bytes_read -= written_bytes;
            ptr += written_bytes;
        }
    }
    close(fd);         
}


///////////////////////////////////////////////////////////////////////////// for admin //////////////////////////////////////////////////////////////////////////


int is_valid(char *amount)
{
    // checking validity of amount
    int i;
    int count=0;
    for(i=0;amount[i];i++)
    {
        if(amount[i]=='.')
        {
            count++;
            if(count>1)
                return 0;
        }       
        else if(amount[i]<='9'&&amount[i]>='0') 
        {
            continue;
        }
        else 
            return 0;
    }
    return 1;
}



void credit_amount(char *id, char *amount,char *trans)
{
    char fileName[MAXIMUM];
    sprintf(fileName,"%s.txt",id);

    FILE *fp = fopen(fileName,"r");
    if(fp == NULL) 
    {
        error("Error in opening user file for balance.");
    }
    
    char *transac = NULL;
    size_t len = 0;
    char *balance;

    while(getline(&transac,&len,fp)!=-1)
    {
        strtok(transac," ");
        strtok(NULL, " ");
        balance = strtok(NULL, " ");
    }

    double amt, cred_amount;
    sscanf(balance, "%lf", &amt);

    free(transac);
    fclose(fp);

    // crediting amount
    sscanf(amount, "%lf", &cred_amount);
    amt += cred_amount;

    fp = fopen(fileName,"a");
    if(fp == NULL) 
    {
        error("Error in opening user file for crediting.");
    }
    
    time_t c_t = time(NULL);
    struct tm tm = *localtime(&c_t);
    fprintf(fp,"\n%.2d-%.2d-%.4d %s %f", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, trans, amt);
    fclose(fp);
}


int debit_amount(char *id, char *amount, char *trans)
{
    char fileName[MAXIMUM];
    sprintf(fileName,"%s.txt",id);

    FILE *fp = fopen(fileName,"r");
    if(fp == NULL) 
    {
        error("Error in opening user file for balance.");
    }
    
    char *transac = NULL;
    size_t len = 0;
    char *balance;

    while(getline(&transac,&len,fp)!=-1)
    {
        strtok(transac," ");
        strtok(NULL, " ");
        balance = strtok(NULL, " ");
    }
    
    double amt, req_amount;
    sscanf(balance, "%lf", &amt);

    free(transac);
    fclose(fp);
 
    sscanf(amount, "%lf", &req_amount);
    if(amt<req_amount) 
    {
        return 0;
    }

    // debiting amount
    amt -= req_amount;

    fp = fopen(fileName,"a");
    if(fp == NULL) 
    {
        error("Error in opening user file for debiting.");
    }
    
    time_t c_t = time(NULL);
    struct tm tm = *localtime(&c_t);
    fprintf(fp,"\n%.2d-%.2d-%.4d %s %f", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, trans, amt);
    fclose(fp);            
    return 1;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void error(char *msg)
{
    perror(msg);
    exit(1);
}


int credentialsCheck(int sockFd, char *user_type,int *cust_id)
{
    int n;
    char buffer[MAXIMUM];
    char *user, *pass;

    // reading username and password from buffer
    memset(buffer,0,MAXIMUM); 
    n = read(sockFd,buffer,MAXIMUM-1);

    if(n<0) 
    {
        error("error reading from the socket");
    }
    
    // breaking it
    user = strtok(buffer, "$$$");
    pass = strtok(NULL, "$$$"); 
       
    user[strlen(user)-1] = '\0';
    pass[strlen(pass)-1] = '\0';


    if(strlen(user)==0 || strlen(pass)==0) 
    {
        return 0;
    }

    // checking for validity
    FILE *fp = fopen("login_file.txt","r");
    
    if(fp==NULL) 
    {
        error("Error in opening login_file.");
    }

    char *cred = NULL;
    size_t len = 0;
    
    while(getline(&cred,&len,fp)!=-1)
    {
        char *username = strtok(cred," ");
        char *password = strtok(NULL," ");
        char *usertype = strtok(NULL, " ");
        if(!strcmp(username,user)&&!strcmp(password,pass))
        {
            *user_type = usertype[0];
            *cust_id = atoi(username);
            free(cred);
            fclose(fp);
            return 1;
        } 
    }
    fclose(fp);
    free(cred);
    return 0;
}


void services(int sockFd)
{
    int count=1;
    int n;
    char buffer[MAXIMUM];
    int cust_id;
    char user_type;
    
    while(!credentialsCheck(sockFd,&user_type,&cust_id))
    {

        if(count>=3)
        {
            memset(buffer,0,MAXIMUM); 
            strcpy(buffer,"exit");
            n = write(sockFd,buffer,strlen(buffer));
            return;
        }
        fprintf(stdout, "Verification for client with ip address '%s' failed. \n", clientIp);            
        // sending false to the buffer
        memset(buffer,0,MAXIMUM);
        strcpy(buffer,"false");
        n = write(sockFd,buffer,strlen(buffer));
        (n < 0) ? error("error writing to the socket!") : printf("");
        count++;
    }
    
    // successfully verified user password

    fprintf(stdout, "Verification for client with ip address '%s' successful. \n", clientIp);            
    // sending user type into buffer
    memset(buffer,0,MAXIMUM); 
    buffer[0] = user_type;
    buffer[1] = '\0';
    n = write(sockFd,buffer,strlen(buffer));
    (n < 0) ? error("error writing to the socket!") : printf("");
    
    // calling helper function

    helper(sockFd,user_type,cust_id);

    return;
}




int main(int argc, char *argv[])
{
    int sockFd, clientFd, port, clientLen;
    int pid, processId;
    int childCount = 0;
    int enable = 1;
    
    struct sockaddr_in server_addr, client_addr;
    
    argc<2 ? (fprintf(stderr,"Please provide the PORT!!!.\n") , exit(1)) : printf("");

    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    
    sockFd<0 ? error("ERROR in socket opening!!!") : printf("");  
    
    memset((char *) &server_addr,0,sizeof(server_addr));
    
    port = atoi(argv[1]);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    
    (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) < 0 ?  error("Setting of socket as Reusable failed!!!") : printf("");

    (bind(sockFd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) ? error("ERROR in binding!!!") : printf("");

    fprintf(stdout, "Done Binding.\n");

    // listening mode server
    listen(sockFd,7);
    fprintf(stdout, "Started listening with queue length 7.\n");

    clientLen = sizeof(client_addr);
    
    while (1) 
    {
    	// accepting client request
        clientFd = accept(sockFd, (struct sockaddr *) &client_addr, &clientLen);

        (clientFd < 0) ? error("error!! in accepting the connection from client.\n") : printf("");

        pid = fork();
        (pid < 0) ? error("error while forking.") : printf("");

        if (pid == 0)  
        {
            close(sockFd);
            
            clientIp = inet_ntoa(client_addr.sin_addr);
            clientPort = ntohs(client_addr.sin_port);
            fprintf(stdout, "Connection accepted for client with ip address '%s' on port '%d'. \n", clientIp, clientPort);
            
            // giving service according to the customer type and need
            services(clientFd);
            
            close(clientFd);
            fprintf(stdout, "Connection closed for client with ip address '%s' on port '%d'. \n", clientIp, clientPort);
            
            exit(0);
        }
        else 
        {
        	// closing the client
            close(clientFd);
            // handling the "zombie process"
            while (childCount) 
            {
                // Non-blocking wait 
                processId = waitpid((pid_t) -1, NULL, WNOHANG); 
                if (processId < 0) 
                    error("Error in cleaning the \"zombie process\".");
                else if(processId == 0) 
                    break; 
                else 
                    childCount--; 
            }
        }
    } 
     return 0; 
}
