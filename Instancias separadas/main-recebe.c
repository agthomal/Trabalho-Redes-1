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
    int t = cria_raw_socket("lo");
    // int t = cria_raw_socket("lo");

    char buffer[TAM_MSG + OFFSET];

    FILE* arq1 = fopen(argv[1], "w+");

    char c;
    for (;;) {
        int recebe = recebe_mensagem(t, 200, buffer, TAM_MSG + OFFSET);
        //
        //
        // IMPORTANTE: A MENSAGEM RECEBE DUAS VEZES PELA FORMA QUE O LOOPBACK FUNCIONA. TESTES DESSA PARTE VAO SER NECESSARIOS QUANDO TROCAR PRA DUAS MAQUINAS
        //
        //
        if (recebe != -1) {
            recebe = recebe_mensagem(t, 200, buffer, TAM_MSG + OFFSET);
        }

        if (buffer[1] == 0x04 && buffer[2] == '\0') {
            break;
        }

        if (recebe != -1 && strlen(buffer) > 0) {
            int tam = obtem_tamanho(buffer);
            printf("Tamanho: %d\n", tam);
            // fwrite(buffer + OFFSET, recebe - OFFSET, 1, arq1);
            fwrite(buffer + OFFSET, tam, 1, arq1);
            printf("%d\n", obtem_sequencia(buffer));
        }
        sleep(1);
    }
    fclose(arq1);
    // fclose(arq2);
}