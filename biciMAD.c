#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


// DEFINICION DE TIPOS DE DATOS
typedef struct {    // Estructura para recoger los datos del archivo de entrada
    int numUsers;
    int numStations;
    int numSlotsPerStation;
    int minChooseTime;
    int maxChooseTime;
    int minRideTime;
    int maxRideTime;
} BasicData;

typedef struct {    // Tipo de dato de usuario
    int id;
    int actualStation;
} User;

typedef struct {    // Tipo de dato de estación
    int id;
    int availableBikes;
    pthread_mutex_t station_mutex;
    pthread_cond_t not_empty_cond, not_full_cond;
} Station;


// DECLARACION DE VARIABLES
BasicData data;
char *inputFileName;
char outputFileName[50];
FILE *inputFile;
FILE *outputFile;
int i, j; // Contadores
User *users;
Station *stations;
pthread_t *userThreads;


// FUNCIONES AUXILIARES
void getTimeDate(char *strTime, size_t tamano) {
    time_t actualTime;
    struct tm *infoTime;

    time(&actualTime);
    infoTime = localtime(&actualTime);

    strftime(strTime, tamano, "%H%M%d%m%Y", infoTime);
}

void printIntro(BasicData * printData){
    fprintf(outputFile,"BiciMAD: CONFIGURACION INICIAL\n");
    fprintf(outputFile,"Usuarios: %d\n", printData->numUsers);
    fprintf(outputFile,"Numero de Estaciones: %d\n", printData->numStations);
    fprintf(outputFile,"Numero de huecos por estacion: %d\n", printData->numSlotsPerStation);
    fprintf(outputFile,"Tiempo minimo de espera para decidir coger una bici: %d\n", printData->minChooseTime);
    fprintf(outputFile,"Tiempo maximo de espera para decidir coger una bici: %d\n", printData->maxChooseTime);
    fprintf(outputFile,"Tiempo minimo que pasa un usuario montando una bici: %d\n", printData->minRideTime);
    fprintf(outputFile,"Tiempo maximo que pasa un usuario montando una bici: %d\n\n", printData->maxRideTime);
    fprintf(outputFile,"SIMULACION FUNCIONAMIENTO BiciMAD\n\n");
    fprintf(stdout,"BiciMAD: CONFIGURACION INICIAL\n");
    fprintf(stdout,"Usuarios: %d\n", printData->numUsers);
    fprintf(stdout,"Numero de Estaciones: %d\n", printData->numStations);
    fprintf(stdout,"Numero de huecos por estacion: %d\n", printData->numSlotsPerStation);
    fprintf(stdout,"Tiempo minimo de espera para decidir coger una bici: %d\n", printData->minChooseTime);
    fprintf(stdout,"Tiempo maximo de espera para decidir coger una bici: %d\n", printData->maxChooseTime);
    fprintf(stdout,"Tiempo minimo que pasa un usuario montando una bici: %d\n", printData->minRideTime);
    fprintf(stdout,"Tiempo maximo que pasa un usuario montando una bici: %d\n\n", printData->maxRideTime);
    fprintf(stdout,"SIMULACION FUNCIONAMIENTO BiciMAD\n\n");
}

void printStationArray(Station *st, int stationId){
    char *sequency = (char*)malloc(sizeof(char) * data.numSlotsPerStation * 2 + 1);
    int index = 0;

    for (i = 0; i < st->availableBikes; ++i) {
        sequency[index++] = '1';
        sequency[index++] = ' ';
    }

    for (i = st->availableBikes; i < data.numSlotsPerStation; ++i) {
        sequency[index++] = '0';
        sequency[index++] = ' ';
    }

    sequency[index] = '\0';
    printf("Estacion %d: %s\n", stationId, sequency);
    fprintf(outputFile, "Estacion %d: %s\n", stationId, sequency);
    free(sequency);
}

