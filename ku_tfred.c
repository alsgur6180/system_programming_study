#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>
#include<fcntl.h>

int k = 0;
int freq[100] = {0};
pthread_mutex_t mutex;

typedef struct data{
	int nop;
	int size;
	int interval;
	char* file_name;
	int fp;
	int size_byte;
}DATA;

void* frequency(void* data){
	char buffer[6] = {0};
	int num = 0;
	int index = 0;
	int i = 0;
	DATA* temp;
	temp = (DATA*)malloc(sizeof(DATA));
	temp =(DATA*)data;
	int fd = temp->fp;
	int interval = temp->interval;
	char* name = temp->file_name;
	int range = temp->size/temp->nop;
	int size_byte = temp->size_byte;
	pthread_mutex_lock(&mutex);
	int j = k;
	k++;
	pthread_mutex_unlock(&mutex);
	int start = j*range;
	int end = (j+1)*range;
	if(j+1 == temp->nop)end = temp->size;
	for(i =start;i<end;i++){
		pread(fd,buffer,5,i*5+size_byte);
		num = atoi(buffer);
		memset(buffer,0,6);
		index = num/interval;
		pthread_mutex_lock(&mutex);
		freq[index]++;
		pthread_mutex_unlock(&mutex);
	}
}

int main(int argc,char* argv[]) {
	pthread_t p_thread[10];
	int thr_id;
	int i;
	char temp[15];
	DATA data = {0};
	FILE* fp = fopen(argv[3],"r");
	fgets(temp,15,fp);
	data.size = atoi(temp);
	data.size_byte = strlen(temp);
	fclose(fp);
	int fd;
	fd = open(argv[3],O_CREAT|O_RDWR);
	data.nop = atoi(argv[1]);
	data.interval = atoi(argv[2]);
	data.file_name = argv[3];
	data.fp = fd;
	int range = data.size/data.nop;

	int result_number = 10000/data.interval;
	pthread_mutex_init(&mutex,NULL);
	for(i=0;i<data.nop;i++){
		thr_id = pthread_create(&p_thread[i],NULL,frequency,(void*)&data);
		if(thr_id<0){
			perror("pthread_create()");
			exit(0);
		}
	}
	for(i=0;i<data.nop;i++){
		pthread_join(p_thread[i],NULL);
	}
	for(i = 0;i<result_number;i++){
		printf("%d\n",freq[i]);
	}
	close(fd);
	pthread_mutex_destroy(&mutex);
	return 0;
}

