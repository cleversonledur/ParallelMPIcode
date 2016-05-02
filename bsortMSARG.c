#include<stdio.h>
#include"mpi.h"
#include<stdlib.h>
#include<math.h>
#define TAGKILL 12

int vector_col, vector_row, intervalo=0;
int rank,numprocess;
void imprime(int *numbers);
int slave();
int master();
void bubble_sort(int *list, int n);
int main(int argc, char* argv []){

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    vector_row = atoi(argv[1]);
    vector_col = atoi(argv[2]);
    intervalo = atoi(argv[3]);

    if(rank==0) //master
        master();
    else
        slave();

    MPI_Finalize();
    return 0;
}

int master(){
    double t1, t2;
    t1 = MPI_Wtime();
    MPI_Comm_size( MPI_COMM_WORLD, &numprocess );

    MPI_Status s;

    int *numbers;
    numbers = (int*)malloc(vector_row * vector_col * sizeof(int));

    int completo = 0;

    int *controle;
    controle = (int*)malloc(vector_row * sizeof(int));

    int i,j,k;

    //INICIALIZA A MATRIZ COM VALORES ALEATÓRIOS
    for (i=0;i<vector_row;i++){
        for(j=0;j<vector_col;j++){
            int a;
            a+=(1+((int)(i*j+a)));
            void srand ( unsigned int seed );
            srand(a);
            numbers[i*vector_col+j] = rand()%intervalo;
        }
            controle[i]=-2; //não processado
    }

    //imprime(numbers);

    while(completo<vector_row){
            char comando;
            MPI_Recv(&comando,1,MPI_CHAR,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
            //MPI_Wait(&request, &s);
            int source = s.MPI_SOURCE;
            //printf("Rank %d Master recebeu comando %c de Source: %d Completo: %d\n",rank,comando,source,completo);
            //int x = 0;  //Aceito
            //MPI_Send(&x, 1, MPI_INT, source, 6 , MPI_COMM_WORLD);

            if(comando=='A'){
                //ENVIAR UMA LINHA DA MATRIZ
                //CUIDAR PARA NÃO ENVIAR A MESMA LINHA 2 VEZES OU MAIS
                for(i=0;i<vector_row;i++){
                    if(controle[i]==-2){
                        MPI_Send(&numbers[i*vector_col], vector_col, MPI_INT, source, 0 , MPI_COMM_WORLD);
                        //printf("Enviado do Root a linha %d para o rank %d\n",i,source);
                        controle[i] = source;
                        break;
                    }
                }
            }else if(comando=='S'){
                //RECEBER UMA LINHA DA MATRIZ
                for(i=0;i<vector_row;i++){
                    if(controle[i]==source){
                        MPI_Recv(&numbers[i*vector_col], vector_col, MPI_INT, source, MPI_ANY_TAG ,MPI_COMM_WORLD,&s);
                        //printf("Rank 0 recebeu a linha %d do rank %d\n", i,source);
                        controle[i] = -3; //-3 significa completo
                        completo++;
                        break;
                    }
                }
            }

        }
       // imprime(numbers);

        //Mata processos
        int mortos=0;
        char comando;
        for(i=1;i<numprocess;i++){
            MPI_Request req;
            MPI_Irecv(&comando,1,MPI_CHAR,i,MPI_ANY_TAG,MPI_COMM_WORLD,&req);
            MPI_Isend(0, 0, MPI_INT, i, TAGKILL, MPI_COMM_WORLD, &req);
            mortos++;
        }
        t2 = MPI_Wtime();
        printf("\n%d\t%d\t%d\t%1.5f", vector_row,vector_col,intervalo,t2-t1);
        fflush(stdout);
        /*int processosMortos = 0;
        while(processosMortos<numprocess-1){
            char comando;
            MPI_Recv(&comando,1,MPI_CHAR,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
            //MPI_Wait(&request, &s);
            int source = s.MPI_SOURCE;
            MPI_Send(0, 0, MPI_INT, source, TAGKILL, MPI_COMM_WORLD);
            processosMortos++;
            printf("Matando processo %d e Processos Mortos = %d Numprocess: %d\n ",source, processosMortos,numprocess);
        }
        */

}

int slave(){
    char comando;
    int *numbers;
    numbers = (int*)malloc(vector_col * sizeof(int));

    int primos_aux_size = sqrt(intervalo);
    int primos_aux[primos_aux_size];
    int i,j,k,a;

    MPI_Status s;

    int morrer = 0;
    while(morrer!=TAGKILL){
            //printf("Sou o Rank %d\n",rank);
            comando = 'A';

            MPI_Bsend(&comando, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);

            MPI_Recv(&numbers[0], vector_col, MPI_INT, 0, MPI_ANY_TAG ,MPI_COMM_WORLD,&s);

            morrer = s.MPI_TAG;

            if(morrer==TAGKILL){
                break;//return 0;
            }else{

                    bubble_sort(numbers,vector_col);

                    comando = 'S';

                    MPI_Send(&comando, 1, MPI_CHAR, 0, 0 , MPI_COMM_WORLD);

                    MPI_Send(&numbers[0], vector_col, MPI_INT, 0, 0 , MPI_COMM_WORLD);
                }
            }

}

void bubble_sort(int *list, int n)
{
  long c, d, t;

  for (c = 0 ; c < ( n - 1 ); c++)
  {
    for (d = 0 ; d < n - c - 1; d++)
    {
      if (list[d] > list[d+1])
      {
        t         = list[d];
        list[d]   = list[d+1];
        list[d+1] = t;
      }
    }
  }
}

void imprime(int *numbers){
    int i,j;
    for(i=0;i<vector_row;i++){
        for(j=0;j<vector_col;j++)
            printf("[%d]\t", numbers[i*vector_col+j]);
            printf("\n");
        }
}
