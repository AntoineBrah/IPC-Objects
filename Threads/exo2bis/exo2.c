#include <string.h>
#include <stdio.h>//perror
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include "calcul.h"

using namespace std;

struct paramFonction{
    int x1;
    int x2;
    int *sum;
    pthread_mutex_t *verrou;
};

void* maFonction(void* param){
    cout << "[*] Thread Secondaire en cours d'execution, pid : " << getpid() << ", tid : " << gettid() << endl;

    struct paramFonction *parametre = (struct paramFonction *)(param);

    // On obtient le verrou (début zone critique)
    pthread_mutex_lock(parametre->verrou);

    int *res = (int*)malloc(1*sizeof(int));
    *res = (parametre->x1)*(parametre->x2); 

    *(parametre->sum) += *res;

    // On libère le verrou (fin zone critique)
    pthread_mutex_unlock(parametre->verrou);

    pthread_exit(NULL);
}


int main(int argc, char* argv[]){

    if(argc != 2){
        cout << "Veuillez saisir un paramètre (la dimension des vecteurs)" << endl;
        exit(1); // On quitte le programme
    }

    int *v1 = (int*)malloc(sizeof(int)*atoi(argv[1]));
    int *v2 = (int*)malloc(sizeof(int)*atoi(argv[1]));

    for(int i=0; i<atoi(argv[1]); i++){
        cout << "v1[" << i << "] = ";
        cin >> v1[i];
    }

    for(int i=0; i<atoi(argv[1]); i++){
        cout << "v2[" << i << "] = ";
        cin >> v2[i];
    }

    // On a notre tableau d'identifiant de thread
    pthread_t *idThreads = (pthread_t*)malloc(sizeof(pthread_t)*atoi(argv[1]));

    // On créer un tableau qui contient une structure propre à chaque thread
    struct paramFonction *paramThread = (struct paramFonction*)malloc(sizeof(struct paramFonction)*atoi(argv[1])); 

    cout << "\n[**] Thread Principal en cours d'execution, pid : " << getpid() << ", tid : " << gettid() << endl;


    /** Données communes aux threads **/
    int sum = 0;
    pthread_mutex_t verrou;

    if(pthread_mutex_init(&verrou, NULL) != 0){
        cerr << "Erreur initialisation mutex" << endl;
        exit(1);
    }

    for(int i=0; i<atoi(argv[1]); i++){

        paramThread[i].x1 = v1[i];
        paramThread[i].x2 = v2[i];
        paramThread[i].sum = &sum;
        paramThread[i].verrou = &verrou;
        
        if(pthread_create(&idThreads[i], NULL, maFonction, &paramThread[i]) != 0){
            cerr << "Erreur de création du thread n°" << i << endl;
            exit(1);
        }
        

        if(pthread_join(idThreads[i], NULL) != 0){
            cerr << "Erreur join" << endl;
            exit(1);
        }

    }

    cout << "\nTotal : " << sum << endl;

    return 0;
}