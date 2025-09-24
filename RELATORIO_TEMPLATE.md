# Relatório: Mini-Projeto 1 - Quebra-Senhas Paralelo

**Aluno(s):** 
Jiye Huang - RA:10438990
Gustavo Kiyoshi Ikeda - RA:10439179
Pedro Montarroyos de Pinho - RA:10440213
---

## 1. Estratégia de Paralelização


**Como você dividiu o espaço de busca entre os workers?**

Pegamos o numero total de senhas e dividimos por numero de workers

**Código relevante:** Cole aqui a parte do coordinator.c onde você calcula a divisão:
```c
long long passwords_per_worker = total_space / num_workers;
long long remaining = total_space % num_workers;
```
---

## 2. Implementação das System Calls

**Descreva como você usou fork(), execl() e wait() no coordinator:**
O coordinator usa a chamada de sistema fork para criar um processo filho para cada worker, se o retorno for 0, significa que é um processo filho, e executa execl para rodar o programa worker com os argumentos de busca certos, se o retorno for positivo, é um processo pai, que guarda o PID do filho em um array e segue criando os workers. Depois de criar todos, o pai entra em um loop com wait para esperar a finalização de cada filho, garantindo que seja sincronizado e evite a criação de processos zumbis.

**Código do fork/exec:**
```
pid_t pid = fork();

if (pid < 0) {
        perror("fork failed");
        exit(1);
}
       
else if (pid == 0) {
        execl("./worker", "worker", target_hash, start_password, end_password, charset, password_len_str, worker_id_str, NULL);
        perror("execl failed");
        exit(1);
}
```
---

## 3. Comunicação Entre Processos

**Como você garantiu que apenas um worker escrevesse o resultado?**
A escrita é feita por um único  worker.c através da chamada open. Em que eu utilizei as flags CREAT e EXCL. A CREAT faz com que o sistema tente  criar um arquivo se ele não existir e a EXCL garante que isso não ira acontecer se o arquivo já existir.  Dessa forma, o primeiro worker que encontrar a senha e tentar criar o arquivo password_found conseguirá e o próximo worker que também encontrar a senha e tentar criar o arquivo logo em seguida, não será possível. Isso evita a condição de corrida e garante que apenas a primeira senha encontrada seja registrada.
Leia sobre condições de corrida (aqui)[https://pt.stackoverflow.com/questions/159342/o-que-%C3%A9-uma-condi%C3%A7%C3%A3o-de-corrida]

**Como o coordinator consegue ler o resultado?**
O coordenador consegue ler o resultado através de duas etapas, primeiro, depois da conclusão de todos os workers, ele abre o arquivo password_found em modo de leitura e copia seu conteúdo para um buffer usando. Em seguida, faz o parsing da string formato separando o ID e a senha, e por fim recalcula o hash da senha obtida com md5_string para compará-lo ao hash salvo, validando o resultado.

---

## 4. Análise de Performance
Complete a tabela com tempos reais de execução:
O speedup é o tempo do teste com 1 worker dividido pelo tempo com 4 workers.

| Teste | 1 Worker | 2 Workers | 4 Workers | Speedup (4w) |
|-------|----------|-----------|-----------|--------------|
| Hash: 202cb962ac59075b964b07152d234b70<br>Charset: "0123456789"<br>Tamanho: 3<br>Senha: "123" | 0.007s | 0.007s | 0.010s | 2 |
| Hash: 5d41402abc4b2a76b9719d911017c592<br>Charset: "abcdefghijklmnopqrstuvwxyz"<br>Tamanho: 5<br>Senha: "hello" | _4.618__s | __8.133__s | _1.942__s | _5.7__ |

**O speedup foi linear? Por quê?**
[O speedup não foi linear, pois a otimização do programa é limitada a parte não paralelizavel do código, de acordo com a Lei de Amdahl, assim tendo que ser executada de maneira sequencial]

---

## 5. Desafios e Aprendizados
**Qual foi o maior desafio técnico que você enfrentou?**
[Tivemos maior dificuldade durante a criação de novos processos com o fork para evitar um fork bomb, assim foi criado um loop que cria apenas a quantidade de workers desejada e logo em seguida designando a tarefa do worker para o novo filho criado]

---

## Comandos de Teste Utilizados

```bash
# Teste básico
./coordinator "900150983cd24fb0d6963f7d28e17f72" 3 "abc" 2

# Teste de performance
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 1
time ./coordinator "202cb962ac59075b964b07152d234b70" 3 "0123456789" 4

# Teste com senha maior
time ./coordinator "5d41402abc4b2a76b9719d911017c592" 5 "abcdefghijklmnopqrstuvwxyz" 4
```
---

**Checklist de Entrega:**
- [✅] Código compila sem erros
- [✅] Todos os TODOs foram implementados
- [✅] Testes passam no `./tests/simple_test.sh`
- [✅] Relatório preenchido
