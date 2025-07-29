#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "list.h"

extern char **lst;
int static i = 0;
extern jmp_buf begin;
char static *buf;
int skobka = 0;
int num = -1;
int er = 0;

typedef struct cmd_inf *tree;
typedef struct cmd_inf 
{
    char ** argv; // список из имени команды и аргументов
    char *infile; // переназначенный файл стандартного ввода
    int typeout; // =1 если >> 
    char *outfile; // переназначенный файл стандартного вывода
    int backgrnd; // =1, если команда подлежит выполнению в фоновом режиме cmd_inf* psubcmd; // команды для запуска в дочернем shell
   // int num; // кол-во процессов в pr1 | pr2 | pr3...
    tree psubcmd; //запуск дочернего
    tree pipe; // следующая команда после “|”
    tree next_one; // следующая после “;” (или после “&”)
    tree next_two; //следущая после "||"
    tree next_three; //следующая после  "&&"
}info;




tree null_tree();
tree postr();
void clear_tree(tree tr);

tree tr = NULL;

tree create_tree()
{
    i = 0;
    skobka = 0;
    //tree tr = NULL;
    if (strcmp(lst[0], ">") == 0 || strcmp(lst[0], ">>") == 0 || 
        strcmp(lst[0], "<") == 0 || strcmp(lst[0], "|") == 0 ||
        strcmp(lst[0], "||") == 0 || strcmp(lst[0], "&&") == 0 ||
        strcmp(lst[0], "&") == 0 || strcmp(lst[0], ";") == 0)
        {
            er = 1;
            printf("ОШИБКА: не указан исполняемый файл\n");
            longjmp(begin,1);
        }
    tr = postr();
    if (er) 
    {
        clear_tree(tr);
        tr = NULL;
        er = 0;
        longjmp(begin,1);
    }
    return tr;
}



tree null_tree()
{
    tree t = malloc(sizeof(info));
    t->argv = NULL;
    t->infile = NULL;
    t->infile = NULL;
    t->typeout = 0;
    t->outfile = NULL;
    t->backgrnd = 0;
    t->psubcmd = NULL;
    t->pipe = NULL;
    t->next_one = NULL;
    t->next_two = NULL;
    t->next_three = NULL;
    return t;
}


