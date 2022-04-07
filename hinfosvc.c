#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>


#define BR_MSG "400 Bad Request\n"
#define SOCK_START_TEMPLATE "HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n"
#ifdef MAC_OS_VER
#define GET_CPU_NAME "sysctl -a | grep \"machdep.cpu.brand_string\" | awk 'BEGIN {FS = \":\"} { print $2 }'" // for macOS compile with -DMAC_OS_VER
#else
#define GET_CPU_NAME "cat /proc/cpuinfo| head -5 | tail -1 | awk 'BEGIN { FS = \":\"} { print $2; }'" // for Linux
#endif

#define BAD_ARGS 1
#define GET_SMTH_ERR 2
#define SOCKET_ERRS 3


void getUsageCPU(int socket)
{

	unsigned long int prevIdle, prevIowait, prevNonIdle, prevUser, prevNice, prevSys, // for every column of cpuinfo BEFORE
		              prevIrq, prevSoftIrq, prevSteal;

	unsigned long int idle, iowait, nonIdle, user, nice, sys,   // AFTER
                      irq, softIrq, steal;

	unsigned long int prevTotal, total, idled, totald;

	double cpuPercentage;

	FILE* stats = popen("cat /proc/stat | grep \"cpu \"", "r"); // stats is now first row of common CPU info
	if(!stats)
	{
		printf("Couldn't open file!\n");
		exit(GET_SMTH_ERR);	
	}
	fscanf(stats, "%*[^0123456789]%lu %lu %lu %lu %lu %lu %lu %lu",  // regex to extract datas
                        &prevUser, &prevNice, &prevSys, &prevIdle, &prevIowait, &prevIrq, &prevSoftIrq, &prevSteal);	
	fclose(stats);

	sleep(1);  // to compare load before and after

	stats = popen("cat /proc/stat | grep \"cpu \"", "r");	
	fscanf(stats, "%*[^0123456789]%lu %lu %lu %lu %lu %lu %lu %lu",  // same operation
			&user, &nice, &sys, &idle, &iowait, &irq, &softIrq, &steal); 
	fclose(stats);	

	prevNonIdle = prevUser + prevNice + prevSys + prevIrq + prevSoftIrq + prevSteal;
	nonIdle = user + nice + sys + irq + softIrq + steal;
	
	
	prevTotal = prevNonIdle + prevIdle;
	total = nonIdle + idle;
	
	
	totald = total - prevTotal; 
	idled = idle - prevIdle;

	cpuPercentage = (double)100*(((double)totald - (double)idled)/(double)totald);
	
	char load[4];
	snprintf(load, 4, "%.2f", cpuPercentage); // convert number to string
	strcat(load, "%\n");
	write(socket, load, strlen(load));	
	//printf("%lf\n", cpuPercentage);
}


void getHostname(int socket)
{
    char hostname[50];
    int check = gethostname(hostname, sizeof(hostname));
    if(!check) strcat(hostname, "\n");// if getting the hostname was successful => zero
    else strcpy(hostname, "Can't get hostname...\n");
    write(socket, hostname, strlen(hostname));
}


void getCPUname(int socket)
{
    FILE* specs = popen(GET_CPU_NAME, "r");
    if(!specs)
    {
        fprintf(stderr, "%s: can't open file!\n", __func__);
        exit(GET_SMTH_ERR);
    }

    char* cpuName = malloc(sizeof(char));
    char c = fgetc(specs);
    c = fgetc(specs);
    for (int i = 1;; i++)
    {   
        cpuName[i-1] = c;
        cpuName = realloc(cpuName, sizeof(char) * (i+1));
        c = fgetc(specs);
        if(c == EOF)
        {
            cpuName[i] = '\0';
            fclose(specs);
            break;
        }
    }
    write(socket, cpuName, strlen(cpuName));
    free(cpuName);
}


int main(int argc, char** argv)
{
    if(argc <= 1)
    {
        fprintf(stderr, "ERROR!\nSyntax of execution: /hinfosvc [port]\nPut the port please!\n");
        exit(BAD_ARGS);
    }
    else if(argc > 2)
    {
        fprintf(stderr, "ERROR!\nToo much arguments!\n");
        exit(BAD_ARGS);
    }
    else if (!atoi(argv[1]))
    {
        fprintf(stderr, "ERROR!\nBad port!\n");
        exit(BAD_ARGS);
    }

    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sin_len = sizeof(clientAddress);
    int fdServer; //(file descriptor) will be return value from the socket function
    int fdClient;
    char request[2048]; // here will be the sended request
    int on = 1;   // for setsockopt settings



    fdServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // create socket with necessary flags
    if(fdServer < 0)
    {
        fprintf(stderr, "Socket error!\n");
        exit(SOCKET_ERRS);
    }

    setsockopt(fdServer, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;         // set all the values(flags)
    serverAddress.sin_port = htons(atoi(argv[1]));      // set port


    if(bind(fdServer, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
    {
        fprintf(stderr, "bind error!\n");
        close(fdServer);
        exit(SOCKET_ERRS);
    }
   
    if(listen(fdServer, 3) == -1) // put socket in the "listening" mode
    {
        fprintf(stderr, "listen error!\n");
        close(fdServer);
        exit(SOCKET_ERRS);
    }


    while(1)
    {
        fdClient = accept(fdServer, (struct sockaddr *) &clientAddress, (socklen_t*) &sin_len);

        if(fdClient == -1)
        {
            fprintf(stderr, "Accept error!\nConnection failed.\nTrying to connect...\n");
            continue;
        }

 	
        recv(fdClient, request, 2048, 0);    // wait till recieve the request from browser/curl
        
        if(!strncmp(request, "GET /hostname ", strlen("GET /hostname") + 1)) // compare first n letters 
        {
            write(fdClient, SOCK_START_TEMPLATE, strlen(SOCK_START_TEMPLATE));
            getHostname(fdClient);
        }
        else if(!strncmp(request, "GET /cpu-name ", strlen("GET /cpu-name") + 1)) // space and strlen + 1 to correctly detect valid request
        {
            write(fdClient, SOCK_START_TEMPLATE, strlen(SOCK_START_TEMPLATE));
            getCPUname(fdClient);
        }
        else if(!strncmp(request, "GET /load ", strlen("GET /load") + 1))
        {
            write(fdClient, SOCK_START_TEMPLATE, strlen(SOCK_START_TEMPLATE));
            getUsageCPU(fdClient);
        }
        else write(fdClient, BR_MSG, strlen(BR_MSG));

        close(fdClient);
    }

    return 0;
}  
