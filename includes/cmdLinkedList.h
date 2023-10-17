#ifndef CMDLINKEDLIST_H
#define CMDLINKEDLIST_H

#define MAX_COMMAND_NAME 128
#define MAX_OPTION_NAME 256

////////////////////////////////////////////////
// Options
////////////////////////////////////////////////

typedef struct Option {
    char optionName[MAX_OPTION_NAME];
    struct Option *nextOption;
} Option;

typedef struct OptionList {
    struct Option *head;
    struct Option *tail;
} OptionList;

// Cria uma lista de opções vazia
OptionList* createOptionList();

// Adiciona uma opção na lista
void addOption(OptionList *optionList, char *optionName);

// Imprime todas as opções da lista
void printOptionList(OptionList *optionList);

void freeOptionList(OptionList *optionList);

////////////////////////////////////////////////
// Commands
////////////////////////////////////////////////

typedef struct Command {
    char commandName[MAX_COMMAND_NAME];
    struct OptionList optionList;
    struct Command *nextCommand;
} Command;

typedef struct CommandList {
    struct Command *head;
    struct Command *tail;
} CommandList;

// Cria uma lista de comandos vazia
CommandList* createCommandList();

// Adiciona um comando na lista
void addCommand(CommandList *commandList, char *commandName);

// Imprime todos os comandos e suas respectivas opcoes
void printCommandList(CommandList *commandList);

// Esvazia todos os comandos e opcoes associadas.
void freeCommandList(CommandList *commandList);

// Busca pelo comando de nome `commandName`
struct Command *findCommand(char *commandName, CommandList *commandList);

#endif