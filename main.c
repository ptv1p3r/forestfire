/************************************
 FUNDAMENTOS PROGRAMAÇÃO
 Pedro Roldan, a21501217
 Leandro Moreira, a21401956
 Trabalho : Forest Fire
************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define MAX_VIZINHANCA 9

enum { EMPTY = 0, TREE = 1, BURNING = 2, BURNOUT = 3 };
enum { MOORE = 0, VONNEUMANN = 1 };
const char *disp[] = {"  ", "\033[32m/\\\033[m", "\033[07;31m/\\\033[m", "\033[31m/\\\033[m"};
static volatile int continuaSimulacao = 1;

/* Prototipos */
void intHandler(int);
void printWorld(int **arrForest, int intLinhas, int intColunas,float Delay,int Geracoes,float t, float p, float f,int v);
int **InitFloresta(int **arrForest, int intLinhas, int intColunas);
int getVizinhanca(int **arrForest, int y, int x,int Modo);
float getProbabilidade(float Probabilidade);
int getRandomNumber();
void setSleep(unsigned int segundos);
void printHelp();
int ***getFileLoad (char *nomeFicheiro);
int ***InitArray (int ***array, int cells);
int **setForest (int ***array, int **matriz);
void freeArray(int ***array, int cells);


int main(int argc, char *argv[]){
    int intCount = 0,intHeight=0,intWidth=0,intCells=0,i,j,intGeracoes = 0,intVizinhancaCount = 0, intVizinhancaActiva = MOORE, intEstadosAlterados = 1;
    char *chrFilename;
    float fltProbTree = 0.00, fltProbTreeFlash= 0.00, fltProbTreeBurn = 0.00, fltProbVizinhancaBurn = 0.00, fltDelay = 0.00;
    int FileLoadRandom = 0,FileLoad = 0,FileLoadFilename = 0,Delay = 0,ProbTreeGrow = 0,ProbTreeFlash = 0, ProbTreeBurn = 0;
    int **Forest=NULL,**ForestCache=NULL, ***intInfo=NULL;

    srand((unsigned)time(NULL));

    if (argc>1) {

        /* valida argumentos */
        for (intCount = 1; intCount < argc; intCount++) {
            if ((strcmp(argv[intCount], "--help") == 0)){   /* Valida opção --help */
                printHelp();
                exit(EXIT_SUCCESS);
            }
            else if (strcmp(argv[intCount], "-i") == 0) {   /* Valida opção -i load file */
                FileLoad = 1;   /* carrega a flag de -i com true */

                if (intCount+1 < argc) {
                    chrFilename = argv[intCount+1];
                    FileLoadFilename = 1;   /* carrega flag de nome de ficheiro com true */
                }
                else {
                    printf("Uso: -i [filename]\n");
                    exit(EXIT_SUCCESS);
                }
            }
            else if (strcmp(argv[intCount], "-a") == 0) {   /* Valida opção -a load matriz aleatoria */
                FileLoadRandom = 1;   /* carrega a flag de -a com true */
            }
            else if (strcmp(argv[intCount], "-d") == 0) {   /* Valida opção -d delay */
                Delay = 1;   /* carrega a flag de -d com true */

                if (intCount+1 < argc) {
                    fltDelay = atof(argv[intCount+1]);
                }
                else {
                    printf("Uso: -d [0.00 seconds]\n");
                    exit(EXIT_SUCCESS);
                }
            }
            else if (strcmp(argv[intCount], "-t") == 0) {   /* Valida opção -d probabilidade de uma arvore nascer */
                ProbTreeGrow = 1;   /* carrega a flag de -t com true */

                if (intCount+1 < argc) {
                    fltProbTree=atof(argv[intCount+1]);
                }
                else {
                    printf("Uso: -t [0.00]\n");
                    exit(EXIT_SUCCESS);
                }
            }
            else if (strcmp(argv[intCount], "-f") == 0) {   /* Valida opção -f probabilidade de uma arvore ser atingida por um relâmpago */
                ProbTreeFlash = 1;   /* carrega a flag de -f com true */

                if (intCount+1 < argc) {
                    fltProbTreeFlash = atof(argv[intCount+1]);
                }
                else {
                    printf("Uso: -f [0.00]\n");
                    exit(EXIT_SUCCESS);
                }
            }
            else if (strcmp(argv[intCount], "-p") == 0) {   /* Valida opção -p probabilidade de uma arvore arvore arder caso seja atingida por um relâmpago */
                ProbTreeBurn = 1;   /* carrega a flag de -p com true */

                if (intCount+1 < argc) {
                    fltProbTreeBurn = atof(argv[intCount+1]);
                }
                else {
                    printf("Uso: -p [0.000]\n");
                    exit(EXIT_SUCCESS);
                }
            }
            else if (strcmp(argv[intCount], "-v") == 0) {   /* Valida opção -v tipo de vizinhança */

                if (intCount+1 < argc) {
                    intVizinhancaActiva = atoi(argv[intCount+1]);
                }
                else {
                    printf("Uso: -v [0 - Moore / 1 - Von Neumann]\n");
                    exit(EXIT_SUCCESS);
                }
            }
        }

        /* Valida conjunto de opcoes escolhidas */
        if (
            (!FileLoadRandom && !FileLoad) || /* valida opcoes -a ou -i */
            (FileLoad && !FileLoadFilename) || /* -i sem nome de ficheiro */
            (!Delay) || (!ProbTreeGrow) || (!ProbTreeFlash) || (!ProbTreeBurn)){ /* restantes flags -d -t -f -p */
            printHelp();
            exit(EXIT_SUCCESS);
        }



        if (FileLoad && FileLoadFilename) { /* flag -i file load */

            /*carrega array de configuracao retornado de ficheiro*/
            intInfo=getFileLoad(chrFilename);

            intHeight=intInfo[0][0][0]; /* valor matriz inicial valor linhas retorno de ficheiro */
            intWidth=intInfo[0][1][0];  /* valor matriz inicial valor colunas retorno de ficheiro */
            intCells=intInfo[1][0][0];  /* valor matriz inicial nr celulas retorno de ficheiro */

            /* inicializa a  ambas as matrizes floresta +2 para moldura */
            Forest=InitFloresta(Forest,intHeight+2,intWidth+2);
            ForestCache=InitFloresta(ForestCache,intHeight+2,intWidth+2);

            for(i=2 ; i<intCells+2 ; i++) { /*Passa a posição e as coordenadas da celula para a Floresta*/
                Forest[(intInfo[i][0][0])+1][(intInfo[i][1][0])+1]=intInfo[i][1][1];
            }

            for (i=1;i<=intHeight;i++){     /*Iguala a ForestCache a Forest*/
                for (j=1;j<=intWidth;j++){
                    ForestCache[i][j]=Forest[i][j];
                }
            }

        }
        else if (FileLoadRandom){ /* inicia floresta de forma aleatoria */

            intHeight=getRandomNumber();
            intWidth=getRandomNumber();

            /* inicializa a  ambas as matrizes floresta +2 para moldura*/
            Forest=InitFloresta(Forest,intHeight+2,intWidth+2);
            ForestCache=InitFloresta(ForestCache,intHeight+2,intWidth+2);

            /* povoa a floresta inicial */
            for (i=1;i<=intHeight;i++){
                for (j=1;j<=intWidth;j++){
                   ForestCache[i][j]=Forest[i][j]=getProbabilidade(fltProbTree)?TREE:EMPTY; /* povoa com uma arvore ou com vazio */
                }
            }
        }
        else {
            printHelp();
        }

        /* regista sinal e respectivo handler */
        signal(SIGINT,intHandler); /* captura o SIGINT(CTRL+C) */


        while (intEstadosAlterados>0 || continuaSimulacao){

            intEstadosAlterados=0;

            /* Display da simulacao */
            printWorld(Forest,intHeight,intWidth,fltDelay,intGeracoes,fltProbTree,fltProbTreeBurn,fltProbTreeFlash,intVizinhancaActiva);

            setSleep(fltDelay); /* aplica delay entre cada estado/iteracao */

            /* aplicação de regras à simulação */
            /* percorre toda a matrix celula a celula */
            for (i=1;i<=intHeight;i++){
                for (j=1;j<=intWidth;j++){

                /* valida cada celula */
                    switch(Forest[i][j]){
                        case EMPTY: /* celula vazia probabilidade de nascimento de nova arvore (t) */

                             /* probabilidade de nascer uma arvore */
                            ForestCache[i][j]=getProbabilidade(fltProbTree)?TREE:EMPTY; /* povoa com uma arvore ou com vazio */

                            /* contador de estados */
                            if (ForestCache[i][j]==TREE){
                                intEstadosAlterados++;
                            }
                            break;

                        case BURNING: /* arvore em fogo */
                            ForestCache[i][j]=BURNOUT; /* coloca arvore em burnout */
                            intEstadosAlterados++; /* contador de estados */
                            break;

                        case BURNOUT: /* arvore em burnout */
                            ForestCache[i][j]=EMPTY; /* arvore ja ardeu totalmente (2 iteracoes) */
                            intEstadosAlterados++; /* contador de estados */
                            break;

                        case TREE: /* arvore */
                            intVizinhancaCount=getVizinhanca(Forest,i,j,intVizinhancaActiva); /* verifica se existem arvores a arder na vizinhança */

                            /* probabilidade arvore ser atingida por relampago (f) */
                            if(ForestCache[i][j]==TREE && getProbabilidade(fltProbTreeFlash)){

                                /* probabilidade de a arvore pegar fogo (p) */
                                ForestCache[i][j]=getProbabilidade(fltProbTreeBurn)?BURNING:TREE;

                                /* contador de estados */
                                if (ForestCache[i][j]==BURNING){
                                    intEstadosAlterados++;
                                }

                            }
                            else if(intVizinhancaCount>0){ /* vizinhanca com x nr de arvores com fogos */

                                fltProbVizinhancaBurn=(float)intVizinhancaCount/(intVizinhancaActiva==0?8:4); /* probabilidade x arvores de vizinhanca a arder */
                                ForestCache[i][j]=getProbabilidade(fltProbVizinhancaBurn)?BURNING:TREE; /* probabilidade de a arvore pegar fogo */

                                /* contador de estados */
                                if (ForestCache[i][j]==BURNING){
                                    intEstadosAlterados++;
                                }
                            }
                            intVizinhancaCount=0;
                            break;
                    }
                }
            }

            /* actualiza forest com forestcache com as alteracoes */
            for (i=1;i<=intHeight;i++){
                for (j=1;j<=intWidth;j++){
                    Forest[i][j]=ForestCache[i][j];
                }
            }

            intGeracoes++; /* incrementa geracao */

            /* valida estados alterados apos a iteracao */
            if (intEstadosAlterados==0){ /* nao houve alteracao de estados */
                continuaSimulacao=0; /* termina simulacao */
            }
        }

        /* termina simulacao */
        if (continuaSimulacao==0){
            printf("Simulação ForestFire terminou, obrigado!\n");
            exit(EXIT_SUCCESS);
        }
    }

    /* eftua as limpezas finais */
    free(Forest);
    free(ForestCache);

    if (FileLoad && FileLoadFilename) {
        freeArray(intInfo, intCells);
    }

    return 0;
}