void takeOutBike(Station *st, int userId, int stationId) {
    printf("Usuario %d quiere coger bici en Estacion %d\n", userId, stationId);
    fprintf(outputFile,"Usuario %d quiere coger bici en Estacion %d\n", userId, stationId);
    sleep(rand() % (data.maxChooseTime - data.minChooseTime) + data.minChooseTime);

    pthread_mutex_lock(&(st->station_mutex));   // Bloqueo mutex estacion
    while (st->availableBikes == 0)
        pthread_cond_wait(&(st->not_empty_cond), &(st->station_mutex));

    users[userId-1].actualStation = stationId;
    st->availableBikes--;
    pthread_cond_signal(&(st->not_full_cond));

    printf("Usuario %d coge bici de Estacion %d\n", userId, stationId);
    fprintf(outputFile,"Usuario %d coge bici de Estacion %d\n", userId, stationId);

    printStationArray(st, stationId);
    pthread_mutex_unlock(&(st->station_mutex)); // Desbloqueo mutex estacion
}

void putBike(Station *st, int userId, int stationId) {
    printf("Usuario %d montando en bici ...\n", userId);
    fprintf(outputFile, "Usuario %d montando en bici ...\n", userId);
    sleep(rand() % (data.maxRideTime - data.minRideTime) + data.minRideTime);
    
    pthread_mutex_lock(&(st->station_mutex));   // Bloqueo mutex estacion
    printf("Usuario %d quiere dejar bici de Estacion %d\n", userId, stationId);
    fprintf(outputFile,"Usuario %d quiere dejar bici de Estacion %d\n", userId, stationId);

    while (st->availableBikes == data.numSlotsPerStation)
        pthread_cond_wait(&(st->not_full_cond), &(st->station_mutex));

    users[userId-1].actualStation = -1;
    st->availableBikes++;
    pthread_cond_signal(&(st->not_empty_cond));

    printf("Usuario %d deja bici en Estacion %d\n", userId, stationId);
    fprintf(outputFile,"Usuario %d deja bici en Estacion %d\n", userId, stationId);

    printStationArray(st, stationId);
    pthread_mutex_unlock(&(st->station_mutex)); // Desbloqueo mutex estacion
}

void * userThreadFunction(void * args){
    User * userInfo = (User *) args;
    int randomStation;
    srand((unsigned int)time(NULL) + userInfo->id); // Semilla random
    
    while (1){
        if (userInfo->actualStation == -1){
            sleep(rand() % data.numUsers + 1);
            randomStation = rand() % data.numStations + 1;
            takeOutBike(&stations[randomStation-1], userInfo->id, randomStation);
        }else {
            randomStation = rand() % data.numStations + 1;
            putBike(&stations[randomStation-1], userInfo->id, randomStation);
        }
    }
    pthread_exit(0);
}


