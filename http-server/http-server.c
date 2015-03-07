#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

/*
 *
 * http-server
 *
 * lab7, part 2a
 *
 */

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 5000 /* Size of receive buffer */

static void DieWithError(char *errorMessage) /*error handling function */
{
    perror(errorMessage);
    exit(1);
}

int main (int argc, char *argv[])
{
    int servSock;       /* socket descriptor for http server */
    int clntSock;       /* socket descriptor for http client */
    int mdbSock;    /*socket descriptor for mdbLookup socket */
    char* mdbHost; /*host name in string form*/
    struct hostent *server; /*for mdbLookupHost*/
    struct sockaddr_in httpServAddr; /* local address */
    struct sockaddr_in httpClntAddr; /* http client address */
    struct sockaddr_in mdbServ; /*mdb server address*/
    unsigned short httpServPort;     /* http server port */
    unsigned short mdbServPort; /*mdb server port*/
    char rcvBuffer[RCVBUFSIZE];     /* buffer for receving  msg */
    unsigned int clntLen; /* length of http client address data structure */
    
    
    //check for bad input
    if (argc !=5){
        fprintf(stderr, "Usage: %s <server_port> <web_root> <mdb-lookup-host> <mdb-lookup-port> \n;",argv[0]);
        exit(1);
    }
    
    //////open socket and establish connection with mdbLookup//////////
    
    if ((mdbSock = socket (AF_INET, SOCK_STREAM, 0))<0)
        DieWithError("socket() failed");

    mdbHost = argv[3];
    server = gethostbyname(mdbHost); /*mdbLookup host*/
    if (server==NULL){
        DieWithError("ERROR, no such host\n");
    }
    
    mdbServPort = atoi(argv[4]); /*mdbLookup port */

    bzero((char*) &mdbServ, sizeof(mdbServ));
    mdbServ.sin_family = AF_INET;
    mdbServ.sin_port = htons(mdbServPort);
    mdbServ.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *) server->h_addr));
    if (connect(mdbSock, (struct sockaddr*) &mdbServ, sizeof(mdbServ))<0)
        DieWithError("Error connecting to mdbLookup server");
   
    //read from mdbsock as a file
    FILE *mdbLookupResults = fdopen(mdbSock, "r");
   
   
   ///////////now look for http-clients connecting/////////////

    httpServPort = atoi(argv[1]); /*second arg: local port */
   
    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
        DieWithError("socket() failed");
    
    /* Construct local address structure */
    memset(&httpServAddr, 0, sizeof(httpServAddr));
    httpServAddr.sin_family = AF_INET;
    httpServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    httpServAddr.sin_port = htons(httpServPort);

    /*Bind to the local address */
    if (bind(servSock, (struct sockaddr*) &httpServAddr,
            sizeof(httpServAddr))<0)
        DieWithError("bind() failed");

    /*Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING)<0)
        DieWithError("listen() failed");

    for (;;) /*Run forever */
    {
        clntLen = sizeof(httpClntAddr);
        
        if((clntSock = accept(servSock,
               (struct sockaddr *) &httpClntAddr, &clntLen))<0)
            DieWithError("accept() failed");

        fprintf(stderr, "connection started from: %s\n",
                inet_ntoa(httpClntAddr.sin_addr));
        
       bzero(rcvBuffer, sizeof(rcvBuffer));

        /*receive message from client*/
        if ((recv(clntSock, rcvBuffer, RCVBUFSIZE, 0))==-1){
            DieWithError("recv() failed");      
        } 

        //if the client fails before a request is sent
        //or if the client sends a blank request
        //we close the socket and restart the loop
        if  ( rcvBuffer[0] == '\0'){   
            bzero(rcvBuffer, sizeof(rcvBuffer));
            close(clntSock);
        }
        else{
            char* nImpMsg = "HTTP/1.0 501 Not Implemented\r\n\r\n<html><body><h1>501 Not Implemented</h1></body></html>";
            char* bRMsg = "HTTP/1.0 400 Bad Request\r\n\r\n<html><body><h1>400 Bad Request</h1></body></html>n";      
            char* nfMsg = "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>";
            char* get = "GET";
            char* http10 = "HTTP/1.0";
            char* http11 = "HTTP/1.1";
            char* okMSG = "HTTP/1.0 200 OK\r\n\r\n";
            char* slashdotdot = "/..";
            char* mdbLookupInit = "/mdb-lookup";
            char* mdbLookupSearch = "/mdb-lookup?key=";
            char* requestLine = strtok(rcvBuffer, "\r\n");
            char *token_separators = "\t \r\n"; // tab, space, new line
            char *method = strtok(requestLine, token_separators);
            char *requestURI = strtok(NULL, token_separators);
            char *httpVersion = strtok(NULL, token_separators);
            
            //return not implemented message if request is not 'get'
            if (strcmp(method, get) !=0){
                if(send(clntSock, nImpMsg, strlen(nImpMsg), 0)
                        != strlen(nImpMsg)){
                    DieWithError("send() failed");
                }
                fprintf(stdout, "%s \"%s %s %s\" 501 Not Implemented\n",
                        inet_ntoa(httpClntAddr.sin_addr),
                        method, requestURI, httpVersion); 
            }

            //return not implemented message if not http1.0 or 1.1
            else if ((strcmp(httpVersion, http10) !=0) &&
                    (strcmp(httpVersion, http11) !=0)){
                if(send(clntSock, nImpMsg, strlen(nImpMsg), 0)
                        != strlen(nImpMsg)){
                    DieWithError("send() failed");
                }
                fprintf(stdout, "%s \"%s %s %s\" 501 Not Implemented\n",
                        inet_ntoa(httpClntAddr.sin_addr), 
                        method, requestURI, httpVersion); 
            }

            //return bad request message it '/' is not first character
            //of the request URI
            else if (strchr(requestURI, '/') !=NULL &&
                    strcmp(requestURI, strchr(requestURI, '/')) !=0){
                if (send(clntSock, bRMsg, strlen(bRMsg), 0) != strlen(bRMsg)){
                    DieWithError("send() failed");
                } 
                fprintf(stdout, "%s \"%s %s %s\" 400 Bad Request\n",
                        inet_ntoa(httpClntAddr.sin_addr),
                        method, requestURI, httpVersion); 
            }

            //return bad request message if URI contains '/..'
            else if ((strstr(requestURI, slashdotdot)) !=NULL){
                 if (send(clntSock, bRMsg, strlen(bRMsg), 0) != strlen(bRMsg)){
                    DieWithError("send() failed");
                }
                fprintf(stdout, "%s \"%s %s %s\" 400 Bad Request\n",
                        inet_ntoa(httpClntAddr.sin_addr),
                        method, requestURI, httpVersion);
            }
           
            //otherwise, the request is good, and we initialize some values
            else{ 
                char sendBuffer[4096];
                bzero(sendBuffer, sizeof(sendBuffer));
                
                //form sent to client for mdb-lookups
                const char *form =
                          "<h1>mdb-lookup</h1>\n"
                           "<p>\n"
                            "<form method=GET action=/mdb-lookup>\n"
                             "lookup: <input type=text name=key>\n"
                              "<input type=submit>\n"
                               "</form>\n"
                                "<p>\n";


                ///////mdb-lookup option////////////

                //this loop executes when URI is just 'mdb-lookup';
                //sends the OK message and then form initiating mdb-lookup
                if (strcmp(requestURI, mdbLookupInit)==0){ 
                   if (send(clntSock, okMSG, strlen(okMSG), 0) != strlen(okMSG)){
                   DieWithError("send() failed");
                   } 
                   fprintf(stdout, "%s \"%s %s %s\" 200 OK\n",
                       inet_ntoa(httpClntAddr.sin_addr),
                       method, requestURI, httpVersion);
                   if(send(clntSock, form, strlen(form), 0) != strlen(form)){
                         DieWithError("send() failed");
                     }
                }
                
                //this loop executes when URI has 'mdb-lookup?key=...';
                //runs mdb-lookup and sends results to client
                else if (strstr (requestURI, mdbLookupSearch) != NULL
                        && strcmp(requestURI, strstr(requestURI, mdbLookupSearch)) ==0){ 
                   char* htmltableBorder = "<p><table border>\r\n";
                   char* htmltable = "</table>\r\n";
                   
                   if (send(clntSock, okMSG, strlen(okMSG), 0) != strlen(okMSG)){
                         DieWithError("send() failed");
                   }
                   fprintf(stdout, "%s \"%s %s %s\" 200 OK\n",
                       inet_ntoa(httpClntAddr.sin_addr),
                       method, requestURI, httpVersion);
                   
                   if (send(clntSock, form, strlen(form), 0) != strlen(form)){
                       DieWithError("send() failed");
                   }

                   if (send(clntSock, htmltableBorder, strlen(htmltableBorder), 0)!= strlen(htmltableBorder)){
                       DieWithError("send() failed");
                   }
                   
                   //split request URI at '=' symbol, take segment after = as
                   //the lookupstring
                   strtok(requestURI, "=");
                   char* lookupstringURI = strtok(NULL, "=");
                   char* newline = "\n";
                   
                   //if user didn't enter a lookup term,
                   //we just send mdb-server an empty line
                   if (lookupstringURI == NULL){
                      if (send(mdbSock, newline, strlen(newline), 0) !=
                             strlen (newline)){
                       DieWithError("send() failed");
                      }
                   }

                   //otherwise, we add on newline character to the lookup term
                   else{

                       char lookupstring[strlen(lookupstringURI) + 1];
                       strcpy(lookupstring, lookupstringURI);
                       strcat(lookupstring, newline);
                                 

                        if (send(mdbSock, lookupstring, strlen(lookupstring), 0) !=
                                strlen(lookupstring)){
                       DieWithError("send() failed");
                        }
                   }
                   
                   //zero out rcbBuffer because we will be using it
                   bzero(rcvBuffer, sizeof(rcvBuffer));
                   

                   if (mdbLookupResults == NULL){
                       DieWithError("fdopen failed");
                   }
                   
                   //fgets is looped, for continuous line-byline
                   //reading from mdb-lookup server
                   while (fgets(rcvBuffer, sizeof(rcvBuffer), mdbLookupResults)){
                    
                       //stops reading if we reached a blank line 
                       if (strchr(rcvBuffer, '\n') != NULL 
                               && strcmp(rcvBuffer, (strchr(rcvBuffer, '\n'))) ==0){
                            break;
                       }
                       
                       //puts the result just read into an html table row
                       int tableRowLength = 9 + strlen(rcvBuffer); 
                       char tableRow [tableRowLength];
                       char* htmlTableRow = "<tr><td> ";
                       strcpy(tableRow, htmlTableRow);
                       strcat(tableRow, rcvBuffer);
                       
                       //sends the table row to the http client
                       if (send(clntSock, tableRow, strlen(tableRow), 0)!= strlen(tableRow)){
                           DieWithError("send() failed");
                       }
                   }
                   
                   //sends the end of the html table
                   if (send(clntSock, htmltable, strlen(htmltable), 0) != strlen(htmltable)){
                       DieWithError("send() failed");
                   }
                   
                   //bzero the rcv buffer for next use in loop
                   bzero(rcvBuffer, sizeof(rcvBuffer)); 
                }





                //////////html file reading option
                else{

                    //filename needs to be 10 chars bigger than the filepath
                    //in case we need to append 'index.html'
                    char fileName[sizeof(argv[2])+sizeof(requestURI)+10];
                    strcpy(fileName, argv[2]);
                    strcat(fileName, requestURI);
                    
                    //if request URI ends in /, then append index.html
                    if(requestURI[((int) strlen(requestURI))-1]=='/'){
                          char* indexhtml = "index.html";
                          strcat(fileName, indexhtml); 
                    }
                    
                    //open the file to read
                    FILE* fileToRead = fopen(fileName, "r");
               

                    //send 404 message if file can't be found
                    if (fileToRead==NULL){
                        
                        if(send(clntSock, nfMsg, strlen(nfMsg), 0) != strlen(nfMsg)){ 
                            DieWithError("send() failed");
                        }
                        fprintf(stderr, "%s \"%s %s %s\" 404 Not Found\n",
                               inet_ntoa(httpClntAddr.sin_addr),
                               method, requestURI, httpVersion);
                               bzero(requestURI, sizeof(requestURI)); 
                    }
                    
                    //read file if it can be found
                    else{

                       if (send(clntSock, okMSG, strlen(okMSG), 0) != strlen(okMSG)){
                       DieWithError("send() failed");
                       }  
                       fprintf(stdout, "%s \"%s %s %s\" 200 OK\n",
                           inet_ntoa(httpClntAddr.sin_addr),
                           method, requestURI, httpVersion);
               
                    //read from file and send to clients in chunks
                    //of 4096 bytes
                    size_t n;
                    while((n = fread(sendBuffer, 1, sizeof(sendBuffer), fileToRead))>0){   
                        if (send(clntSock, sendBuffer, sizeof(sendBuffer),0)
                                !=sizeof(sendBuffer)){
                              DieWithError("send() failed");
                        }
                        
                        //zero sendBuffer after every send
                        bzero(sendBuffer, sizeof(sendBuffer));
                    }
               
                    //ensure sendBuffer is cleared of any remnants
                    bzero(sendBuffer, sizeof(sendBuffer));
                        
                    //close the file when done reading
                    fclose(fileToRead);
                    }
                  }
              }
           }
        
        //close the http socket after responding to request
        close(clntSock); 
    }

    exit (0);
}


    
