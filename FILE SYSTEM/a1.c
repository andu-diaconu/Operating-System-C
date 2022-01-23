#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>

int afisareDirector(char *path){
	DIR *dir = opendir(path);
	struct dirent *entry = NULL;
	if(dir!=NULL){
		printf("SUCCESS\n");
		while((entry=readdir(dir))!=NULL)
			if(strcmp(entry->d_name,".") && strcmp(entry->d_name,".."))
				printf("%s/%s\n",path,entry->d_name);	
		
		closedir(dir);
		return 0;
	}
	else
		return -1;
}

int afisareRecursivaDirector(char *path, bool aFostAfisat){
	DIR *dir = opendir(path);
	struct dirent *entry=NULL;
	char  fullPath[512];
	struct stat statbuf;
	
	if(dir!=NULL){
	
		if(!aFostAfisat){
			printf("SUCCESS\n");
			aFostAfisat = true;
		}
	
		while((entry=readdir(dir))!=NULL)
			if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
				snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
				if(lstat(fullPath,&statbuf) == 0){
					printf("%s\n", fullPath);
					if(S_ISDIR(statbuf.st_mode))
						afisareRecursivaDirector(fullPath,aFostAfisat);
				}
			}

		closedir(dir);
		return 0;
	}
	else 
		return -1;
}

int afisareSmallerThan(char *path, int value){

		DIR *dir = opendir(path);
		struct dirent *entry=NULL;
		struct stat statbuf;
		char fullPath[512];
		
		if(dir!=NULL){
		
		printf("SUCCESS\n");

		while((entry=readdir(dir))!=NULL)
			if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
				snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
				if(lstat(fullPath,&statbuf) == 0)
					if(S_ISDIR(statbuf.st_mode) == 0)
						if(statbuf.st_size < value)
							printf("%s\n",fullPath);
			strcpy(fullPath,"");		
		}

		closedir(dir);
		return 0;
		}		

		else 
			return -1;	

}

int afisareCuPermisiune(char *path){
	DIR *dir = opendir(path);
	struct dirent *entry=NULL;
	struct stat statbuf;
	char fullPath[512];

	if(dir!=NULL){

		printf("SUCCESS\n");
		while((entry=readdir(dir))!=NULL)
			if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
				snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
				if(lstat(fullPath,&statbuf) == 0){
					int number = statbuf.st_mode & 0777;
					if(number & 128)
						printf("%s\n",fullPath);
				}
			strcpy(fullPath,"");		
		}

	closedir(dir);
	return 0;
	}
	else
		return -1;
}

int afisareRecursivaPermisiva(char *path, bool aFostAfisat){
	DIR *dir = opendir(path);
	struct dirent *entry=NULL;
	struct stat statbuf;
	char fullPath[512];

	if(dir!=NULL){

		if(!aFostAfisat){
			printf("SUCCESS\n");
			aFostAfisat = true;
		}

		while((entry=readdir(dir))!=NULL)
			if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
				snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
				if(lstat(fullPath,&statbuf) == 0){
					int number = statbuf.st_mode & 0777;
					if(number & 128)
						printf("%s\n",fullPath);
					if(S_ISDIR(statbuf.st_mode))
						afisareRecursivaPermisiva(fullPath,aFostAfisat);
				}		
			}
		
		closedir(dir);
		return 0;
	}
	else
		return -1;
}


int afisareRecursivaDimensiune(char *path , bool aFostAfisat, int value){
	DIR *dir = opendir(path);
	struct dirent *entry=NULL;
	struct stat statbuf;
	char fullPath[512];
		
	if(dir!=NULL){
		
		if(!aFostAfisat){
			printf("SUCCESS\n");
			aFostAfisat = true;
		}

		while((entry=readdir(dir))!=NULL)
			if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
				snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
				if(lstat(fullPath,&statbuf) == 0){
					if(S_ISDIR(statbuf.st_mode) == 0)
						if(statbuf.st_size < value)
							printf("%s\n",fullPath);
					if(S_ISDIR(statbuf.st_mode) !=0 )
						afisareRecursivaDimensiune(fullPath,aFostAfisat,value);			
				}
		}

		closedir(dir);
		return 0;
	}		
	else 
		return -1;		

}


