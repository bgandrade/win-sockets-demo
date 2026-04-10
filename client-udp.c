#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

/*
 * BUFFER_SIZE:
 * Define o tamanho máximo do conteúdo que o cliente tentará colocar
 * em cada datagrama UDP.
 *
 * Em UDP, cada chamada de sendto() envia uma unidade independente
 * de dados. Neste exemplo, escolhemos 128 bytes porque:
 * 1) é um valor pequeno e fácil de raciocinar;
 * 2) evita buffers grandes sem necessidade;
 * 3) mantém a leitura do código simples;
 * 4) é suficiente para mensagens curtas digitadas no terminal.
 *
 * Em aplicações reais, o tamanho do buffer depende do protocolo,
 * do volume de dados esperado e das limitações práticas da rede.
 */
#define BUFFER_SIZE 128

/*
 * EXIT_MESSAGE:
 * Texto usado para encerrar o envio de mensagens de forma explícita.
 *
 * Em vez de comparar diretamente com a string "#sair" em vários pontos
 * do código, centralizamos essa informação em uma constante nomeada.
 * Isso melhora a leitura e evita o uso de "string mágica".
 */
#define EXIT_MESSAGE "#sair"

/*
 * IP_TEXT_BUFFER_SIZE:
 * Espaço reservado para armazenar um IPv4 em formato textual com margem.
 *
 * Um IPv4 no formato decimal pontuado, como "255.255.255.255",
 * precisa de 15 caracteres, mais o terminador '\0'.
 * Portanto, 16 já seria suficiente.
 *
 * Aqui usamos 32 por didática e margem de segurança, sem impacto
 * relevante no consumo de memória.
 */
#define IP_TEXT_BUFFER_SIZE 32

