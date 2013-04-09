#ifndef SERVCMDS_H_INCLUDED
#define SERVCMDS_H_INCLUDED

#define BUFFER_SIZE 1024

//Helpers
void parseCommand(char buffer[BUFFER_SIZE], char *cmd[]);
int fileExists(const char* file);

//Server commands
char* addFile(char *cmd[], int argc, int thread_id);
char* updateFile(char *cmd[], int argc, int thread_id);
char* readFile(char *cmd[], int argc, char* buffer, char* len, int thread_id);

#endif