#include <stdlib.h>
#include <sys/types.h>
#include <iostream>
#include <sys/ipc.h> // CH2
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h> // CH1
#include "calcul/calcul.h" // Fichiers : calcul.h, calculC.o, calculCC.o

#define RESET   "\033[0m"       /* White */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold green */

using namespace std;

// Variables globales
int nbClient = 0;
int cpt = 0;
int cpt2 = 0;

/*
* Il s'agit du msg que l'on va envoyer
* L'étiquette désigne le type de calcul. (1: addition, 2: soustraction, 3: multiplication, 4: division).
* op1 et op2 les opérandes ; res le résultat.
*/
struct message{
  long client; // id du client (Thread)
  struct contenu{
    int idOp; // 1: addition, 2: soustraction, 3: multiplication, 4: division
    int op1;
    int op2;
    int res;
  }contenu;
};

/*
* Il s'agit de la structure que l'on va passer en paramètre de la fonction sendMsg.
* Elle est composé de l'id de la File et du msg à envoyer.
*/
struct paramFonction{
  int idFile;
  pthread_mutex_t *verrou;
  pthread_cond_t *cond; // la variable conditionnelle ne sert à rien ici
  struct message msg;
};

void* clientSendMsg(void* param){
  calcul(1); // On attend 1sec
  struct paramFonction *p = (struct paramFonction*)param;
  cout << "[*] Thread en cours d'execution" << BOLDGREEN << " tid : " << gettid() << " [id : " << p->msg.client << "]" << RESET << endl;
  calcul(1); // On attend 1sec

  /* ==== Début zone critique ==== */
  if(pthread_mutex_lock(p->verrou) != 0){ 
    cerr << "erreur mutex lock" << endl;
    exit(1);
  }

  cpt++;

  while(cpt < nbClient){
    if(pthread_cond_wait(p->cond, p->verrou) != 0){
      cerr << "erreur cond wait" << endl;
      exit(1);
    }
  }

  do{
    cout << "[i] Calcul du client " << p->msg.client << endl;

    cout << "-> operateur (1 : +, 2 : -, 3 : *, 4 : /) : "; // Saisie opérateur
    cin >> p->msg.contenu.idOp;

    cout << "-> operande 1 : "; // Saisie opérande 1
    cin >> p->msg.contenu.op1;

    cout << "-> operande 2 : "; // Saisie opérande 2
    cin >> p->msg.contenu.op2;
  }while(p->msg.contenu.idOp < 1 || p->msg.contenu.idOp > 4);

  calcul(1);

  if(pthread_mutex_unlock(p->verrou) != 0){
    cerr << "erreur mutex unlock" << endl;
    exit(1);
  }
  if(pthread_cond_broadcast(p->cond) != 0){
    cerr << "erreur broadcast" << endl;
    exit(1);
  }
  /* ==== Fin zone critique ==== */


  if(msgsnd(p->idFile, (void*)&p->msg, sizeof(p->msg.contenu), 0) == -1){ // Chaque Thread (client) envoi son msg à la calculatrice
    cerr << "erreur send" << endl;
    exit(1);
  }

  /*
  * Chaque client attend pour recevoir son msg.
  * Si son msg est présent dans la file alors il l'extrait et affiche le résultat.
  * Sinon il passe à l'état bloqué et il attend de recevoir son msg
  */
  if(msgrcv(p->idFile, (void*)&p->msg, sizeof(p->msg.contenu), p->msg.client, 0) == -1){ 
    cerr << "erreur receive" << endl;
    exit(1);
  }


  /* ==== Début zone critique ==== */
  if(pthread_mutex_lock(p->verrou) != 0){ 
    cerr << "erreur mutex lock" << endl;
    exit(1);
  }

  cpt2++;

  while(cpt2 < nbClient){
    if(pthread_cond_wait(p->cond, p->verrou) != 0){
      cerr << "erreur cond wait" << endl;
      exit(1);
    }
  }

  switch(p->msg.contenu.idOp){
    case 1:
        cout << "[Res] calcul du client " << p->msg.client << ", " << p->msg.contenu.op1 << "+" << p->msg.contenu.op2 << " = " << p->msg.contenu.res << endl;
        break;
    case 2:
        cout << "[Res] calcul du client " << p->msg.client << ", " << p->msg.contenu.op1 << "-" << p->msg.contenu.op2 << " = " << p->msg.contenu.res << endl;
        break;
    case 3:
        cout << "[Res] calcul du client " << p->msg.client << ", " << p->msg.contenu.op1 << "*" << p->msg.contenu.op2 << " = " << p->msg.contenu.res << endl;
        break;
    case 4:
        cout << "[Res] calcul du client " << p->msg.client << ", " << p->msg.contenu.op1 << "/" << p->msg.contenu.op2 << " = " << p->msg.contenu.res << endl;
        break;
    default:
      cout << "erreur switch" << endl;
      break;
  }

  if(pthread_mutex_unlock(p->verrou) != 0){
    cerr << "erreur mutex unlock" << endl;
    exit(1);
  }
  if(pthread_cond_broadcast(p->cond) != 0){
    cerr << "erreur broadcast" << endl;
    exit(1);
  }
  /* ==== Fin zone critique ==== */

  
  pthread_exit(NULL);
}



