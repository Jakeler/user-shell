void dumpContext(procContext* con);
void dumpStr(char* str);
char** processCmd(char* cmd, char** paras);
void executeProcess(char** parameters, procContext* con);
int parseConfig(config_file_t* cf, procContext* con);

