#ifndef TRACE_H
#define TRACE_H

// Exibe o resultado do parser
//#define ENABLE_PARSER_TRACE

// Exibe o comportamento do processo filho
// - Quando um cliente conectou/desconectou
// - Quando está aguardando dados
// - Timeout do poll
// - Quando uma leitura foi bem sucedida
// - Quando um envio foi bem sucedido
#define ENABLE_CHLD_TRACE

// Exibe o comportamento do processo pai
#define ENABLE_PARENT_TRACE

// Exibe o conteúdo das respostas e requisições
//#define ENABLE_CHLD_VERBOSE_TRACE

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// Trace do parser
#ifdef ENABLE_PARSER_TRACE
    #define PARSER_TRACE printf("============================== PARSER CMD LIST ==============================\n\n"); printCommandList(cmdList); printf("\n\n")
#else
    #define PARSER_TRACE 
#endif

// Trace do processo filho
#ifdef ENABLE_CHLD_TRACE
    #define CHLD_CREATED_TRACE      printf("[%d] Conectou\n", getpid())
    #define CHLD_EXITED_TRACE       printf("[%d] Desconectou\n", getpid())
    #define CHLD_WAIT_TRACE         printf("[%d] Aguardando dados\n", getpid());fflush(stdout)
    #define CHLD_TIMEDOUT_TRACE     fprintf(stderr, "[%d] poll() timed out\n", getpid())
    #define CHLD_READ_TRACE         printf("[%d] Leitura concluída (%ld bytes)\n", getpid(), bytes_received)
    #define CHLD_RESP_TRACE         printf("[%d] %s - Mensagem enviada (%ld bytes).\n", getpid(), getHttpStatusText(req.httpCode), req.bytes)
#else
    #define CHLD_CREATED_TRACE 
    #define CHLD_EXITED_TRACE 
    #define CHLD_WAIT_TRACE
    #define CHLD_TIMEDOUT_TRACE
    #define CHLD_READ_TRACE
    #define CHLD_RESP_TRACE req.bytes = req.bytes; // Avoid waning
#endif

#ifdef ENABLE_CHLD_VERBOSE_TRACE
    #define CHLDV_REQ_TRACE printf("============================== REQUISIÇÃO RECEBIDA ==============================\n\n%s\n", request); printf("bytes_received: %d\n\n", (int)bytes_received);
    #define CHLDV_RESP_TRACE printf("============================== RESPOSTA ENVIADA ==============================\n\n%s\n", response); printf("bytes_enviados: %d\n\n", (int)bytes_enviados);
#else
    #define CHLDV_REQ_TRACE
    #define CHLDV_RESP_TRACE
#endif

#ifdef ENABLE_PARENT_TRACE
    #define SERVER_START_TRACE printf("\n%s já está aceitando conexões de clientes HTTP em %d.\n\n", argv[0], port)
    #define SERVER_ACCEPTING_TRACE printf("[%d] Aguardando conexões... %d filho(s) livre(s)\n", getpid(), MAX_NUMBER_CHLD - requests)
    #define SERVER_FULL_TRACE printf("[%d] Aguardando conexões... (todos filhos ocupados)\n", getpid())
    #define SERVER_OVERLOAD_TRACE printf("[%d] Mensagem de erro enviada\n", getpid())
#else
    #define SERVER_START_TRACE
    #define SERVER_ACCEPTING_TRACE
    #define SERVER_FULL_TRACE {}; // Avoid warning
    #define SERVER_OVERLOAD_TRACE
#endif

#endif