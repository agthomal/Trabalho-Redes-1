#ifndef DADOS_H
#define DADOS_H

void envia_dados(int socket_send, int socket_recv, char buffer[], int *seq, char bufferRecv[], FILE* arqRecebe);

void recebe_dados(int socket_send, int socket_recv, char buffer[], int *seq, int *seqRec, char bufferSend[], FILE* arqRecebe);

#endif