/*
 DESCRIÇÃO: Procedimento a ser chamado quando o sinal SIGINT(CTRL+C) é capturado
*/
void intHandler(int sig){
    continuaSimulacao=0; /* termina simulacao */
    printf("Simulação ForestFire terminou, obrigado!\n");
    exit(sig);
}

/*
 DESCRIÇÃO: Efetua o display da lista de comandos disponiveis
*/
void printHelp() {
    printf("Uso: forestfire -i filename [-a [-d 0.00 sec][-t 0.000][-f 0.000][-p 0.000] [-v 0]]\n\n");
    printf("Opção           Descrição\n");
    printf(" --help         Lista completa de comandos\n");
    printf(" -i filename    Carrega ficheiro de matriz inicial\n");
    printf(" -a             Matriz inicial aleatoria\n");
    printf(" -d             Delay entre estados (segundos)\n");
    printf(" -t             Probabilidade de uma arvore nascer\n");
    printf(" -f             Probabilidade de uma arvore ser atingida por um relâmpago\n");
    printf(" -p             Probabilidade de uma arvore arder caso seja atingida por um relâmpago\n");
    printf(" -v             Tipo de vizinhança (0-Moore/1-Von Neumann)\n");
}

/*
 DESCRIÇÃO: Gera um valor decimal aleatorio entre 0 e 1
*/
float getProbabilidade(float Probabilidade){
   return (float)rand() < (float)(RAND_MAX * Probabilidade);
}

