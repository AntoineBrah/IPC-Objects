#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <sys/ipc.h> // CH2
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>

#define RESET   "\033[0m"       /* White */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold green */

using namespace std;

bool estPremier(unsigned int n){

    bool res = true;

    if(n==0 || n==1){
        res = false;
    }
    else{
        for(int i=2; i<n; i++){
            if(n%i == 0){
                res = false;
                break;
            }
        }
    }

    return res;
}

int plusPetitDiveur(int n){

    int ppd = 0;

    for(int i=2; i<=n; i++){
        if(n%i == 0){
            ppd = i;
            break;
        }
    }

    return ppd;
}

/*** Structure Message ***/
struct message{
    long idMsg; // id du msg, 1 : Nombre à diviser, 2 : Plus petit diviseur 
    struct contenu{
        int n; 
    }contenu;
};


// Fonction pour le pere
void pere_fonction(int msgid, int nbFils, int n){

    struct message msg;

    int *tabDiv = (int*)malloc(sizeof(int)*nbFils);

    for(int i=0; i<nbFils; i++){
        tabDiv[i] = 0;
    }

    int j = 0;
    int prod = 1;

    while(prod != n){

        if(msgrcv(msgid, &msg, sizeof(msg.contenu), 2, 0) == -1){
            cerr << "fin" << endl;
            break;
        }

        cout << "(pere) msgrcv ok (idmsg=2)" << endl;

        tabDiv[j] = msg.contenu.n;
        prod *= msg.contenu.n;

        j++;

        cout << BOLDGREEN << "prod = " << prod << RESET << endl;
    }

    cout << BOLDGREEN << "=============> La décomposion en nombre premier du nombre " << n << " est : ";
    for(int i=0; i<nbFils; i++){
        cout << tabDiv[i] << " ";
    }
    cout << RESET << endl;

}

// Fonction pour les fils
void fils_fonction(int msgid){

    struct message msg;
    
    if(msgrcv(msgid, &msg, sizeof(msg.contenu), 1, 0) == -1){
        cerr << "erreur msgrcv" << endl;
    }

    cout << "(fils) msgrcv ok (idmsg=1)" << endl;

    if(estPremier(msg.contenu.n)){
        msg.idMsg = 2;
        if(msgsnd(msgid, &msg, sizeof(msg.contenu), 0) == -1){ // On envoi le msg sur la file
            cerr << "erreur send" << endl;
            exit(1);
        }

        cout << "(fils) msgsend ok (idmsg=2)" << endl;
    }
    else{
        cout << BOLDGREEN << "msgcontenu = " << msg.contenu.n << endl;
        msg.idMsg = 1;
        int ppd = plusPetitDiveur(msg.contenu.n);
        cout << "ppd = " << ppd << endl;
        int div = (msg.contenu.n/ppd);
        cout << "diviseur = " << div << RESET << endl;
        msg.contenu.n = div;
        
        struct message msg2;
        msg2.idMsg = 2;
        msg2.contenu.n = ppd;

        if(msgsnd(msgid, (void*)&msg, sizeof(msg.contenu), 0) == -1){ // On envoi le msg sur la file contenant le nombre à diviser
            cerr << "erreur send" << endl;
            exit(1);
        }

        cout << "(fils) msgsend ok (idmsg=1)" << endl;

        if(msgsnd(msgid, &msg2, sizeof(msg2.contenu), 0) == -1){ // On envoi le msg sur la file contenant un entier premier qui décompose le nombre initial
            cerr << "erreur send" << endl;
            exit(1);
        }

        cout << "(fils) msgsend ok (idmsg=2)" << endl;

    }

}


int main(int argc, char* argv[]){

    if(argc != 3){
        cerr << "./exe n [nbFils]" << endl;
        exit(1);
    }

    cout << "(pere) Processus en cours d'execution" << BOLDGREEN << " pid : " << getpid() << RESET << endl;

    pid_t pidPere = getpid();

    key_t clef = ftok("fichier.txt", 'z'); // Création de la clef

    if (clef == -1) {
        cerr << "erreur ftok" << endl;
        exit(1);
    }

    cout << "ftok ok" << endl;

    /*
    * Si la file existe déjà, on obtient son id,
    * Sinon on la créer
    */
    int msgid = msgget(clef, IPC_CREAT|0666);

    if(msgid == -1){
        cerr << "erreur msgget" << endl;
        exit(1);
    }

    cout << "msgget ok " << endl;


    struct message msg;
    msg.idMsg = 1;
    msg.contenu.n = atoi(argv[1]);
    
    if(msgsnd(msgid, (void*)&msg, sizeof(msg.contenu), 0) == -1){ // On envoi le msg sur la file
        cerr << "erreur send" << endl;
        exit(1);
    }

    cout << "msgsnd ok" << endl;

    pid_t *fils = (pid_t*)malloc(sizeof(pid_t)*atoi(argv[2])); // Tableau de fils

    for(int i=0; i<atoi(argv[2]); i++){

        fils[i] = fork();

        if(fils[i] == 0){
            cout << "(fils) Processus en cours d'execution" << BOLDGREEN << " pid : " << getpid() << RESET << endl;
            break;
        }
    }
    
    if(pidPere == getpid())
        pere_fonction(msgid, atoi(argv[2]), atoi(argv[1]));
    else
        fils_fonction(msgid);
    

    if(pidPere == getpid()){
        cout << "(pere) FIN processus en cours d'execution" << BOLDGREEN << " pid : " << getpid() << RESET << endl;

        if(msgctl(msgid, IPC_RMID, NULL) == -1){
            cerr << "erreur msgctl" << endl;
        }
    }
    else
        cout << "(fils) FIN processus en cours d'execution" << BOLDGREEN << " pid : " << getpid() << RESET << endl;
    return 0;
}