int parsareSF(char *path){

	int fd=-1;
	fd=open(path,O_RDONLY);
	if(fd<0){
		perror("Invalid path");
		return -2;
	}

	
	char magic[4],name[7];
	int version=0, header_size=0,no_of_sections=0,offset=0,size=0;
	lseek(fd,0,SEEK_END);
	lseek(fd,-6,SEEK_CUR);
	
	if(read(fd,&header_size,2)!=2){
		perror("Eroare la citirea header size-ului");
		close(fd);
		return -2;
	}
	magic[4]=0;
	if(read(fd,magic,4)!=4){
		perror("Eroare la citire magic");
		close(fd);
		return -2;
	}

	if(strcmp(magic,"cM6b")!=0){
		close(fd);
		return -1;
	}
	else{
		lseek(fd,-header_size,SEEK_CUR);
		if(read(fd,&version,2)!=2){
			perror("Eroare la citirea versiunii");
			close(fd);
			return -2;
		}
		if(version<63 || version>99){
			close(fd);
			return -3;
		}
		else{
			if(read(fd,&no_of_sections,1)!=1){
				perror("Eroare la citirea numarului de sectiuni");
				close(fd);
				return -2;
			}
			if(no_of_sections<6 || no_of_sections>19){
				close(fd);
				return -4;
			}
			else{
				int index = 1;
				int type=0;
				for(int i=0;i<no_of_sections;i++){
					read(fd,name,7);
					name[7]=0;
					read(fd,&type,1);
					read(fd,&offset,4);
					read(fd,&size,4);
					if(type!=37&&type!=46&&type!=59&&type!=70){
						close(fd);
						return -5;
					}
				}
				lseek(fd,-(16*no_of_sections),SEEK_CUR);
				for(int i=0;i<no_of_sections;i++){
					read(fd,name,7);
					name[7]=0;
					read(fd,&type,1);
					read(fd,&offset,4);
					read(fd,&size,4);
					if(type==37 || type==46 || type ==59 || type ==70){
						if(index == 1)
							printf("SUCCESS\nversion=%d\nnr_sections=%d\n",version,no_of_sections);
						printf("section%d: %s %d %d\n",index,name,type,size);
						index++;
					}
					else{
						close(fd);
						return-5;
					}

				}
			}
		}
	}

	close(fd);
	return 0;


}

int extractSF(char *path,int sect,int line){

	int fd=-1;
	fd=open(path,O_RDONLY);
	if(fd<0)
		return -1;

	lseek(fd,-6,SEEK_END);
	int hSize =0,version=0,no_of_sections=0,type=0;
	char magic[4];
	read(fd,&hSize,2);
	read(fd,magic,4);
	lseek(fd,-hSize,SEEK_END);
	read(fd,&version,2);
	read(fd,&no_of_sections,1);
	if(no_of_sections < sect){
		close(fd);
		return -2;
	}
	lseek(fd,16*(sect-1)+7,SEEK_CUR);
	read(fd,&type,1);
	if( strcmp(magic,"cM6b")!=0  || version<63 || version>99 || no_of_sections<6 || no_of_sections>19 || (type !=37 && type !=46 && type !=59 && type !=70)){
		close(fd);
		return -1;
	}
	int offset =0,size =0;
	read(fd,&offset,4);
	read(fd,&size,4);
	lseek(fd,offset,SEEK_SET);
	char *buffer=malloc(sizeof(char)*size);
	read(fd,buffer,size);
	int count =1;
	bool exista = false;
	for(int i=0;i<size;i++){
		if(buffer[i]=='\n')
			count++;
		if(count == line && buffer[i]=='\n'){
			printf("SUCCESS\n");
			exista = true;
		}
		if(count == line && buffer[i]!='\n')
			printf("%c",buffer[i]);
		if(count > line)
			break;
	}
	if(!exista){
		free(buffer);
		close(fd);
		return -3;
	}

	free(buffer);
	close(fd);
	return 0;
}


int checkSF(char *buf){
	int fd=-1;
	fd=open(buf,O_RDONLY);
	if(fd<0)
		return -1;
	lseek(fd,-6,SEEK_END);
	int hSize =0,version=0,no_of_sections=0,type=0,offset=0,size=0;
	char magic[4];
	read(fd,&hSize,2);
	read(fd,magic,4);
	lseek(fd,-hSize,SEEK_END);
	read(fd,&version,2);
	read(fd,&no_of_sections,1);
	if(strcmp(magic,"cM6b")!=0 ||  version<63 || version>99 || no_of_sections<6 || no_of_sections>19|| no_of_sections<6 || no_of_sections>19){
		close(fd);
		return -2;
	} 
	for(int i=0;i<no_of_sections;i++){	
		lseek(fd,(-hSize+3)+16*i+7,SEEK_END);
		read(fd,&type,1);
		if(type!=37&&type!=46&&type!=59&&type!=70){
			close(fd);
			return -2;
		}
	}
	bool are = false;
	for(int i=0;i<no_of_sections;i++){
		int countLines = 0;
		lseek(fd,(-hSize+3)+16*i+8,SEEK_END);
		read(fd,&offset,4);
		read(fd,&size,4);
		lseek(fd,offset,SEEK_SET);
		char *buffer = malloc(sizeof(char)*size);
		read(fd,buffer,size);
		for(int index = 0;index<size;index++){
			if(buffer[index]=='\n')
				countLines++;
			if(countLines == 14){
				are= true;
				break;
			}
		}
		free(buffer);
		if(are){
			close(fd);
			return 1;
		}
		
	}
	close(fd);
	return -2;
}