/*
 * Exibe uma mensagem de erro e termina o programa.
 *
 * A ideia desta função é concentrar o comportamento de falha em um só lugar.
 * Isso reduz repetição e mantém o fluxo principal mais legível.
 */
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

    /*
     * Inicializa a biblioteca Winsock antes de qualquer operação de rede.
     *
     * No Windows, diferente de muitos ambientes Unix-like, não basta chamar
     * socket(), bind(), sendto() ou recvfrom() diretamente. Antes disso,
     * é obrigatório inicializar a infraestrutura da API de sockets com
     * WSAStartup().
     *
     * MAKEWORD(2, 2) informa que queremos trabalhar com Winsock 2.2.
     * O segundo argumento recebe uma estrutura WSADATA preenchida pela API
     * com informações sobre a implementação carregada.
     */
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        msg_err_exit("WSAStartup() failed\n");

    /*
     * Lê o IP do servidor UDP em formato textual.
     *
     * Exemplo de entrada válida: 127.0.0.1
     * O formato "%31s" limita a leitura ao tamanho do buffer menos 1,
     * preservando espaço para o caractere terminador '\0'.
     */
    printf("IP do servidor UDP: ");
    if (scanf("%31s", remote_ip) != 1)
    {
        WSACleanup();
        msg_err_exit("IP inválido\n");
    }

    /*
     * Lê a porta do servidor e valida o intervalo permitido.
     *
     * Portas TCP/UDP usam 16 bits, então o intervalo válido vai de
     * 0 a 65535. Aqui usamos um unsigned int para ler com scanf()
     * e só depois convertemos para unsigned short.
     */
    printf("Porta do servidor: ");
    if (scanf("%u", &input_port) != 1 || input_port > 65535)
    {
        WSACleanup();
        msg_err_exit("Porta inválida\n");
    }

    remote_port = (unsigned short)input_port;
    getchar();

    /*
     * Cria um socket UDP para envio de datagramas sem conexão.
     *
     * AF_INET:
     * Define que usaremos IPv4.
     *
     * SOCK_DGRAM:
     * Define o tipo de comunicação como datagrama, isto é, UDP.
     *
     * Protocolo 0:
     * Deixa o sistema escolher automaticamente o protocolo padrão
     * compatível com AF_INET + SOCK_DGRAM, que neste caso é UDP.
     */
    remote_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (remote_socket == INVALID_SOCKET)
    {
        WSACleanup();
        msg_err_exit("socket() failed\n");
    }

    /*
     * Monta a estrutura de endereço do servidor que receberá os datagramas.
     *
     * sockaddr_in é a estrutura específica para endereços IPv4.
     * Ela guarda principalmente:
     * - a família de endereços;
     * - a porta;
     * - o IP de destino.
     *
     * Antes de preencher os campos, zeramos a estrutura inteira para evitar
     * lixo de memória em campos não usados.
     */
    memset(&remote_address, 0, sizeof(remote_address));
    remote_address.sin_family = AF_INET;
    /*
     * inet_pton() converte o IP digitado em texto para o formato binário
     * usado internamente pela pilha de rede.
     *
     * Argumento 1: AF_INET
     * Informa que a conversão será feita para um endereço IPv4.
     *
     * Argumento 2: remote_ip
     * É a string lida do teclado, por exemplo "127.0.0.1".
     *
     * Argumento 3: &remote_address.sin_addr
     * É o campo da estrutura sockaddr_in que receberá o IP já convertido.
     * Esse campo será usado depois nas chamadas de envio para indicar
     * exatamente para qual host o datagrama UDP deve ser encaminhado.
     */
    if (inet_pton(AF_INET, remote_ip, &remote_address.sin_addr) != 1)
    {
        closesocket(remote_socket);
        WSACleanup();
        msg_err_exit("IP inválido\n");
    }
    /*
     * htons() converte a porta da ordem de bytes do host para a ordem
     * de bytes da rede.
     *
     * Em máquinas comuns, o processador costuma usar little-endian.
     * Em protocolos de rede, o padrão esperado é big-endian.
     * Por isso, antes de colocar a porta dentro de sockaddr_in,
     * fazemos essa conversão.
     */
    remote_address.sin_port = htons(remote_port);

    /*
     * Loop principal de envio:
     * 1) lê a mensagem do teclado
     * 2) remove o '\n' ao final, quando existir
     * 3) envia o datagrama para o servidor informado
     * encerra quando o usuário digitar EXIT_MESSAGE
     *
     * Como UDP é orientado a datagramas, cada iteração envia uma mensagem
     * independente. Não existe "conexão estabelecida" como no TCP.
     */
    printf("Digite as mensagens\n");
    do
    {
        memset(message, 0, BUFFER_SIZE);

        printf("Mensagem usando UDP para o servidor: ");
        if (fgets(message, BUFFER_SIZE, stdin) == NULL)
            break;

        message_length = (int)strlen(message);

        if (message_length > 0 && message[message_length - 1] == '\n')
        {
            message[message_length - 1] = '\0';
            message_length--;
        }
        /*
         * Envia o datagrama ao endereço do servidor configurado anteriormente.
         *
         * sendto() recebe:
         * 1) o socket UDP já criado;
         * 2) o ponteiro para os dados;
         * 3) a quantidade de bytes úteis da mensagem;
         * 4) flags, aqui igual a 0;
         * 5) o endereço de destino;
         * 6) o tamanho da estrutura de endereço.
         *
         * O cast para (const struct sockaddr *) é necessário porque a função
         * aceita um tipo genérico de endereço, enquanto remote_address é uma
         * estrutura específica de IPv4 (sockaddr_in).
         */
        if (sendto(remote_socket,
                   message,
                   message_length,
                   0,
                   (const struct sockaddr *)&remote_address,
                   sizeof(remote_address)) == SOCKET_ERROR)
        {
            closesocket(remote_socket);
            WSACleanup();
            msg_err_exit("sendto() failed\n");
        }

    } while (strcmp(message, EXIT_MESSAGE));

    /*
     * Libera os recursos de rede antes de encerrar o processo.
     *
     * closesocket() libera o descritor de socket criado no Winsock.
     * WSACleanup() informa ao sistema que esta aplicação não precisa mais
     * da infraestrutura de sockets inicializada por WSAStartup().
     */
    printf("Encerrando\n");
    closesocket(remote_socket);
    WSACleanup();

    return 0;
}
