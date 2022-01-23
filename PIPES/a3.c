#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#define CONNECT "CONNECT"
#define SUCCESS "SUCCESS"
#define ERROR "ERROR"


int main(){

	unsigned const int varianta = 72803, connect_size = strlen(CONNECT), success_size = strlen(SUCCESS), error_size = strlen(ERROR);
	unsigned int openSHM = 0, nr_create_shm = 0, size = 0, write_smh_offset = 0, write_smh_value = 0, file_name_size=0,offset_file = 0,bytes_file = 0;
	unsigned int sf_section = 0, sf_offset = 0, sf_bytes = 0, logical_offset = 0, logical_bytes = 0;
	int fd_map = -1;
	char *sharedMem=NULL, *x=NULL;
	off_t fd_map_size;

	unlink("RESP_PIPE_72803");	
	if(mkfifo("RESP_PIPE_72803",0600)!=0){
		printf("ERROR\n cannot create the pipe");
	}

	int fd_r = open("REQ_PIPE_72803", O_RDONLY);
	int fd_w = open("RESP_PIPE_72803", O_WRONLY);
	if(write(fd_w,&connect_size,1)==1 && write(fd_w,&CONNECT,connect_size)==7)
		printf("SUCCESS\n");


	while(1){
		read(fd_r,&size,1);
		char *mesaj = (char *)malloc(size*sizeof(char));
		read(fd_r,mesaj,size);
		mesaj[size+1]=0;
		if(strcmp(mesaj,"PING") == 0){
			write(fd_w,&size,1);
			write(fd_w,mesaj,size);
			write(fd_w,&size,1);
			write(fd_w,"PONG",4);
			write(fd_w,&varianta,5);
		}

		if(strcmp(mesaj,"CREATE_SHM")==0){
			read(fd_r,&nr_create_shm,sizeof(nr_create_shm));
			openSHM = shm_open("/m9NsmoE",O_CREAT | O_RDWR, 0644);
			if(openSHM<0){
				write(fd_w,&size,1);
				write(fd_w,mesaj,size);
				write(fd_w,&error_size,1);
				write(fd_w,&ERROR,error_size);
			}

			ftruncate(openSHM,nr_create_shm);
			if(openSHM>=0){
				sharedMem=(char*)mmap(0,nr_create_shm,PROT_WRITE | PROT_READ, MAP_SHARED, openSHM, 0);
				write(fd_w,&size,1);
				write(fd_w,mesaj,size);
				write(fd_w,&success_size,1);
				write(fd_w,&SUCCESS,success_size);
			}

		}

		if(strcmp(mesaj,"WRITE_TO_SHM")==0){
			
			read(fd_r,&write_smh_offset,sizeof(write_smh_offset));
			read(fd_r,&write_smh_value,sizeof(write_smh_value));	
			
			if(write_smh_offset+sizeof(write_smh_value)<=nr_create_shm && write_smh_offset >0 && write_smh_offset<nr_create_shm){
				memcpy((write_smh_offset+sharedMem),&write_smh_value,sizeof(write_smh_value));
				write(fd_w,&size,1);
				write(fd_w,mesaj,size);
				write(fd_w,&success_size,1);
				write(fd_w,SUCCESS,success_size);
			}

			else{
				write(fd_w,&size,1);
				write(fd_w,mesaj,size);
				write(fd_w,&error_size,1);
				write(fd_w,ERROR,error_size);
				//return 1;
			}
		}

		if(strcmp(mesaj,"MAP_FILE") == 0){
			
			read(fd_r,&file_name_size,1);
			char *file_name = (char *)malloc(file_name_size*sizeof(char));
			read(fd_r,file_name,file_name_size);
			file_name[file_name_size] = 0;
			fd_map = open(file_name,O_RDONLY);
			if(fd_map < 0){
				write(fd_w,&size,1);
				write(fd_w,mesaj,size);
				write(fd_w,&error_size,1);
				write(fd_w,ERROR,error_size);
				close(fd_map);				
			}

			fd_map_size = lseek(fd_map,0,SEEK_END);
			lseek(fd_map,0,SEEK_SET);
			x = (char*)mmap(NULL,fd_map_size,PROT_READ,MAP_PRIVATE,fd_map,0);
			write(fd_w,&size,1);
			write(fd_w,mesaj,size);
			write(fd_w,&success_size,1);
			write(fd_w,SUCCESS,success_size);
		}

		if(strcmp(mesaj,"READ_FROM_FILE_OFFSET")==0){

			read(fd_r,&offset_file,sizeof(offset_file));
			printf("%d\n",offset_file);
			read(fd_r,&bytes_file,sizeof(bytes_file));
			printf("%d\n", bytes_file);

			void *buffer = malloc(bytes_file);
			if((offset_file+bytes_file)<=fd_map_size){
				memcpy(buffer,x+offset_file,bytes_file);
				memcpy(sharedMem,buffer,bytes_file);
				write(fd_w,&size,1);
				write(fd_w,mesaj,size);
				write(fd_w,&success_size,1);
				write(fd_w,SUCCESS,success_size);
			}
			else{
				write(fd_w,&size,1);
				write(fd_w,mesaj,size);
				write(fd_w,&error_size,1);
				write(fd_w,ERROR,error_size);

			}
		}

		if(strcmp(mesaj,"READ_FROM_FILE_SECTION")==0){
			read(fd_r,&sf_section,sizeof(sf_section));
			read(fd_r,&sf_offset,sizeof(sf_offset));
			read(fd_r,&sf_bytes,sizeof(sf_bytes));
			int header_size = 0;
			memcpy(&header_size,x+fd_map_size-6,2);
			int no_sections = 0;
			memcpy(&no_sections,x+fd_map_size-header_size+2,1);
			int offset_sections = 0;
			memcpy(&offset_sections,x+fd_map_size -header_size + 3 +(sf_section-1)*16+8,4);
			int size_sections = 0;
			memcpy(&size_sections,x+fd_map_size -header_size + 3 +(sf_section-1)*16+12,4);

			if(sf_section<=no_sections && sf_offset+sf_bytes<=size_sections){
				memmove(sharedMem,x+offset_sections+sf_offset,sf_bytes);
				write(fd_w,&size,1);
				write(fd_w,mesaj,size);
				write(fd_w,&success_size,1);
				write(fd_w,SUCCESS,success_size);
			}else{
				write(fd_w,&size,1);
				write(fd_w,mesaj,size);
				write(fd_w,&error_size,1);
				write(fd_w,ERROR,error_size);
				
			}
		}

		if(strcmp(mesaj,"READ_FROM_LOGICAL_SPACE_OFFSET")==0){
		
			read(fd_r,&logical_offset,sizeof(logical_offset));
			read(fd_r,&logical_bytes,sizeof(logical_bytes));

			int count_sections = 0, dimensiune_header = 0;
			memcpy(&dimensiune_header,x+fd_map_size-6,2);
			memcpy(&count_sections,x+fd_map_size-dimensiune_header+2,1);

			int last_addr = 0, current_size = 0, current_offset = 0;
			for(int i=1;i<=count_sections;i++){
				memcpy(&current_offset,x+fd_map_size-dimensiune_header+3+(i-1)*16+8,4);
				memcpy(&current_size,x+fd_map_size-dimensiune_header+3+(i-1)*16+12,4);
			
				last_addr += current_size%1024 == 0 ? (current_size/1024)*1024 : (current_size/1024+1)*1024;
				if(logical_offset+current_offset-last_addr+logical_bytes>fd_map_size){
						write(fd_w,&size,1);
						write(fd_w,mesaj,size);
						write(fd_w,&error_size,1);
						write(fd_w,ERROR,error_size);
				}
				else if(last_addr > logical_offset){
					last_addr -= current_size%1024 == 0 ? (current_size/1024)*1024 : (current_size/1024+1)*1024;	
					memmove(sharedMem,x+logical_offset+current_offset-last_addr,logical_bytes);
					write(fd_w,&size,1);
					write(fd_w,mesaj,size);
					write(fd_w,&success_size,1);
					write(fd_w,SUCCESS,success_size);
					break;
				}
			}

		}


		if(strcmp(mesaj,"EXIT")==0){
			unlink("RESP_PIPE_72803");
			close(fd_r);
			close(fd_w);
			return 0;
		}
	
	}

	return 0;
}