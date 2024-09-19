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


int main(){
    srand(time(NULL));
    double parametro_chegada;
    printf("Informe o tempo médio entre as chegadas (s): ");
    scanf("%lF", &parametro_chegada);
    parametro_chegada = 1.0/parametro_chegada;

    double parametro_saida;
    printf("Informe o tempo médio de atendimento (s): ");
    scanf("%lF", &parametro_saida);
    parametro_saida = 1.0/parametro_saida;

    double tempo_simulacao;
    printf("Informe o tempo de simulacao (s): ");
    scanf("%lF", &tempo_simulacao);

    double tempo_decorrido = 0.0;

    double tempo_chegada = gera_tempo(parametro_chegada);
    double tempo_saida = DBL_MAX;
    double tempo_medicao = 100.0;

    unsigned long int fila = 0;
    unsigned long int fila_max = 0;

    double soma_ocupacao = 0.0;

    double lambda = 0.0;

    double excedente = 0.0;

    // Variáveis para a medicao
    double en_atual = 0.0;
    double ew_atual = 0.0;

    /**
     * variaveis little
     */

    little en;
    little ew_chegadas;
    little ew_saidas;

    inicia_little(&en);
    inicia_little(&ew_chegadas);
    inicia_little(&ew_saidas);
    


    /**
        Variaveis little medicao
     */

     double ew_chegadasMedicao = 0.0;
     double ew_saidasMedicao = 0.0;

     double erroLittle = 0.0;

     
    
    

    while(tempo_decorrido <= tempo_simulacao){
        tempo_decorrido = 
            min(tempo_chegada, tempo_saida, tempo_medicao);

        
        /*
        if (tempo_decorrido >= tempo_medicao){

            tempo_medicao += 100.0;
        }
        */


        //chegada
        if(tempo_decorrido == tempo_chegada){
            //sistema esta ocioso?
            if(!fila){
                tempo_saida =
                tempo_decorrido +
                gera_tempo(parametro_saida);
                   
                soma_ocupacao += tempo_saida - tempo_decorrido;
                
                if(tempo_saida > tempo_medicao){
                    excedente = tempo_saida - tempo_medicao;
                }
                
            }
            fila++;
            fila_max = fila > fila_max?
                fila:
                fila_max;

            tempo_chegada =
            tempo_decorrido +
            gera_tempo(parametro_chegada);


            /**
             * Little
             */
            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.num_eventos++;
            en.tempo_anterior = tempo_decorrido;

            ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_chegadas.num_eventos++;
            ew_chegadas.tempo_anterior = tempo_decorrido;

        }else if(tempo_decorrido == tempo_saida){
            fila--;
            tempo_saida = DBL_MAX;
            //tem mais requisicoes na fila?
            if(fila){
                tempo_saida =
                tempo_decorrido +
                gera_tempo(parametro_saida);

                soma_ocupacao += tempo_saida - tempo_decorrido;
                
                if(tempo_saida > tempo_medicao){
                    excedente = tempo_saida - tempo_medicao;
                }

            }
            /**
             * Little
             */
            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.num_eventos--;
            en.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
            ew_saidas.num_eventos++;
            ew_saidas.tempo_anterior = tempo_decorrido;

            
        }else{
            /*
            ew_chegadasMedicao = ew_chegadas.soma_areas + (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_saidasMedicao = ew_saidas.soma_areas + (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;

            en_atual = en.soma_areas/tempo_decorrido;
            ew_atual = (ew_chegadasMedicao - ew_saidasMedicao)/ew_chegadas.num_eventos;

            */

            en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
            en.tempo_anterior = tempo_decorrido;

            ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;
            ew_saidas.tempo_anterior = tempo_decorrido;

            ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
            ew_chegadas.tempo_anterior = tempo_decorrido;

            en_atual = en.soma_areas/tempo_decorrido;
            ew_atual = (ew_chegadas.soma_areas - ew_saidas.soma_areas)/ew_chegadas.num_eventos;


            lambda = ew_chegadas.num_eventos/tempo_decorrido;

            erroLittle = en_atual - lambda * ew_atual;


            printf("\nTempo: %lf\n", tempo_decorrido);
            printf("Ocupação: %lf\n", (soma_ocupacao - excedente)/tempo_decorrido);
            printf("E[N] = %lF\n", en_atual);
            printf("E[W] = %lF\n", ew_atual);
            printf("Lambda = %lF\n", lambda);
            printf("Erro de Little = %lF\n", erroLittle);

            tempo_medicao += 100;
        

        }
    }

    // en.soma_areas += (tempo_decorrido - en.tempo_anterior) * en.num_eventos;
    ew_chegadas.soma_areas += (tempo_decorrido - ew_chegadas.tempo_anterior) * ew_chegadas.num_eventos;
    ew_saidas.soma_areas += (tempo_decorrido - ew_saidas.tempo_anterior) * ew_saidas.num_eventos;

    printf("Maior tamanho de fila alcancado: %ld\n",fila_max);
    printf("Ocupacao: %lF\n",(soma_ocupacao - excedente)/tempo_decorrido);

    double en_final = en.soma_areas/tempo_decorrido;
    double ew_final = (ew_chegadas.soma_areas - ew_saidas.soma_areas)/ew_chegadas.num_eventos;


    lambda = ew_chegadas.num_eventos/tempo_decorrido;

    printf("E[N]: %lF\n",en_final);
    printf("E[W]: %lF\n",ew_final);
    printf("Erro de little: %lF\n",en_final - lambda * ew_final);
    printf("Tempo decorrido: %lF\n", tempo_decorrido);

    return 0;
}