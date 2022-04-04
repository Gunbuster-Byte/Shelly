#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

// Shelly, the next-gen bash!

//sturcture for node
struct node
{
    char *data; //stores strings
    struct node *next; //points to next node
};

//global variables
char *currentdir = NULL; //current directory value
struct node* histories;
char *re = NULL; //stores replay commands
int re_flag = 0; //flag for replay

//prompt & input
int prompt();

//commands
int movetodir(const char *path, char **currentdir);
int whereami(const char *currentdir);
int history(struct node* head, FILE *fp, int op);
int byebye();
int start_prog(const char *path, char *args[]);
int replay(struct node *head, int n, char **data);
int background_prog(const char *path, char *args[]);
int dalek(pid_t ID);
int dwelt(const char *currentdir, const char *path);
int maik(const char *currentdir, const char *path);
int coppy(const char *currentdir, const char *source, const char *destination);

//stack linked list operations
int empty(struct node *head);
struct node* push(struct node *head, char *data);
struct node* pop(struct node *head, char **data);
struct node* init();
struct node* clear(struct node *head);

struct node *prev_commands;

int main()
{    
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    
    fp = fopen("history.txt", "r");

    if(!fp)
        histories = init();
    else
    { 
        //get number of lines
        char ch;
        int lineCount = 0;
        while((ch = fgetc(fp)) != EOF)
        {
            if(ch == '\n')
                lineCount++;
        }
        //set pointer to beginning of file
        fseek(fp, 0, SEEK_SET);
        //create array with line height but unkown length
        char *copy[lineCount];
        //reusing variable for count
        lineCount = 0;
        
        ssize_t n;
        //get each line
        while((n = getline(&line, &len, fp)) != -1)
        {
            int flag = 0;
            int count = 0;

            copy[lineCount] = malloc(sizeof(ssize_t)*n);

            for(int i = 0; i < n; i++)
            {
                //removes %d: portion of string  and retrieves command
                if(line[i] == ' ' && flag == 0)
                    flag = 1;
                else if(flag == 1)
                {
                        copy[lineCount][count] = line[i];
                        count++;
                }
            }
            //insert null terminating value at end of string
            copy[lineCount][count-1] = '\0';
            //increment line count
            lineCount++;
        }
        //start from last element of array and push strings into data structure
        for(int i = lineCount-1; i >= 0; i--)
        {
            histories =  push(histories, copy[i]);
            free(copy[i]);
        }
    }

    free(line);
    
    //get current directory
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) != NULL)
    {
        currentdir = strdup(cwd);
    }
    else
    {
        perror("getcwd() error.\n");
        
        fclose(fp);
        free(re);
        clear(histories);
        
        return 1;
    }

    //The Shell
    while(prompt() != 0);

    fp = fopen("history.txt", "w");

    if(fp == NULL)
        exit(EXIT_FAILURE);

    //Write history to file
    history(histories, fp, 1);
    
    //Clean up
    fclose(fp);
    free(re);
    clear(histories);

    return 0;
}

