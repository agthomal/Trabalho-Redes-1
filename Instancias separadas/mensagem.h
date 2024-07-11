#ifndef MENSAGEM
#define MENSAGEM

int cria_raw_socket(char* nome_interface_rede);

long long timestamp();

int protocolo_e_valido(char* buffer, int tamanho_buffer);

int recebe_mensagem(int soquete, int timeoutMillis, char* buffer, int tamanho_buffer);

void prepara_mensagem(unsigned char msg[], unsigned char marcador, unsigned char tamanho, unsigned char sequencia, unsigned char tipo);

unsigned char obtem_tamanho(unsigned char msg[]);

unsigned char obtem_sequencia(unsigned char msg[]);

unsigned char obtem_tipo(unsigned char msg[]);

#endif