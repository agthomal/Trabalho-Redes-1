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

#define TAM_MSG 0x8000
#define TAM_MIN 14
#define OFFSET 3

int cria_raw_socket(char* nome_interface_rede) {
    // Cria arquivo para o socket sem qualquer protocolo
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete == -1) {
        fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
        exit(-1);
    }

    int ifindex = if_nametoindex(nome_interface_rede);

    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;
    // Inicializa socket
    if (bind(soquete, (struct sockaddr*) &endereco, sizeof(endereco)) == -1) {
        fprintf(stderr, "Erro ao fazer bind no socket\n");
        exit(-1);
    }

    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    // Não joga fora o que identifica como lixo: Modo promíscuo
    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        fprintf(stderr, "Erro ao fazer setsockopt: "
            "Verifique se a interface de rede foi especificada corretamente.\n");
        exit(-1);
    }

    return soquete;
}

// usando long long pra (tentar) sobreviver ao ano 2038
long long timestamp() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec*1000 + tp.tv_usec/1000;
}
 
int protocolo_e_valido(char* buffer, int tamanho_buffer) {
    if (tamanho_buffer <= 0) { return 0; }
    // insira a sua validação de protocolo aqui
    return buffer[0] == 0x7f;
    return 1;
}
 
// retorna -1 se deu timeout, ou quantidade de bytes lidos
int recebe_mensagem(int soquete, int timeoutMillis, char* buffer, int tamanho_buffer) {
    long long comeco = timestamp();
    struct timeval timeout = { .tv_sec = timeoutMillis/1000, .tv_usec = (timeoutMillis%1000) * 1000 };
    setsockopt(soquete, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    int bytes_lidos;
    do {
        bytes_lidos = recv(soquete, buffer, tamanho_buffer, 0);
        if (protocolo_e_valido(buffer, bytes_lidos)) { return bytes_lidos; }
    } while (timestamp() - comeco <= timeoutMillis);
    return -1;
}

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
        /*if (strlen(buffer) == 0)
            printf("Falhou\n");
        else
            printf("%ld\n", strlen(buffer)); */

        if (buffer[1] == 0x04 && buffer[2] == '\0') {
            break;
        }
        // printf("%s\n", buffer);

        /*while (strlen(buffer2) == 0 && strlen(buffer) != 0) {
            envio = send(s, buffer, buf_size, 0);
            recebe = recebe_mensagem(t, 200, buffer2, buf_size);
            printf("Bloco nao conseguiu ser enviado. Tentando novamente\n");
        } */
        if (recebe != -1 && strlen(buffer) > 0) {
            fwrite(buffer + OFFSET, recebe - OFFSET, 1, arq1);
            printf("%d\n", obtem_sequencia(buffer));
        }
        sleep(1);
    }
    fclose(arq1);
    // fclose(arq2);
}