/*
 DESCRIÇÃO: Efetua um delay baseado num valor em segundos.
*/
void setSleep(unsigned int segundos){
    int intFinal;

    intFinal=time(0)+segundos;
    while (time(0)<intFinal);
}

/*
 DESCRIÇÃO: Devolve um numero inteiro aleatorio entre (5-50)
*/
int getRandomNumber(){
    return (rand() % 50) + 5;
}

/*
 DESCRIÇÃO: Inicializa a 0 uma matriz de mxn.
*/
int **InitFloresta(int **arrForest, int intLinhas, int intColunas) {
    int i;

    arrForest = (int **)calloc(intLinhas,sizeof(int *));
    if (!arrForest){
        perror("Erro na alocação de memória!\n");
        exit(EXIT_FAILURE);
    }
    for (i=0;i<intLinhas;i++){
        (arrForest)[i] = (int *)calloc(intColunas,sizeof(int));
        if (!(arrForest)[i]){
            for (i=0;i<intLinhas;i++){
                free((arrForest)[i]);
            }
            free(arrForest);
            perror("Erro na alocação de memória!\n");
            exit(EXIT_FAILURE);
        }
    }

    return arrForest;
}

/*
 DESCRIÇÃO: Efetua o display da simulação.
*/
void printWorld(int **arrForest, int intLinhas, int intColunas,float Delay,int Geracoes,float t, float p, float f,int v){
    int i,j;
    system("clear"); /* limpa consola */

    printf("\033[H"); /* cursor top-left */
    printf("\033[7mIterações:%d  CTRL+C para terminar                                               \033[0m\n",Geracoes);
    printf("\033[7mDelay: %.3f sec t:%.3f p:%.3f f:%.3f v:%s                    Matriz:%dx%d \033[0m\n",Delay,t,p,f,v?"Von Neumann":"Moore",intLinhas,intColunas);
    printf("\033[7mISMAT FP 2015 Forest-Fire Model Pedro Roldan-Leandro Moreira                     \033[0m\n");

    for (i=1;i<=intLinhas;i++){
        for (j=1;j<=intColunas;j++){
            printf("%s",disp[arrForest[i][j]]);
        }
        printf("\033[E"); /* next line */
    }
    fflush(stdout);
}

