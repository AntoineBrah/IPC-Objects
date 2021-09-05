#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

// ce fichier contient des bouts de codes du client pour l'exercice 1 du TP IPC.

int main (int argc, char * argv[]){
    
    key_t cle = ftok("fichier.txt", 'z');

    if (cle == -1) {
        perror("erreur ftok");
        exit(1);
    }

    printf("ftok ok\n");

    // la clé numérique calculée, je réupère l'identifiant de la file (ici je suppose que la file existe).
    int msgid = msgget(cle, 0666);

    if(msgid == -1) {
        perror("erreur msgget");
        exit(1);
    }

    printf("msgget ok\n");

    // structure des requetes 
    struct req{

        long etiqReq; // 1

        struct ctn{
        int idop;  // 1: +, 2 : -, 3: *, 4: /
        int op1;
        int op2;
        int res;
        } contenu;

    } requete;

    ssize_t m_rcv = msgrcv(msgid, &requete, sizeof(requete.contenu), 1, 0);

    if(m_rcv == -1){
        cerr << "erreur receive" << endl;
        exit(1);
    }

    cout << "msgrcv ok" << endl;

    switch(requete.contenu.idop){
        case 1:
            requete.contenu.res = requete.contenu.op1+requete.contenu.op2;
            break;
        case 2:
            requete.contenu.res = requete.contenu.op1-requete.contenu.op2;
            break;
        case 3:
            requete.contenu.res = requete.contenu.op1*requete.contenu.op2;
            break;
        case 4:
            requete.contenu.res = requete.contenu.op1/requete.contenu.op2;
            break;
        default:
            cout << "Erreur switch" << endl;
            break;
    }

    if(msgsnd(msgid, (void*)&requete, sizeof(requete.contenu), 0) == -1){
        perror("erreur send");
        exit(1);
    }

    cout << "msgsnd ok" << endl;

    return 0;
}