int main (int argc, char * argv[]){
  
  if(argc != 2){
    cout << "lancement : ./client [nbClient]" << endl;
    exit(1);
  }

  nbClient = atoi(argv[1]);

  cout << "[*] Thread en cours d'execution" << BOLDGREEN << " tid : " << gettid()  << " [id : 0]" << RESET << endl;
  
  key_t cle = ftok("fichier.txt", 'z'); // Obtention de la clef

  if (cle == -1) {
    cerr << "erreur ftok" << endl;
    exit(1);
  }

  cout << "ftok ok" << endl;

  // Création de la File.
  int msgid = msgget(cle, IPC_CREAT|0666); // Obtention de l'id de la File

  if(msgid == -1) {
    cerr << "erreur msgget" << endl;
    exit(1);
  }

  cout << "msgget ok" << endl;

  pthread_t *idThread = (pthread_t*)malloc(sizeof(pthread_t)*atoi(argv[1])); // Notre tableau d'id de Threads
  struct paramFonction *parametre = (struct paramFonction*)malloc(sizeof(struct paramFonction)*atoi(argv[1])); // Notre tableau de structure (parametre de chaque Threads)

  /* Données communes aux Threads */
  pthread_mutex_t verrou;
  pthread_cond_t cond;
  int client = 0; // Permet d'identifier les Threads

  if(pthread_mutex_init(&verrou, NULL) != 0 || pthread_cond_init(&cond, NULL) != 0){
    cerr << "erreur initialisation verrou ou variable conditionnelle" << endl;
    exit(1);
  }

  for(int i=0; i<atoi(argv[1]); i++){
    parametre[i].idFile = msgid;
    parametre[i].verrou = &verrou;
    parametre[i].cond = &cond;
    parametre[i].msg.client = ++client;

    /*
    * On initialise avec des valeurs par défaut les différents msg.
    * L'affectation efficiente aura lieu dans la fonction clientSendMsg().
    */ 
    parametre[i].msg.contenu.idOp = (long)0;
    parametre[i].msg.contenu.op1 = 0;
    parametre[i].msg.contenu.op2 = 0;
    parametre[i].msg.contenu.res = 0;

    if(pthread_create(&idThread[i], NULL, clientSendMsg, &parametre[i]) != 0){
      cerr << "erreur création thread" << endl;
      exit(1);
    }
  }

  for(int i=0; i<atoi(argv[1]); i++){
    pthread_join(idThread[i], NULL); // Le Thread principal termine uniquement quand ses Threads secondaires termineront
  }

  // Destruction du mutex
  if(pthread_mutex_destroy(&verrou) != 0){
    cerr << "erreur destruction mutex" << endl;
    exit(1);
  }

  // Destruction de la variable conditionnelle
  if(pthread_cond_destroy(&cond) != 0){
    cerr << "erreur destruction variable conditionnelle" << endl;
    exit(1);
  }

  // Suppression de la file
  if(msgctl(msgid, IPC_RMID, NULL) == -1){
    cout << "erreur suppression File" << endl;
    exit(1);
  }

  cout << "suppresion file ok" << endl;

  return 0;
}
