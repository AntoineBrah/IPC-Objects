#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

int x = 0;

// ce fichier contient des bouts de codes du client pour l'exercice 1 du TP IPC.

int main (int argc, char * argv[]){
  
  if(argc != 4){
    printf("lancement : ./client operator operande1 operande2\n");
    cout << "\noperator : \n\n1 : addition\n2 : soustraction\n3 : multiplication\n4 : division\n" << endl;
    exit(1);
  }

  // récuperer l'identifiant de la file de message qu'on souhaite
  // utiliser. La clé est une paire : chemin vers un fichier existant
  // et un caractère (ou entier en fonction de l'OS). La même paire
  // permet d'identifier une seule file de message. Donc tous les
  // processus qui utiliseront la même paire, partageront la même
  // file de message (s'ils en ont les droits aussi)
  key_t cle = ftok("fichier.txt", 'z');

  if (cle == -1) {
    perror("erreur ftok");
    exit(1);
  }

  printf("ftok ok\n");

  // la clé numérique calculée, je récupère l'identifiant de la file (ici je suppose que la file existe).
  int msgid = msgget(cle, IPC_CREAT|0666);

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

  };

  // initialiser un message avant de l'envoyer.
  struct req requete;
  requete.etiqReq = 1;
  requete.contenu.op1 = atoi(argv[2]);
  requete.contenu.op2 = atoi(argv[3]);
  requete.contenu.idop = atoi(argv[1]);
  requete.contenu.res = 0;

  // envoi requete 

  int m_snd = msgsnd(msgid, (void*)&requete, sizeof(requete.contenu), 0);

  if(m_snd == -1){
    perror("erreur send");
    exit(1);
  }

  printf("msgsnd ok\n");


  cout << "Veuillez appuyer sur [ENTRÉE]." << endl;
  cin >> x;

  if(msgrcv(msgid, &requete, sizeof(requete.contenu), (long)1, 0) == -1){
      cerr << "erreur receive" <<  endl;
      exit(1);
  }

  cout << "msgrcv ok" << endl;

  switch(requete.contenu.idop){
    case 1:
      cout << "[*] " << requete.contenu.op1 << " + " << requete.contenu.op2 << " = " << requete.contenu.res << endl;
      break;
    case 2:
      cout << "[*] " << requete.contenu.op1 << " - " << requete.contenu.op2 << " = " << requete.contenu.res << endl;
      break;
    case 3:
      cout << "[*] " << requete.contenu.op1 << " * " << requete.contenu.op2 << " = " << requete.contenu.res << endl;
      break;
    case 4:
      cout << "[*] " << requete.contenu.op1 << " / " << requete.contenu.op2 << " = " << requete.contenu.res << endl;
      break;
    default:
      cout << "erreur switch" << endl;
      break;
  }

  if(msgctl(msgid, IPC_RMID, NULL) == -1){
    cout << "erreur remove" << endl;
  }

  cout << "remove ok" << endl;

  return 0;
}
