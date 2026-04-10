#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

/*
 * BUFFER_SIZE:
 * Tamanho máximo do conteúdo que o servidor tentará ler de cada datagrama.
 *
 * Como este projeto é didático, 128 bytes são suficientes para mensagens
 * curtas digitadas no terminal e mantêm o raciocínio simples.
 *
 * Em uma aplicação real, esse tamanho dependeria do protocolo adotado,
 * do volume esperado de dados e da estratégia de tratamento de mensagens
 * maiores do que o buffer disponível.
 */
#define BUFFER_SIZE 128

/*
 * EXIT_MESSAGE:
 * Mensagem de controle usada pelo cliente para indicar encerramento.
 *
 * Mantemos essa informação em uma constante para evitar repetição de texto
 * literal e deixar explícito que essa string tem papel de comando.
 */
#define EXIT_MESSAGE "#sair"

/*
 * Exibe uma mensagem de erro e termina o programa.
 *
 * Centralizar esse comportamento evita repetição e mantém o fluxo
 * principal do servidor mais fácil de ler.
 */
void msg_err_exit(const char *msg)
{
    fputs(msg, stderr);
    exit(EXIT_FAILURE);
}

int main(void)
{
    SOCKET local_socket = INVALID_SOCKET;
    WSADATA wsa_data;
    struct sockaddr_in local_address;
    struct sockaddr_in remote_address;
    int remote_address_length = 0;
    int message_length = 0;
    unsigned int input_port = 0;
    unsigned short local_port = 0;
    char message[BUFFER_SIZE];

    /*
     * Inicializa a API Winsock antes de qualquer operação de rede.
     *
     * No Windows, o uso de sockets depende dessa etapa prévia.
     * Sem WSAStartup(), chamadas como socket(), bind() e recvfrom()
     * não devem ser usadas.
     */
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        msg_err_exit("WSAStartup() failed\n");

    /*
     * Cria um socket UDP, adequado para envio e recepção de datagramas.
     *
     * AF_INET define IPv4.
     * SOCK_DGRAM define comunicação por datagramas, isto é, UDP.
     * O protocolo 0 deixa o sistema escolher automaticamente o protocolo
     * padrão correspondente a essa combinação.
     */
    local_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (local_socket == INVALID_SOCKET)
    {
        WSACleanup();
        msg_err_exit("socket() failed\n");
    }

    /*
     * Lê e valida a porta em que o servidor ficará escutando.
     *
     * Usamos um unsigned int na leitura para validar primeiro e só depois
     * converter para unsigned short, que é o tipo efetivamente usado
     * pela estrutura de endereço IPv4.
     */
    printf("Porta local: ");
    if (scanf("%u", &input_port) != 1 || input_port > 65535)
    {
        closesocket(local_socket);
        WSACleanup();
        msg_err_exit("Porta inválida\n");
    }
    local_port = (unsigned short)input_port;

    /*
     * Zera a estrutura para evitar conteúdo indefinido em campos não usados.
     *
     * Isso é uma prática importante ao trabalhar com estruturas de rede,
     * pois garante que apenas os campos que realmente configuramos terão
     * valores relevantes.
     */
    memset(&local_address, 0, sizeof(local_address));

    /*
     * Configura o endereço local como IPv4, na porta informada,
     * aceitando tráfego em qualquer interface da máquina.
     *
     * sin_family:
     * Identifica a família de endereços usada pela estrutura.
     *
     * sin_port:
     * Guarda a porta em ordem de bytes de rede.
     *
     * sin_addr.s_addr = htonl(INADDR_ANY):
     * Faz o servidor aceitar datagramas vindos de qualquer interface local,
     * por exemplo loopback (127.0.0.1) e interfaces de rede reais.
     */
    local_address.sin_family = AF_INET;
    local_address.sin_port = htons(local_port);
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);

    /*
     * Associa o socket UDP ao endereço local para receber datagramas
     * nessa porta específica.
     *
     * Em termos práticos, bind() "reserva" a porta para este processo.
     * Sem isso, o sistema não saberia em qual porta o servidor deseja
     * escutar os datagramas recebidos.
     */
    if (bind(local_socket, (struct sockaddr *)&local_address, sizeof(local_address)) == SOCKET_ERROR)
    {
        closesocket(local_socket);
        WSACleanup();
        msg_err_exit("bind() failed\n");
    }

    printf("Aguardando mensagens UDP...\n");
    do
    {
        /*
         * Reinicia o buffer e o tamanho da estrutura de origem antes
         * de cada recepção.
         *
         * remote_address_length precisa informar ao recvfrom() quanto espaço
         * existe disponível para armazenar o endereço do remetente.
         */
        memset(message, 0, BUFFER_SIZE);
        remote_address_length = sizeof(remote_address);

        /*
         * recvfrom() recebe o próximo datagrama disponível no socket.
         *
         * Além de copiar os dados para o buffer message, a função também
         * preenche remote_address com as informações de origem do datagrama,
         * isto é, o IP e a porta do cliente que enviou a mensagem.
         *
         * Isso é especialmente importante em UDP, porque não existe uma
         * conexão contínua como no TCP. Cada datagrama pode, em teoria,
         * vir de um remetente diferente.
         */
        message_length = recvfrom(
            local_socket,
            message,
            BUFFER_SIZE - 1,
            0,
            (struct sockaddr *)&remote_address,
            &remote_address_length);

        if (message_length == SOCKET_ERROR)
        {
            closesocket(local_socket);
            WSACleanup();
            msg_err_exit("recvfrom() failed\n");
        }

        message[message_length] = '\0';

        /*
         * Exibe no console o remetente e o conteúdo recebido.
         *
         * inet_ntoa() converte o IP binário do remetente para texto.
         * ntohs() converte a porta da ordem de bytes da rede para a
         * ordem de bytes do host, tornando o valor legível localmente.
         */
        printf("%s:%u -> %s\n",
               inet_ntoa(remote_address.sin_addr),
               ntohs(remote_address.sin_port),
               message);
    } while (strcmp(message, EXIT_MESSAGE));

    /*
     * Libera o socket e finaliza a API de rede ao encerrar o servidor.
     *
     * closesocket() encerra o uso do socket criado.
     * WSACleanup() finaliza o uso da infraestrutura Winsock por este processo.
     */
    printf("Encerrando\n");
    closesocket(local_socket);
    WSACleanup();

    return 0;
}