bool suc = false;
int findAll(char *path){

	DIR *dir = opendir(path);
	struct dirent *entry = NULL;
	char  fullPath[512];
	struct stat statbuf;
	if(dir!=NULL){
		while((entry=readdir(dir))!=NULL)
			if(strcmp(entry->d_name,".") && strcmp(entry->d_name,"..")){
				snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
				if(lstat(fullPath,&statbuf) == 0){
					if(S_ISDIR(statbuf.st_mode))
						findAll(fullPath);
					else{
						int rez = checkSF(fullPath);
						if(rez == 1){
							if(suc == false){
								printf("SUCCESS\n");
								suc = true;
							}
							printf("%s\n",fullPath);
						}
					}
				}
			}
		if( suc == false)
			printf("SUCCESS");
		closedir(dir);
		return 0;
	}
	else 
		return -1;
	




}

int main(int argc, char **argv){

    int size = 0;
    
    if(argc >= 2){

        size = argc - 1;
    
        if(strcmp(argv[1], "variant") == 0){
            printf("72803\n");
        }
    	
        
		char *buf; // path-ul de la linia de comanda
      	bool rec = false, aFostAfisat = false, permission = false, seVaLista = false, parsare = false, extr = false, find = false;
      	int value = -1, sect = 0, line = 0;

       	for(int index = 1; index <= size; index++){
       		if(strstr(argv[index],"path=")){
       			strtok(argv[index],"=");
       			buf = strtok(NULL,"=");
       		}
       		if(strstr(argv[index],"list"))
       			seVaLista = true;
       		if(strstr(argv[index],"recursive"))
       			rec = true;
       		if(strstr(argv[index],"size_smaller="))
       			value = atoi(argv[index]+13);
        	if(strstr(argv[index],"has_perm_write"))
        		permission = true;
        	if(strstr(argv[index],"parse"))
        		parsare	= true;
       		if(strstr(argv[index],"extract"))
       			extr = true;
       		if(strstr(argv[index],"section="))
       			sect=atoi(argv[index]+8);
       		if(strstr(argv[index],"line="))
       			line = atoi(argv[index]+5);
       		if(strstr(argv[index],"findall"))
       			find = true;
       	}


        if(seVaLista){
      	
     		if(size == 2)
     			if(afisareDirector(buf)==-1){
     				perror("Invalid path");
     				return 1;
     			}
        

     		if(size == 3){
     			if(rec)
     				if(afisareRecursivaDirector(buf,aFostAfisat) == -1){
     					perror("Invalid path");
     					return 1;
     				}
     			if(value != -1)
     				if(afisareSmallerThan(buf, value) == -1){
     					perror("Invalid path");
     					return 1;
     				}
     			if(permission)
     				if(afisareCuPermisiune(buf)==-1){
     					perror("Invalid path");
     					return 1;
     				}
     		}

     		if(size == 4){
     			if(permission && value == -1)
     				if(afisareRecursivaPermisiva(buf, aFostAfisat) == -1){
     					perror("Invalid path");
     					return 1;
     				}
     			if(value != -1 && !permission)
     				if(afisareRecursivaDimensiune(buf,aFostAfisat , value) == -1){
     					perror("Invlaid path");
     					return 1;
     				}

     		}

        }


        if(parsare){
        	int rez = parsareSF(buf);
        	if(rez==-1){
      			printf("ERROR\nwrong magic");
        		return 1;
        	}
      		if(rez==-3){
      			printf("ERROR\nwrong version");
      			return 1;
      		}
      		if(rez==-4){
      			printf("ERROR\nwrong sect_nr");
      			return 1;
      		}
      		if(rez==-5){
      			printf("ERROR\nwrong sect_types");
      			return 1;
      		}
        	
        }

        if(extr){
        	int rez = extractSF(buf,sect,line);
        	if(rez == -1){
        		printf("ERROR\nInvalid path");
        		return 1;
        	}
        	if(rez == -2){
        		printf("ERROR\nInvalid section");
        		return 1;
        	}
        	if(rez == -3){
        		printf("ERROR\nInvalid line");
        		return 1;
        	}
        }

        if(find)
        	if(findAll(buf)<0){
        		perror("Invalid path");
        		return 1;
        	}
    }

    return 0;
}