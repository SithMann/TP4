#include "messages.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

/* VARIABLES GLOBALES (utilisees dans les handlers)*/
int id;

/* HANDLERS */
void handlerCoureur(int sig){
    requete_t R;
    reponse_t Rep;
    R.type = PC_COURSE;
    R.corps.dossard = getpid();
    R.corps.etat = ABANDON;
    msgsnd(id, &R, sizeof(R.corps), 0);
    msgrcv(id, &Rep, sizeof(Rep.corps), getpid(),0);
    if(Rep.corps.compte_rendu == ABANDON_ACK){
        fprintf(stderr,"Interruption par signal\n");
        exit(sig);
    }
}

int main(int argc, char * argv[]){

    /* Capture des paramètres */
    if(argc != 2){
        fprintf(stderr,"Nombre de paramètres incorrect : ./%s <clé>\n", argv[0]);
        exit(1);
    }
    int cle = atoi(argv[1]);

    /* Déclarations et définitions sur la requete et la reponse */
    requete_t R;
    reponse_t Rep;
    Rep.corps.etat = EN_COURSE;
    R.type = PC_COURSE;
    R.corps.dossard = getpid();
    R.corps.etat = EN_COURSE;
    id = msgget(cle, 0666);

    /* Si le coureur reçoit un signal de type SIGINT, il abandonne */
    struct sigaction action;

    action.sa_handler = handlerCoureur;
    sigemptyset(&(action.sa_mask));
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);


    while(Rep.corps.etat == EN_COURSE){

        /* Envoie d'une requete au PC course */
        msgsnd(id, &R, sizeof(R.corps), 0);

        /* Reception d'une réponse */
        msgrcv(id, &Rep, sizeof(Rep.corps), getpid(),0);
        R.corps.etat = Rep.corps.etat;

        system("clear");
        messages_afficher_reponse( &Rep);
        messages_afficher_parcours( &Rep);
        if(Rep.corps.etat == EN_COURSE)
            messages_attendre_tour();
    
    }
}