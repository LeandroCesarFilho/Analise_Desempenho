// gcc simulacao.c -lm -o simulacao

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

typedef struct {
    unsigned long int num_eventos;
    double tempo_anterior;
    double soma_areas;
} little;


double uniforme(){
    double u = rand() / ((double) RAND_MAX + 1);
    //u == 0 --> ln(u) <-- problema
    //limitando u entre (0,1]
    u = 1.0 - u;
    return u;
}

double gera_tempo(double l){
    return (-1.0/l) * log(uniforme());
}

double min(double n1, double n2, double n3){
    if (n1 < n2 && n1 < n3){
        return n1;
        }else if(n2 < n3){
            return n2;
        }else{
            return n3;
        }   
}

void inicia_little(little *n){
    n->num_eventos = 0;
    n->soma_areas = 0.0;
    n->tempo_anterior = 0.0;
}


void simular_fila(double parametro_chegada, double parametro_saida, double tempo_simulacao, char *nome_arquivo) {
    srand(5);

    FILE *file = fopen(nome_arquivo, "w");

    parametro_chegada = 1.0 / parametro_chegada;
    parametro_saida = 1.0 / parametro_saida;

    double tempo_decorrido = 0.0;
    double tempo_chegada = gera_tempo(parametro_chegada);
    double tempo_saida = DBL_MAX;
    double tempo_medicao = 100.0;

    unsigned long int fila = 0;
    unsigned long int fila_max = 0;

    double soma_ocupacao = 0.0;
    double lambda = 0.0;
    double excedente = 0.0;

    // Variáveis para a medição
    double en_atual = 0.0;
    double ew_atual = 0.0;

    little en, ew_chegadas, ew_saidas;
    inicia_little(&en);
    inicia_little(&ew_chegadas);
    inicia_little(&ew_saidas);

    double erroLittle = 0.0;

    while (tempo_decorrido <= tempo_simulacao) {
        tempo_decorrido = fmin(fmin(tempo_chegada, tempo_saida), tempo_medicao);

        // Chegada
        if (tempo_decorrido == tempo_chegada) {
            if (!fila) {
                tempo_saida = tempo_decorrido + gera_tempo(parametro_saida);
                soma_ocupacao += tempo_saida - tempo_decorrido;
                if (tempo_saida > tempo_medicao) {
                    excedente = tempo_saida - tempo_medicao;
                }
            }
            fila++;
            fila_max = fmax(fila, fila_max);
            tempo_chegada = tempo_decorrido + gera_tempo(parametro_chegada);

            // Little
            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.num_eventos++;
            en.tempo_anterior = tempo_decorrido;

            ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_chegadas.num_eventos++;
            ew_chegadas.tempo_anterior = tempo_decorrido;

        } else if (tempo_decorrido == tempo_saida) {
            fila--;
            tempo_saida = DBL_MAX;
            if (fila) {
                tempo_saida = tempo_decorrido + gera_tempo(parametro_saida);
                soma_ocupacao += tempo_saida - tempo_decorrido;
                if (tempo_saida > tempo_medicao) {
                    excedente = tempo_saida - tempo_medicao;
                }
            }

            // Little
            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.num_eventos--;
            en.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
            ew_saidas.num_eventos++;
            ew_saidas.tempo_anterior = tempo_decorrido;

        } else {
            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
            ew_saidas.tempo_anterior = tempo_decorrido;

            ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_chegadas.tempo_anterior = tempo_decorrido;

            en_atual = en.soma_areas / tempo_decorrido;
            ew_atual = (ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos;

            lambda = ew_chegadas.num_eventos / tempo_decorrido;
            erroLittle = en_atual - lambda * ew_atual;

            // Ocupação
            fprintf(file,"%lf ", (soma_ocupacao - excedente) / tempo_decorrido);
            // E[N]
            fprintf(file,"%lf ", en_atual);
            // E[W]
            fprintf(file,"%lf ", ew_atual);
            // Lambda
            fprintf(file, "%lf ", lambda);
            // Erro de Little
            fprintf(file, "%lf\n", erroLittle);

            tempo_medicao += 100;
        }
    }
    fclose(file);

    ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
    ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;

    printf("Maior tamanho de fila alcançado: %ld\n", fila_max);
    printf("Ocupação: %lf\n", (soma_ocupacao - excedente) / tempo_decorrido);

    double en_final = en.soma_areas / tempo_decorrido;
    double ew_final = (ew_chegadas.soma_areas - ew_saidas.soma_areas) / ew_chegadas.num_eventos;

    lambda = ew_chegadas.num_eventos / tempo_decorrido;

    printf("E[N]: %lf\n", en_final);
    printf("E[W]: %lf\n", ew_final);
    printf("Erro de little: %lf\n", en_final - lambda * ew_final);
    printf("Tempo decorrido: %lf\n", tempo_decorrido);
}

int main () {
    // Semente fixada de 5
    // Ocupação de 85%
    simular_fila(0.5,0.425,100000,"arquivo85");
    // Ocupação de 90%
    simular_fila(0.5,0.45,100000,"arquivo90");
    // Ocupação de 95%
    simular_fila(0.5,0.475,100000,"arquivo95");
    // Ocupação de 99%
    simular_fila(0.5,0.495,100000,"arquivo99");
    return 0;
}