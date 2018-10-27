/*	B131210035 Tugay ERSOY	*/
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define error(args...) do { \
    fprintf(stderr, "mrsh: ");  \
    fprintf(stderr, args);  \
    fprintf(stderr, "\n");  \
    exit(EXIT_FAILURE);     \
} while (0)

int quit_function(char **args)
{
  return 0;
}
char *builtin_str[] = {
  "quit"
};

int (*builtin_func[]) (char **) = {
  &quit_function
};

int builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}
int isle(char **args)
{
	int i;

	if(args[0] == NULL)
	{
		return 1; //bos komut
	}
	for(i=0; i<builtins();i++)
	{
		if(strcmp(args[0],builtin_str[i])==0)
		{
			return (*builtin_func[i])(args);
		}
	}
	return process_olustur(args);
}
//girilen komut okunuyor
char *satir_oku(void)
{
	char *line = NULL;
	ssize_t bufsize = 0;
	getline(&line, &bufsize, stdin); //satır okuyor ve yer ayırıyor 
	return line;
	
}

#define TOKEN_DELIM " \t\r\n\a"
//girilen komutu sözcüklere tokenlere bölüyor
char **satir_bol(char *line)
{
	int buffersize = 128;
	int position = 0;
	char **tokens = malloc(buffersize * sizeof(char*));
	char *token;
	char **tokens_backup;
	
	token = strtok(line,TOKEN_DELIM);
	while (token != NULL)
	{
		tokens[position] = token;
		position++;
		
		if(position >= buffersize)
		{	
			buffersize +=128;
			tokens_backup = tokens;
			tokens = realloc(tokens,buffersize *sizeof(char*));
			if(!tokens) {
					free(tokens_backup);
					exit(EXIT_FAILURE);
				     }
		}
		token = strtok(NULL,TOKEN_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}
void yorumlayici(void)
{
	char *line;
	char **args;
	int status;

	do{ //prompt
		printf(">");
		line = satir_oku();
		args = satir_bol(line);
		status = isle(args);

		free(line);
		free(args);
		fflush(stdout);
	}while (status);

}
int process_olustur(char **args)
{
	int fd;	
	int pid;
	int bpid;
	int status;
	
	pid = fork();
	if (pid > 0)
	{
		do {
			bpid = waitpid(pid, &status, WUNTRACED);
		}while (!WIFEXITED(status) && !WIFSIGNALED(status));
		printf("[%d] retval:%d \n",pid,WEXITSTATUS(status));
	}
	else if(pid ==0)
	{

    int fd0,fd1,i,in=0,out=0;
    char input[64],output[64];

    // komut içerisinde '>' ve '<' yönlendirmeleri buluyor onlara null değer atıyor.

    for(i=0;args[i]!='\0';i++)
    {
	if(strcmp(args[i],"<")==0)
        {        
            args[i]=NULL;
            strcpy(input,args[i+1]);
            in=2;           
        }               

        else if(strcmp(args[i],">")==0)
        {      
            args[i]=NULL;
            strcpy(output,args[i+1]);
            out=2;
        }         
    }

    //girilen komut giriş yönlendirme ise içerisinde '<'varsa
    if(in)
    {   

        // fdo is file-descriptor
        int fd0;
        if ((fd0 = open(input, O_RDONLY, 0600)) < 0) {
            perror("Giris Dosyasi Bulunamadi");
            exit(0);
        }           
        // dosya içeriğini kopyalıyor
        dup2(fd0, STDIN_FILENO);

        close(fd0); // necessary
    }

    //çıkış yönledirme yapıldığında,içerisinde '>'varsa bulunduğunda 
    if (out)
    {

        int fd1 ;
        if ((fd1 = creat(output , 0644)) < 0) {
            perror("Couldn't open the output file");
            exit(0);
        }           

        dup2(fd1, STDOUT_FILENO); 
        close(fd1);
    }

    execvp(*args, args);
    perror("execvp");
    _exit(1);


}
	return 1;	
	}




int main(int argc, char **argv)
{
	yorumlayici();	
	
	return EXIT_SUCCESS;

}