//start command
int start_prog(const char *path, char *args[])
{
    pid_t pid, w;
    int status;
    int ret;
    pid = fork();
    
    char *res;

    if(pid == -1)
    {
        perror("Can't fork, error.\n");
        return 1;
    }
    else if(pid == 0) //child process
    {   
        //absolute path
        if(path[0] == '/')
        {
            res = realpath(path, NULL);
            
            if(res != NULL)
            {
                if(execv(res, args) == -1)
                {
                    printf("Error! Program cannot be executed!\n");
                    free(res);
                    exit(EXIT_FAILURE);
                }
                free(res);
                exit(0);
            }
            else
            {
                printf("Error! Program cannot be executed!\n");
                exit(0);
            }
        }
        else //relative path
        {   
            //convert relative to absolute path with respect to currentdir
            char p[PATH_MAX];        
            strcpy(p, currentdir);
            strcat(p, "/");
            strcat(p, path);
            
            res = realpath(p, NULL);
            
            if(res != NULL)
            {
                if(execv(res, args) == -1)
                {
                    printf("Error! Program cannot be executed!\n");
                    free(res);
                    exit(EXIT_FAILURE);
                }
                free(res);
                exit(0);
            }
            else
            {
                printf("Error! Program cannot be executed!\n");
                exit(0);
            }
        }
    }
    else if(pid > 0) //parent process
    {
        //stalls parent until child program exits
        do
        {
            w = waitpid(pid, &status, 0);

            if(w == -1)
            {
                perror("waitpid error\n");
                return 2;
            }

        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
         
    }

    return 0;
}

//background command
int background_prog(const char *path, char *args[])
{
    pid_t pid, w;
    int status;
    int ret;
    pid = fork();

    char *res; 
    if(pid == -1)
    {
        perror("Can't fork, error.\n");
        return 1;
    }
    else if(pid == 0) //child process
    {   
        if(path[0] == '/')
        {
            res = realpath(path, NULL);
            
            if(res != NULL)
            {
                if(execv(res, args) == -1)
                {
                    printf("Error! Program cannot be executed!\n");
                    free(res);
                    exit(EXIT_FAILURE);
                }
                free(res);
                exit(0);
            }
            else
            {
                printf("Error! Program cannot be executed!\n");
                exit(0);
            }
        }
        else
        {   
            char p[PATH_MAX];        
            strcpy(p, currentdir);
            strcat(p, "/");
            strcat(p, path);
            
            res = realpath(p, NULL);
            
            if(res != NULL)
            {
                if(execv(res, args) == -1)
                {
                    printf("Error! Program cannot be executed!\n");
                    free(res);
                    exit(EXIT_FAILURE);
                }
                free(res);
                exit(0);
            }
            else
            {
                printf("Error! Program cannot be executed!\n");
                exit(0);
            }
        }
    }    
    else if(pid > 0) //parent process
    {
        printf("PID: %d\n", pid);
    }

    return 0;
}

//dalek command
int dalek(pid_t ID)
{
    pid_t ret_val, w;
    
    int status; 
   
    //kills process
    ret_val = kill(ID, SIGKILL);
    
    if(ret_val != 0)
    {
        printf("Unable to kill process.\n");
        return 1;
    }
    //stalls parent until child program exits
    do
    {
        w = waitpid(ID, &status, 0);

        if(w == -1)
        {
            perror("waitpid error\n");
            return 2;
        }

        if(w > 0)
            printf("Process %d successfully killed.\n", w);

    }while(!WIFEXITED(status) && !WIFSIGNALED(status));

    return 0;
}

//replay command
int replay(struct node *head, int n, char **data)
{
    struct node *current;
    current = head;
    int count = 0;

    if(current != NULL && n >= 0)
    {
        //Traverses list and gets data when node n is found
        do
        {
            if(count == n + 1)
            {
                *data = strdup(current->data);
                break;
            }
            current = current->next;
            count++;
        }
        while(current != NULL);
        
        if(current == NULL)
            return 2;
    }
    else
    {
        printf("ERROR!\n");
        return 1;
    }
    return 0;
}

//movetodir command
int movetodir(const char *path, char **currentdir)
{
    char *res; 

    //check if directory is a relative or absolute path 
    if(path[0] == '/')
    {
	res = realpath(path, NULL);
    }
    else
    {
	if(currentdir != NULL)
	{
	    char p[PATH_MAX];        
	    strcpy(p, *currentdir);
	    strcat(p, "/");
	    strcat(p, path);

	    res = realpath(p, NULL);
	}
	else
	{
	    printf("Cannot retrieve current directory.\n");
	    return 1;
	}
    }
    
    if(res != NULL)
    {
        DIR *dir = opendir(res); //determines if path exists
        if(dir)
        {
            if(*currentdir != NULL)
                free(*currentdir);

            *currentdir = strdup(res);
        }
        else
        {
	    free(res);
            printf("Error!\n");
            return 2;
        }

        free(res);
        closedir(dir);   
    }
    else
    {
        printf("Directory does not exist.\n");
        return 1;
    }

    return 0;
}

//whereami command
int whereami(const char *currentdir)
{
    if(currentdir == NULL)
        printf("Cannot read current directory.\n");
    else
    {
        printf("%s\n", currentdir);
    }
    return 0;
}

//dwelt command
int dwelt(const char *currentdir, const char *path)
{
    char *res;

    //if the path is absolute
    if(path[0] == '/')
    {
	res = realpath(path, NULL);
	
	if(res != NULL)
	{
	    struct stat path_stat;
	    stat(res, &path_stat);

	    if(S_ISREG(path_stat.st_mode))
	    {
		printf("Dwelt indeed.\n");
	    }
	    else if(S_ISDIR(path_stat.st_mode))
	    {
		printf("Abode is.\n");
	    }
	    
	    free(res);   
	}
	else
	    printf("Dwelt not.\n");
    }
    //If the path is relative
    else
    {
	char p[PATH_MAX];        
        strcpy(p, currentdir);
        strcat(p, "/");
        strcat(p, path);

	res = realpath(p, NULL);
	
	if(res != NULL)
	{
	  struct stat path_stat;
	  stat(res, &path_stat);

	  if(S_ISREG(path_stat.st_mode))
	  {
	      printf("Dwelt indeed.\n");
	  }
	  else if(S_ISDIR(path_stat.st_mode))
	  {
	      printf("Abode is.\n");
	  }

	  free(res);
	}
	else
	    printf("Dwelt not.\n");
    }
    
    return 0;
}

//maik command
int maik(const char *currentdir, const char *path)
{
    FILE *fp;

    struct stat file_exist;
    
    //if the path is absolute
    if(path[0] == '/')
    {
	if(stat(path, &file_exist) != 0)
	{
	    fp = fopen(path, "w+");

	    if(fp == NULL)
	    {
		printf("File cannot be created.\n");
		return 2;
	    }
	    else
	    {
		fprintf(fp, "Draft");
		fclose(fp);
	    }
	}
	else if(S_ISREG(file_exist.st_mode))
	{
	    printf("File already exists.\n");
	    return 1;
	}
	else if(S_ISDIR(file_exist.st_mode))
	{
	    printf("Argument is a directory.\n");
	    return 1;
	}
	else
	{
	    printf("Unknown error.\n");
	    return 1;
	}
    }
    //if the path is relative
    else
    {
	char p[PATH_MAX];        
        strcpy(p, currentdir);
        strcat(p, "/");
        strcat(p, path);
	
	if(stat(p, &file_exist) != 0)
	{
	    fp = fopen(p, "w+");

	    if(fp == NULL)
	    {
		printf("File cannot be created.\n");
		return 2;
	    }
	    else
	    {
		fprintf(fp, "Draft");
		fclose(fp);
	    }
	}
	else if(S_ISREG(file_exist.st_mode))
	{
	    printf("File already exists.\n");
	    return 1;
	}
	else if(S_ISDIR(file_exist.st_mode))
	{
	    printf("Argument is a directory.\n");
	    return 1;
	}
	else
	{
	    printf("Unknown error.\n");
	    return 1;
	}
    }
    return 0;
}

//coppy command
int coppy(const char *currentdir, const char *source, const char *destination)
{

    FILE *fs;
    FILE *fd;
    
    struct stat file_exist;

    int src_flag = 0;
    int dst_flag = 0;
    int src_abs = 0;
    int dst_abs = 0;
    
    char s[PATH_MAX];
    char d[PATH_MAX];
    
    //if the source and/or destination is absolute
    if(source[0] == '/' || destination[0] == '/')
    {
	if(source[0] == '/')
	{
	    src_abs = 1;
	    
	    if(stat(source, &file_exist) == 0)
	    {
		src_flag = 1;
		if(S_ISDIR(file_exist.st_mode))
		{
		    printf("Source argument is a directory.\n");
		    return 1;
		}
	    }
	    else
	    {
		printf("Source file does not exist.\n");
		return 1;
	    }
	}
	if(destination[0] == '/')
	{
	    dst_abs = 1;
	    
	    if(stat(destination, &file_exist) != 0)
	    {
		dst_flag = 1;
	    }
	    else if(S_ISREG(file_exist.st_mode))
	    {
		printf("File already exists in destination.\n");
		return 1;
	    }
	    else if(S_ISDIR(file_exist.st_mode))
	    {
		printf("Destinatnion Argument is a directory.\n");
		return 1;
	    }
	    else
	    {
		printf("Destination Unknown error.\n");
		return 1;
	    }
	}
    }
    //if the source and/or destination is relative
    if(src_flag == 0 || dst_flag == 0)
    {
	if(src_flag == 0)
	{
	    strcpy(s, currentdir);
	    strcat(s, "/");
	    strcat(s, source);
	    
	    if(stat(s, &file_exist) == 0)
	    {
		src_flag = 1;
		if(S_ISDIR(file_exist.st_mode))
		{
		    printf("Source argument is a directory.\n");
		    return 1;
		}
	    }
	    else
	    {
		printf("Source file does not exist.\n");
		return 1;
	    }
	}
	if(dst_flag == 0)
	{
	    strcpy(d, currentdir);
	    strcat(d, "/");
	    strcat(d, destination);
	    
	    if(stat(d, &file_exist) != 0)
	    {
		dst_flag = 1;
	    }
	    else if(S_ISREG(file_exist.st_mode))
	    {
		printf("File already exists in destination.\n");
		return 1;
	    }
	    else if(S_ISDIR(file_exist.st_mode))
	    {
		printf("Destinatnion Argument is a directory.\n");
		return 1;
	    }
	    else
	    {
		printf("Destination Unknown error.\n");
		return 1;
	    }
	}
	
    }

    if((src_flag == 1) && (dst_flag == 1))
    {
	//start the copy process...
	if(src_abs == 1)
	    fs = fopen(source, "r");
	else
	    fs = fopen(s, "r");
	
	if(fs == NULL)
	{
	    printf("Source file cannot be opened.\n");
	    return 2;
	}
	
	if(dst_abs == 1)
	    fd = fopen(destination, "w");
	else
	    fd = fopen(d, "w");
	
	if(fd == NULL)
	{
	    fclose(fs);
	    printf("Destination's directory does not exist.\n");
	    return 2;
	}
	//copying..
	int c = fgetc(fs);
	while(c != EOF)
	{
	    fputc(c, fd);
	    c = fgetc(fs);
	}

	fclose(fs);
	fclose(fd);
    }
    else
    {
	printf("Invalid Source or Destination.\n");
    }
    return 0;
}

//history command
int history(struct node* head, FILE *fp, int op)
{
    struct node *current;
    current = head;
    int count = 0;

    //traverses list and either prints to console or file
    if(current != NULL)
    {
        do
        {   //print to file
            if(op == 1)
            {   
                fprintf(fp, "%d: ", count);
                fprintf(fp, "%s\n", current->data);
            }
            //print to console
            else if(op == 0)
            {   
                printf("%d: ", count);
                printf("%s\n", current->data);
            }
            current = current->next;
            count++;
        }
        while(current != NULL);
    }
    else
    {
        printf("ERROR!\n");
        return 1;
    }

   return 0;
}

//The Shell
int prompt()
{
    char *line; //input 

    //Get user input if replay string is null
    if(re == NULL)
    {
        printf("# ");
        
        int ch;
        size_t len = 0;
        size_t size = 10;

        line = realloc(NULL, size * sizeof(*line));
    
        if(line == NULL)
        {
            printf("\n Could not allocate mem.\n");
            return 0;
        }

        while((ch = fgetc(stdin)) != EOF && ch != '\n')
        {
            line[len++] = ch;
            
	          if(len == size)
	          {      
                line = realloc(line, sizeof(*line) * (size += 16));
	              if(line == NULL)
                {
                    printf("\n Could not allocate mem.\n");
                    return 0;
                }
	          }
        }

        line[len++] = '\0';

        line = realloc(line, sizeof(*line) * len);

    }
    else //execute replay string from replay command instead
    {
        line = strdup(re);
        free(re);
        re = NULL;
    }     

    int length = (int)strlen(line) + 1;

    if(length - 1 == 0) //Don't do anything if string is empty
    {
        free(line);
        return 1;
    }

    char string[length];
    strcpy(string, line);
    
    char *command, *token, *context;
    command = strtok_r(string, " ", &context);

    int argCount = 0;
    char *argv[50]; //upto fifty arguements can be saved
    
    int code = 0;
    
    if(re_flag != 1)
    {
        histories = push(histories, line);
    }
    else if(!strcmp(command, "replay"))
    {
        printf("Replaying a replay command is forbbiden.\n");
        re_flag = 0;
        return 1;
    }
    else
        re_flag = 0;

    if(command != NULL)
    {
        token = strtok_r(NULL, " ", &context);
        
        //store all tokens after command token as arguments
        while(token != NULL && argCount < 50)
        {
            argv[argCount] = strdup(token);
            argCount++;
            token = strtok_r(NULL, " ", &context);
        } 

        if(!strcmp(command, "byebye"))
        {
            if(argCount > 0)
                printf("Too many arguments.\n");
            else
            {
                free(line);
                free(currentdir);
                for(int i = 0; i < argCount; i++)
                {
                    free(argv[i]);
                }
                return 0;
            }
        }
        else if(!strcmp(command, "movetodir"))
        {
            if(argCount != 1)
            {
                if(argCount > 1)
                    printf("Too many arguments.\n");
                else
                    printf("Too few arguments.\n");
            }
            else
            {   
                char *path = argv[0];
                code = movetodir(path, &currentdir); 
            }
        }
        else if(!strcmp(command, "whereami"))
        {
            if(argCount > 0)
                printf("Too many arguments.\n");
            else
            {
                 code = whereami(currentdir);
            }
        }
        else if(!strcmp(command, "history"))
        {
            if(argCount > 1)
                printf("Too many arguments.\n");
            else
            {
                if(argCount == 1)
                {
                    if(strcmp(argv[0], "-c") == 0)
                    {
                        histories = clear(histories);
                    }
                    else
                        printf("Invalid argument.\n");
                }
                else
                   code = history(histories, NULL, 0);
            }
        }
        else if(!strcmp(command, "replay"))
        {
            if(argCount != 1)
            {
                if(argCount > 1)
                    printf("Too many arguments.\n");
                else
                    printf("Too few arguments.\n");
            }
            else
            {
                char *val = strdup(argv[0]);
                char *ptr;
                long int n = strtol(val, &ptr, 10); //gets first integer from string
                free(val);
                
                //exectures when no integer greater than or equal to 0 was found in string
                if(n == 0 && strcmp(argv[0], "0") != 0)
                {
                    printf("Input not a valid number.\n");
                }
                else
                {
                    code = replay(histories,(int) n, &re);
                    if(code == 2)
                        printf("No such history was found.\n");
                    else
                        re_flag = 1;
                }
            }
        }
        else if(!strcmp(command, "start"))
        {
            if(argCount < 1)
                printf("Too few arguments.\n");
            else
            {
                char *path;
                path = strdup(argv[0]); //path for program to start
                
                char *args[argCount];
                
                //store parameters for program (first element is the path)
                for(int i = 0; i < argCount; i++)
                    args[i] = strdup(argv[i]);
                
                args[argCount] = NULL; //last element needs to be a null pointer

                code = start_prog(path, args);

                for(int j = 0; j < argCount + 1; j++)
                    free(args[j]);

                free(path);
            }
        }
        //same logic as above
        else if(!strcmp(command, "background")) 
        {
            if(argCount < 1)
                printf("Too few arguments.\n");
            else
            {
                char *path;
                path = strdup(argv[0]);
                
                char *args[argCount + 1];

                for(int i = 0; i < argCount; i++)
                    args[i] = strdup(argv[i]);
                
                args[argCount] = NULL;

                code = background_prog(path, args);

                for(int j = 0; j < argCount; j++)
                    free(args[j]);

                free(path);
            }
        }
        else if(!strcmp(command, "dalek"))
        {
            if(argCount != 1)
            {
                if(argCount > 1)
                    printf("Too many arguments.\n");
                else
                    printf("Too few arguments.\n");
            }
            else
            {
                pid_t ID = atoi(argv[0]);
                if(ID > 0)
                    code = dalek(ID);
                else
                    printf("Entry must be a number greater than 0.\n");
            }
        }
	else if(!strcmp(command, "dwelt"))
        {
            if(argCount != 1)
            {
                if(argCount > 1)
                    printf("Too many arguments.\n");
                else
                    printf("Too few arguments.\n");
            }
            else
            {   
                char *path = argv[0];
                code = dwelt(currentdir, path);
            }
        }
	else if(!strcmp(command, "maik"))
        {
            if(argCount != 1)
            {
                if(argCount > 1)
                    printf("Too many arguments.\n");
                else
                    printf("Too few arguments.\n");
            }
            else
            {   
                char *path = argv[0];
                code = maik(currentdir, path);
            }
        }
	else if(!strcmp(command, "coppy"))
        {
            if(argCount != 2)
            {
                if(argCount > 2)
                    printf("Too many arguments.\n");
                else
                    printf("Too few arguments.\n");
            }
            else
            {   
                char *source = argv[0];
		char *destination = argv[1];
                code = coppy(currentdir, source, destination);
            }
        }
         else
            printf("Invalid Command.\n");
    }

    free(line);
    for(int i = 0; i < argCount; i++)
        free(argv[i]);

    return 1;
}

//initalize node
struct node *init()
{
    return NULL;
}

//check if node is empty
int empty(struct node* head)
{
    return head == NULL ? 1 : 0;
}

//add new node to list
struct node *push(struct node *head, char *data)
{
    struct node *tmp = (struct node *)malloc(sizeof(struct node));

    if(tmp == NULL)
    {
        return NULL;
    }
    tmp->data = strdup(data);
    tmp->next = head;
    head = tmp;

    return head;
}

//remove node from list (FILO)
struct node *pop(struct node *head, char **data)
{
    struct node* tmp = head;
    
    if(tmp->data != NULL)
    {
        *data = strdup(tmp->data);
    }
    else
        *data = NULL;

    head = head->next;

    free(tmp->data);
    free(tmp);

    return head;
}

//clear list
struct node *clear(struct node *head)
{
    if(head != NULL)
    {   
        char *temp;
        do
        {
            head = pop(head, &temp);
            free(temp);
        }
        while(head != NULL);
    }

    return head;
}
