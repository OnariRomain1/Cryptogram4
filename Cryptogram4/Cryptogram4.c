#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <ctype.h>


/*cd /mnt/c/Users/onari/Downloads/Cryptogram4
gcc -lpthread Cryptogram4.c -o crypto4
http://localhost:8000/secret/
*/
struct ParseRequest {
    char *request;
    int clientSocket;
};

struct Quote
{
    char *Phrase;
    char *Author;
    struct Quote *nextQuote;
};
char *filepath;

const char* Puzzle;
//encryption key with a collection of 26 pairings: 
char EncryptionKey[26];
//What the player has entered so far
char PlayerEntered[26];
// the encrypted string is the randomized puzzle string 
char *pEncryptedString = NULL;
struct Quote *quoteListHead = NULL;
int QuoteEntries = 0;

void QuoteStructure(struct Quote **QuoteNode){
    *QuoteNode = (struct Quote*) malloc(sizeof(struct Quote));
    if (*QuoteNode == NULL){
        printf("Memory not allocated. \n");
        exit(0);
    }
    (*QuoteNode)->Author = NULL;
    (*QuoteNode)->Phrase = NULL;
    (*QuoteNode)->nextQuote = NULL;
}
void FreeQuotes(){
    struct Quote *currentNode = quoteListHead;
    while (currentNode !=NULL)
    {
        struct Quote *freeNode = currentNode;
        free(freeNode->Phrase);
        free(freeNode->Author);
        currentNode = currentNode->nextQuote;
        free(freeNode);
     

    }
    quoteListHead = NULL;
     
}


void LoadPuzzles(void){
    char line[400];
    char *copyReadLine = NULL;
    QuoteStructure(&quoteListHead);
    struct Quote *currentQuote =NULL;
    char *AuthorString = NULL;
    int firstLine = true;

    //Opens file and checks if its null
    FILE *file = fopen("quotes.txt","r");
     if (file == NULL){
        perror("Error Opening file");
    }
    currentQuote =  quoteListHead;
    //Reading each line 
        while ((fgets(line,sizeof(line),file)) != NULL)
        {

           if (strlen(line) < 3){
            //makes a new Quote node and appends the old one in the list
                struct Quote *newNode;
                QuoteStructure(&newNode);
                //setting the new node as the head
                newNode->nextQuote = quoteListHead;
                quoteListHead = newNode;
                currentQuote = newNode;
                //counting the number of nodes created
                QuoteEntries++;
            //checking if the line  equals -- then setting the currentQuotes author to the line
           }else if (line[0] == '-' && line[1] == '-'){
             size_t authorStringLength = strlen(line) + 1;
        
            currentQuote->Author = (char*)malloc(authorStringLength);
            
            if (currentQuote->Author == NULL) {
                perror("Author Memory allocation failed");
                exit(0);
            }
            strcpy(currentQuote->Author, line);
           }else {
                 //allocating space and copying the line and putting it in the currenQuotes phrase
                 size_t phraseLength = strlen(line) + 1;
                copyReadLine = (char*) malloc(phraseLength);
                if (copyReadLine == NULL){
                    printf("CopyReadline Memory not allocated. \n");
                    exit(0);
                }
                strcpy(copyReadLine, line);
                 if (currentQuote->Phrase == NULL) {
                currentQuote->Phrase = copyReadLine;
                 } else { 

                char *existingString = NULL;
                if (currentQuote != NULL && currentQuote->Phrase != NULL){
                //allocating space for the existing line and the readline
                size_t existingStringLength = strlen(currentQuote->Phrase);
                char *existingString = (char*) realloc(currentQuote->Phrase, existingStringLength + phraseLength);
                  if (existingString == NULL){
                    printf("Existing sting Memory not allocated. \n");
                    exit(0);
                }
                currentQuote->Phrase=existingString;
             //concatenating the current phrase and the line
                strcat(currentQuote->Phrase, line);

                free(copyReadLine);
            }
            }
            
           }
        }   
   
    if (EOF == fclose(file)){
        printf("fail on close!\n");
    }
    printf("QuoteEntries: %d\n", QuoteEntries);
  
}


