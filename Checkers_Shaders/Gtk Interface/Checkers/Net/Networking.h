#pragma once

#include "dyad.h"

void Net_Init(char Ip[16], int* Port);

void Net_GameStart(
    struct CheckerTable* Table,
    const char* Ip, int Port,
    const char* RpcIp, int RpcPort,
    int StartWhite);
