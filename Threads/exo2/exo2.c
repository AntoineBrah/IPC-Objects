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
};

void* maFonction(void* param){

    struct paramFonction *parametre = (struct paramFonction *)(param);
    
    cout << "[*] Thread Secondaire en cours d'execution, pid : " << getpid() << ", tid : " << gettid() << endl;

    int *res = (int*)malloc(1*sizeof(int));

    *res = (parametre->x1)*(parametre->x2); 

    pthread_exit(res);
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

    int sum = 0;

    for(int i=0; i<atoi(argv[1]); i++){

        paramThread[i].x1 = v1[i];
        paramThread[i].x2 = v2[i];
        
        if(pthread_create(&idThreads[i], NULL, maFonction, &paramThread[i]) != 0){
            cerr << "Erreur de création du thread n°" << i << endl;
            exit(1);
        }


        void* res = (int*)malloc(1*sizeof(int));
        

        if(pthread_join(idThreads[i], &res) != 0){
            cerr << "Erreur join" << endl;
            exit(1);
        }

        sum += (*(int*)res);

    }

    cout << "\nTotal : " << sum << endl;

    return 0;
}