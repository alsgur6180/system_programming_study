#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<mqueue.h>
#include<sys/wait.h>
#include<sys/types.h>

#define MSG_SIZE 4
#define NAME "/m_queue"
#define MAX_MSG 10
#define MAX_PROCESS 10

typedef struct data{
        int nop;
        int size;
        int interval;
        char* file_name;
        int fp;
        int size_byte;
	int number;
}DATA;
int* frequency(DATA* temp){
        char buffer[6] = {0};
        int num = 0;
        int index = 0;
        int i = 0;
	int* freq;
        int fd = temp->fp;
        int interval = temp->interval;
	freq = (int*)malloc(sizeof(int)*(10000/interval));
	memset(freq,0,sizeof(int)*(10000/interval));
        char* name = temp->file_name;
        int range = temp->size/temp->nop;
        int size_byte = temp->size_byte;
        int start = temp->number*range;
        int end = (temp->number+1)*range;
        if(temp->number+1 == temp->nop)end = temp->size;
        for(i =start;i<end;i++){
                pread(fd,buffer,5,i*5+size_byte);
                num = atoi(buffer);
                memset(buffer,0,6);
                index = num/interval;
		freq[index]++;
	}
	return freq;
}

int main(int argc,char* argv[]) {
	int i;
	int pn;//process number
	char *buffer;
	buffer = (char*)malloc(sizeof(char)*10);
	int state;
	int value;
	int index;
	int*temper;
        char temp[15];
	int prio;
        DATA data = {0};
        FILE* fp = fopen(argv[3],"r");
        fgets(temp,15,fp);
        data.size = atoi(temp);
        data.size_byte = strlen(temp);
        fclose(fp);
        int fd;
        fd = open(argv[3],O_CREAT|O_RDWR);
        data.nop = atoi(argv[1]);
	pid_t pid;
        data.interval = atoi(argv[2]);
        data.file_name = argv[3];
        data.fp = fd;
        int range = data.size/data.nop;
	int *freq;
        int result_number = 10000/data.interval;
	freq = (int*)malloc(sizeof(int)*result_number);
	memset(freq,0,sizeof(int)*result_number);	
	struct mq_attr attr;
	mqd_t mqdes[MAX_PROCESS];

	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MSG_SIZE;
	for(pn = 0;pn<data.nop;pn++){
		mqdes[pn] = mq_open(NAME,O_CREAT|O_RDWR,0666,&attr);
		if(mqdes[pn]<0){
		perror("mq_open()");
		exit(0);
		}
	}
	
	for(pn=0;pn<data.nop;pn++){	
		data.number = pn;
		pid = fork();
		if(pid ==0)break;
	}
	if(pid == 0){
		temper = frequency(&data);
		for(i=0;i<result_number;i++){
			if(mq_send(mqdes[data.number],(char*)&temper[i],MSG_SIZE,i)==-1){
				perror("mq_send()");
				exit(0);
			}
		}
		close(fd);
		mq_close(mqdes[data.number]);
		
		exit(0);
	}
	if(pid>0){
		for (i = 0; i < data.nop; i++) {
			wait(&state);
		}
		value = 0;
		int parent =0;
		for(parent =0;parent<data.nop;parent++){
			while(mq_receive(mqdes[parent],(char*)&value,MSG_SIZE,&i)!=-1){
				freq[i] += value;
				mq_getattr(mqdes[parent],&attr);
				if(attr.mq_curmsgs ==0)break;

			}
		}
		for(i=0;i<result_number;i++){
			printf("%d\n",freq[i]);
		}
		close(fd);
		for(parent = 0;parent<data.nop;parent++){
			mq_close(mqdes[parent]);
		}
	}
	free(frequency(&data));
	free(buffer);
	free(freq);
	mq_unlink(NAME);
	
	return 0 ;
}
