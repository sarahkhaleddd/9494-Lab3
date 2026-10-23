
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"


void parse(std::vector<Token> &tokens)
{
    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    for (int i=0; i<tokens.size(); i++){
	if (tokens[i].type == TOKEN_EOF)
		break;
    if (tokens[i].type == TOKEN_COMMAND && tokens[i].value=="exit"){
        std::cout << "Goodbye!!" << std::endl;
        exit(0);
    }
    if(tokens[i].type==TOKEN_COMMAND&& tokens[i].value=="cd")
    {const char *path=NULL;
    if(i+1 < tokens.size() && tokens[i+1].type!=TOKEN_EOF && tokens[i+1].type!=TOKEN_BACKGROUND)
    {
        path=tokens[i+1].value.c_str();
    }
    else{
        path=getenv("HOME");
    }
    if(chdir(path)!=0){
        perror("cd");
    }
    currentCommand.clear();
    return;
}

	if (tokens[i].type == TOKEN_COMMAND || tokens[i].type == TOKEN_ARGUMENT){
	char *argument= strdup(tokens[i].value.c_str());
	currentSimpleCommand->insertArgument(argument);
	}
    else if (tokens[i].type ==TOKEN_REDIRECT || tokens[i].type ==TOKEN_APPEND ){
        if (i+1<tokens.size() && tokens[i+1].type != TOKEN_EOF){
            currentCommand._outFile=strdup(tokens[i+1].value.c_str());
            if (tokens[i].type==TOKEN_APPEND)
            currentCommand._append=1;

            i++;
        }
    }
    else if(tokens[i].type== TOKEN_ERROR)
    {
        if(i+1<tokens.size()&&tokens[i+1].type!=TOKEN_EOF){
        currentCommand._errFile=strdup(tokens[i+1].value.c_str());
            i++;
        }
    }
    else if(tokens[i].type==TOKEN_REDIRECT_AND_ERROR)
    {
    if(i+1<tokens.size()&&tokens[i+1].type!=TOKEN_EOF)
    {
        currentCommand._outFile=strdup(tokens[i+1].value.c_str());
        currentCommand._errFile=strdup(tokens[i+1].value.c_str());
        currentCommand._out_error=1;
        i++;
    }
    }
    else if(tokens[i].type== TOKEN_INPUT){
        if(i+1<tokens.size()&&tokens[i+1].type!=TOKEN_EOF){
            currentCommand._inputFile=strdup(tokens[i+1].value.c_str());
            i++;
        }
    }
    else if(tokens[i].type==TOKEN_PIPE){
         if (currentSimpleCommand->_numberOfArguments > 0)
        currentCommand.insertSimpleCommand(currentSimpleCommand);
        currentSimpleCommand= new SimpleCommand();
    }
    else if (tokens[i].type == TOKEN_BACKGROUND)
    {
    currentCommand._background = 1;
    }

    }
    if (currentSimpleCommand->_numberOfArguments >0){
	currentCommand.insertSimpleCommand(currentSimpleCommand);
}
    currentCommand.execute();
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _numberOfSimpleCommands = 0;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _out_error=0;
    _background = 0;
    _append=0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
   if (_numberOfSimpleCommands==0){
        clear();
        return;
    }
    print();
    int previousInput=-1;
    int pipefd[2];
    for(int i=0;i<_numberOfSimpleCommands;i++)
    {
    if(i<_numberOfSimpleCommands-1){
            if(pipe(pipefd)==-1)
            {
                perror("pipe");
                clear();
                return;
            }
        }
        pid_t pid=fork();
        if(pid==0){
            if(previousInput!=-1){
                dup2(previousInput,STDIN_FILENO);
                close(previousInput);
            }
            else if(_inputFile!=NULL)
            {
                int fd=open(_inputFile,O_RDONLY);
                if(fd<0)
                {
                    perror("open");
                    exit(1);
                }
                dup2(fd,STDIN_FILENO);
                close(fd);
            }
            if(i<_numberOfSimpleCommands-1)
            {
                dup2(pipefd[1],STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }
            if (_outFile != NULL && i == _numberOfSimpleCommands - 1)
            {
                int flags=O_WRONLY|O_CREAT;
                if(_append)
                    flags|=O_APPEND;
                else
                    flags|=O_TRUNC;
                int fd=open(_outFile,flags,0644);
                if(fd<0)
                {
                    perror("open");
                    exit(1);
                }
                dup2(fd,STDOUT_FILENO);
                if(_out_error)
                    dup2(fd,STDERR_FILENO);
                close(fd);
            }
            if(_errFile!=NULL&&!_out_error)
            {
                int fd=open(_errFile,O_WRONLY|O_CREAT|O_TRUNC,0644);
                if(fd<0)
                {
                    perror("open");
                    exit(1);
                }
                dup2(fd,STDERR_FILENO);
                close(fd);
}
            execvp(_simpleCommands[i]->_arguments[0],_simpleCommands[i]->_arguments);
            perror("execvp");
            exit(1);
        }
        else if(pid<0)
        {
            perror("fork");
            clear();
            return;
        }
        if(previousInput!=-1)
            close(previousInput);
        if(i<_numberOfSimpleCommands-1)
        {
            close(pipefd[1]);
            previousInput=pipefd[0];
        }
    }
    if(previousInput!=-1)
        close(previousInput);
    if(!_background)
    {
        for(int i=0;i<_numberOfSimpleCommands;i++)
            wait(NULL);
    }
    clear();
}

void Command::prompt()
{

    std::string input;
    while (true)
    {
        printf("myshell>");
        fflush(stdout);

        if(!std::getline(std::cin, input))
        {
            printf("\n");
            break;
        }
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;
void childHandler(int signal){
    int status;
    pid_t pid;
    while ((pid = waitpid(-1,&status, WNOHANG))>0){
        FILE *log=fopen("child.log","a");
        if(log!=NULL){
            fprintf(log, "Child process %d terminated\n", pid);
            fclose(log);
        }
    }
}
int main()
{
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, childHandler);
    Command::_currentCommand.prompt();
    return 0;
}
