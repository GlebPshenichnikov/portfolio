#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include "list.h"
#include "tree.h"
#include "exec.h"

#define BUFF_SIZE 10
jmp_buf begin;
List lst = NULL; // текущая лексема
//backgrndList *bckgrnd; // список фоновых процессов
int exit_val = 0;
tree static tr = NULL;

bckgrnd bk = NULL;

void inv() {
    printf("%s", "\x1b[32m"); /*здесь изменяется цвет на зеленый */
    char s[100]; /* ограничение: имя хоста и текущей директории не должно быть слишком длинным! */ gethostname(s, 100);
    printf("%s@%s", getenv("USER"), s);
    printf("%s", "\x1B[37m"); /* здесь изменяется цвет на серый */
    //getcwd(s, 100);
    printf(" %% ");
}

void zombi()
{
    if (bk == NULL) return;

    intlist *p = bk, *g;

    if(waitpid(p->pid, NULL, WNOHANG)) 
    {
        kill(p->pid, SIGKILL);
        bk = bk -> next;
        free(p);
        zombi();
    }

    if (p->next == NULL) return;
    while (p->next != NULL)
    {
        if(waitpid(p->next->pid, NULL, WNOHANG)) 
        {
            kill(p->next->pid, SIGKILL);
            g = p->next;
            p -> next = p->next->next;
            free(g);
        }
        else p = p->next;
    }
}

void jdem()
{
    intlist *p;
    while (bk != NULL)
    {
        waitpid(bk->pid, NULL, 0);
        p = bk;
        bk = bk->next;
        free(p);
    }
}

void rfv()
{
    printf("\n");
    longjmp(begin,1);
}





//sudo leaks -atExit -- ./task5
int main()
{
    signal(SIGINT, rfv);
    bk = NULL;
    while (1 == 1)
    {
        setjmp(begin);
        //print_tree(tr);

        //отчистить дерево
        clear_tree(tr);
        tr = NULL;

        //отчистить список лексем 
        clearlist(lst);
        lst = NULL;


        //удаление зомби
        zombi();

        inv();

        lst = create_list();

        if (lst == NULL) 
        {
            continue;
        }
        if (lst[0][0] == EOF)
        {
            jdem();
            printf("\n");
            clearlist(lst);
            exit(0);
        }
        if (strcmp(lst[0], "exit") == 0)
        {
            jdem();
            clearlist(lst);
            exit(0);
        }
        if (lst[0][0] == '#')
        {
            printf("%s", lst[0]);
            printf("\n");
            continue;
        } 
        //построить дерево
        tr = create_tree();
        //print_tree(tr);
        //exit(0);

        zombi();

        ///выполение
        do_exec(tr);

    }


}
