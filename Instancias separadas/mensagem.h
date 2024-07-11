#ifndef MENSAGEM
#define MENSAGEM

void prepara_mensagem(unsigned char msg[], unsigned char marcador, unsigned char tamanho, unsigned char sequencia, unsigned char tipo);

unsigned char obtem_tamanho(unsigned char msg[]);

unsigned char obtem_sequencia(unsigned char msg[]);

unsigned char obtem_tipo(unsigned char msg[]);

#endif