/*
 DESCRIÇÃO: Devolve um numero inteiro baseado na vizinhanca (Moore/Von Neumann) da coordenada fornecida.
*/
int getVizinhanca(int **arrForest, int x, int y, int Modo) {
    int i,j,count = 0;

    if (Modo) {
        /* vizinhanca de von neumann */

        if ((arrForest[x][y]==BURNING) || (arrForest[x][y]==BURNOUT)){           /* P   */
            count++;
        }
        else if ((arrForest[x][y-1]==BURNING) || (arrForest[x][y-1]==BURNOUT)){  /* P1 N*/
            count++;
        }
        else if ((arrForest[x+1][y]==BURNING) || (arrForest[x+1][y]==BURNOUT)) { /* P2 E*/
            count++;
        }
        else if ((arrForest[x][y+1]==BURNING) || (arrForest[x][y+1]==BURNOUT)) { /* P3 S*/
            count++;
        }
        else if ((arrForest[x-1][y]==BURNING) || (arrForest[x-1][y]==BURNOUT)){  /* P4 O*/
            count++;
        }

    } else {

       /* vizinhanca de moore */
       /* [x,y] P  */
       /* [x-1, y-1] P1 NO*/
       /* [x-1, y] P2 N*/
       /* [x-1, y+1] P3 NE*/
       /* [x, y+1] P4 E*/
       /* [x+1, y+1] P5 SE*/
       /* [x+1, y] P6 S*/
       /* [x+1, y-1] P7 SO*/
       /* [x, y-1] P8 O*/

        for (i=x-1;i<=x+1;i++){
            for (j=y-1;j<=y+1;j++){
                if ((arrForest[i][j]==BURNING) || (arrForest[i][j]==BURNOUT)){
                    count++;
                }
            }
        }
    }

   return count;
}

