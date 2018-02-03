#include <stdio.h>
#include "quakedef.h"
#include "q_shared.h"
#include "cvar.h"
#include "EX_browser.h"

int main(void)
{
    printf("Hello World!\n");

    /*

typedef struct {
    netadrtype_t    type;

    byte            ip[4];

    unsigned short  port;
} netadr_t;

    */
    //45.63.78.66:30000
    netadr_t addr;
    addr.ip[0]=45;
    addr.ip[1]=63;
    addr.ip[2]=78;
    addr.ip[3]=66;
    addr.port=30000;
    addr.type=NA_IP;
    Reload_Sources();
    Rebuild_Servers_List();
    GetServerPingsAndInfosProc(1);
    SB_PingTree_Build();
    SB_PingTree_ConnectBestPath(&addr);
    return 0;
}

