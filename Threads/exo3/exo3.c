#include <string.h>
#include <stdio.h>//perror
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include "calcul.h"

#define RESET   "\033[0m"       /* White */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold green */

using namespace std;

// Variable globale
int nbThread;
//int cpt;
//pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct paramFonction{
    pthread_mutex_t *verrou;
    pthread_cond_t *cond;
    int *cpt;
};

void* maFonction(void* param){

    cout << "[*] Thread Secondaire en cours d'execution, pid : " << getpid() << "," << BOLDGREEN << " tid : " << gettid() << RESET << endl;
   
    struct paramFonction *parametre = (struct paramFonction*)param;



    pthread_mutex_lock(parametre->verrou);
    *(parametre->cpt) += 1; // On effectue nos modifications
    pthread_mutex_unlock(parametre->verrou); // On libère le verrou (fin zone critique)

    while(*(parametre->cpt) < nbThread){

        // Le thread se bloque et attend d'être réveillé avec le verrou
        cout << "[!] Thread Secondaire passe à l'état BLOQUÉ, pid : " << getpid() << "," << BOLDGREEN << " tid : " << gettid() << RESET << endl;
        pthread_cond_wait(parametre->cond, parametre->verrou);
        cout << "[!] Thread Secondaire vient de se RÉVEILLER (avec le VERROU), pid : " << getpid() << "," << BOLDGREEN << " tid : " << gettid() << RESET << endl;

    }

    calcul(1);
        
    if(pthread_cond_broadcast(parametre->cond) == 0){
        pthread_mutex_unlock(parametre->verrou); // Le thread libère le verrou
    }
    else{
        cerr << "Erreur broadcast" << endl;
    }

    int *pid = (int*)malloc(sizeof(int)*1);
    *pid = gettid();

    pthread_exit(pid);
}


int main(int argc, char* argv[]){

    if(argc != 2){
        cout << "Veuillez saisir un paramètre (le nombre de Threads)" << endl;
        exit(1); // On quitte le programme
    }

    // On précise le nb de thread
    nbThread = atoi(argv[1]);

    cout << "Nombre THREAD (secondaires) : " << nbThread << endl;

    cout << "[**] Thread Principal en cours d'execution, pid : " << getpid() << "," << BOLDGREEN << " tid : " << gettid() << RESET << endl;

    // Tableau d'id de Threads
    pthread_t *idThread = (pthread_t*)malloc(sizeof(pthread_t)*atoi(argv[1]));

    // Tableau de struct
    struct paramFonction *parametre = (struct paramFonction*)malloc(sizeof(struct paramFonction)*atoi(argv[1]));


    /** Données Communes **/
    pthread_mutex_t verrou;
    pthread_cond_t cond;
    int cpt = 0;

    if(pthread_mutex_init(&verrou, NULL) != 0){
        cerr << "Erreur initialisation du verrou" << endl;
        exit(1);
    }

    if(pthread_cond_init(&cond, NULL) != 0){
        cerr << "Erreur initialisation de la variable conditionnelle" << endl;
        exit(1);
    }
    
    for(int i=0; i<atoi(argv[1]); i++){

        parametre[i].verrou = &verrou;
        parametre[i].cond = &cond;
        parametre[i].cpt = &cpt;

        if(pthread_create(&idThread[i], NULL, maFonction, &parametre[i]) != 0){
            cerr << "Erreur création thread" << endl;
            exit(1);
        }

        //pthread_join(idThread[i], NULL);
    }

    void* tid = (int*)malloc(sizeof(int)*1);

    for(int i=0; i<atoi(argv[1]); i++){
        pthread_join(idThread[i], &tid);

        int res = (*(int*)tid);
        cout << "[>>>] Fin du Thread Secondaire de " << BOLDGREEN << "tid : " << res << RESET << endl;
    }



    cout << "[>>>] Fin du Thread Principal" << endl;

    return 0;
}
