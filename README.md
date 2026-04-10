# Cliente-Servidor Demo em C com TCP e UDP

[![C](https://img.shields.io/badge/C-C17-blue.svg)](https://en.wikipedia.org/wiki/C17_(C_standard_revision))
[![Windows](https://img.shields.io/badge/Plataforma-Windows-0078D6.svg)](https://learn.microsoft.com/windows/win32/winsock/windows-sockets-start-page-2)
[![Winsock](https://img.shields.io/badge/API-Winsock%202.2-2ea44f.svg)](https://learn.microsoft.com/windows/win32/winsock/windows-sockets-start-page-2)
[![TCP/UDP](https://img.shields.io/badge/Protocolos-TCP%20%7C%20UDP-orange.svg)](https://en.wikipedia.org/wiki/Internet_protocol_suite)
[![Status](https://img.shields.io/badge/Status-Educacional-important.svg)](#)
[![Build](https://img.shields.io/badge/Build-Makefile-lightgrey.svg)](./Makefile)

Projeto demo para estudo de comunicação em rede no **Windows**, usando **Winsock 2.2**. O repositório contém duas implementações paralelas:

- uma versão **TCP**, orientada a conexão;
- uma versão **UDP**, orientada a datagramas.

O foco do projeto é **aprendizado**. Por isso, o código foi escrito com comentários explicativos e com uma estrutura simples o suficiente para facilitar leitura, depuração e comparação entre os dois protocolos.

## Objetivo

Este projeto serve para estudar, na prática:

- inicialização e encerramento do Winsock com `WSAStartup()` e `WSACleanup()`;
- criação de sockets com `socket()`;
- montagem de endereços com `sockaddr_in`;
- diferenças entre **TCP** e **UDP**;
- uso de chamadas como `bind()`, `listen()`, `accept()`, `connect()`, `send()`, `recv()`, `sendto()` e `recvfrom()`;
- conversões de ordem de bytes com `htons()`, `htonl()` e `ntohs()`;
- envio e recepção de mensagens simples no terminal.

## Requisitos

Para compilar no Windows, você precisa de:

- compilador `gcc` com suporte a Windows;
- `mingw32-make` ou `make`;
- suporte à compilação em **C17**;
- opcionalmente, `gdb` para depuração.
- **VS Code** com extensão de C/C++, se quiser usar as tasks/debug já configuradas

## Compilação

Para compilar tudo:

```bash
mingw32-make all
```

Os executáveis serão gerados na pasta `build/`.

## Como executar

## Fluxo TCP

### 1. Inicie o servidor TCP

```bash
.\build\server-tcp.exe
```

O programa pedirá a **porta local** em que ficará escutando.

### 2. Inicie o cliente TCP

```bash
.\build\client-tcp.exe
```

Informe:

- o IP do servidor, por exemplo `127.0.0.1`;
- a porta configurada no servidor.

### 3. Envie mensagens

Digite as mensagens no cliente. O servidor exibirá o conteúdo recebido.

Para encerrar, digite:

```text
#sair
```

## Fluxo UDP

### 1. Inicie o servidor UDP

```bash
.\build\server-udp.exe
```

Informe a **porta local** que ficará aguardando datagramas.

### 2. Inicie o cliente UDP

```bash
.\build\client-udp.exe
```

Informe:

- o IP do servidor UDP;
- a porta configurada no servidor.

### 3. Envie mensagens

Cada mensagem digitada no cliente é enviada como um **datagrama UDP independente**.  
O servidor exibirá:

- o IP de origem;
- a porta de origem;
- o conteúdo recebido.

Para encerrar, digite:

```text
#sair
```

## Diferenças observáveis entre TCP e UDP neste projeto

### TCP

- exige conexão entre cliente e servidor;
- usa `listen()`, `accept()` e `connect()`;
- mantém uma sessão ativa de comunicação;
- é adequado quando a entrega confiável é prioridade.

### UDP

- não estabelece conexão formal;
- usa `sendto()` e `recvfrom()`;
- cada envio é independente;
- é mais simples no fluxo, mas não garante entrega, ordem ou retransmissão.

## Conceitos que vale observar no código

Ao ler os arquivos, vale prestar atenção especialmente nestes pontos:

- por que `sockaddr_in` é usado para preencher endereços IPv4;
- por que as funções da API recebem `struct sockaddr *` ou `const struct sockaddr *`;
- por que portas e endereços precisam de conversão de ordem de bytes;
- por que o UDP usa `recvfrom()` para descobrir quem enviou a mensagem;
- por que o TCP precisa aceitar conexão antes de receber dados;
- por que a string `#sair` foi transformada em constante nomeada.

## Limitações intencionais

Este projeto é uma **demo educacional**, então algumas simplificações são deliberadas:

- não há tratamento avançado de timeout;
- não há múltiplos clientes simultâneos;
- não há retransmissão ou confirmação no UDP;
- o protocolo de aplicação é apenas texto simples;
- o foco está em clareza didática, não em arquitetura de produção.
