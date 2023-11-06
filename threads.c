#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 4
#define QUEUE_MAX_SIZE 4

typedef struct {
    // Inclua quaisquer dados necessários para a tarefa
    // ...
    // Por exemplo, um ponteiro para uma função que a thread deve executar
    void (*function)(void*);
    void* data;
} Task;

typedef struct {
    pthread_t threads[NUM_THREADS];
    Task taskQueue[QUEUE_MAX_SIZE]; // Fila de tasks
    int freeThreads; // Número de threads disponíveis
    int queueSize;   // Número atual de tasks
    int queueMaxSize; // Tamanho máximo da fila de tasks
    int head, tail; // Indíce do começo e fim da fila de task (buffer-circular)
    pthread_mutex_t queueLock;
    pthread_cond_t notEmpty;
    pthread_cond_t notFull;
    int shutdown;
} ThreadPool;

ThreadPool threadpool;

// Garantir que a leitura da ID da thread vai ser legível por humanos
pthread_t get_thread_hId(){
    for (int i = 0; i < NUM_THREADS; i++){
        if (pthread_self()==threadpool.threads[i])
            return (pthread_t)(i+1);
    }
    return (pthread_t)0;
}

// Função que cada thread executa
void* thread_function(void* arg);

void thread_pool_init(ThreadPool* pool) {
    pool->queueMaxSize = QUEUE_MAX_SIZE;
    pool->queueSize = 0;
    pool->head = pool->tail = 0;
    pool->shutdown = 0;
    pool->freeThreads = NUM_THREADS;

    pthread_mutex_init(&pool->queueLock, NULL);
    pthread_cond_init(&pool->notEmpty, NULL);
    pthread_cond_init(&pool->notFull, NULL);

    // Criação de cada uma das threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&pool->threads[i], NULL, thread_function, (void*)pool);
    }
}

void* thread_function(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;

    while (1) {

        // Quando uma thread chegar aqui, ela vai travar a fila (somente ela mexe)
        pthread_mutex_lock(&pool->queueLock);

        // Enquanto o tamanho da fila de tarefas for 0 vai esperar
        while (pool->queueSize == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->notEmpty, &pool->queueLock);
        }

        // Caso de shutdown
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queueLock);
            pthread_exit(NULL);
        }

        // Pega uma tarefa da fila e avança em uma posição 
        // usando um buffer circular
        Task task = pool->taskQueue[pool->head];
        pool->head = (pool->head + 1) % pool->queueMaxSize;
        pool->queueSize--;

        // Avisa que a lista de tarefas não está mais cheia e destrava a fila
        pthread_cond_signal(&pool->notFull);
        pthread_mutex_unlock(&pool->queueLock);
        // =========================================================

        // Executa a tarefa
        task.function(task.data);
    }
    return NULL;
}

void thread_pool_add_task(ThreadPool* pool, void (*function)(void*), void* data) {
    
    // Trava a fila
    pthread_mutex_lock(&pool->queueLock);

    // Você não se preocupa com tamanho de fila, pois enquanto todas as threads estiverem
    // ocupadas, nada vai ser adicionado.
    
    /*while (pool->queueSize == pool->queueMaxSize && !pool->shutdown) {
        // Espera enquanto a fila está cheia
        pthread_cond_wait(&pool->notFull, &pool->queueLock);
    }*/

    // Caso de shutdown
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->queueLock);
        return;
    }

    // Cria uma tarefa, associando uma função a ser executada e os parâmetros
    Task task;
    task.function = function;
    task.data = data;

    // Adiciona a task na fila
    pool->taskQueue[pool->tail] = task;
    pool->tail = (pool->tail + 1) % pool->queueMaxSize;
    pool->queueSize++;


    // Sinaliza que a lista de tasks não está vazia
    pthread_cond_signal(&pool->notEmpty);
    pthread_mutex_unlock(&pool->queueLock);
}

void thread_pool_shutdown(ThreadPool* pool) {
    // Encerra a pool

    pthread_mutex_lock(&pool->queueLock);
    pool->shutdown = 1;
    pthread_mutex_unlock(&pool->queueLock);

    pthread_cond_broadcast(&pool->notEmpty);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->queueLock);
    pthread_cond_destroy(&pool->notEmpty);
    pthread_cond_destroy(&pool->notFull);
}


void task_function(void* data) {
    // A tarefa ser executada
    
    // Algum processamento demorado
    sleep(5); 
   
   // Quando acabar, libera a thread
    pthread_mutex_lock(&(threadpool.queueLock));
    threadpool.freeThreads++;
    printf("[%ld] Thread finalizou %s (%d threads livres)\n", get_thread_hId(), (char *)data, threadpool.freeThreads);
    pthread_mutex_unlock(&(threadpool.queueLock));
}

int main(){
    thread_pool_init(&threadpool);
    
    // Adicione algumas tarefas à thread pool
    for (int i = 0; i < 10; i++) {
        sleep(1);
        // accept(socket...)
        
        pthread_mutex_lock(&(threadpool.queueLock));
        
        // Verifica se threads disponíveis
        if (threadpool.freeThreads > 0){
            // Aloca uma thread
            threadpool.freeThreads--;
            printf("[%ld] Alocando thread (%d threads livres)\n", get_thread_hId(), threadpool.freeThreads);
            pthread_mutex_unlock(&(threadpool.queueLock));
            
            // Coloca a thread na lista de tasks
            char* message = (char*)malloc(50);
            snprintf(message, 50, "Task %d", i+1);
            thread_pool_add_task(&threadpool, task_function, message);
        } else {
            printf("[%ld] Enviando mensagem de sobrecarga (0 threads livres)\n", get_thread_hId());
            pthread_mutex_unlock(&(threadpool.queueLock));
        }
    }

    sleep(10);

    thread_pool_shutdown(&threadpool);

    return EXIT_SUCCESS;
}
