#include <string.h>
#include <stdio.h>//perror
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include "calcul.h"

#define WHITE   "\033[0m"       /* White */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold green */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */

using namespace std;

// Variable globale
int nbThread;
int* zone;
//int cpt;
//pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct paramFonction{
    pthread_mutex_t *verrou;
    pthread_cond_t *cond;
    int id;
};

void* maFonction(void* param){
   
    struct paramFonction *parametre = (struct paramFonction*)param;

    cout << "[*] Thread Secondaire en cours d'execution, tid : " << gettid() << "," << BOLDGREEN << " id : " << parametre->id << WHITE << endl;


    if(parametre->id == 1){
        for(int i=0; i<nbThread; i++){
            calcul(1);
            pthread_mutex_lock(parametre->verrou);
            zone[i] = parametre->id;
            pthread_mutex_unlock(parametre->verrou); // Le thread libère le verrou
            cout << BOLDBLUE <<  "=> Thread (id : " << parametre->id << "), vient de traiter : zone[" << i << "] = " << zone[i] << WHITE << endl;
            pthread_cond_broadcast(parametre->cond);
        }
    }
    else{
        for(int i=0; i<nbThread; i++){


            while(zone[i] != parametre->id-1){
                cout << "[!] Thread Secondaire passe à l'état BLOQUÉ, tid : " << gettid() << "," << BOLDGREEN << " id : " << parametre->id << WHITE << endl;
                pthread_cond_wait(parametre->cond, parametre->verrou);
                cout << "[!] Thread Secondaire vient de se RÉVEILLER (avec le VERROU), tid : " << gettid() << "," << BOLDGREEN << " id : " << parametre->id << WHITE << endl;

            }

            calcul(1);
            zone[i] = parametre->id;
            pthread_mutex_unlock(parametre->verrou); // Le thread libère le verrou
            cout << BOLDBLUE <<  "=> Thread (id : " << parametre->id << "), vient de traiter : zone[" << i << "] = " << zone[i] << WHITE << endl;
            pthread_cond_broadcast(parametre->cond);
        }        
    }

    int *pid = (int*)malloc(sizeof(int)*1);
    *pid = parametre->id;

    pthread_exit(pid);
}


int main(int argc, char* argv[]){

    if(argc != 2){
        cout << "Veuillez saisir le nombre de zones ordonnées (et donc le nombre de Threads)" << endl;
        exit(1); // On quitte le programme
    }

    // On précise le nb de thread
    nbThread = atoi(argv[1]);

    cout << "Nombre THREAD (secondaires) : " << nbThread << endl;

    cout << "[**] Thread Principal en cours d'execution, tid : " << gettid() << "," << BOLDGREEN << " id : " << 0 << WHITE << endl;

    // Tableau d'id de Threads
    pthread_t *idThread = (pthread_t*)malloc(sizeof(pthread_t)*atoi(argv[1]));

    // Tableau de struct
    struct paramFonction *parametre = (struct paramFonction*)malloc(sizeof(struct paramFonction)*atoi(argv[1]));

    // Zones à traiter
    zone = (int*)malloc(sizeof(int)*atoi(argv[1]));
    for(int i=0; i<atoi(argv[1]); i++){
        zone[i] = 0;
        cout << BOLDBLUE << "zone[" << i << "] = " << zone[i] << WHITE << endl; // Aucune zone n'a été traité 
    }


    /** Données Communes **/
    pthread_mutex_t verrou;
    pthread_cond_t cond;

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
        parametre[i].id = i+1;

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
        cout << "[>>>] Fin du Thread Secondaire de " << BOLDGREEN << "id : " << res << WHITE << endl;
    }



    cout << "[>>>] Fin du Thread Principal" << endl;

    return 0;
}
