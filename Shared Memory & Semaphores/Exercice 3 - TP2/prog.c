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

using namespace std;

union semnum{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct semingo *__buf;
};

int main(int argc, char* argv[]){

    // Verif 1
    if(argc != 2){
        cout << "./prog.sh [tailleTableau]" << endl;
        exit(1);
    }

    tailleTab = atoi(argv[1]);

    // Verif 2
    if(tailleTab <= 0){
        cout << "tailleTableau > 0" << endl;
        exit(1);
    }

    key_t cle = ftok("fichier.txt", 'z');

    if(cle == -1){
        cerr << "erreur ftok" << endl;
        exit(1);
    }

    cout << "ftok ok" << endl;

    int shmid = shmget(cle, sizeof(int)*tailleTab, 0666);

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

    /*****************************************/
    /** CRÉATION DE L'ENSEMBLE DE SÉMAPHORE **/
    /*****************************************/

    int semid = semget(cle, tailleTab, 0666);

    if(semid == -1){
        cerr << "erreur semget" << endl;
        exit(1);
    }

    cout << "semget ok [id : " << semid << "]" << endl;

    struct sembuf tabSemBuf[tailleTab][2]; // 0 : P, 1 : V

    for(int i=0; i<tailleTab; i++){

        // P
        tabSemBuf[i][0].sem_num = i;
        tabSemBuf[i][0].sem_op = -1;
        tabSemBuf[i][0].sem_flg = SEM_UNDO;

        // V
        tabSemBuf[i][1].sem_num = i;
        tabSemBuf[i][1].sem_op = +1;
        tabSemBuf[i][1].sem_flg = SEM_UNDO;
    }

    for(int i=0; i<tailleTab; i++){

        if(semop(semid, &tabSemBuf[i][0], 1) == -1){
            cerr << "erreur semop" << endl;
            exit(1);
        }

        cout << "[i] PID : " << BOLDGREEN << getppid() << RESET << ", traite " << BOLDGREEN << "adr[" << i << "]" << RESET << endl;
        *((int*)adr+i) += 1; // zone critique
        sleep(3);
        cout << "[i] PID : " << BOLDGREEN << getppid() << RESET << ", adr[" << i << "] = " << BOLDGREEN << *((int*)adr+i) << RESET << endl;

        if(semop(semid, &tabSemBuf[i][1], 1) == -1){
            cerr << "erreur semop" << endl;
            exit(1);
        }

    }

    // Détachement du segment de mémoire partagé car on ne l'utilise plus
    if(shmdt(adr) == -1){
        cerr << "erreur shmdt" << endl;
        exit(1);
    }

    cout << "[i] PID : " << BOLDGREEN << getppid() << RESET << ". Toutes les zones ont été traité, fin du processus dans 10sec..." << endl;

    return 0;
}