/*
 DESCRIÇÃO: Efetua a leitura do ficheiro
*/
int ***getFileLoad (char *nomeFicheiro) {

    char strLines[5], strColumns[5], strCelulas[5], strTemp[4];
    int i=0, k=0, lines=0, columns=0, celulas=0, intTemp=0;
    int ***array=NULL;
    FILE *ficheiro=NULL;

    ficheiro=fopen(nomeFicheiro, "r");   /*abrir ficheiro*/

    if (ficheiro==NULL) {
        printf("Erro ao abrir o ficheiro!\n");
        exit(EXIT_FAILURE);
    }

    fscanf(ficheiro, "%s %s %s", strLines, strColumns, strCelulas); /*Lê os primeiros 3 valores do ficheiro, linhas, colunas e celulas a ler*/

    if ((lines=atoi(strLines))<=5) {    /*valida linhas*/
        printf("Matriz [Nrº de Linhas invalido]!\n");
        exit(EXIT_FAILURE);
    }

    if ((columns=atoi(strColumns))<=5) {      /*valida colunas*/
        printf("Matriz [Nrº de Colunas invalido]!\n");
        exit(EXIT_FAILURE);
    }

    if ((celulas=atoi(strCelulas))<=1) {      /*valida nr celulas*/
        printf("Matriz [Nrº de Celulas invalido]!\n");
        exit(EXIT_FAILURE);
    }

    array=InitArray(array, celulas); /*Alocação de memória para um array multidimensional*/

    array[0][0][0]=lines;
    array[0][1][0]=columns;
    array[1][0][0]=celulas;


    for (i=2 ; i<celulas+2 ; i++) {     /*Número linhas a ler*/
        for (k=0 ; k<4 ; k++) {
            if (k==3) {   /*verifica se a linha foi lida com o formato correto*/
                intTemp=fgetc(ficheiro);
                if (intTemp=='\n' || intTemp==EOF) {
                        continue;
                } else {
                    printf("Erro na leitura do ficheiro! (Formato célula inválido)\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                fscanf(ficheiro, "%s", strTemp);
                intTemp=atoi(strTemp);

                if (intTemp>=0) {
                    if (k==0) {             /*aceitar linhas entre 0->lines*/
                        if (!(intTemp>=0 && intTemp<lines)) {
                            printf("Valores invalidos (linhas)\n");
                            exit(EXIT_FAILURE);
                        } else {
                        array[i][k][0]=intTemp;
                        }
                    } else if (k==1) {      /*aceitar colunas entre 0->columns*/
                        if (!(intTemp>=0 && intTemp<columns)) {
                            printf("Valores invalidos (colunas)\n");
                            exit(EXIT_FAILURE);
                        } else {
                        array[i][k][0]=intTemp;
                        }
                    } else if (k==2) {      /*aceitar celulas com valores entre 0->2*/
                        if (!(intTemp>=0 && intTemp<3)) {
                            printf("Valores invalidos (estado da celula)\n");
                            exit(EXIT_FAILURE);
                        } else {
                            array[i][k-1][1]=intTemp;
                        }
                    }
                }
            }
        }
    }


    intTemp=fgetc(ficheiro);   /*verificar EOF*/

    if (intTemp!=EOF) {     /*verifica se o ficheiro foi completamente lido*/
        printf("Impossivel ler todos os valores!\n");
        exit(EXIT_FAILURE);
    }

    fclose(ficheiro);

    return array;
}

/*
 DESCRIÇÃO: Inicializa um array multidimensional para guardar os dados lidos do ficheiro
*/
int ***InitArray (int ***array, int cells) {

    int b=2, i=0, j=0, k=0 , l=0;

    array=(int ***)calloc((cells+b), sizeof(int**)); /*alocar nr de linhas*/

    if (array==NULL) { /*liberta a memória caso as linhas nao sejam alocadas*/
        printf("Memória insuficiente!\n");
        exit(EXIT_FAILURE);
    }

    for(i=0 ; i<cells+b ; i++) { /*aloca nr de colunas*/
        array[i]=(int **)calloc(b, sizeof(int*));

        if (!array[i]) { /*liberta a memória caso as colunas nao sejam alocadas*/
            printf("Memória insuficiente!\n");
            for (j=0 ; j<i ; j++) {
                free((array)[j]);
            }
            free(array);
            array=NULL;
            exit(EXIT_FAILURE);
        }


        for (j=0 ; j<b ; j++) {
            array[i][j]=(int*)calloc(b,sizeof(int)); /*aloca nr de colunas*/

            if (!array[i][j]) { /*liberta a memória caso as colunas nao sejam alocadas*/
            printf("Memória insuficiente!\n");
            for (k=0 ; k<i ; k++) {
                for (l=0 ; l<j ; l++) {
                    free(array[k][l]);
                }
            }
            free(array);
            array=NULL;
            exit(EXIT_FAILURE);
            }
        }
    }

    return array;
}

/*
 DESCRIÇÃO: Liberta a memória alocada do array multidimensional
*/
void freeArray(int ***array, int cells) {

    int i, j, b=2;

    for (i=0 ; i<cells+b ; i++) { /*liberta a memória das colunas*/
        for (j=0 ; j<b ; j++) {
            free((array)[i][j]);
        }
    }

    for (i=0 ; i<cells+b ; i++) { /*liberta a memória das colunas*/
        free((array)[i]);
    }

    free(array); /*liberta a memória das linhas*/
    array=NULL;

}
