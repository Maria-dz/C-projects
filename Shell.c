#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#define WordBufSize 64
#define fsBufSize 1024
#define comBufSize 1000
#define STDOUT 1
#define STDIN 0
#define CONV 3
#define ENTER 4
#define ERROR 2
int ErrorInCommand = 0;
struct  CommandInfo {
    char ** args;
    int argCount;
    char *inpSource;
    char *outSource;
    struct CommandInfo *next;
    char *commandSource;
    int qFlag;
    int writeMode;
    int readMode;
    int error;

};


void printingdir(void){
    char *pathname = NULL;
    char buffer[PATH_MAX];
    if ((pathname = getcwd(buffer,PATH_MAX)) != NULL){
        fprintf(stderr, "%s ", buffer);
    }
    else perror("Error in getcwd");
}

char getNextChar ( char **commandString, int *commandPos){
    char nextCh = (*commandString)[(*commandPos)];
    (*commandPos)++;
    return nextCh;
}

char getWord(char **word, int *wordPos, int *wordBuf, char **commandString, int *commandPos){
    char ch;
    int quote = 0;
    (*word) = malloc((*wordBuf)*sizeof(char));
    (*word)[0] = '\0';
    while (isspace(ch = getNextChar(commandString, commandPos)) && (ch != '\n') && (ch != ';')) ;
    while (((!isspace(ch)) && (ch != '>') && (ch != '<') && (ch != '\0') && (ch != '&')&& (ch != ';')) || quote){ //////
        if ((ch != '"')){
            (*word)[(*wordPos)] = ch;
            (*wordPos) ++;
        }
        else {
            if (quote) {
                quote=0;
            }
            else quote++;
        }
        if ((*wordPos) >= (*wordBuf)){
            (*wordBuf) += WordBufSize;
            (*word) = realloc((*word), (*wordBuf) * sizeof(char));
        }
        ch = getNextChar(commandString, commandPos);
    }
    if ((*wordPos) >= (*wordBuf)){
        (*wordBuf) ++;
        (*word) = realloc((*word), (*wordBuf) * sizeof(char));
    }
    if (ch == '\0') {
        (*commandPos)--;
    }
    (*word)[(*wordPos)] = '\0';

    return ch;
}



void createOutSource(char **outSource, int *writeMode,  char *word, char ch, int *error) {
    if ((ch == '>') && (word[0]=='\0')){
        (*writeMode) = 2;
    }
    else if ((ch != '>') && (*writeMode != 2)){
        if (*writeMode){
            *error = 1;
            return;
        }
        (*writeMode) = 1;
    }
    else if (ch == '>'){
        *error = 1;
        return;
    }
    if (word[0] != '\0' && (*outSource == NULL))  {
        (*outSource) = malloc(strlen(word) + 1);

        (*outSource) = strcpy((*outSource), word);
    }

}

void createInpSource(char **inpSource, int *readMode,  char *word, int *error) {
    if ((*readMode) != 1) {
        (*readMode) = 1;
        (*inpSource) = malloc(strlen(word) + 1);

        (*inpSource) = strcpy((*inpSource), word);
    }
    else{
        *error = 1;
        return;
    }
}

