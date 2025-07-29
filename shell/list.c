#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define N 5
#define SIZE 16
int c; //текущий символ
char static **lst = NULL;  //список слов (в виде массива)
char *buf = NULL; // буфер для накопления текущего слова
int sizebuf; // размер буфера текущего слова
int sizelist; // размер списка слов
int curbuf; // индекс текущего символа в буфере
int curlist; // индекс текущего слова в списке
int lg;

int slesh = 0; //0 - если не было '\', 1 - если был

char str[N];
int curstr; //индекс текущего слова в str

void nullbuf() //удаление буфера
{
    //free(buf);
    buf = NULL;
    sizebuf = 0;
    curbuf = 0;
}

void addsym() //добавление символа в буфер
{
if (curbuf > sizebuf - 1)
        buf = realloc(buf, sizebuf += SIZE);//увеличиваем буфер при необходимости 
    buf[curbuf++] = c;
}

void addword()//добавление буфера в список
{
if (curbuf > sizebuf - 1)
        buf = realloc(buf, sizebuf += 1); // для записи ’\0’ увеличиваем буфер при необходимости 
    buf[curbuf++] = '\0';
    //выравниваем используемую память точно по размеру слова
    buf = realloc(buf,sizebuf = curbuf);
if (curlist > sizelist - 1)
    lst = realloc(lst, (sizelist += SIZE) * sizeof(*lst)); // увеличиваем массив под список при необходимости

    char home[] = "$HOME";
    char shell[] = "$SHELL";
    char usr[] = "$USER";
    char id[] = "$EUID";
    if (slesh == 0 && strcmp(buf,home) == 0) 
    {
        buf = realloc(buf, 1 + strlen(getenv("HOME")));
        strcpy(buf,getenv("HOME"));
    }
    
    if (slesh == 0 && strcmp(buf,shell) == 0) 
    {
        buf = realloc(buf, 1 + strlen(getenv("SHELL")));
        strcpy(buf,getenv("SHELL"));
    }

    if (slesh == 0 && strcmp(buf,usr) == 0) 
    {
        buf = realloc(buf, 1 + strlen(getlogin()));
        strcpy(buf, getlogin());
    }

    if (slesh == 0 && strcmp(buf,id) == 0) 
    {
        int n = geteuid();
        int x = n;
        int ans = 0;
        while (x > 0)
        {
            ans++;
            x = x / 10;
        }
        free(buf);
        buf = NULL;
        buf = realloc(buf, ans);
        for (int i = 0; i < ans; i++)
        {
            buf[ans - 1 - i] = n % 10 + '0';
            n = n / 10;
        }
    }

    lst[curlist++] = buf;
    slesh = 0;
}



void sort()
{
    if (lst == NULL) return;
    int ans;
    char *tmp;
    for (int i = sizelist - 2; i >= 0; i--)
    {
        ans = 1; //флаг что были переставлены эл-ты
        for (int j = 0; j < i; j++)
        {
            if (strcmp(lst[j], lst[j + 1]) > 0)
            {
                tmp = lst[j];
                lst[j] = lst[j+1];
                lst[j+1] = tmp;
                ans = 0;
            }
        }
        if (ans) break;
    }
}


void clearlist(char **lst) //удаление спика
{
    int i;
    sizelist = 0;
    curlist = 0;
    if (lst == NULL) return;
    for (i = 0; lst[i] != NULL; i++)
        free(lst[i]);
    free(lst);
    lst = NULL;
}

void null_list() //обнуление списка
{
    sizelist = 0;
    curlist = 0;
    lst = NULL;
}




void termlist() //завершение списка
{
    if (lst == NULL) return;
    if (curlist > sizelist - 1)
            lst = realloc(lst,(sizelist+1)*sizeof(*lst));
    lst[curlist] = NULL;
    //выравниваем используемую под список память точно по размеру списка
    lst = realloc(lst,(sizelist=curlist+1)*sizeof(*lst));
}



void printlist(char **lst) //печать списка
{
    int i;
    if (lst == NULL) return;
    for (i = 0; i < sizelist - 1; i++)
        printf("%s\n",lst[i]);
}


int symset(int c) //возвращает 1 если любой сивол кроме специальных
{
    return c!='\n' && c!= ' ' && c!='\t' && c!='>' && c!= EOF && c!='|' && c!= '<' && c!= ')' && c!='(' && c!= '&' && c!= ';';
}


int getsym()
{
   // int si;
    if (curstr >= N - 1)
    {
        for (int i = 0; i < N; i++)
            str[i] = '\0';
        scanf("%3[^\n]", str);
        if ((str[3] = getchar()) == EOF) return EOF;
        curstr = 0;
    }
    if (str[curstr] == '\0') //есди мы встретили его то
    {
        
        curstr = N;
        return '\n';
    }
    return str[curstr++];

}


char **create_list() 
{
    typedef enum {Start, Word, Greater, Greater2, Stop, Kov, Resh} vertex;
    vertex V = Start;
    
    curstr = N;


    c = getsym();
    null_list();
    while(1 == 1)
    {
        switch(V)
        {
            case Start:
                if(c == ' '|| c == '\t') 
                {
                    while (c == ' ') c = getsym();
                }
                if (c == EOF) 
                {
                    nullbuf();
                    addsym();
                    addword();
                    termlist();
                    return lst;
                }
                else if (c == '\n')
                {
                    termlist();
                    //printlist();
                    
                    V = Stop;
                }
                else
                {
                    if (c == '\"')
                    {
                        nullbuf();
                        addsym();
                        if (slesh == 1)
                        {
                            V = Word;
                            slesh = 0;
                            break;
                        }
                        V = Kov;
                        c = getsym();
                        break;
                    }
                    if (c == '#')
                    {
                        nullbuf();
                        addsym();
                        c = getsym();
                        if (slesh == 1)
                        {
                            V = Word;
                            slesh = 0;
                            break;
                        }
                        V = Resh;
                        break;
                    }
                    if (c == '\\')
                    {
                        nullbuf();
                        c = getsym();
                        if (slesh == 1)
                        {
                            V = Word;
                            slesh = 0;
                            break;
                        }
                        slesh = 1;
                        V = Start;
                        break;
                    }
                    
                    nullbuf();
                    addsym();
                    V = (c == '>' || c == '|' || c == '&' || c == '<' || c == ';' || c == '(' || c == ')') ? Greater: Word;
                    c = getsym();
                }
                break;
            case Resh:
                if (c == '\n')
                {
                    addword();
                    V = Start;
                }   
                else 
                {
                    addsym();
                    c = getsym();
                }
                break;
            case Kov:
                if (c == '\"')
                {
                    addsym();
                    addword();
                    c = getsym();
                    V = Start;
                }
                else
                {
                    addsym();
                    c = getsym();
                }
                break;
                
            case Word:
                if (symset(c))
                {
                    addsym();
                    c = getsym();
                }
                else
                {
                    V = Start;
                    addword();
                }
                break;
                
            case Greater:
                if (c == '>' || c == '|' || c == '&' || c == '\\')
                {
                    addsym();
                    c = getsym();
                    V = Greater2;
                }
                else
                {
                    V = Start;
                    addword();
                }
                break;
                
            case Greater2:
                V = Start;
                addword();
                break;
                
            case Stop:
                return lst;;
                break;
        }
    }
}
    