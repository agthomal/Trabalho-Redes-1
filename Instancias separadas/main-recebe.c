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
    int s = cria_raw_socket("lo");

    char buffer[TAM_MSG + OFFSET];
    char bufferSend[TAM_MSG + OFFSET];

    FILE* arq1 = fopen(argv[1], "w+");

    char c;

    int modo = M_RECEBE;
    int tipo_msg;
    int seq = 0;
    int seqRec = 0;
    int termina = 0;

    for (;;) {
        if (modo == M_RECEBE) {
            // printf("recebe\n");
            int recebe = recebe_mensagem(t, 200, buffer, TAM_MSG + OFFSET);
            if (strlen(buffer) == 0)
                continue;
            //
            //
            // IMPORTANTE: A MENSAGEM RECEBE DUAS VEZES PELA FORMA QUE O LOOPBACK FUNCIONA. TESTES DESSA PARTE VAO SER NECESSARIOS QUANDO TROCAR PRA DUAS MAQUINAS
            //
            //
            if (recebe != -1) {
                // printf("inicio = %x\n", buffer[0]);
                // printf("tamanho: %d | sequencia: %d | tipo: %x\n", obtem_tamanho(buffer), obtem_sequencia(buffer), obtem_tipo(buffer));
                recebe = recebe_mensagem(t, 200, buffer, TAM_MSG + OFFSET);
            }

            if ((obtem_tipo(buffer) == ACK || obtem_tipo(buffer) == NACK) && recebe != -1) {
                if (obtem_tipo(buffer) == ACK)
                    printf ("ack\n");
                if (obtem_tipo(buffer) == NACK)
                    printf("nack\n");
                continue;
            }
            if (recebe != -1)
                printf("tamanho: %d | sequencia: %d | tipo: %x\n", obtem_tamanho(buffer), obtem_sequencia(buffer), obtem_tipo(buffer));

            if (seqRec % 32 != obtem_sequencia(buffer)) {
                recebe = -1;
                printf("ordem errada; ordem atual deve ser %d mas e %d\n", seqRec % 32, obtem_sequencia(buffer));
            }

            if (recebe != -1) {
                seqRec++;
                if (seqRec > 31)
                    seqRec = 0;
            }

            if (obtem_tipo(buffer) == FIM_TX) {
                printf("acabou\n");
                termina = 1;
            }

            if (recebe != -1 && strlen(buffer) > 0) {
                int tam = obtem_tamanho(buffer);
                // printf("Tamanho: %d\n", tam);
                // fwrite(buffer + OFFSET, recebe - OFFSET, 1, arq1);
                fwrite(buffer + OFFSET, tam, 1, arq1);
                printf("Enviando ACK: %d %d %d\n", obtem_sequencia(buffer), seqRec, recebe);
                tipo_msg = ACK;
            }
            else {
                // printf("%d %d %d\n", obtem_sequencia(buffer), seqRec, recebe);
                tipo_msg = NACK;
            }
            // sleep(1);
            modo = M_ENVIA;
        }
        else {
            printf("envia\n");
            prepara_mensagem(bufferSend, 0x7f, 0, seq, tipo_msg);
            seq++;
            if (seq >= 32)
                seq = 0;

            int envio;
            envio = send(s, bufferSend, TAM_MSG + OFFSET, 0);
            // printf("%d\n", obtem_sequencia(bufferSend));
            // printf("recebe\n");
            modo = M_RECEBE;
            if (termina)
                break;
        }
    }
    fclose(arq1);
    // fclose(arq2);
}