void checkArgs(char ***args, int argPos, int *error){
	int i;
	for (i = 0; i < argPos; i++){
		if ((strcmp((*args)[i],"&") == 0) && ((i + 1) != argPos)) *error = 1;
	}
}
void getArgs(char **commandString, char ***args, int *argCount, char **inpSource, char **outSource, int *writeMode, int *readMode, int *error, int *qFlag){
    int wordBuf = WordBufSize;
    char *word;

    int wordPos = 0;
    int argPos = 0;
    int argBuf = 1;
    char ch;
    int isNextWordArg = 1;
    char prevChar = '\0';
    int commandPos = 0;
    while ((*commandString)[commandPos] != '\0'){
		
        ch = getWord(&word, &wordPos, &wordBuf, &(*commandString), &commandPos);        
        if ((prevChar == '<') || (prevChar == '>')) isNextWordArg = 0;
        else isNextWordArg = 1;

        if ((isNextWordArg) && (word[0] != '\0')){
            (*args)[argPos] = malloc(strlen(word) + 1);

            (*args)[argPos] = strcpy((*args)[argPos],word);
            (*argCount) ++;
            argPos++;

            if (argPos >= argBuf){
                argBuf ++;
                (*args) = realloc((*args),argBuf * sizeof(char*));
            }
        }
        if (ch == '&'){
			*qFlag = 1;
            char *wordAnd = malloc(2);

            wordAnd[0] = '&';
            wordAnd[1] = '\0';
            (*args)[argPos] = malloc(strlen(wordAnd)+1);

            (*args)[argPos] = strcpy((*args)[argPos], wordAnd);
            (*argCount) ++;
            argPos++;

            if (argPos >= argBuf){
                argBuf ++;
                (*args) = realloc((*args),argBuf * sizeof(char*));
            }
            free(wordAnd);

        }
        if (prevChar == '>'){
            createOutSource(outSource, writeMode, word, ch, error);
        }
        if (prevChar == '<'){
            createInpSource(inpSource, readMode, word, error);
        }

        prevChar = ch;
        free(word);

        wordPos = 0;
        wordBuf = WordBufSize;
    }
    (*args)[argPos] = NULL;
    (*argCount)++;
    checkArgs(args, argPos , error);
}

int NotFirst = 0;
int SecondIfFirst = 0;

int readFullString (char **fullstring){
    char ch;
    int quote = 0;
    int fsPos = 0;
    int fsBuf = fsBufSize;
    (*fullstring) = malloc((fsBuf)*sizeof(char));
    ch = '\0';
    while (((ch != '\n') && (ch !=';')) || quote){
        ch = getchar();
        if ((ch == '|') && !quote){		
			if ((ch = getchar()) == '|'){
				(*fullstring)[(fsPos)] = ';';
				ch = ';';
				NotFirst = 1;
				break;
			}
			else {
				if ((fsPos) >= (fsBuf)){
					(fsBuf) += fsBufSize;
					(*fullstring) = realloc((*fullstring), (fsBuf) * sizeof(char));
				}

				(*fullstring)[(fsPos)] = '|';
				(fsPos)++;
				if ((ch == '\n') || (ch == ';')) break;
				(*fullstring)[(fsPos)] = ch;
				(fsPos)++;
				ch = getchar();	
			}
		}
		if ((ch == '&') && !quote){		
			if ((ch = getchar()) == '&'){
				(*fullstring)[(fsPos)] = ';';
				SecondIfFirst = 1;
				ch = ';';
				break;
			}
			else {
				if ((fsPos) >= (fsBuf)){
					(fsBuf) += fsBufSize;
					(*fullstring) = realloc((*fullstring), (fsBuf) * sizeof(char));
				}

				(*fullstring)[(fsPos)] = '&';
				(fsPos)++;
				if ((ch == '\n') || (ch == ';')) break;
				(*fullstring)[(fsPos)] = ch;
				(fsPos)++;
				ch = getchar();
				
			}
		}
        if ((ch == EOF) && (fsPos != 0)){
            continue;
        }

        if (ch == '"'){
            if (quote){
                quote = 0;
            }
            else quote ++;
        }

        if (ch == EOF) {
            (*fullstring) = strcpy((*fullstring), "exit\n");
            return ch;
        }
        if ((fsPos) >= (fsBuf)){
            (fsBuf) += fsBufSize;
            (*fullstring) = realloc((*fullstring), (fsBuf) * sizeof(char));
        }

        (*fullstring)[(fsPos)] = ch;
        (fsPos)++;
    }
    return ch;
}
char *getSubstring(char **curstring, int startPos, int lastPos){
    int i;
    char *command = malloc((lastPos-startPos+1)*sizeof(char));

    for (i = startPos; i < lastPos; i++){
        command[i - startPos] = (*curstring)[i];
    }
    command[lastPos-startPos] = '\0';
    return command;
}

