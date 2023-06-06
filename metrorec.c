#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

struct estacao {
    pthread_mutex_t mu_estacao;
    pthread_cond_t passageiro_embarcou;
    pthread_cond_t vagao_chegou;
    // não precisamos mais da condição vagão saiu (vamos usar a variavel vagao)
    int passageiros_aguardando;
    int vagas_vagao;
    int vagao; // variavel pra indicar que um vaga esta aguardando passageiros na estacao
    int passageiros_para_embarque; // variavel pra indicar a quantidade de passageiros que irao subir no vagao (separado de vagas_vagao)
};

void estacao_init(struct estacao *estacao) {
    if (estacao == NULL) {
        estacao = malloc(sizeof(struct estacao));
    }

    pthread_mutex_init(&estacao->mu_estacao, NULL);
    pthread_cond_init(&estacao->passageiro_embarcou, NULL);
    pthread_cond_init(&estacao->vagao_chegou, NULL);
    estacao->passageiros_aguardando = 0;
    estacao->vagas_vagao = 0;
    estacao->vagao = 0; // inicializar variavel nova
    estacao->passageiros_para_embarque = 0; // inicializar variavel nova
}

void estacao_preencher_vagao(struct estacao * estacao, int assentos) {
    if (estacao == NULL) {
        return;
    }

    pthread_mutex_lock(&estacao->mu_estacao);
    estacao->vagao = 1; // O vagao chegou, usa essa variavel pra demonstrar isso
    estacao->vagas_vagao = assentos;
    if (estacao->passageiros_aguardando > assentos) {
        estacao->passageiros_para_embarque = assentos;
    } else {
        estacao->passageiros_para_embarque = estacao->passageiros_aguardando;
    } // A quantidade de passageiros que vai embarcar é o minimo entre a quantidade de vagas no vagao e a quantidade de passageiros aguardando

    pthread_cond_broadcast(&estacao->vagao_chegou);


    while(estacao->passageiros_aguardando && estacao->vagas_vagao) {
        pthread_cond_wait(&estacao->passageiro_embarcou, &estacao->mu_estacao);
    }

    estacao->vagao = 0; // o vagao saiu
    pthread_mutex_unlock(&estacao->mu_estacao);
    // não precisa dar um sinal que o vagao saiu, basta colocar a variavel de vagao em 0
}

void estacao_embarque(struct estacao * estacao) {
    if (estacao == NULL) {
        return;
    }

    pthread_mutex_lock(&estacao->mu_estacao);

    while(!estacao->vagao){ // vamos aguardar o vagao chegar de fato na estação
		pthread_cond_wait(&estacao->vagao_chegou,&estacao->mu_estacao);
	}

    estacao->passageiros_aguardando--;
    estacao->vagas_vagao--; // precisamos ocupar uma vaga no vagao tambem
    pthread_cond_signal(&estacao->passageiro_embarcou);
    pthread_mutex_unlock(&estacao->mu_estacao);
}

void estacao_espera_pelo_vagao(struct estacao * estacao) {
    if (estacao == NULL) {
        return;
    }

    pthread_mutex_lock(&estacao->mu_estacao);
    estacao->passageiros_aguardando++;

    while (!estacao->vagao || !estacao->passageiros_para_embarque) { // Acrescentar regra para quando não tiver mais passageiros para embarcar (mesmo se ainda tiver vagas) e esperar o vagao de fato chegar
        pthread_cond_wait(&estacao->vagao_chegou, &estacao->mu_estacao);
    }

    estacao->passageiros_para_embarque--; // essa funcao so cuida da quantidade de passageiros que ainda tem para embarque, a função estacao_embarque que vai cuidar da qtd de vagas e passageiros na estacao (depois do embarque)
    pthread_mutex_unlock(&estacao->mu_estacao);
}