// MAIN
int main(int argc, char *argv[]) {

    if (argc == 1) {
        // Caso: Sin argumentos
        inputFileName = "entrada_BiciMAD.txt";
        strcpy(outputFileName, "salida_sim_BiciMAD");
        getTimeDate(outputFileName + strlen(outputFileName), sizeof(outputFileName) - strlen(outputFileName));
        strcat(outputFileName, ".txt");
    } else if (argc == 2) {
        // Caso: Un argumento (nombre_fichero_entrada)
        inputFileName = argv[1];
        strcpy(outputFileName, "salida_sim_BiciMAD");
        getTimeDate(outputFileName + strlen(outputFileName), sizeof(outputFileName) - strlen(outputFileName));
        strcat(outputFileName, ".txt");
    } else if (argc == 3) {
        // Caso: Dos argumentos (nombre_fichero_entrada y nombre_fichero_salida)
        inputFileName = argv[1];
        getTimeDate(outputFileName, sizeof(outputFileName));
        strcat(outputFileName, argv[2]);
    } else {
        printf("Demasiados argumentos. Uso: %s [nombre_fichero_entrada] [nombre_fichero_salida]\n", argv[0]);
        exit(1);
    }
    
    // Control de existencia del fichero de entrada y su extension ".txt"
    if (access(inputFileName, F_OK) != 0){
        printf("Error al leer el fichero: El fichero no exite.\n");
        exit(2);
    }else if (strstr(inputFileName,".txt") == NULL){
        printf("Error al leer el fichero: El fichero no tiene extension \".txt\".\n");
        exit(3);
    }

    // APERTURA FICHERO DE ENTRADA
    inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo %s\n", inputFileName);
        exit(4);
    }

    if (!(fscanf(inputFile, "%d", &data.numUsers) == 1 &&
           fscanf(inputFile, "%d", &data.numStations) == 1 &&
           fscanf(inputFile, "%d", &data.numSlotsPerStation) == 1 &&
           fscanf(inputFile, "%d", &data.minChooseTime) == 1 &&
           fscanf(inputFile, "%d", &data.maxChooseTime) == 1 &&
           fscanf(inputFile, "%d", &data.minRideTime) == 1 &&
           fscanf(inputFile, "%d", &data.maxRideTime) == 1)) {
        printf("Error al leer el fichero: El fichero tiene un formato incorrecto\n");
        exit(5);
    }

    if(data.minChooseTime < 1){
        printf("Error: El campo \"Tiempo minimo de espera para decidir coger una bici\" tiene que ser >= 1\n");
        exit(6);
    }
    if (data.minRideTime < 1){
        printf("Error: El campo \"Tiempo minimo que pasa un usuario montando una bici\" tiene que ser >= 1\n");
        exit(7);
    }

    // CIERRE FICHERO DE ENTRADA
    fclose(inputFile);
    
    //INICIALIZACION DE VARIABLES CON LOS DATOS OBTENIDOS
    users = (User *) malloc(sizeof(User) * data.numUsers);
    userThreads = (pthread_t *) malloc(sizeof(pthread_t) * data.numUsers);
    stations = (Station *) malloc(sizeof(Station) * data.numStations);

    // APERTURA DE FICHERO DE SALIDA
    outputFile = fopen(outputFileName, "w");

    if (outputFile == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo %s\n", outputFileName);
        exit(8);
    }

    // IMPRESION DE DATOS LEIDOS DEL FICHERO DE ENTRADA AL COMIENZO DE LA EJECUCION
    printIntro(&data);

    // INICIALIZACION DE LOS USUARIOS Y CREACION DE LOS HILOS POR CADA USUARIO
    for (i = 0; i < data.numUsers; i++) {
        users[i].id = i+1;
        users[i].actualStation = -1;
        pthread_create(&userThreads[i], NULL, userThreadFunction, &users[i]);
    }

    // CREACION DE LOS MUTEX Y CONDICIONALES DE CADA ESTACION
    for (j = 0; j < data.numStations; j++) {
        stations[j].id = j+1;
        stations[j].availableBikes = data.numSlotsPerStation * 3 / 4;
        pthread_mutex_init(&stations[j].station_mutex, NULL);
        pthread_cond_init(&stations[j].not_empty_cond, NULL);
        pthread_cond_init(&stations[j].not_full_cond, NULL);
    }

    // PUESTA EN MARCHA DE LOS HILOS
    for (i = 0; i < data.numUsers; i++) {
        pthread_join(userThreads[i], NULL);
    }

    // CIERRE FICHERO DE SALIDA
    fclose(outputFile);

    // DESTRUCCION DE LOS MUTEX Y LAS CONDICIONES
    for (j = 0; j < data.numStations; j++) {
        pthread_mutex_destroy(&stations[j].station_mutex);
        pthread_cond_destroy(&stations[j].not_empty_cond);
        pthread_cond_destroy(&stations[j].not_full_cond);
    }
    
    // LIBERACION DE ESPACIO
    free(users);
    free(userThreads);
    free(stations);
    return 0;
}