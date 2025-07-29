#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include "tree.h"
extern jmp_buf begin;
int fd[2];
int n = 0;
int ind = 0;
typedef struct backgrndList {
    int pid;
    struct backgrndList *next;
} intlist;

typedef intlist *bckgrnd;
extern bckgrnd bk;

extern tree tr;

int errr = 0;

void usr1()
{
    errr = 1;
}


void pwd_pipe(tree tr)
{
    if (tr->argv[1] != NULL)
    {
        printf("ОШИБКА: команда pwd вводится без параметров\n");
        exit(1);
    }

    if (tr->infile != NULL)
    {
        int f = open(tr->infile, O_RDONLY);
        if (f == -1)
        {
            printf("ОШИБКА: такого файла не существует\n");
            exit(1);
        }
        dup2(f, 0);
        close(f);
    }

    if (tr->outfile != NULL)
    {
        int f;
        if (tr->typeout == 1) f = open(tr->outfile, O_WRONLY | O_APPEND, 0666);
        else f = open(tr->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        dup2(f, 1);
        close(f);
    }


    char *buf;
    int n;
    n = 2;
    buf = (char*) malloc (n * sizeof(char));
    int i;
    while (1 == 1)
    {
        for (i = 0; i < n; i++) buf[i] = '\0';
        if (getcwd(buf, n) == NULL)
        {
            n++;
            buf = realloc(buf, n * sizeof(char));
        }
        else
            break;
    } 
    printf("%s", buf);
    printf("\n");
    free(buf);
    exit(0);
}

void pwd(tree tr)
{
    if (tr->argv[1] != NULL)
    {
        printf("ОШИБКА: команда pwd вводится без параметров\n");
        return;
    }

    int sin = dup(1);
    if (tr->infile != NULL)
    {
        int f = open(tr->infile, O_RDONLY);
        if (f == -1)
        {
            printf("ОШИБКА: такого файла не существует\n");
            return;
        }
        dup2(f, 0);
        close(f);
    }

    if (tr->outfile != NULL)
    {
        int f;
        if (tr->typeout == 1) f = open(tr->outfile, O_WRONLY | O_APPEND | O_CREAT, 0644);
        else f = open(tr->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        dup2(f, 1);
        close(f);
    }

    char *buf;
    int n;
    n = 2;
    buf = (char*) malloc (n * sizeof(char));
    int i;
    while (1 == 1)
    {
        for (i = 0; i < n; i++) buf[i] = '\0';
        if (getcwd(buf, n) == NULL)
        {
            n++;
            buf = realloc(buf, n * sizeof(char));
        }
        else
            break;
    } 
    printf("%s", buf);
    printf("\n");

    dup2(sin, 1);
    free(buf);
    return;
}

void cd_pipe(tree tr)
{
    char *buf;
    if (tr->argv[1] == NULL) buf = NULL;
    else if (tr->argv[2] == NULL) buf = tr->argv[1];
    else
    {
        printf("ОШИБКА: неверные аргументы для команды cd\n");
        exit(1);
    }


    if (tr->infile != NULL)
    {
        int f = open(tr->infile, O_RDONLY);
        if (f == -1)
        {
            printf("ОШИБКА: такого файла не существует\n");
            exit(1);
        }
        dup2(f, 0);
        close(f);
    }

    if (tr->outfile != NULL)
    {
        int f;
        if (tr->typeout == 1) f = open(tr->outfile, O_WRONLY | O_APPEND, 0666);
        else f = open(tr->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        dup2(f, 1);
        close(f);
    }

    if (buf == NULL)
    {
        if (chdir(getenv("HOME")) != 0)
        {
            printf("ОШИБКА: ошибка перемещения в домашнюю директорию\n");
            exit(1);
        }
    }
    else
    {
        if (chdir(buf) != 0)
        {
            printf("ОШИБКА: неверно указан путь перемещения\n");
            exit(1);
        }
    }
    exit(0);
}


void cd(tree tr)
{
    char *buf;
    if (tr->argv[1] == NULL) buf = NULL;
    else if (tr->argv[2] == NULL) buf = tr->argv[1];
    else
    {
        printf("ОШИБКА: неверные аргументы для команды cd\n");
        return;
    }


    if (tr->infile != NULL)
    {
        int f = open(tr->infile, O_RDONLY);
        if (f == -1)
        {
            printf("ОШИБКА: такого файла не существует\n");
            return;
        }
        dup2(f, 0);
        close(f);
    }

    if (tr->outfile != NULL)
    {
        int f;
        if (tr->typeout == 1) f = open(tr->outfile, O_WRONLY | O_APPEND, 0666);
        else f = open(tr->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        dup2(f, 1);
        close(f);
    }

    if (buf == NULL)
    {
        if (chdir(getenv("HOME")) != 0)
        {
            printf("ОШИБКА: ошибка перемещения в домашнюю директорию\n");
            return;
        }
    }
    else
    {
        if (chdir(buf) != 0)
        {
            printf("ОШИБКА: неверно указан путь перемещения\n");
            return;
        }
    }
    return;
}

void cmd(tree tr)
{
    
    if (tr->infile != NULL)
    {
        int f = open(tr->infile, O_RDONLY);
        if (f == -1)
        {
            printf("ОШИБКА: такого файла не существует\n");
            kill(getppid(), SIGUSR1);
            exit(1);
        }
        dup2(f, 0);
        close(f);
    }

    if (tr->outfile != NULL)
    {
        int f;
        if (tr->typeout == 1) f = open(tr->outfile, O_WRONLY | O_APPEND | O_CREAT, 0644);
        else f = open(tr->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        dup2(f, 1);
        close(f);
    }

    if (execvp(tr->argv[0], tr->argv) == -1)
    {
        printf("ОШИБКА: команда %s не существует\n", tr->argv[0]);
        kill(getppid(), SIGUSR1);
        exit(1);
    }
}


int wait_status(tree tr, int pid)
{
    int status;
    if (tr->backgrnd == 0)
    {
        waitpid(pid,&status,0);
        //kill(pid, SIGTERM);
    }
    else
    {
        //printf("11");
        bckgrnd g = NULL;
        g = malloc(sizeof(intlist));
        g->pid = pid;
        g->next = NULL;
        if (bk == NULL) bk = g;
        else 
        {
            g->next = bk;
            bk = g; 
        }
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0 && errr == 0) 
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}




int konez(tree tr)
{
    if (tr->pipe == NULL
    && tr->next_one == NULL
    && tr->next_two == NULL
    && tr->next_three == NULL) return 1;
    return 0;
}


int vnutr(tree tr)
{
    if (strcmp(tr->argv[0], "pwd") == 0 || strcmp(tr->argv[0], "cd") == 0 || strcmp(tr->argv[0], "exit") == 0) return 1;
    return 0;
}

void go(tree tr)
{
    int i, f;
    if (tr == NULL) return;
    if (tr->pipe != NULL)
    {
        int fd[2];
        int n = 2;
        pipe(fd);
        int pid;
        int pipe_in, pipe_out, pipe_next;
        pipe_out = fd[1];
        pipe_next = fd[0];
        if (!(vnutr(tr)) && (pid = fork()) == 0)
        {
            close(pipe_next);
		    dup2(pipe_out,1);
		    close(pipe_out);
            cmd(tr);
        }

        pipe_in = pipe_next;
        tr = tr->pipe;

        while(tr->pipe != NULL)
        {   
            n++;
            close(pipe_out);
		    pipe(fd);
		    pipe_out = fd[1];
		    pipe_next = fd[0];
            if (!(vnutr(tr)) && (pid = fork()) == 0)
            {
                n++;

                close(pipe_next);
			    dup2(pipe_in, 0);
			    close(pipe_in);
			    dup2(pipe_out, 1);
			    close(pipe_out);

                cmd(tr);
            }


            close(pipe_in);
		    pipe_in = pipe_next;
            tr = tr->pipe;
        }

        close(pipe_out);
        
        if (!(vnutr(tr)) && (pid = fork()) == 0)
        {
            dup2(pipe_in,0);
		    close(pipe_in);

            cmd(tr);
        }
        close(pipe_in);
        for (i = 0; i < n; i++) wait(NULL);
        
        if (konez(tr)) return;

    }

    if (tr->next_one != NULL)
    {
        int pid;
        while(tr->next_one != NULL)
        {
            if (strcmp(tr->argv[0], "cd") == 0) cd(tr);
            else if (strcmp(tr->argv[0], "pwd") == 0) pwd(tr);
            else if (strcmp(tr->argv[0], "exit") == 0) exit(0);
            else 
            {
                if ((pid = fork()) == 0)
                {
                    if (tr->backgrnd == 1) signal (SIGINT, SIG_IGN);
                    
                    cmd(tr);
                }
                    
                i = wait_status(tr,pid);
            }
            //printf("%d", pid);
            tr = tr->next_one;
        }

        
    }

    if (tr->next_two != NULL)
    {
        int pid;
        errr = 0;
        while(tr->next_two != NULL)
        {
            if (strcmp(tr->argv[0], "cd") == 0) cd(tr);
            else if (strcmp(tr->argv[0], "pwd") == 0) pwd(tr);
            else if (strcmp(tr->argv[0], "exit") == 0) exit(0);
            else 
            {
                if ((pid = fork()) == 0)
                {
                    cmd(tr);
                }
                i = wait_status(tr,pid);
            }
            if (i) break;
            tr = tr->next_two;
        }
        if (i == 0)
        {
            if ((pid = fork()) == 0)
            {
                cmd(tr);
            }
            i = wait_status(tr,pid);
        }
        while(tr->next_two != NULL) tr = tr->next_two;
        if (konez(tr)) return;
    }

    if (tr->next_three != NULL)
    {
        int pid;
        errr = 0;
        while(tr->next_three != NULL)
        {
            if (strcmp(tr->argv[0], "cd") == 0) cd(tr);
            else if (strcmp(tr->argv[0], "pwd") == 0) pwd(tr);
            else if (strcmp(tr->argv[0], "exit") == 0) exit(0);
            else 
            {
                if ((pid = fork()) == 0)
                {   
                    signal(SIGINT, SIG_IGN);
                    
                    cmd(tr);
                }
                i = wait_status(tr,pid);
            }
            if (i == 0) break;
            tr = tr->next_three;
        }
        if (i == 1)
        {
            if ((pid = fork()) == 0)
            {   
                signal(SIGINT, SIG_IGN);
                
                cmd(tr);
            }
            i = wait_status(tr,pid);
        }
        while(tr->next_three != NULL) tr = tr->next_three;
        if (konez(tr)) return;
    }

    if (tr == NULL) return;

    if (tr->pipe == NULL
    && tr->next_one == NULL
    && tr->next_two == NULL
    && tr->next_three == NULL)
    {
        //printf ("tyu");
        int pid;
        if (strcmp(tr->argv[0], "cd") == 0) cd(tr);
        else if (strcmp(tr->argv[0], "pwd") == 0) pwd(tr);
        else if (strcmp(tr->argv[0], "exit") == 0) exit(0);
        else 
        {
            if ((pid = fork()) == 0)
            {
                if (tr->backgrnd == 1) signal (SIGINT, SIG_IGN);
                    
                cmd(tr);
            }
            
            i = wait_status(tr,pid);
        }
        return;
    }
    else go(tr);

}

void do_exec(tree tr)
{
    signal(SIGUSR1, usr1);
    tree t = tr;
    go(t);
    //if (bk == NULL) printf("0000");
    //return bk;
}
