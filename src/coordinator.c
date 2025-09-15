#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "hash_utils.h"

/**
 * PROCESSO COORDENADOR - Mini-Projeto 1: Quebra de Senhas Paralelo
 * 
 * Este programa coordena múltiplos workers para quebrar senhas MD5 em paralelo.
 * O MD5 JÁ ESTÁ IMPLEMENTADO - você deve focar na paralelização (fork/exec/wait).
 * 
 * Uso: ./coordinator <hash_md5> <tamanho> <charset> <num_workers>
 * 
 * Exemplo: ./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 4
 * 
 * SEU TRABALHO: Implementar os TODOs marcados abaixo
 */

#define MAX_WORKERS 16
#define RESULT_FILE "password_found.txt"

/**
 * Calcula o tamanho total do espaço de busca
 * 
 * @param charset_len Tamanho do conjunto de caracteres
 * @param password_len Comprimento da senha
 * @return Número total de combinações possíveis
 */
long long calculate_search_space(int charset_len, int password_len) {
    long long total = 1;
    for (int i = 0; i < password_len; i++) {
        total *= charset_len;
    }
    return total;
}

/**
 * Converte um índice numérico para uma senha
 * Usado para definir os limites de cada worker
 * 
 * @param index Índice numérico da senha
 * @param charset Conjunto de caracteres
 * @param charset_len Tamanho do conjunto
 * @param password_len Comprimento da senha
 * @param output Buffer para armazenar a senha gerada
 */
void index_to_password(long long index, const char *charset, int charset_len, 
                       int password_len, char *output) {
    for (int i = password_len - 1; i >= 0; i--) {
        output[i] = charset[index % charset_len];
        index /= charset_len;
    }
    output[password_len] = '\0';
}

/**
 * Função principal do coordenador
 */
