#ifndef TREE_H
#define TREE_H

typedef struct cmd_inf *tree;
typedef struct cmd_inf 
{
    char ** argv; // список из имени команды и аргументов
    char *infile; // переназначенный файл стандартного ввода
    int typeout; // =1 если >> 
    char *outfile; // переназначенный файл стандартного вывода
    int backgrnd; // =1, если команда подлежит выполнению в фоновом режиме cmd_inf* psubcmd;
   // int num; // кол-во процессов в pr1 | pr2 | pr3...
    tree psubcmd;
    tree pipe; // следующая команда после “|”
    tree next_one; // следующая после “;” (или после “&”)
    tree next_two; //следущая после "||"
    tree next_three; //следующая после  "&&"
}info;



tree create_tree();
void print_tree(tree tr);
void clear_tree(tree tr);



#endif
