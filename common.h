#ifndef COMMON_H
#define COMMON_H
#define popen _popen
#define pclose _pclose
#define SIZE_BUFFER 16384
#define SIZE_STR 128
#define PART "PART"
#define JOIN "JOIN"
#define PONG "PONG "
#define PING "PING"
#define PRIVMSG "PRIVMSG"
#define USER "USER"
#define NICK "NICK"
#define MODE "MODE"
#define MOTD "/MOTD"
#define QUIT "QUIT"
#define MSG ":bye"
#define NL "\n"
#define PRIVMSG_TO_ADMIN PRIVMSG " " REMOTE_USER " :"
#define JOIN_CHANNEL JOIN " " CHANNEL NL
#define PART_CHANNEL PART " " CHANNEL NL
#define QUIT_MSG QUIT " " MSG NL
#define ERROR_CLOSE_LINK "ERROR :Closing Link"

#define CMD_RECONNECT "!change"
#define CMD_STOP "!stop"

#define CHANNEL "#h3xb0t"
#define REMOTE_USER "hexfox"
#define PREFIX_NAME "zname"
#define PREFIX_NICK "znick"
#define BOT_NAME "hexbot"
#define PORT "6667"
#define HOST1 "chat.freenode.net"
#define HOST2 "irc.757.org"
#define HOST3 "irc.eth-0.nl"

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
    char *sub_buff_from,
    buffer_from[SIZE_BUFFER], //msg from server
    buffer_to[SIZE_BUFFER],     //msg to server
    buffer_tmp[SIZE_BUFFER];
};

//commands
struct commands{

    char *cmd;       //command from remote user
    char *cmd_ping;  //ping to server
    char *cmd_join;
    char * cmd_pong;
    char * cmd_part;
    char * cmd_quit;
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
