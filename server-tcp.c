#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

/*
 * BACKLOG_MAX:
 * Número máximo de conexões pendentes na fila do listen().
 * O valor 5 é adequado para exemplo didático e evita fila grande desnecessária.
 */
#define BACKLOG_MAX 5

/*
 * BUFFER_SIZE:
 * Tamanho máximo de dados lidos por mensagem.
 * 128 bytes mantém o exemplo simples, com consumo de memória baixo.
 */
#define BUFFER_SIZE 128

/*
 * EXIT_MESSAGE:
 * Comando textual usado pelo cliente para sinalizar encerramento da sessão.
 */
#define EXIT_MESSAGE "#sair"

/* Exibe uma mensagem de erro e termina o programa */
void msg_err_exit(const char *msg)
{
    fputs(msg, stderr);
    exit(EXIT_FAILURE);
}

int main(void)
{
    SOCKET local_socket = INVALID_SOCKET;
    SOCKET remote_socket = INVALID_SOCKET;
    WSADATA wsa_data;
    struct sockaddr_in local_address;
    struct sockaddr_in remote_address;
    int remote_address_length = 0;
    int message_length = 0;
    unsigned int input_port = 0;
    unsigned short local_port = 0;
    char message[BUFFER_SIZE];

    /* Inicializa a API Winsock antes de criar sockets. */
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        msg_err_exit("WSAStartup() failed\n");

    /* Cria o socket TCP que ficará escutando conexões na porta local. */
    local_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (local_socket == INVALID_SOCKET)
    {
        WSACleanup();
        msg_err_exit("socket() failed\n");
    }

    /* Lê e valida a porta de escuta do servidor. */
    printf("Porta local: ");
    if (scanf("%u", &input_port) != 1 || input_port > 65535)
    {
        closesocket(local_socket);
        WSACleanup();
        msg_err_exit("Porta inválida\n");
    }
    local_port = (unsigned short)input_port;

    /* Zera a estrutura para evitar lixo de memória em campos não usados. */
    memset(&local_address, 0, sizeof(local_address));

    /* Define IPv4 como família de endereços. */
    local_address.sin_family = AF_INET;

    /* Converte a porta para formato de rede (big-endian). */
    local_address.sin_port = htons(local_port);

    /*
     * Aceita conexões em qualquer interface local (INADDR_ANY),
     * útil para testes em loopback e também na rede local.
     */
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Associa o socket ao endereço/porta escolhidos. */
    if (bind(local_socket, (struct sockaddr *)&local_address, sizeof(local_address)) == SOCKET_ERROR)
    {
        closesocket(local_socket);
        WSACleanup();
        msg_err_exit("bind() failed\n");
    }

    /* Coloca o socket em modo passivo para aguardar conexões de clientes. */
    if (listen(local_socket, BACKLOG_MAX) == SOCKET_ERROR)
    {
        closesocket(local_socket);
        WSACleanup();
        msg_err_exit("listen() failed\n");
    }

    remote_address_length = sizeof(remote_address);

    printf("Aguardando conexão...\n");

    /*
     * Aceita a próxima conexão pendente.
     * A partir daqui, remote_socket representa a sessão com um cliente.
     */
    remote_socket = accept(local_socket, (struct sockaddr *)&remote_address, &remote_address_length);

    if (remote_socket == INVALID_SOCKET)
    {
        closesocket(local_socket);
        WSACleanup();
        msg_err_exit("accept() failed\n");
    }

    printf("Conexão estabelecida com %s\n", inet_ntoa(remote_address.sin_addr));
    printf("Aguardando mensagens...\n");
    do
    {
        /* Limpa o buffer para facilitar depuração e evitar resíduos visuais. */
        memset(message, 0, BUFFER_SIZE);

        /*
         * Recebe dados do cliente:
         * - SOCKET_ERROR indica falha de rede
         * - 0 indica que o cliente encerrou a conexão
         */
        message_length = recv(remote_socket, message, BUFFER_SIZE - 1, 0);
        if (message_length == SOCKET_ERROR)
        {
            closesocket(remote_socket);
            closesocket(local_socket);
            WSACleanup();
            msg_err_exit("recv() failed\n");
        }
        if (message_length == 0)
            break;

        message[message_length] = '\0';

        /* Exibe no console o IP de origem e o conteúdo recebido. */
        printf("%s: %s\n", inet_ntoa(remote_address.sin_addr), message);
    } while (strcmp(message, EXIT_MESSAGE));

    /* Libera sockets e finaliza o Winsock ao encerrar o servidor. */
    printf("Encerrando\n");
    closesocket(remote_socket);
    closesocket(local_socket);
    WSACleanup();

    return 0;
}
