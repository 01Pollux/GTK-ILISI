#include "Networking.h"
#include "../Checker.h"
#include <iphlpapi.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    CheckerTable* Table;

    dyad_Stream* ServerStream;
    dyad_Stream* ClientStream;
    dyad_Stream* RemoteStream;
} NetCheckerTable;

void GetLocalIpAddress(char Ip[16])
{
    memset(Ip, 0, sizeof(Ip));

    ULONG SizeOfAddresses = sizeof(IP_ADAPTER_ADDRESSES);
    GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, NULL, &SizeOfAddresses);

    IP_ADAPTER_ADDRESSES* OutAddresses = (IP_ADAPTER_ADDRESSES*)malloc(SizeOfAddresses);
    GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, OutAddresses, &SizeOfAddresses);
    PIP_ADAPTER_ADDRESSES Iter = OutAddresses;

    while (Iter)
    {
        if (!lstrcmpW(Iter->FriendlyName, L"WiFi") || !lstrcmpW(Iter->FriendlyName, L"Wi-Fi"))
        {
            // print logic address
            for (IP_ADAPTER_UNICAST_ADDRESS* Unicast = Iter->FirstUnicastAddress; Unicast; Unicast = Unicast->Next)
            {
                if (Unicast->Address.lpSockaddr->sa_family == AF_INET)
                {
                    char Ipx[16];
                    DWORD SizeOfIp = sizeof(Ipx);
                    WSAAddressToStringA(Unicast->Address.lpSockaddr, Unicast->Address.iSockaddrLength, NULL, Ipx, &SizeOfIp);
                    memcpy(Ip, Ipx, sizeof(Ipx));
                    break;
                }
            }
            break;
        }
        Iter = Iter->Next;
    }

    free(OutAddresses);
}

//

static void Server_onAccept(dyad_Event* e)
{
    NetCheckerTable* Table = (NetCheckerTable*)e->udata;
    Table->RemoteStream = e->remote;
    printf("connecting: %s\n", dyad_getAddress(e->remote));
}

static void Server_onError(dyad_Event* e)
{
    printf("server error: %s\n", e->msg);
}

void Net_Init(char Ip[16], int* Port)
{
    srand((unsigned int)time(NULL));
    dyad_init();

    *Port = 3000 + rand() % 1000;
    printf("port = %d\n", *Port);

    GetLocalIpAddress(Ip);
    printf("IP address: %s\n", Ip);
}

dyad_Stream* Net_Host(NetCheckerTable* Table, const char* Ip, int Port)
{
    dyad_Stream* Stream = dyad_newStream();

    dyad_addListener(Stream, DYAD_EVENT_ERROR, Server_onError, Table);
    dyad_addListener(Stream, DYAD_EVENT_ACCEPT, Server_onAccept, Table);
    dyad_listenEx(Stream, Ip, Port, 511);

    return Stream;
}

//

static void Client_onConnect(dyad_Event* e)
{
    printf("connected: %s\n", e->msg);
}

static void Client_onError(dyad_Event* e)
{
    printf("error: %s\n", e->msg);
}

static void Client_onData(dyad_Event* e)
{
    CheckerPieceInfo* Info = (CheckerPieceInfo*)e->data;
    NetCheckerTable* NetTable = (NetCheckerTable*)e->udata;

    CheckerPieceInfo* CurInfo = &NetTable->Table->Final;
    CurInfo->Pieces = Info->Pieces;

    for (int i = 0; i < 12; i++)
    {
        CurInfo->WhitePieces[i].Pos = Info->WhitePieces[i].Pos;
        CurInfo->BlackPieces[i].Pos = Info->BlackPieces[i].Pos;
    }

    NetTable->Table->Active = *CurInfo;
    CheckerTable_SetNextTurn(NetTable->Table);
    CheckerTable_Refresh(NetTable->Table);
}

dyad_Stream* Net_Connect(NetCheckerTable* Table, const char* ip, int port)
{
    dyad_Stream* Stream = dyad_newStream();
    dyad_addListener(Stream, DYAD_EVENT_ERROR, Client_onError, Table);
    dyad_addListener(Stream, DYAD_EVENT_CONNECT, Client_onConnect, Table);
    dyad_addListener(Stream, DYAD_EVENT_DATA, Client_onData, Table);
    dyad_connect(Stream, ip, port);
    return Stream;
}

//

static DWORD Net_Heartbeat(NetCheckerTable* NetChecker)
{
    while (dyad_getStreamCount() > 0)
    {
        dyad_update();
    }
    return 0;
}

static gboolean Net_GtkUpdate(NetCheckerTable* NetChecker)
{
    if (NetChecker->Table->NeedNetUpdate)
    {
        dyad_write(NetChecker->RemoteStream, &NetChecker->Table->Final, sizeof(NetChecker->Table->Final));
        NetChecker->Table->NeedNetUpdate = FALSE;
    }
    return TRUE;
}

void Net_GameStart(
    struct CheckerTable* Table,
    const char* Ip, int Port,
    const char* RpcIp, int RpcPort,
    int StartWhite)
{
    NetCheckerTable* NetChecker = (NetCheckerTable*)calloc(1, sizeof(NetCheckerTable));

    NetChecker->Table = Table;

    NetChecker->ServerStream = Net_Host(NetChecker, Ip, Port);
    NetChecker->ClientStream = Net_Connect(NetChecker, RpcIp, RpcPort);

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(&Net_Heartbeat), NetChecker, 0, NULL);
    g_timeout_add(500, G_SOURCE_FUNC(Net_GtkUpdate), NetChecker);
}
