#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>
#include "calcul/calcul.h" // Fichiers : calcul.h, calculC.o, calculCC.o

using namespace std;

// ce fichier contient des bouts de codes du client pour l'exercice 1 du TP IPC.

int main (int argc, char * argv[]){
    
    key_t cle = ftok("fichier.txt", 'z'); // Obtention de la clef

    if (cle == -1) {
        perror("erreur ftok");
        exit(1);
    }

    printf("ftok ok\n");

    // Création de la File.
    int msgid = msgget(cle, IPC_CREAT|0666); // Obtention de l'id de la File

    if(msgid == -1) {
        perror("erreur msgget");
        exit(1);
    }

    printf("msgget ok\n");

    // Structure d'un message
    struct message{
        long client; // id du client
        struct contenu{
            int idOp; // 1: addition, 2: soustraction, 3: multiplication, 4: division
            int op1;
            int op2;
            int res;
        }contenu;
    }msg;
    
    while(1){
        // ici le 4ieme paramètre vaut 0, ça signifie qu'on extrait TOUS les MSG en Tête de File
        if(msgrcv(msgid, &msg, sizeof(msg.contenu), 0, 0) == -1){
            cerr << "fin" << endl;
            break;
        }

        cout << "msgrcv ok" << endl;

        switch(msg.contenu.idOp){
            case 1:
                msg.contenu.res = msg.contenu.op1+msg.contenu.op2;
                break;
            case 2:
                msg.contenu.res = msg.contenu.op1-msg.contenu.op2;
                break;
            case 3:
                msg.contenu.res = msg.contenu.op1*msg.contenu.op2;
                break;
            case 4:
                msg.contenu.res = msg.contenu.op1/msg.contenu.op2;
                break;
            default:
                cout << "Erreur switch" << endl;
                break;
        }

        cout << "traitement calcul du client : " << msg.client << endl;
        calcul(1);
        if(msgsnd(msgid, (void*)&msg, sizeof(msg.contenu), 0) == -1){
            perror("erreur send");
            exit(1);
        }
        cout << "msgsnd ok" << endl;
    }


    return 0;
}
