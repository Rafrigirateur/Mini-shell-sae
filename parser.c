#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h>
#include <unistd.h> 
#include <string.h>
#include <sys/wait.h>
typedef enum {MOT, TUB, INF, SUP, SPP, NL, FIN} LEX;

static LEX getlex(char *mot){ 
    enum {Neutre,  Spp, Equote, Emot } etat=Neutre; 
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

void addToList(char *args[], char *mot) {
    int i = 0;
    while (args[i] != NULL) {
        i++;
    }


    if (mot[0] == '$') {
        char *valeur = getenv(mot + 1); // on saute le '$' avec +1
        if (valeur != NULL) {
            args[i] = strdup(valeur);
        } else {
            // Si la variable n'existe pas, on met une chaine vide ou on ignore
            args[i] = strdup(""); 
        }
    } else {
        args[i] = strdup(mot);
    }
    
    args[i + 1] = NULL;
}

void commande(char *args[]){
    if (!strcmp(args[0], "cd")) {
        char *target;
        if (args[1] == NULL) {
            target = getenv("HOME");
        }else{
            target = args[1];
        }
        if (chdir(target) != 0) {
            perror("\033[31mcd");
        }
        return;
    }

    if (!strcmp(args[0], "set")) {
        if (args[1] == NULL) {
            extern char **environ;
            for (char **env = environ; *env; ++env) {
                printf("%s\n", *env);
            }
        } 
        else if (args[2] != NULL) {
            if (setenv(args[1], args[2], 1) != 0) {
                perror("\033[31mErreur setenv");
            }
        } else {
            printf("\033[31mUsage: set <VAR> <VALEUR>\n");
        }
        return;
    }

    switch(fork()){
        case -1:
            // Erreur de fork
            perror("\033[31mErreur fork");
            exit(EXIT_FAILURE);
        case 0:
            // Processus FILS
            execvp(args[0], args);
            perror("\033[31mErreur execvp");
            exit(EXIT_FAILURE);
        default:
            // Processus PARENT
            wait(NULL);
    }
}

int main(int argc, char *argv[]){ 
    char *args[50] = {NULL};
    char mot[200]; 
    int infoSystemPrinted = 0;

    int fd;
    int std_in = dup(0);
    int std_out = dup(1);

    // Mise en place des couleurs
    const char *GREEN = "\033[32m"; 
    const char *BLUE = "\033[34m"; 
    const char *WHITE = "\033[0m";

    // Autre style
    const char *BOLD = "\033[1m"; 
    const char *RESET = "\033[0m"; 

    while(1) {
        if (!infoSystemPrinted) {
            char directory[1024];
            char hostname[1024];
            char *user;
            char *home;
            
            getcwd(directory, sizeof(directory));
            gethostname(hostname, sizeof(hostname));
            user = getenv("USER"); 
            if (user == NULL) { 
                user = "unknown"; 
            }
            home = getenv("HOME");
            char displayDir[1024]; 
            if (home != NULL && strncmp(directory, home, strlen(home)) == 0) { 
                snprintf(displayDir, sizeof(displayDir), "~%s", directory + strlen(home)); 
            } else { 
                snprintf(displayDir, sizeof(displayDir), "%s", directory); 
            }

            printf("%s%s%s@%s%s:%s%s%s%s%s$", BOLD, GREEN, user, hostname, WHITE, BOLD, BLUE, displayDir, RESET, WHITE);

            fflush(stdout);
            infoSystemPrinted = 1;
        } 

        switch(getlex(mot)){ 
            case MOT: 
                // printf("MOT: %s\n",mot);
                // enregistrement du mot dans la liste des arguments
                addToList(args, mot);
                break; 
            case TUB:
                // printf("TUBE\n"); 
                int p[2];
                if (pipe(p) == -1) {
                    perror("pipe");
                    exit(1);
                }

                if (fork() == 0) {
                    dup2(p[1], 1);
                    close(p[0]);
                    close(p[1]);
                    
                    execvp(args[0], args);
                    perror("\033[31mErreur execvp pipe");
                    exit(1);
                }

                dup2(p[0], 0);
                close(p[1]);
                close(p[0]);

                for (int i = 0; args[i] != NULL; i++) {
                    free(args[i]);
                    args[i] = NULL;
                }
                break;
            case INF: 
                // printf("REDIRECTION ENTREE\n"); 
                getlex(mot);
                fd = open(mot, O_RDONLY, 0666);
                if (fd == -1){
                    perror("\033[31mErreur fichier inconue");
                    exit(1);
                }
                dup2(fd,0);
                close(fd);
                break; 
            case SUP: 
                // printf("REDIRECTION SORTIE\n"); 
                getlex(mot);
                fd = open(mot, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd == -1){
                    perror("\033[31mErreur fichier inconue");
                    exit(1);
                }
                dup2(fd,1);
                close(fd);
                break; 
            case SPP: 
                // printf("REDIRECTION AJOUT\n"); 
                getlex(mot);
                fd = open(mot, O_WRONLY | O_CREAT | O_APPEND, 0666);
                if (fd == -1){
                    perror("\033[31mErreur fichier inconue");
                    exit(1);
                }
                dup2(fd,1);
                close(fd);
                break; 
            case NL: 
                //printf("NOUVELLE LIGNE \n");

                // execution de la commande ici
                commande(args);
                // remise des stdin et stdout par default
                dup2(std_in, 0);
                dup2(std_out, 1);
                close(std_in);
                close(std_out);

                // reinitialisation de la liste des arguments
                for (int i = 0; args[i] != NULL; i++) {
                    free(args[i]);
                    args[i] = NULL;
                }
                infoSystemPrinted = 0;
                break;
            case FIN: 
                // printf("FIN \n"); 
                exit(0); 
        }
    }
}