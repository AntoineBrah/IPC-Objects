/*
*   Borne entrante :
*       - indique les voitures qui arrivent et donc décrémente le nombre de place dispo 
*       - ici en gros on doit faire un truc du style P(semaphore, 1) pour réserver une place de parking
*/

// Récupérer la valeur d'un sémaphore : semctl(idSem,0,GETVAL)

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

int main(int argc, char* argv[]){

    key_t cle = ftok("fichier.txt", 'z');

    if(cle == -1){
        cerr << "erreur ftok" << endl;
        exit(1);
    }

    int shmid = shmget(cle, sizeof(int), 0666);

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

    /************************************************/
    /** RÉCUPÉRATION ID DE L'ENSEMBLE DE SÉMAPHORE **/
    /************************************************/

    int idSem = semget(cle, 1, 0666);

    if(idSem == -1){
        cerr << "erreur semget" << endl;
        exit(1);
    }

    cout << "semget ok [id : " << idSem << "]" << endl;
    
    /** Description des opérations sur les sémaphores **/

    struct sembuf opp; // on décrémente de 1 : entrée dans la zone critique (DEMANDE DE RESSOURCE : P)

    opp.sem_num = 0; // Il n'y a qu'1 seul sémaphore dans le tableau et on l'obtient à l'indice 0
    opp.sem_op = -1; // On enlève une place (car on l'a réservé)
    opp.sem_flg = SEM_UNDO;

    struct sembuf opv; // on incrémente de 1 : sortie de la zone critique (REND RESSOURCE : V)

    opv.sem_num = 0; // Il n'y a qu'1 seul sémaphore dans le tableau et on l'obtient à l'indice 0
    opv.sem_op = +1;  // On ajoute une place car on l'a libère (on sort du parking)
    opv.sem_flg = SEM_UNDO;

    struct sembuf opz; // on incrémente de 1 : sortie de la zone critique

    opz.sem_num = 0; // Il n'y a qu'1 seul sémaphore dans le tableau et on l'obtient à l'indice 0
    opz.sem_op = 0;  // On attend que le sémaphore ait la valeur 0
    opz.sem_flg = SEM_UNDO;

    /***************************************************/

    while(1){


        if(*(int*)adr > 0){
            cout << "Demande acceptée" << endl;

            if(semop(idSem, &opp, 1) == -1){
                cerr << "erreur semop" << endl;
                exit(1);
            }
            *(int*)adr -= 1; // zone critique
            if(semop(idSem, &opv, 1) == -1){
                cerr << "erreur semop" << endl;
                exit(1);
            }

            cout << "Impression ticket" << endl;
            cout << "[i] Il reste " << *(int*)adr << " place(s)" << endl;
        }
        else{
            cout << "Pas de place" << endl;
        }

        sleep(2);
    }



    

    return 0;
}