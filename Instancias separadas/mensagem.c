#include <stdio.h>
#include <stdlib.h>

void prepara_mensagem(unsigned char msg[], unsigned char marcador, unsigned char tamanho, unsigned char sequencia, unsigned char tipo) {
    msg[0] = marcador;
    unsigned char aux = tamanho << 2;
    aux += sequencia >> 3;
    msg[1] = aux;

    aux = sequencia << 5;
    aux += tipo;
    msg[2] = aux;
}

unsigned char obtem_tamanho(unsigned char msg[]) {
    return msg[1] >> 2;
}

unsigned char obtem_sequencia(unsigned char msg[]) {
    unsigned char aux = msg[1] << 6;
    aux = aux >> 3;
    aux += msg[2] >> 5;
    return aux;
}

unsigned char obtem_tipo(unsigned char msg[]) {
    unsigned char aux = msg[2] << 3;
    aux = aux >> 3;
    return aux;
}