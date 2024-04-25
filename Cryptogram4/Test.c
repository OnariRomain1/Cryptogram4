#include <stdio.h>
#include <string.h>

char *Path = "crypt?move=AS";
/*
int containsVal(char *word){
    char* crypt = "crypt";
    char* crypto = "crypto";

    for (int i = 0; i < strlen(crypt);i++){

    }
    for (int i = 0; i < strlen(crypto); i++){

    }

/*
    for(int i = 0; i < strlen(word);i++){
        if (Path[i] == word[i]){
            printf("%c\n",Path[i]);
        } else{
            printf("loading files instead");
            break;
        }
    }

for (int i =0; i < strlen(Path);i++){
    if (Path[i] == '?'){
        break;
    }
    printf("%c\n", Path[i]);
}
}*/

void checkUrl(char*Path){
    char* crypt = "crypt";
    char* crypto = "crypto";
    char* move = "?move=";
    int cryptCount = 0;
    int cryptoCount = 0;
    char Move[2];
    
    for (int i =0; Path[i] != '\0'; i++){
        if (strncmp(&Path[i], crypto, strlen(crypto)) == 0) {
            printf("%s\n", crypto);
        }
        if ((strncmp(&Path[i], crypt, strlen(crypt)) == 0) && !((strncmp(&Path[i], crypto, strlen(crypto)) == 0)) && !((strncmp(&Path[i], "?", 1) == 0))){
            printf("%s\n", crypt);
            
        }
        
          
        if (strncmp(&Path[i], move, strlen(move)) == 0){
            printf("%s\n", move);
            for (int i = 0; i < strlen(Path); i++){
                if (Path[i] == '='){
                    Move[0] = Path[i+1];
                    Move[1] = Path[i+2];
                }
            }
        }
        
}
 Move[2] = '\0';
 /*
   int distance = word[0] - 'A';
   char replacement = word[1];
   printf("Word: %d, %c\n",distance, replacement);

    populatePlayerKey(PlayerEntered,distance,replacement);
 printf("Move: %s\n", Move);
*/
    

    
}
int main(){
    checkUrl(Path);
}