int main(int argc, char *argv[]) {
    // TODO 1: Validar argumentos de entrada
    // Verificar se argc == 5 (programa + 4 argumentos)
    // Se não, imprimir mensagem de uso e sair com código 1
    
    // IMPLEMENTE AQUI: verificação de argc e mensagem de erro
    if(argc != 5)
    {
        printf("Parametros invalidos!\n");
        return 1;
    }
    // Parsing dos argumentos (após validação)
    const char *target_hash = argv[1];
    int password_len = atoi(argv[2]);
    const char *charset = argv[3];
    int num_workers = atoi(argv[4]);
    int charset_len = strlen(charset);
    
    // TODO: Adicionar validações dos parâmetros
    // - password_len deve estar entre 1 e 10
    // - num_workers deve estar entre 1 e MAX_WORKERS
    // - charset não pode ser vazio
    if(password_len < 1 || password_len > 10)
    {
        printf("Erro: Tamanho da senha deve estar entre 1 e 10.\n");
        return 1;
    }
    if(num_workers < 1 || num_workers > MAX_WORKERS)
    {
        printf("Erro: Numero de workers deve estar entre 1 e %d.\n", MAX_WORKERS);
        return 1;
    }
    if(charset_len == 0)
    {
        printf("Erro: charset vazio.\n");
        return 1;
    }
    
    printf("=== Mini-Projeto 1: Quebra de Senhas Paralelo ===\n");
    printf("Hash MD5 alvo: %s\n", target_hash);
    printf("Tamanho da senha: %d\n", password_len);
    printf("Charset: %s (tamanho: %d)\n", charset, charset_len);
    printf("Número de workers: %d\n", num_workers);
    
    // Calcular espaço de busca total
    long long total_space = calculate_search_space(charset_len, password_len);
    printf("Espaço de busca total: %lld combinações\n\n", total_space);
    
    // Remover arquivo de resultado anterior se existir
    unlink(RESULT_FILE);
    
    // Registrar tempo de início
    time_t start_time = time(NULL);
    
    // TODO 2: Dividir o espaço de busca entre os workers
    // Calcular quantas senhas cada worker deve verificar
    // DICA: Use divisão inteira e distribua o resto entre os primeiros workers
    
    // IMPLEMENTE AQUI:
    long long passwords_per_worker = total_space / num_workers;
    long long remaining = total_space % num_workers;
    long long current_index = 0;
    
    // Arrays para armazenar PIDs dos workers
    pid_t workers[MAX_WORKERS];
    
    // TODO 3: Criar os processos workers usando fork()
    printf("Iniciando workers...\n");
    
    // IMPLEMENTE AQUI: Loop para criar workers
    for (int i = 0; i < num_workers; i++) {
        long long passwords_to_check = passwords_per_worker;
        if (i < remaining) {
            passwords_to_check++;
        }
        
        char start_password[password_len + 1];
        char end_password[password_len + 1];
        
        index_to_password(current_index, charset, charset_len, password_len, start_password);
        index_to_password(current_index + passwords_to_check - 1, charset, charset_len, password_len, end_password);
        
        // TODO: Converter indices para senhas de inicio e fim
        char password_len_str[10];
        char worker_id_str[10];
        sprintf(password_len_str, "%d", password_len);
        sprintf(worker_id_str, "%d", i);

        // TODO 4: Usar fork() para criar processo filho
        pid_t pid = fork();
        
        // TODO 7: Tratar erros de fork() e execl()
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
        
        // TODO 6: No processo filho: usar execl() para executar worker
        else if (pid == 0) {
            execl("./worker", "worker", target_hash, start_password, end_password, charset, password_len_str, worker_id_str, NULL);
            perror("execl failed");
            exit(1);
        }
        
        // TODO 5: No processo pai: armazenar PID
        else {
            workers[i] = pid;
            printf("  Worker %d (PID %d): %s até %s\n", i, pid, start_password, end_password);
        }
        current_index += passwords_to_check;
    }
    
    printf("\nTodos os workers foram iniciados. Aguardando conclusão...\n");
    
    // TODO 8: Aguardar todos os workers terminarem usando wait()
    // IMPORTANTE: O pai deve aguardar TODOS os filhos para evitar zumbis
    
    // IMPLEMENTE AQUI:
    int estado;
    pid_t pidTerminado;
    
    // O loop continua enquanto wait() retorna o PID de um filho que terminou.
    // Quando todos os filhos terminarem, wait() retornará -1.
    while ((pidTerminado = wait(&estado)) > 0) {
        printf("worker finalizado - PID %d\n", pidTerminado);
    }
    // Registrar tempo de fim
    time_t end_time = time(NULL);
    double elapsed_time = difftime(end_time, start_time);
    
    printf("\n=== Resultado ===\n");
    
    // TODO 9: Verificar se algum worker encontrou a senha
    // Ler o arquivo password_found.txt se existir
    // - Abrir arquivo RESULT_FILE para leitura
    int arquivoDescritor = open(RESULT_FILE, O_RDONLY); //variável que identifica o arquivo no sistema
    
    // - Ler conteúdo do arquivo
    if (arquivoDescritor >= 0) {
        char buffer[256];
        ssize_t bytes = read(arquivoDescritor, buffer, sizeof(buffer) - 1); 
        
        if (bytes > 0) {
            buffer[bytes] = '\0';
            
            // - Fazer parse do formato "worker_id:password"
            char* doisPontos = strchr(buffer, ':'); //ponteiro para procurar os :
            if (doisPontos != NULL) {
                *doisPontos = '\0';
                int workerID = atoi(buffer); //variavel que armazena o worker que encontrou a senha
                char* senhaEncontrada = doisPontos + 1; //marca o inicio da senha
                
                char* novaLinha = strchr(senhaEncontrada, '\n'); //variavel para encontrar a quebra de linha
                if (novaLinha != NULL) {
                    *novaLinha = '\0';
                }

                // - Verificar o hash usando md5_string()
                char hash_verificado[33];
                md5_string(senhaEncontrada, hash_verificado); //garante que a senha encontrada é realmente a que procuramos

                // - Exibir resultado encontrado
                if (strcmp(hash_verificado, target_hash) == 0) {
                    printf("Senha: %s, foi encontada pelo o worker: %d\n", senhaEncontrada, workerID);
                } else {
                    printf("A senha que foi encontada não corresponde");
                }
            }
        }
        close(arquivoDescritor);
    } else {
        printf("Senha não encontrada\n");
    }
    
    // Estatísticas finais (opcional)
    printf("tempo de execução para achar a senha: %.5f", elapsed_time);
    
    return 0;
}
