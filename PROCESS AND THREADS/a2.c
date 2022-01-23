#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include "a2_helper.h"


sem_t sph1,sph2;
pthread_mutex_t mut1,mut2;

int count=0,ok=0,done=1;

typedef struct {
	int id,source;
	sem_t *sem1;
	sem_t *sem2;
	pthread_mutex_t *lock;
} TH_STRUCT;



void *thread_sec2(void *p){
	TH_STRUCT *s = (TH_STRUCT*)p;

	if(s->source == 8 && s->id == 2)
		sem_wait(s->sem1);
	
	sem_t *namedS1 = sem_open("semaforu1",O_CREAT,0644,0);	
	if(s->source == 8 && s->id == 3)
		sem_wait(namedS1);

	sem_t *namedS2 = sem_open("semforu2",O_CREAT,0644,0);
	if(s->source == 9 && s->id == 1){
		sem_wait(namedS2);		
	}

	info(BEGIN,s->source,s->id);

	if(s->source == 8 && s->id == 4){
		sem_post(s->sem1);
		sem_wait(s->sem2);
	}

	info(END,s->source,s->id);


	if(s->source == 9 && s->id == 2){
		sem_post(namedS1);
	}

	if(s->source == 8 && s->id == 3)
		sem_post(namedS2);

	if(s->source == 8 && s->id == 2)
		sem_post(s->sem2);

	return NULL;
}

void *thread_sec3(void *p){
	TH_STRUCT *s = (TH_STRUCT *)p;

	//intra maxim 6 thread-uri
	sem_wait(&sph1);
	info(BEGIN,2,s->id);


	pthread_mutex_lock(s->lock);
	count++;
	pthread_mutex_unlock(s->lock);

	if(s->id!=14){
			for(;;){
				if(ok==0)
					break;
			}
			sem_wait(&sph2);
			count --;
			done++;
			info(END,2,s->id);
			sem_post(&sph2);		
	}
	else{
		ok = 1;
		for(;;){
			sem_wait(&sph2);
			if(count == 6 || done==47){
				info(END,2,14);
				count--;
				ok = 0;
				sem_post(&sph2);
				break;
			}
			sem_post(&sph2);
		}
	}

	sem_post(&sph1);
	return NULL;
}

void *thread_sec4(void *p){

	TH_STRUCT *s = (TH_STRUCT*)p;

	if(s->id == 1)
		pthread_mutex_lock(&mut1);
	info(BEGIN,9,s->id);

	//deblocat unu blocat altu`
	if(s->id == 2){
		pthread_mutex_unlock(&mut1);
		pthread_mutex_lock(&mut2);
	}
	info(END,9,s->id);

	if(s->id == 3)
		pthread_mutex_unlock(&mut2);

	return NULL;
}

int main(){
    init();

    info(BEGIN, 1, 0);

    pid_t pid2, pid3, pid4, pid5, pid6, pid7, pid8, pid9;
    

    pthread_t tids8 [4];
    TH_STRUCT params8 [4];
    sem_t sem8;
    sem_t sem8_1;
    sem_init(&sem8,0,0);
    sem_init(&sem8_1,0,0);

    pthread_t tids2 [47];
    TH_STRUCT params2 [47];
    pthread_mutex_t lacat = PTHREAD_MUTEX_INITIALIZER;
    sem_init(&sph1,0,6);
    sem_init(&sph2,0,1);


    pthread_t tids9 [4];
    TH_STRUCT params9 [4];



    pid2 = fork();
    if(pid2 == 0){
    	info(BEGIN,2,0);

    	for(int i=0;i<47;i++){
    		params2[i].id = i+1;
    		params2[i].lock = &lacat;
    		pthread_create(&tids2[i],NULL,thread_sec3,&params2[i]);
    	}

    	pid6 = fork();
    	if(pid6 == 0){
    		info(BEGIN,6,0);
    		info(END,6,0);
    	}
    	else{
    		pid8 = fork();
    		if(pid8 == 0){
    			info(BEGIN,8,0);
    			for(int i=0;i<4;i++){
    				params8[i].id = i+1;
    				params8[i].sem1 = &sem8;
    				params8[i].sem2 = &sem8_1;
    				params8[i].source = 8;	
    				if(pthread_create(&tids8[i],NULL,thread_sec2,&params8[i])){
    					perror("Cant create the thread");
    					return 1;
    				}
    			}
    			for(int i=0;i<4;i++)
    				pthread_join(tids8[i],NULL);

    			info(END,8,0);
			}
			else{
				waitpid(pid6,NULL,0);
				waitpid(pid8,NULL,0);
				for(int i=0;i<47;i++)
					pthread_join(tids2[i],NULL);
				info(END,2,0);
			}
    	}
    }
    else{
    	pid3 = fork();
    	if(pid3 == 0){
    		info(BEGIN,3,0);
    		pid4 = fork();
    		if(pid4 == 0){
    			info(BEGIN,4,0);
    			pid5 = fork();
    			if(pid5 == 0){
    				info(BEGIN,5,0);
    				pid9 = fork();
    				if(pid9 == 0){
    					info(BEGIN,9,0);
    					
    					for(int i=0;i<4;i++){
    						params9[i].id = i+1;
    						params9[i].source = 9;
    						pthread_create(&tids9[i],NULL,thread_sec2,&params9[i]);
    					}

    					for(int i=0;i<4;i++)
    						pthread_join(tids9[i],NULL);

    					info(END,9,0);
    				}
    				else{
    					waitpid(pid9,NULL,0);
    					info(END,5,0);
    				}
    			}
    			else{
    				waitpid(pid5,NULL,0);
    				info(END,4,0);
    			}
    		}
    		else{
    			pid7 = fork();
    			if(pid7 == 0){
    				info(BEGIN,7,0);
    				info(END,7,0);
    			}
    			else{
    				waitpid(pid4,NULL,0);
    				waitpid(pid7,NULL,0);
    				info(END,3,0);
    			}
    		}
    	}
    	else{
    		waitpid(pid2,NULL,0);
    		waitpid(pid3,NULL,0);
   			info(END,1,0);
    	}
    }


    return 0;
}
