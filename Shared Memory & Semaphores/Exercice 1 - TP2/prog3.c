/*
*   - Créer les objets IPC : shm et semaphore
*/

#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <sys/ipc.h> // CH2
#include <sys/msg.h> // CH2
#include <sys/shm.h> // CH2
#include <sys/sem.h> // CH2
#include <unistd.h>
#include <stdio.h>

#define RESET   "\033[0m"       /* White */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold green */

using namespace std;


union semnum{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

int main(int argc, char* argv[]){

    key_t cle = ftok("fichier.txt", 'z');

    if(cle == -1){
        cerr << "erreur ftok" << endl;
        exit(1);
    }

    int shmid = shmget(cle, sizeof(int), IPC_CREAT|0666);

    if(shmid == -1){
        cerr << "erreur shmget" << endl;
        exit(1);
    }

    cout << "shmget ok " << "[id : " << shmid  << "]" << endl; 

    void *adr = NULL; // Pointeur contenant l'adresse de la variable dont la valeur est le nombre de place dispo

    if((adr = shmat(shmid, NULL, 0666)) == (int*)-1){
        cerr << "erreur shmat" << endl;
        exit(1);
    }

    cout << "shmat ok" << endl;

    *(int*)adr = 5; // On définit le nombre initial de place dispo à 5

    /*****************************************/
    /** CRÉATION DE L'ENSEMBLE DE SÉMAPHORE **/
    /*****************************************/

    int idSem = semget(cle, 1, IPC_CREAT|0666);

    if(idSem == -1){
        cerr << "erreur semget" << endl;
        exit(1);
    }

    cout << "semget ok [id : " << idSem << "]" << endl;

    /*
    *   Initilisation du tableau de sémaphore
    */


   union semnum egCtrl;
   egCtrl.val =  1; // On définit la valeur du sémaphore

   if(semctl(idSem, 0, SETVAL, egCtrl) == -1){
       cerr << "erreur semctl (init)" << endl;
       exit(1);
   }

   cout << "semctl ok" << endl;

   cout << "[i] Nombre de place du parking : " << *(int*)adr << endl;

   while(1){
       int x;
       cout << "[i] Détruire les objets IPC (0 : non, 1 : oui) : "; 
       cin >> x;

       if(x==1){
           if(shmctl(shmid, IPC_RMID, NULL) == -1){
               cerr << "erreur shmctl" << endl;
               exit(1);
           }

           cout << "suppression du segment de mémoire partagée : OK" << endl;

           if(semctl(idSem, 0, IPC_RMID) == -1){
               cerr << "erreur semctl" << endl;
               exit(1);
           }

           cout << "suppression du tableau de sémaphore : OK" << endl;
           break;
       }
   }

    return 0;
}