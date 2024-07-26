#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/time.h>
#include <unistd.h>

#include "mensagem.h"
#include "dados.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Inserir arquivo no argumento\n");
        return 0;
    }
    // lo: LOOPBACK(Maquina envia pra si mesma)
    int socket_recv = cria_raw_socket("lo");
    int socket_send = cria_raw_socket("lo");

    char buffer[TAM_MSG + OFFSET];
    char bufferSend[TAM_MSG + OFFSET];

    FILE* arq1 = fopen(argv[1], "w+");

    char c;

    // int modo = M_RECEBE;
    int tipo_msg;
    int seq = 0;
    int seqRec = 0;
    int termina = 0;

    recebe_dados(socket_send, socket_recv, buffer, &seq, &seqRec, bufferSend, arq1);
    fclose(arq1);
    // fclose(arq2);
}