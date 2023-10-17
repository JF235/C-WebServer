#include "../includes/essentials.h"

////////////////////////////////////////////////
// Options
////////////////////////////////////////////////

OptionList *createOptionList()
{
    OptionList *optionList = (OptionList *)malloc(sizeof(OptionList));
    if (optionList == NULL)
    {
        perror("Erro ao alocar memória para a lista de opções");
        exit(EXIT_FAILURE);
    }
    optionList->head = NULL;
    optionList->tail = NULL;
    return optionList;
}

void addOption(OptionList *optionList, char *optionName)
{
    Option *newOption = (Option *)malloc(sizeof(Option));
    if (newOption == NULL)
    {
        perror("Erro ao alocar memória para a opção");
        exit(EXIT_FAILURE);
    }
    strcpy(newOption->optionName, optionName);

    // O novo elemento não aponta para ninguém
    newOption->nextOption = NULL;
    if (optionList->head == NULL)
        // Quando a lista está vazia, tail e head apontam para o novo elemento
        optionList->head = newOption;
    else
        // Quando a lista tem elementos, tail aponta para o novo elemento
        optionList->tail->nextOption = newOption;
    // tail se torna o novo elemento
    optionList->tail = newOption;
}

void printOptionList(OptionList *optionList)
{
    Option *currentOption = optionList->head;
    int first = 1;
    printf("<");
    while (currentOption != NULL)
    {
        if (first)
        {
            printf("\"%s\"", currentOption->optionName);
            first = 0;
        }
        else
        {
            printf(",\"%s\"", currentOption->optionName);
        }
        currentOption = currentOption->nextOption;
    }
    printf(">");
}

void freeOptionList(OptionList *optionList)
{
    Option *currentOption = optionList->head;
    while (currentOption != NULL)
    {
        Option *tempOption = currentOption;
        currentOption = currentOption->nextOption;
        free(tempOption);
    }
}

////////////////////////////////////////////////
// Commands
////////////////////////////////////////////////

CommandList *createCommandList()
{
    CommandList *commandList = (CommandList *)malloc(sizeof(CommandList));
    if (commandList == NULL)
    {
        perror("Erro ao alocar memória para a lista de comandos");
        exit(EXIT_FAILURE);
    }
    commandList->head = NULL;
    return commandList;
}

void addCommand(CommandList *commandList, char *commandName)
{
    Command *newCommand = (Command *)malloc(sizeof(Command));
    if (newCommand == NULL)
    {
        perror("Erro ao alocar memória para o comando");
        exit(EXIT_FAILURE);
    }
    strcpy(newCommand->commandName, commandName);

    newCommand->optionList.head = NULL;
    newCommand->optionList.tail = NULL;

    // O novo elemento não aponta para ninguém
    newCommand->nextCommand = NULL;
    if (commandList->head == NULL)
        // Quando a lista está vazia, tail e head apontam para o novo elemento
        commandList->head = newCommand;
    else
        // Quando a lista tem elementos, tail aponta para o novo elemento
        commandList->tail->nextCommand = newCommand;
    // tail se torna o novo elemento
    commandList->tail = newCommand;

    // newCommand->nextCommand = commandList->head;
    // commandList->head = newCommand;
}

void printCommandList(CommandList *commandList)
{
    Command *currentCommand = commandList->head;
    while (currentCommand != NULL)
    {
        printf("Comando: %s\n", currentCommand->commandName);
        printf("\tOpções: ");
        printOptionList(&(currentCommand->optionList));
        printf("\n");
        currentCommand = currentCommand->nextCommand;
    }
}

void freeCommandList(CommandList *commandList)
{
    Command *currentCommand = commandList->head;
    while (currentCommand != NULL)
    {
        Command *tempCommand = currentCommand;
        currentCommand = currentCommand->nextCommand;
        freeOptionList(&(tempCommand->optionList));
        free(tempCommand);
    }
    free(commandList);
}

struct Command *findCommand(char *commandName, CommandList *commandList){
    Command *currentCommand = commandList->head;
    while (currentCommand != NULL)
    {
        if(!strcmp(currentCommand->commandName, commandName))
            return currentCommand;
        
        currentCommand = currentCommand->nextCommand;
    }
    return NULL;
}