void initNode( char *command, struct CommandInfo *node) {
    node->commandSource = (command);
    node->next = NULL;
    node->inpSource = NULL;
    node->outSource = NULL;
    node->args = (char**)malloc(sizeof(char*));

    node->argCount = 0;
    node->writeMode = 0;
    node->readMode =0;
    node->error = 0;
    node->qFlag = 0;
}

void fillNode( char *command, struct CommandInfo *node){
    getArgs(&command, &(node->args), &(node->argCount), &(node->inpSource), &(node->outSource),&(node->writeMode), &(node->readMode), &(node->error),&(node->qFlag));
}

struct CommandInfo *parseCommand(char *command){
    struct CommandInfo *node = malloc(sizeof(struct CommandInfo));

    initNode(command, node);
    fillNode(command, node);
    return node;
}



void clearCommandList(struct CommandInfo *CommandList){
    if (CommandList == NULL) {
        return;
    }
    struct CommandInfo *CurPointer = (CommandList);
    if (CurPointer->next != NULL) {
        clearCommandList((CurPointer->next));
        CurPointer->next = NULL;
    }
    free(CurPointer->commandSource);


    if (CurPointer->outSource != NULL) {
        free(CurPointer->outSource);
    }
    if (CurPointer->inpSource != NULL) {
        free(CurPointer->inpSource);
    }
    if (CurPointer->argCount != 0) {
        for (int i = 0; i < CurPointer->argCount; i++) {

            free((CurPointer->args)[i]);
        }
    }
    free(CurPointer->args);
    free(CurPointer);

}

int parseString(char **fullstring, struct CommandInfo **CommandList){
    //заполняем структуру данными
    int i = 0;
    int lastPos = 0;
    int startPos = 0;
    int quote = 0;
    char *command;
    struct CommandInfo *CurPointer;
    (*CommandList) = NULL;
    while (((*fullstring)[lastPos] != '\n')&&((*fullstring)[lastPos] !=';')){

        while ((((*fullstring)[i] != '|') && ((*fullstring)[i] != '\n') && (*fullstring)[i] != ';')||quote){
            if ((*fullstring)[i] == '"'){
                if (quote){
                    quote = 0;
                }
                else quote ++;
            }
            i++;
        }
        lastPos = i;
        command = getSubstring(&(*fullstring), startPos, lastPos);
        startPos = i+1;
        struct CommandInfo *commandNode = parseCommand(command);
        if ((commandNode->qFlag) && ((*fullstring)[lastPos] != '\n')) commandNode->error = 1;


        if ((*CommandList) == NULL) {
            (*CommandList) = commandNode;
            CurPointer = commandNode;
        }
        else{
            CurPointer->next = commandNode;
            CurPointer = commandNode;
        }

        if (commandNode->error > 0) {
            fprintf(stderr, "Error in command\n");
            return ERROR;
        }

        i++;

    }

    return 0;
}

int openOutput (char *name, int openMode){
    if (openMode == 2) {
        return open(name, O_WRONLY|O_CREAT|O_APPEND, 0777);
    }
    else return open(name,O_WRONLY|O_CREAT|O_TRUNC, 0777);
}
int openInput (char *name){
    return open(name, O_RDONLY);
}
int execCommandNode(struct CommandInfo *curNode, int fd[2], int qMode) {
    int inF = 0, outF = 0;
    pid_t ret;
    pipe(fd);
	
    switch(ret=fork())
    {
        case -1: return 1;
        case 0 :
			if (!qMode){
				signal(SIGINT, SIG_DFL); 
			}
            if (curNode->writeMode != 0) {
                int outF = openOutput(curNode->outSource,curNode->writeMode);
                dup2(outF, 1);
            }
            else if ((curNode->next != NULL)){
                dup2(fd[1],1);
            }

            if (curNode->readMode != 0){
                inF = openInput(curNode->inpSource);
                dup2(inF, 0);
            } 
            
            if (inF) close (inF);
            if (outF) close(outF);
            close(fd[0]);
            close(fd[1]);
            if (!strcmp((curNode->args)[0],"cd")){
                char *homedir;
                if (((curNode->args)[1] == NULL) || !strcmp((curNode->args)[1],"~")){
                    homedir = strdup(getenv("HOME"));
                    chdir(homedir);
                    free(homedir);
                    homedir = NULL;
                }
                else{
                    if (chdir((curNode->args)[1]) == -1 ){
                        perror("Error in chdir");
                    }
                }
            }
            else{
                execvp((curNode->args)[0], curNode->args);
                perror("Error in exec");
                exit(1);
            }
    }
    if (curNode->readMode != 0){
        inF = openInput(curNode->inpSource);
        dup2(inF, 0);
    }
    else if(curNode->next != NULL){
        dup2(fd[0], 0);
    }
    close(fd[0]); close(fd[1]);
	
    return ret;
}

