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

#define TAM_MSG 63
#define TAM_MIN 14
#define OFFSET 3

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Inserir arquivo no argumento\n");
        return 0;
    }
    // lo: LOOPBACK(Maquina envia pra si mesma)
    int s = cria_raw_socket("lo");
    // int t = cria_raw_socket("lo");

    char buffer[TAM_MSG + OFFSET];

    FILE* arq1 = fopen(argv[1], "r");
    /* printf("insira o nome do arquivo de saida: ");
    scanf("%s", arqsaida); */
    FILE* arq2 = fopen("teste.jpg", "w+");

    char c;
    int seq = 0;
    for (;;) {
        size_t leituraArq = fread(buffer + OFFSET, 1, sizeof buffer - OFFSET, arq1);
        printf("%ld\n", leituraArq);
        prepara_mensagem(buffer, 0x7f, leituraArq, seq, 0b10010);
        seq++;
        if (seq >= 32)
            seq = 0;
        buffer[leituraArq + OFFSET] = '\0';

        if (leituraArq == 0)
            break;
        if (leituraArq < TAM_MSG) {
            for (int i = leituraArq + OFFSET; i < OFFSET + TAM_MSG; i++)
                buffer[i] = '\0';
        }

        int envio;
        if (strlen(buffer) != 0) {
            envio = send(s, buffer, TAM_MSG + OFFSET, 0);
            // printf("%s\n", buffer);
        }

        fwrite(buffer, leituraArq, 1, arq2);
        sleep(1);
    }

    // indicador temporario pro fim da mensagem
    buffer[0] = 0x7f;
    buffer[1] = 0x04;
    buffer[2] = '\0';
    int envio = send(s, buffer, 14, 0);
    envio = send(s, buffer, 14, 0);
    fclose(arq1);
    fclose(arq2);
}