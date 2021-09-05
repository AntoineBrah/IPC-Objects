/*
*   Borne sortante :
*       - indique les voitures qui sortent et donc incrémente le nombre de place dispo 
*       - ici en gros on doit faire un truc du style V(semaphore, 1) pour libérer une place de parking
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

int main(int argc, char* argv[]){

    key_t cle = ftok("fichier.txt", 'z');

    if(cle == -1){
        cerr << "erreur ftok" << endl;
        exit(1);
    }

    cout << "ftok ok" << endl;

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

    struct sembuf opp; // on décrémente de 1 : entrée dans la zone critique

    opp.sem_num = 0; // Il n'y a qu'1 seul sémaphore dans le tableau et on l'obtient à l'indice 0
    opp.sem_op = -1; // On enlève une place (car on l'a réservé)
    opp.sem_flg = SEM_UNDO;

    struct sembuf opv; // on incrémente de 1 : sortie de la zone critique

    opv.sem_num = 0; // Il n'y a qu'1 seul sémaphore dans le tableau et on l'obtient à l'indice 0
    opv.sem_op = +1;  // On ajoute une place car on l'a libère (on sort du parking)
    opv.sem_flg = SEM_UNDO;

    struct sembuf opz; // on incrémente de 1 : sortie de la zone critique

    opz.sem_num = 0; // Il n'y a qu'1 seul sémaphore dans le tableau et on l'obtient à l'indice 0
    opz.sem_op = 0;  // On attend que le sémaphore ait la valeur 0
    opz.sem_flg = SEM_UNDO;

    /***************************************************/

    if(semop(idSem, &opp, 1) == -1){
        cerr << "erreur semop" << endl;
        exit(1);
    }

    cout << "opp ok" << endl;

    cout << "[!] PID : " << getpid() << ", valeur sem : " << semctl(idSem, 0, GETVAL) << endl;

    if(semop(idSem, &opz, 1) == -1){
        cerr << "erreur semop" << endl;
        exit(1);
    }

    cout << "opz ok" << endl;
    

    

    return 0;
}