void prKiller(int sig){
	exit (0);
}


int execCommands (struct CommandInfo *commandList){
    int fd[2];
    int qMode = 0;
    int answer = 0;
    struct CommandInfo *curNode = commandList;
    struct CommandInfo *curCommand;
	curCommand = commandList; 
	while (curCommand->next != NULL){
		curCommand = curCommand->next;
	}
	
	if (curCommand->qFlag){
		qMode = 1;
		free((curCommand->args)[curCommand->argCount - 1]);
		free((curCommand->args)[curCommand->argCount - 2]);
		(curCommand->args)[curCommand->argCount - 2] = NULL; 
		curCommand->argCount --;
	}
		int fpid;
		int pid;
		int lpid;
		int counter=0;
		if ((commandList == NULL) || (curCommand->argCount == 0)) {
			return 0;
		}
		do{
			if ((pid = execCommandNode(curNode, fd, qMode)) != ERROR){
				counter ++;
			}
			if (counter == 1) fpid = pid;
				curNode = curNode->next;		
		}
		while (curNode != NULL);
		lpid = pid;
		if (!qMode){
			int i = 0;
			while (counter != 0){
				for ( i = fpid; i <= lpid; i++){
					int status;
					if (waitpid(i, &status, 0) >= 0){
						if (WEXITSTATUS(status) != 0) answer = 1;
						counter --;
					}
				}
			}
		}
		
	return answer;
}	


int main(){
    int stdIn = dup(0);
    int stdOut = dup(1);
    signal(SIGINT, SIG_IGN);

    while(1){
		NotFirst = 0;
		SecondIfFirst = 0;
		ErrorInCommand = 0;
		
		int pid; int status;
		while ((pid =  waitpid(-1, &status, WNOHANG)) > 0 ){
			if (WIFEXITED(status)){
				fprintf (stderr, "%d done with status %d\n", pid, WEXITSTATUS(status));
			}
			else if (WIFSIGNALED(status)){
				fprintf (stderr, "%d exited with status %d\n", pid, WTERMSIG(status));
			}
		}
        printingdir();

        char *fullString;
        int exitCh;
        struct CommandInfo *commandList;
        exitCh = readFullString(&fullString);  // **
        printf("+++ %d\n", NotFirst);

        int countCom = 0;
        do{
            countCom ++;
            if (countCom != 1){
                exitCh = readFullString(&fullString);  // **
                if (NotFirst && !ErrorInCommand) continue;
                if (SecondIfFirst && ErrorInCommand) continue;
            }
            int parseStatus = 0;
            parseStatus = parseString (&fullString, &commandList);
            if (parseStatus == ERROR){
                ErrorInCommand = 1;
                clearCommandList(commandList);
                free(fullString);
                continue;
            }

            if ((commandList != NULL) && (commandList->next == NULL) &&  ((commandList->args)[0] != NULL) && (strcmp((commandList->args)[0], "exit") == 0)){
                clearCommandList(commandList);
                free(fullString);
                return 0;
            }

            if ((parseStatus != ERROR) && (commandList != NULL)){
                ErrorInCommand = execCommands(commandList);

            }

            clearCommandList(commandList);
            free(fullString);

            dup2(stdOut, 1);
            dup2(stdIn, 0);
        } while (exitCh != '\n');
    }
}
