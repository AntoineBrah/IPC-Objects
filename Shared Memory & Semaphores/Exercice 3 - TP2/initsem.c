#include <stdlib.h>
#include <sys/types.h>
#include <iostream> // C++
#include <sys/ipc.h> // CH2
#include <sys/msg.h> // CH2
#include <sys/shm.h> // CH2
#include <sys/sem.h> // CH2
#include <unistd.h>
#include <stdio.h>

#define RESET   "\033[0m"       /* White */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold green */

int tailleTab = 0;
int nbProcessus = 0;

using namespace std;

union semnum{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct semingo *__buf;
};

int main(int argc, char* argv[]){

    // Verif 1 : nbArgument
    if(argc != 3){
        cout << "./initsem.sh [tailleTableau] [nbProcessus]" << endl;
        exit(1);
    }

    tailleTab = atoi(argv[1]);

    // Verif 2 : taille Tableau
    if(tailleTab <= 0){
        cout << "tailleTableau > 0" << endl;
        exit(1);
    }

    nbProcessus = atoi(argv[2]);

    // Verif : nbProcessus
    if(nbProcessus <= 0){
        cout << "nbProcessus > 0" << endl;
        exit(1);
    }

    key_t cle = ftok("fichier.txt", 'z');

    if(cle == -1){
        cerr << "erreur ftok" << endl;
        exit(1);
    }

    cout << "ftok ok" << endl;

    int shmid = shmget(cle, sizeof(int)*tailleTab, IPC_CREAT|0666);

    if(shmid == -1){
        cerr << "erreur shmget" << endl;
        exit(1);
    }

    cout << "shmget ok [id : " << shmid << "]" << endl;

    void* adr = NULL;

    if((adr = shmat(shmid, NULL, 0666)) == (int*)-1){
        cerr << "erreur shmat" << endl;
        exit(1);
    }

    cout << "shmat ok" << endl;

    for(int i=0; i<tailleTab; i++){
        *((int*)adr+i) = 0;
        cout << "adr[" << i << "] = " << *((int*)adr+i) << endl;
    }

    if(shmdt(adr) == -1){
        cerr << "err shmdt" << endl;
        exit(1);
    }

    cout << "shmdt ok" << endl;

    /*****************************************/
    /** CRÉATION DE L'ENSEMBLE DE SÉMAPHORE **/
    /*****************************************/

    int semid = semget(cle, tailleTab, IPC_CREAT|0666);

    if(semid == -1){
        cerr << "erreur semget" << endl;
        exit(1);
    }

    cout << "semget ok [id : " << semid << "]" << endl;

    union semnum sem;
    sem.val = 1;

    for(int i=0; i<tailleTab; i++){
        if(semctl(semid, i, SETVAL, sem) == -1){
            cout << "semctl erreur" << endl;
            exit(1);
        }
        cout << "sem[" << i << "] = " << semctl(semid, i, GETVAL) << endl;
    }
    
    char buffer[100];

    snprintf(buffer, sizeof(buffer), "gnome-terminal -e \'sh -c \"./prog.sh %d; sleep 10\"\'", tailleTab);  

    for(int i=0; i<nbProcessus; i++){
        system(buffer);
    }

    do{
        int x = 0;
        cout << "[i] Supprimer les objets IPC (0 : non, 1 : oui) : ";

        cin >> x;

        if(x==1){
            // Suppresion segment de mémoire partagé
            if(shmctl(shmid, IPC_RMID, NULL) == -1){
                cerr << "erreur shmctl" << endl;
                exit(1);
            }

            // Suppression tableau de sémaphore
            if(semctl(semid, 0, IPC_RMID) == -1){
                cerr << "erreur segctl" << endl;
                exit(1);
            }

            break;
        }
    }while(1);

    cout << "suppresion des objets IPC : OK" << endl;

    return 0;
}