/*
    The shuffle function
    implements the Fisher-Yates algorithm for 
    changing the orders of the letters in the encryption key

*/
void shuffle(char *encryptionKey){
    int n = 26;
    for(int i = n-1; i > 0;i--){
        int randomnNumber = 1+ (random() % i);
        int temp = encryptionKey[i];
        encryptionKey[i] = encryptionKey[randomnNumber];
        encryptionKey[randomnNumber] = temp;
 
    }
}
/*
    The getPuzzle function 
    returns a constant string 
*/
const char *getPuzzle(){

   if (QuoteEntries == 0) {
        LoadPuzzles();
        
    }
    
    if (quoteListHead != NULL) {
        //returning a random quote based on the number of Quote Entries 
        int randomPuzzle = (random() % QuoteEntries) + 1;
        struct Quote *temp = quoteListHead;
        //traversing through the Quote linked List structure
        for (int i = 1; i < randomPuzzle; i++) {
            if (temp != NULL) {
                temp = temp->nextQuote;
            }
        }
        //returning the phrase
        if (temp != NULL) {
            return temp->Phrase;
        }
    }
    
    return NULL;
}

void populatePlayerKey(char *PlayerEncryptionKey, int distance, char replacement){
     for (int i = 0; i < 26; i++) {
        // If the slot is empty, populate it
        if (PlayerEncryptionKey[i] == '\0') {
            PlayerEncryptionKey[i] = distance + 'A'; // Convert distance back to a character
            PlayerEncryptionKey[i + 1] = replacement;
            printf("PlayerKey populate: %c%c",distance, replacement);
            break;
        }
    }

}


void initialization(){
    srandom(time(NULL));
    Puzzle = getPuzzle();
    //Getting the original string 
    if (Puzzle != NULL){
    for(int i = 0; i < 26;i++){
        EncryptionKey[i] = Puzzle[i];
        EncryptionKey[i] = toupper(EncryptionKey[i]);
    }
    //Shuffling using Fishers algorithm
    shuffle(EncryptionKey);
    
    //populates the player key
    for(int i =0; i < 26;i++){
        PlayerEntered[i] = '\0';
    }
   size_t encryptedStringLength = strlen(Puzzle) + 1;
    pEncryptedString = (char*) malloc(encryptedStringLength);
    if (pEncryptedString == NULL){
        printf("Memory not allocated. \n");
        exit(0);
    }
    //ensuring its empty
   pEncryptedString[0] = '\0';

    for (int i = 0; i < strlen(Puzzle);i++){
        //check if current character is a letter, then makes it upper case
        if (isalpha(Puzzle[i])){

            char upperCase = toupper(Puzzle[i]);
            for (int j =0; j < strlen(EncryptionKey);j++){
                if (upperCase == EncryptionKey[j]){
                    //appends to the encrypted string
                    char append[2] = {upperCase, '\0'}; 
                    strcat(pEncryptedString, append);
                    pEncryptedString[encryptedStringLength - 1] = '\0';
                }
            }
        }
        
    }

    
    }

}

bool isGameOver(){


    bool GameOver = true;
    /*
        I Think that this logic may need to be reworked 
        we are just using this to check if the game 
    */
    for (int i = 0; i < strlen(pEncryptedString); i++) {
        // Check if it's a letter
        if (isalpha(pEncryptedString[i])) {
            bool found = false; 
            // Loop through the player's key
            for (int j = 0; PlayerEntered[j] != '\0'; j += 2) {
                // Check if the encrypted letter matches any in the player's key
                if (PlayerEntered[j + 1] == pEncryptedString[i]) {
                    found = true;
                    
                }
            }
            
            // If not found, print underscore
            if (!found) {
                GameOver = false;
                break;
            }
        } 
    }
    
    return GameOver;
}

void teardown(void){
    free(pEncryptedString);
    printf("All Done\n");
}



/*
    Webserver Part

*/


