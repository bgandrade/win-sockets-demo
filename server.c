#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define BACKLOG_MAX 5
#define BUFFER_SIZE 128
#define Mensagem_para_sair "#sair"

SOCKET local_socket = INVALID_SOCKET;
SOCKET remote_socket = INVALID_SOCKET;

int remote_length = 0;
int message_length = 0;

unsigned short local_port = 0;
unsigned short remote_port = 0;

char message[BUFFER_SIZE];

struct sockaddr_in local_address;
struct sockaddr_in remote_address;

WSADATA wsa_data;

/* Exibe uma mensagem de erro e termina o programa */
void msg_err_exit(const char *msg)
{
    fputs(msg, stderr);
    exit(EXIT_FAILURE);
}

int main(void)
{
    unsigned int input_port = 0;

    // inicia o Winsock 2.0 (DLL)
    if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0)
        msg_err_exit("WSAStartup() failed\n");

    // criando o socket local para o servidor
    local_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (local_socket == INVALID_SOCKET)
    {
        WSACleanup();
        msg_err_exit("socket() failed\n");
    }

    printf("Porta local: ");
    if (scanf("%u", &input_port) != 1 || input_port > 65535)
    {
        closesocket(local_socket);
        WSACleanup();
        msg_err_exit("porta invalida\n");
    }
    local_port = (unsigned short)input_port;

    // zera a estrutura local_address
    memset(&local_address, 0, sizeof(local_address));

    // internet address family
    local_address.sin_family = AF_INET;

    // porta local
    local_address.sin_port = htons(local_port);

    // endereco
    local_address.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr("127.0.0.1")

    // interligando o socket com o endereço (local)
    if (bind(local_socket, (struct sockaddr *)&local_address, sizeof(local_address)) == SOCKET_ERROR)
    {
        closesocket(local_socket);
        WSACleanup();
        msg_err_exit("bind() failed\n");
    }

    // coloca o socket para escutar as conexoes
    if (listen(local_socket, BACKLOG_MAX) == SOCKET_ERROR)
    {
        closesocket(local_socket);
        WSACleanup();
        msg_err_exit("listen() failed\n");
    }

    remote_length = sizeof(remote_address);

    printf("aguardando alguma conexao...\n");

    remote_socket = accept(local_socket, (struct sockaddr *)&remote_address, &remote_length);

    if (remote_socket == INVALID_SOCKET)
    {
        closesocket(local_socket);
        WSACleanup();
        msg_err_exit("accept() failed\n");
    }

    printf("conexao estabelecida com %s\n", inet_ntoa(remote_address.sin_addr));
    printf("aguardando mensagens...\n");
    do
    {
        // limpa o buffer
        memset(&message, 0, BUFFER_SIZE);

        // recebe a mensagem do cliente
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

        // exibe a mensagem na tela
        printf("%s: %s\n", inet_ntoa(remote_address.sin_addr), message);
    } while (strcmp(message, Mensagem_para_sair)); // sai quando receber um "#sair" do cliente

    printf("encerrando\n");
    closesocket(remote_socket);
    closesocket(local_socket);
    WSACleanup();

    system("PAUSE");
    return 0;
}