tree postr()
{
    tree t = NULL;
    //printf("%d", i);
    if (lst[i] == NULL || strcmp(lst[i], ">") == 0 || strcmp(lst[i], ">>") == 0 || 
        strcmp(lst[i], "<") == 0 || strcmp(lst[i], "|") == 0 ||
        strcmp(lst[i], "||") == 0 || strcmp(lst[i], "&&") == 0 ||
        strcmp(lst[i], "&") == 0 || strcmp(lst[i], ";") == 0)
        {
            er = 1;
            printf("ОШИБКА: не указан исполняемый файл\n");
            clear_tree(t);
            return NULL;
        }

    if (strcmp(lst[i], ")") == 0)
    {
        er = 1;
        printf("ОШИБКА: нет открывающей скобки\n");
        clear_tree(t);
        return NULL;
    }


    int n = 1;
    int ind = 1;
    t = null_tree();
    t->argv = (char**) malloc(sizeof(char*));
    t->argv[0] = lst[i++];
    while (lst[i] != NULL && strcmp(lst[i], ">") != 0 && strcmp(lst[i], ">>") != 0 &&
        strcmp(lst[i], "<") != 0 && strcmp(lst[i], "|") != 0 &&
        strcmp(lst[i], "||") != 0 && strcmp(lst[i], "&&") != 0 &&
        strcmp(lst[i], "&") != 0 && strcmp(lst[i], ";") != 0 && strcmp(lst[i], ")") != 0 && strcmp(lst[i], "(") != 0)
    {
        t->argv = realloc(t->argv, (++n) * sizeof(char*));
        t->argv[ind++] = lst[i];
        i++;
    }
    t->argv = realloc(t->argv, (++n) * sizeof(char*));
    t->argv[ind] = NULL;

    if (lst[i] == NULL) return t;
    
    if (strcmp(lst[i], "<") == 0)
    {
        i++;
        if(lst[i] == NULL || strcmp(lst[i], ">") == 0 || strcmp(lst[i], ">>") == 0 || 
            strcmp(lst[i], "<") == 0 || strcmp(lst[i], "|") == 0 ||
            strcmp(lst[i], "||") == 0 || strcmp(lst[i], "&&") == 0 ||
            strcmp(lst[i], "&") == 0 || strcmp(lst[i], ";") == 0 || strcmp(lst[i], ")") == 0 || strcmp(lst[i], "(") == 0)
        {
            er = 1;
            printf("ОШИБКА: не указан входной файл\n");
            clear_tree(t);
            return NULL;
        }
        t->infile = lst[i];
        i++;
    }

    if (lst[i] == NULL) return t;
    if (strcmp(lst[i], ">") == 0)
    {
        i++;
        if(lst[i] == NULL || strcmp(lst[i], ">") == 0 || strcmp(lst[i], ">>") == 0 || 
            strcmp(lst[i], "<") == 0 || strcmp(lst[i], "|") == 0 ||
            strcmp(lst[i], "||") == 0 || strcmp(lst[i], "&&") == 0 ||
            strcmp(lst[i], "&") == 0 || strcmp(lst[i], ";") == 0 && strcmp(lst[i], ")") == 0 || strcmp(lst[i], "(") == 0)
        {
            er = 1;
            printf("ОШИБКА: не указан файл для вывода\n");
            clear_tree(t);
            return NULL;
        }
        t->outfile = lst[i];
        t->typeout = 0;
        i++;

        if (lst[i] == NULL) return t;

        if (strcmp(lst[i], ">>") == 0)
        {
            er = 1;
            printf("ОШИБКА: файл для вывода уже задан\n");
            clear_tree(t);
            return NULL;
        }

        if (strcmp(lst[i], "<") == 0)
        {
            er = 1;
            printf("ОШИБКА: неверный порядок символ ввода в файл и вывода из файла\n");
            clear_tree(t);
            return NULL;
        }
        
    }
    if (lst[i] == NULL) return t;

    
    if (strcmp(lst[i], ">>") == 0)
    {
        i++;
        if(lst[i] == NULL || strcmp(lst[i], ">") == 0 || strcmp(lst[i], ">>") == 0 || 
            strcmp(lst[i], "<") == 0 || strcmp(lst[i], "|") == 0 ||
            strcmp(lst[i], "||") == 0 || strcmp(lst[i], "&&") == 0 ||
            strcmp(lst[i], "&") == 0 || strcmp(lst[i], ";") == 0 && strcmp(lst[i], ")") == 0 || strcmp(lst[i], "(") == 0) 
        {
            er = 1;
            printf("ОШИБКА: не указан файл для вывода\n");
            clear_tree(t);
            return NULL;
        }
        t->outfile = lst[i];
        t->typeout = 1;
        i++;
        if (lst[i] ==  NULL) return t;
        if (strcmp(lst[i], "<") == 0) 
        {
            er = 1;
            printf("ОШИБКА: неверный порядок символ ввода в файл и вывода из файла\n");
            clear_tree(t);
            return NULL;
        }

        if (strcmp(lst[i], ">") == 0)
        {
            er = 1;
            printf("ОШИБКА: файл для вывода уже задан\n");
            clear_tree(t);
            return NULL;
        }

        if (lst[i] != NULL && strcmp(lst[i], "|") != 0 &&
        strcmp(lst[i], "||") != 0 && strcmp(lst[i], "&&") != 0
        && strcmp(lst[i], ";") != 0 && strcmp(lst[i], ")") != 0 && strcmp(lst[i], "(") != 0)
        {
            er = 1;
            printf("ОШИБКА: пропущен символ, связывающий команды\n");
            clear_tree(t);
            return NULL;
        }
    }

    if (lst[i] == NULL) return t;

    if (strcmp(lst[i], "<") == 0) 
    {
        er = 1;
        printf("ОШИБКА: файл для ввода уже задан\n");
        clear_tree(t);
        return NULL;
    }

    if (strcmp(lst[i], ">") == 0  || strcmp(lst[i], ">>") == 0)
    {
        er = 1;
        printf("ОШИБКА: файл для вывода уже задан\n");
        clear_tree(t);
        return NULL;
    }

    if (strcmp(lst[i], "&") == 0) 
    {
        t->backgrnd = 1;
        //i++;
    }
    else t->backgrnd = 0;

    if (lst[i] == NULL) return t;
    if (strcmp(lst[i], "|") == 0)
    {
        i++;
        t->pipe = postr();
        if (er) 
        {
            clear_tree(t);
            return NULL;
        }
        return t;
    }
    if (strcmp(lst[i], ";") == 0 || strcmp(lst[i], "&") == 0)
    {
        if (strcmp(lst[i], "&") == 0 && lst[i + 1] == NULL) return t;
        i++;
        t->next_one = postr();
        if (er) 
        {
            clear_tree(t);
            return NULL;
        }
        return t;
    }
    if (strcmp(lst[i], "||") == 0)
    {
        i++;
        t->next_two = postr();
        if (er) 
        {
            clear_tree(t);
            return NULL;
        }
        return t;
    }
    if (strcmp(lst[i], "&&") == 0)
    {
        i++;
        t->next_three = postr();
        if (er) 
        {
            clear_tree(t);
            return NULL;
        }
        return t;
    }
    if (strcmp(lst[i], ")") != 0)
    {
        er = 1;
        printf("ОШИБКА: нет открывающей скобки\n");
        clear_tree(t);
        return NULL;
    }

    return t;
}




void print_tree(tree tr)
{
    if (tr != NULL)
    {
        int j = 0;
        while(tr->argv[j] != NULL)
        {
            printf("%s ", tr->argv[j++]);
        }
        printf(" & %d; ", tr->backgrnd);
        printf(" () %d; ", tr->psubcmd?1:0);
        printf(" in: %s ", tr->infile);
        printf(" out: %s ", tr->outfile);
        printf(" type_out %d", tr->typeout);
        printf("\n");

        if (tr->pipe != NULL)
        {
            printf(" | \n"); print_tree(tr->pipe); return;
        }
        
        if (tr->next_one != NULL)
        {
            printf(" ; \n"); print_tree(tr->next_one); return;
        }


        if (tr->next_two != NULL)
        {
            printf(" ||  \n"); print_tree(tr->next_two); return;
        }


        if (tr->next_three != NULL)
        {
            printf(" && \n"); print_tree(tr->next_three); return;
        }
    }
}


void clear_tree(tree tr)
{
    if (tr != NULL)
    {
        clear_tree(tr->pipe);
        clear_tree(tr->psubcmd);
        clear_tree(tr->next_one);
        clear_tree(tr->next_two);   
        clear_tree(tr->next_three);  

        int g = 0;
        while (tr->argv[g] != NULL) tr->argv[g++] = "0";
        free(tr->argv);
        free(tr);
    }
}