/*
The server method 
Handles creating and binding the socket then returns the socket 

*/
int server(){
    
    int Socket;
    struct addrinfo hints, *servinfo ,*current;
    memset (&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((getaddrinfo(NULL, "8000", &hints, &servinfo)) != 0){
        perror("getaddrinfo");
        printf("Error number: %d\n", errno);
        return -1;
    }

    for (current = servinfo; current != NULL; current = current->ai_next){
        //checking if its IPv4
      if (current->ai_addr->sa_family == AF_INET){
            
            //create socket here
             if ((Socket = socket(current->ai_family, current->ai_socktype, current->ai_protocol)) == -1){
                 perror("socket creation:");
                continue;
             }
               if (bind(Socket, current->ai_addr, current->ai_addrlen) ==-1){
                perror("binding socket:");
                close(Socket);
                continue;
            }

            break;
             
             
        } 
        //checking if its IPv6
        if(current->ai_addr->sa_family == AF_INET6) {
            
            if ((Socket = socket(current->ai_family, current->ai_socktype, current->ai_protocol)) == -1){
                perror("socket creation:");
                continue;
             }
            if (bind(Socket, current->ai_addr, current->ai_addrlen) == -1){
                perror("binding socket:");
                close(Socket);
                continue;
            }
          
            break;
             
        }
    }
        freeaddrinfo(servinfo);
        if (Socket == -1 ){
            printf("404 Not Found ");
            perror("Socket");
        }
         
        
      return Socket;

}


void displayWorld(int cSocket){
    
    char display[1000];
    int index = 0;
    bool found = false;
    char Decrypted[26];
    for (int i =0; i < 26; i++){
        Decrypted[i] = '_';
    }

    for (int i = 0; i < strlen(pEncryptedString); i++){

        if (isalpha(pEncryptedString[i])){

            for (int j = index; PlayerEntered[j] != '\0';j +=2 ){
                if (PlayerEntered[j + 1] == pEncryptedString[i]){
                    Decrypted[j] = pEncryptedString[i];
                    found = true;
                    break;
                }
            }

        } else{
            Decrypted[index] = pEncryptedString[i];
        }

    }


    sprintf(display,"<html>"
            "<body>"
            "Encrypted: %s<br>"
            "Decrypted: %s<br>"
            "<form action=\"crypt\" method=\"GET\">"
            "<input type=\"text\" name=\"move\" autofocus maxlength=\"2\">"
            "</form>"
            "</body>"
            "</html>",
            pEncryptedString,
            Decrypted);

    char *msg200 = "HTTP/1.1 200 OK\r\ncontent-type: text/html; charset=UTF-8 \r\n\r\n";
    if ((send(cSocket, msg200, strlen(msg200), 0)) ==  -1){
        perror("Error sending display response");
        exit(0);
    } // Send HTTP header
    if((send(cSocket, display, strlen(display), 0)) == -1){
         perror("Error sending display message");
         exit(0);
    } // Send HTML content
    return;
}



void handleGame(char*Path, int cSocket){
    char* move = "crypt?";
    char* newPuzzle = "crypt";
    bool hasQuestionMark = false;
    char Move[3] = {'\0', '\0', '\0'};
    for (int i =0; Path[i] != '\0'; i++){
        if (strncmp(&Path[i], move, strlen(move)) == 0){
            hasQuestionMark = true;
            printf("%s\n", move);
            for (int j = 0; j < strlen(Path); j++){
                if (Path[j] == '='){
                    Move[0] = Path[j+1];
                    Move[1] = Path[j+2];
                } 
            }
        }
        if ((strncmp(&Path[i], newPuzzle, strlen(newPuzzle)) == 0) && !(hasQuestionMark)){
            printf("%s\n", newPuzzle);
            getPuzzle();
            displayWorld(cSocket);
            return;
        }
    }
    if(hasQuestionMark){
        int distance = Move[0] - 'A';
        char replacement = Move[1];
    printf("Word: %d, %c, %s \n",distance, replacement, Move);
    populatePlayerKey(PlayerEntered,distance,replacement);
    }
    
    //since the print statements working i thinkn that populatePlayerKey is causing the seg fault
    
    //Newly added 
    if (isGameOver()){
        char *msg200 = "HTTP/1.1 200 OK\r\ncontent-type: text/html; charset=UTF-8 \r\n\r\n";
        char gameOverPage[] = 
           "<html><body>Congratulations! You solved it! <a href=\"crypto\">Another?</a></body></html>";

            if((send(cSocket, msg200, strlen(msg200), 0)) == -1 ){
                perror("Error sending gameOver response");
                exit(0);
            } // Send HTTP header
            if((send(cSocket, gameOverPage, strlen(gameOverPage), 0)) == -1){
                perror("Error sending gameOver message");
                exit(0);
            } // Send HTML content
            teardown();

            return;
        //need to do the same as response but the actual path would just be the exitPage
    } else {
        displayWorld(cSocket);
        return;
    }

}
bool checkUrl(char*Path, int cSocket){
    char* crypto = "crypto";
    char* crypt = "crypt";
    for (int i =0; Path[i] != '\0'; i++){
        //check if the url starts with crypto then call handleGame
        if ((strncmp(&Path[i], crypto, strlen(crypto)) == 0) || (strncmp(&Path[i], crypt, strlen(crypt)) == 0)) {
            printf("Cryptogram");
            handleGame(Path, cSocket);
            return true;
        }
    }
    return false;
}



/*
    The response method 
    Takes Path and socket as parameters
    concatenates path with the filepath thats used to serve files
    then attempts to open the file 
    if the file exists prints 200 else 404 
    Then creates the response and sends the file to the client then closes the file

*/
void response(char *Path, int cSocket){
    //checking for crypto
    if (checkUrl(Path, cSocket)){
        return;
    }

    
    int fd;
    ssize_t readFile;
    char actualPath[500];
    struct stat file_stat;
    sprintf(actualPath, "%s%s",filepath,Path);

    fd = open(actualPath, O_RDONLY);
    char response[500];
    if (fd == -1){
        printf("404 Not Found at path %s\n", actualPath);
        perror("open:");
        exit(1);
    }

    if (stat(actualPath, &file_stat) == -1) {
        perror("Error getting file information");
        exit(1);
    }
    printf("200 OK\n");
    sprintf(response, "HTTP/1.0 200\r\nContent-Length: %ld\r\n\r\n",file_stat.st_size);
    char buffer[file_stat.st_size];
    printf("Response: %s \n", response);
    if (send(cSocket, response, strlen(response),0) == -1){
        perror("send");
        exit(1);
    }
     while ((readFile = read(fd,buffer,sizeof(buffer))) > 0)
    {
        if (send(cSocket, buffer, strlen(buffer),0) == -1){
            perror("Error sending file");
            exit(1);
        }
    }
     if (readFile == -1) {
            perror("Error reading file");
            exit(1);
    }  

    
    if (close(fd) == -1){
        perror("Error Could Not Close File\n");
    }   
    
}

/*
    ParseGet parses the request
    then calls response passing the path and socket as parameters
*/
void *parseGet(void *r){
    struct ParseRequest * Request = (struct ParseRequest*) r;
    char* request = Request->request;
    int count = 0;
    int startIndex, endIndex;
    
   /*
        Properly getting the start and endIndex of the Path 
   */
    for (int i = 0; i < strlen(request);i++){
        if (request[i] == ' '){
            count++;
        }
        if (request[i] == '/'){
            startIndex = i+1;
        }

        if (count == 2){
            endIndex = i;
            break;
        }
    }
    /*
        storing the path within the path variable 
    */
    char Path[endIndex - startIndex +1];
    int index = 0;
    for (int i = startIndex; i < endIndex;i++,index++){
        Path[index] = request[i];
        count++;
    }
    //null terminate 
    Path[index] = '\0';
    Request->request = Path;
    response(Request->request,Request->clientSocket);
    return NULL;
}



int main(int argc, char *argv[]){
    pthread_t thread;
    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    int backlog = 10;
    int new_fd;
    int recieve;
    struct ParseRequest request;

    if (argc >=2 ){
        filepath = argv[1];
        int Clientsocket = server();
        char pathBuffer[1000];

        if (listen(Clientsocket, backlog) == -1){
            perror("listen");
            printf("Error number: %d\n", errno);
            exit(1);
        }
        while (1)
        {
            sin_size = sizeof(client_addr);
            LoadPuzzles();
            initialization();

           if ((new_fd = accept(Clientsocket, (struct sockaddr *) &client_addr, &sin_size)) == -1){
                printf("Socket: %d\n",new_fd);
                perror("Accept");
                printf("Error number: %d\n", errno);
                exit(1);
            }
            //recieving the request from the client
              if ((recieve = recv(new_fd, pathBuffer, sizeof(pathBuffer), 0)) == -1){
                perror("Failed to Recieve From Client: ");
                printf("Error number: %d\n", errno);
            }
            printf("path: %s\n",pathBuffer);

            request.request = pathBuffer;
            request.clientSocket = new_fd;
            //creating new thread 
            if ((pthread_create(&thread, NULL,parseGet, &request)) != 0 ){
                perror("Creating thread");
                close(new_fd);
                continue;
            }
            FreeQuotes();

        }

    } else{
        printf("No path entered\n");
    }
}