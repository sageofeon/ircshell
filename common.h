#ifndef COMMON_H
#define COMMON_H
#define popen _popen
#define pclose _pclose
#define SIZE_BUFFER 16384
#define SIZE_STR 128

struct server
{
    //must be many servers if one not avialable
    char *hosts[80];
    char *port;
};

struct nickname
{
    char *prefix_name;
    char *prefix_nick;
    char *bot_name;
};

//buffers
struct buffers
{
    char *from_sub,
    from[SIZE_BUFFER], //msg from server
    to[SIZE_BUFFER],     //msg to server
    tmp[SIZE_BUFFER];
};

//commands
struct commands{

    char *cmd;       //command from remote user
    char *ping;  //ping to server
    char *join;
    char *pong;
    char *part;
    char *quit;
};

struct vars_messages{

    //vars for parser msg from server
    char usnd[SIZE_STR];
    char snick[SIZE_STR];
    char nick[SIZE_STR];
    char srvcmd[SIZE_STR];
};

enum FlagsCommandRemoteUser{NotPrivMsg, PrivMsg, Reconnect, Stopped};

#endif // COMMON_H
