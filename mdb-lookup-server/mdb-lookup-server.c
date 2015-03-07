#include <signal.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "mylist.h"
#include "mdb.h"

/*
 * mdb-lookup-server
 *
 * lab6, part1
 *
 */

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define KeyMax 5


static void DieWithError(char *errorMessage)  /* Error handling function */
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    //ignore SIGPIPE so that we don't terminate when we call
    //send() on a disconnected socket.
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        DieWithError("signal() failed");
    
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in mdbLookupServAddr; /* Local address */
    struct sockaddr_in mdbLookupClntAddr; /* Client address */
    unsigned short mdbLookupServPort;     /* Server port */
    unsigned int clntLen;      /* Length of client address data structure */

    if (argc != 3)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Database File> <Server Port> \n",
               argv[0]);
        exit(1);
    }
    

    char *databaseFile = argv[1];       /* first arg: databaseFile*/
    mdbLookupServPort = atoi(argv[2]);  /* Second arg:  local port */

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    /* Construct local address structure */
    memset(&mdbLookupServAddr, 0, sizeof(mdbLookupServAddr));
    mdbLookupServAddr.sin_family = AF_INET;
    mdbLookupServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    mdbLookupServAddr.sin_port = htons(mdbLookupServPort);  

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &mdbLookupServAddr, 
                sizeof(mdbLookupServAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING)<0)
        DieWithError("listen() failed");


    for (;;) /* Run forever */
    {
        clntLen = sizeof(mdbLookupClntAddr);
      
        if ((clntSock = accept(servSock,
                 (struct sockaddr *) &mdbLookupClntAddr, &clntLen)) < 0)
            DieWithError("accept() failed");

       fprintf(stderr, "connection started from: %s\n",
               inet_ntoa(mdbLookupClntAddr.sin_addr)); 
        
        FILE *fp = fopen(databaseFile, "rb");
        if(fp==NULL)
           DieWithError ("fopen() failed");
       
        /*
         * read all records into memory from database file
         */
        
        struct List list;
        initList(&list);

        int loaded = loadmdb(fp, &list);
        if (loaded<0)
            DieWithError ("loadmdb() failed");

        fclose(fp);
        
        /*
         * get input from socket
         */

        FILE *input = fdopen(clntSock, "r");
        if (input==NULL)
            DieWithError ("fdopen() failed");
        
       
        char inputLine[1000];
        char key[KeyMax + 1];
        
        /*
         * run lookup loop
         */
        while (fgets(inputLine, sizeof(inputLine), input) != NULL){ 
            strncpy(key, inputLine, sizeof(key)-1);
            key[sizeof(key) -1]  = '\0';

            size_t last = strlen(key)-1;
            if (key[last] == '\n')
                key[last] = '\0';

            struct Node *node = list.head;
            int recNo = 1;
            while (node){
                struct MdbRec *rec = (struct MdbRec *)node->data;
                if (strstr(rec->name, key) || strstr(rec->msg, key)){
                    char outputLine[75];
                    int length;
                    if((length=sprintf(outputLine, "%4d: {%s} said {%s}\n",
                            recNo, rec->name, rec->msg))<0)
                        DieWithError("sprintf() failed");
                    if(send(clntSock, outputLine,length, 0)<0)
                        DieWithError("send() failed");
                }
                node = node->next;
                recNo++;
            } 
        char blankLine[1] = "\n";
        if(send(clntSock, blankLine, 1, 0)<0)
            DieWithError("send() failed");
        
        bzero(inputLine, sizeof(inputLine));
        bzero(key, sizeof(key));
      }

       fprintf(stderr, "connection terminated from: %s\n",
               inet_ntoa(mdbLookupClntAddr.sin_addr)); 

      freemdb(&list);
      removeAllNodes(&list);
      fclose(input);
   }
   exit(0);
}
