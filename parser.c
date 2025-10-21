#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
typedef enum {MOT, TUB, INF, SUP, SPP, NL, FIN} LEX;
static LEX getlex(char *mot){
    enum {Neutre, Spp, Equote, Emot } etat=Neutre;
    int c;
    char *w;
    w=mot;
    
    while ((c=getchar()) != EOF){
        switch(etat){
        case Neutre:
            switch(c){
            case '<':
                return (INF);
            case '>':
                etat=Spp;
                continue;
            case '|':
                return (TUB);
            case '"':
                etat=Equote;
                continue;
            case ' ':
            case '\t':
                continue;
            case '\n':
                return(NL);
            default:
                etat=Emot;
                *w++=c;
                continue;
            }
        case Spp:
            if(c=='>') 
                return(SPP);
            ungetc(c,stdin);
            return(SUP);
        case Equote:
            switch(c){
            case '\\':
                *w++=c;
                continue;
            case '"':
                *w='\0';
                return(MOT);
            default:
                *w++=c;
                continue;
            }
        case Emot:
            switch(c){
            case '|':
            case '<':
            case '>':
            case ' ':
            case '\t':
            case '\n':
                ungetc(c,stdin);
                *w='\0';
                return(MOT);
            default:
                *w++=c;
                continue;
            }
        }
    }
    return(FIN);
}

/**
void commande(char *args[]){

}
 */

void addToList(char **list, char *item) {
    int i = 0;
    while (list[i] != NULL) {
        i++;
    }
    list[i] = strdup(item);
    list[i + 1] = NULL;
}


void main(int argc, char *argv[]){
    char *args[50] = {NULL};
    char mot[200];
    int afficher_prompt = 1;
    while(1) {
        if (afficher_prompt) {
            printf("%s>$", getenv("PWD"));
            fflush(stdout); 
            afficher_prompt = 0; 
        }

        switch(getlex(mot)){
        case MOT:
            printf("MOT: %s\n",mot);
            addToList(args, mot);
            break;
        case TUB:
            printf("TUBE\n");
            break;
        case INF:
            printf("REDIRECTION ENTREE\n");
            break;
        case SUP:
            printf("REDIRECTION SORTIE\n");
            break;
        case SPP:
            printf("REDIRECTION AJOUT\n");
            break;
        case NL:
            printf("NOUVELLE LIGNE \n");
            pid_t pid_fils;
            pid_fils = fork();

            if(pid_fils == 0){
                execvp(args[0], args);
                perror("Erreur execvp");
                exit(EXIT_FAILURE);
            }else if (pid_fils > 0) {
                // Processus PARENT
                wait(NULL);
            } else {
                // Erreur de fork
                perror("Erreur fork");
            }
            int k = 0;
            while (args[k] != NULL) { // Libération de la mémoire
                free(args[k]);
                args[k] = NULL;
                k++;
            }
            printf("\n");
            afficher_prompt = 1;
            break;
        case FIN:
            printf("FIN \n");
            exit(0);
        }
    }
}