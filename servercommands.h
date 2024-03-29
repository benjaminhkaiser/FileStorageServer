#BEN KAISER
#ifndef SERVCMDS_H_INCLUDED
#define SERVCMDS_H_INCLUDED

#define BUFFER_SIZE 1024

//Helpers
int fileExists(const char* file);
void parseFilename(char cmd[100], char* filename);
int parseSize(char* cmd);
int prefixMatch(char* string, char* searchstring);
void concatData(char* dest, char* source, int len);

//Server commands
char* addFile(int size, char* filename, char** data, int thread_id);
char* updateFile(int size, char* filename, char** data, int thread_id);
int readFile(char* filename, char** data, int thread_id);

#endif