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
    int t = cria_raw_socket("lo");

    char buffer[TAM_MSG + OFFSET];
    char bufferRecv[TAM_MSG + OFFSET];

    FILE* arq1 = fopen(argv[1], "r");
    /* printf("insira o nome do arquivo de saida: ");
    scanf("%s", arqsaida); */
    FILE* arq2 = fopen("teste.jpg", "w+");

    char c;
    int seq = 0;
    int modo = M_ENVIA;
    int fimMsg = 0;
    int termina = 0;
    while (!termina) {
        if (modo == M_ENVIA) {
            size_t leituraArq = fread(buffer + OFFSET, 1, sizeof buffer - OFFSET, arq1);
            prepara_mensagem(buffer, 0x7f, leituraArq, seq, DADOS);
            seq = (seq + 1) % 32;

            buffer[leituraArq + OFFSET] = '\0';

            if (leituraArq == 0) {
                seq = (seq - 1) % 32;
                prepara_mensagem(buffer, 0x7f, leituraArq, seq, FIM_TX);
                fimMsg = 1;
            }
            if (leituraArq < TAM_MSG) {
                for (int i = leituraArq + OFFSET; i < OFFSET + TAM_MSG; i++)
                    buffer[i] = '\0';
            }

            int envio;
            if (strlen(buffer) != 0) {
                envio = send(s, buffer, TAM_MSG + OFFSET, 0);

                int recebe = recebe_mensagem(t, 200, bufferRecv, TAM_MSG + OFFSET);
                recebe = recebe_mensagem(t, 200, bufferRecv, TAM_MSG + OFFSET);

                printf("%d\n", obtem_sequencia(buffer));
                // printf("%s\n", buffer);
            }
            modo = M_RECEBE;
            // sleep(1);
        }
        else {
            int recebe = recebe_mensagem(t, 200, bufferRecv, TAM_MSG + OFFSET);
            //
            //
            // IMPORTANTE: A MENSAGEM RECEBE DUAS VEZES PELA FORMA QUE O LOOPBACK FUNCIONA. TESTES DESSA PARTE VAO SER NECESSARIOS QUANDO TROCAR PRA DUAS MAQUINAS
            //
            //
            if (recebe != -1) {
                recebe = recebe_mensagem(t, 200, bufferRecv, TAM_MSG + OFFSET);
            }
            if (recebe == -1 || strlen(bufferRecv) == 0)
                continue;
            switch (obtem_tipo(bufferRecv)) {
                case ACK:
                    // printf("ACK\n");
                    modo = M_ENVIA;
                    if (fimMsg)
                        termina = 1;
                    break;
                case NACK:
                    // printf("NACK\n");
                    int envio = send(s, buffer, TAM_MSG + OFFSET, 0);

                    // loopback
                    recebe = recebe_mensagem(t, 200, bufferRecv, TAM_MSG + OFFSET);
                    recebe = recebe_mensagem(t, 200, bufferRecv, TAM_MSG + OFFSET);
                    break;
                case LISTA:
                    break;
                case BAIXAR:
                    break;
                case MOSTRA:
                    break;
                case DESCRITOR:
                    break;
                case DADOS:
                    printf("recebeu dados\n");
                    break;
                case FIM_TX:
                    break;
                case ERRO:
                    break;
            }
        }
        // fwrite(buffer, leituraArq, 1, arq2);
        // sleep(1);
    }

    // indicador temporario pro fim da mensagem
/*    buffer[0] = 0x7f;
    buffer[1] = 0x04;
    buffer[2] = '\0';
    int envio = send(s, buffer, 14, 0);
    envio = send(s, buffer, 14, 0); */
    fclose(arq1);
    fclose(arq2);
}