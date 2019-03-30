#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include "common.h"

SOCKET createSocket(char * host_irc, unsigned short port_irc);
void deleteSocket(SOCKET _socket);
boolean pingPong(SOCKET _socket,
              struct buffers *buff,
              struct commands *cmds
              );
enum FlagsCommandRemoteUser
        commandFromRemoteUser(SOCKET _socket,
                           struct buffers *buffs,
                           struct commands *cmds,
                           struct vars_messages *vars_msgs
                           );
void joinToChannel(SOCKET _socket,
                   struct commands *cmds,
                   struct buffers *buffs);
void joinToChannel(SOCKET _socket,
                   struct commands *cmds,
                   struct buffers *buffs);
void sendUserNick(  SOCKET _socket,
                    struct nickname *nicknm,
                    struct vars_messages *vars_msg);
#endif // FUNCTIONS_H
