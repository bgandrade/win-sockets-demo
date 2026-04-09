#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

/*
 * BUFFER_SIZE:
 * Define o tamanho máximo do texto enviado em cada mensagem.
 * O valor 128 é suficiente para demonstração, evita uso excessivo de memória
 * e simplifica o controle de limites de leitura/escrita.
 */
#define BUFFER_SIZE 128

/*
 * EXIT_MESSAGE:
 * Palavra-chave usada para encerrar a conversa de forma explícita.
 * Mantemos em constante para evitar "string mágica" espalhada no código.
 */
#define EXIT_MESSAGE "#sair"

/*
 * IP_TEXT_BUFFER_SIZE:
 * Tamanho máximo para armazenar um IPv4 em texto, incluindo '\0'.
 * "255.255.255.255" tem 15 caracteres, então 16 seria suficiente.
 * Usamos 32 para margem de segurança sem impacto relevante.
 */
#define IP_TEXT_BUFFER_SIZE 32

/* Exibe uma mensagem de erro e termina o programa */
void msg_err_exit(const char *msg)
{
    fputs(msg, stderr);
    exit(EXIT_FAILURE);
}

int main(void)
{
    SOCKET remote_socket = INVALID_SOCKET;
    WSADATA wsa_data;
    struct sockaddr_in remote_address;
    int message_length = 0;
    unsigned int input_port = 0;
    unsigned short remote_port = 0;
    char remote_ip[IP_TEXT_BUFFER_SIZE];
    char message[BUFFER_SIZE];

    /* Inicializa a biblioteca Winsock antes de qualquer chamada de rede. */
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        msg_err_exit("WSAStartup() failed\n");

    /* Lê o IP do servidor em formato textual (ex.: 127.0.0.1). */
    printf("IP do servidor: ");
    if (scanf("%31s", remote_ip) != 1)
    {
        WSACleanup();
        msg_err_exit("IP inválido\n");
    }

    /* Lê a porta do servidor e valida o intervalo permitido (0 a 65535). */
    printf("Porta do servidor: ");
    if (scanf("%u", &input_port) != 1 || input_port > 65535)
    {
        WSACleanup();
        msg_err_exit("Porta inválida\n");
    }
    remote_port = (unsigned short)input_port;
    getchar();

    /* Cria um socket TCP para comunicação orientada a conexão. */
    remote_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (remote_socket == INVALID_SOCKET)
    {
        WSACleanup();
        msg_err_exit("socket() failed\n");
    }

    /* Preenche a estrutura de endereço remoto que será usada no connect(). */
    memset(&remote_address, 0, sizeof(remote_address));
    remote_address.sin_family = AF_INET;
    if (inet_pton(AF_INET, remote_ip, &remote_address.sin_addr) != 1)
    {
        closesocket(remote_socket);
        WSACleanup();
        msg_err_exit("IP inválido\n");
    }
    remote_address.sin_port = htons(remote_port);

    /* Tenta estabelecer conexão TCP com o servidor informado. */
    printf("Conectando ao servidor %s...\n", remote_ip);
    if (connect(remote_socket, (struct sockaddr *)&remote_address, sizeof(remote_address)) == SOCKET_ERROR)
    {
        closesocket(remote_socket);
        WSACleanup();
        msg_err_exit("connect() failed\n");
    }

    /*
     * Loop principal de envio:
     * 1) lê a mensagem do teclado
     * 2) remove '\n' ao final (quando existir)
     * 3) envia ao servidor
     * encerra quando o usuário digitar EXIT_MESSAGE
     */
    printf("Digite as mensagens\n");
    do
    {
        memset(message, 0, BUFFER_SIZE);

        printf("Mensagem para o servidor: ");
        if (fgets(message, BUFFER_SIZE, stdin) == NULL)
            break;

        message_length = (int)strlen(message);
        if (message_length > 0 && message[message_length - 1] == '\n')
        {
            message[message_length - 1] = '\0';
            message_length--;
        }

        /* Envia exatamente a quantidade de bytes úteis da mensagem. */
        if (send(remote_socket, message, message_length, 0) == SOCKET_ERROR)
        {
            closesocket(remote_socket);
            WSACleanup();
            msg_err_exit("send() failed\n");
        }
    } while (strcmp(message, EXIT_MESSAGE));

    /* Libera recursos de rede em ordem segura antes de encerrar o processo. */
    printf("Encerrando\n");
    closesocket(remote_socket);
    WSACleanup();

    return 0;
}
