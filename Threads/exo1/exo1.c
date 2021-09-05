#include <string.h>
#include <stdio.h>//perror
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include "../calcul.h"

using namespace std;

struct paramFonction{
    int id; // Nous permet d'identifier chaque thread

};

void* maFonction(void* param){

    struct paramFonction *parametre = (struct paramFonction *)(param);
    
    cout << "[*] Thread Secondaire en cours d'execution, pid : " << getpid() << ", tid : " << gettid() << ", [id : " << (*parametre).id << "]" << endl;

    calcul(1);

    cout << "[i] FIN Thread Secondaire, [id : " << (*parametre).id << "]" << endl;

    pthread_exit(NULL);
}


int main(int argc, char* argv[]){

    if(argc != 2){
        cout << "Veuillez saisir un paramètre (le nb de thread)" << endl;
        exit(1); // On quitte le programme
    }

    cout << "[*] Thread Principal en cours d'execution, pid : " << getpid() << ", tid : " << gettid() << ", [id : 0]" << endl;

    // On a notre tableau d'identifiant de thread
    pthread_t *idThreads = (pthread_t*)malloc(sizeof(pthread_t)*atoi(argv[1]));

    // On créer un tableau qui contient une structure propre à chaque thread
    struct paramFonction *paramThread = (struct paramFonction*)malloc(sizeof(struct paramFonction)*atoi(argv[1])); 

    for(int i=0; i<atoi(argv[1]); i++){

        paramThread[i].id = i+1;
        
        if(pthread_create(&idThreads[i], NULL, maFonction, &paramThread[i]) != 0){
            cerr << "Erreur de création du thread n°" << i << endl;
            exit(1);
        }
        

        if(pthread_join(idThreads[i], NULL) != 0){
            cerr << "Erreur join" << endl;
            exit(1);
        }

    }

    cout << "[i] FIN Thread Principal, [id :  0]" << endl;

    return 0;
}