#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>


void writer(int shmid1,int shmid2,int num);
int reader(int shmid1,int shmid2);
void down(int sem);
void up(int);
int num2 = 0;//buffer index for consumer

void handler(int signal);

union Semun
{
	int val;               /* value for SETVAL */
	struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
	ushort *array;         /* array for GETALL & SETALL */
	struct seminfo *__buf; /* buffer for IPC_INFO */
	void *__pad;
};

	int sem1,sem2,sem3,shmid_num,shmid_consumer;// these are globals inorder to delete them
	union Semun semun;

/* Produce value(s) */
int main()
{
	/*2 sharedmemorys for num and i
	/*
	*/
	//->>> [500] (char)1
	//->>> [500] (int)1
	
	//int _mem_size=15; // this should be a shared memory buffer which will have a certain size somehow

	//int Messages_size = 25;
	int num =0;// this is maybe useless
	
	int counter = 0;
	//Initializing the shm and sem
	int shmid,key_shm1,key_shm2,key_sem1,key_sem2,key_sem3;
	int Producer_mem_size=15;
	key_shm1= ftok("keyfile",69661);
	key_shm2= ftok("keyfile",69672);
	key_sem1=ftok("keyfile",69683);
	key_sem2=ftok("keyfile",69694);
	key_sem3=ftok("keyfile",69705);
	sem1 = semget(key_sem1, 1, 0666 | IPC_CREAT | IPC_EXCL); // producer semaphore??
	sem2 = semget(key_sem2, 1, 0666 | IPC_CREAT | IPC_EXCL);// consumer semaphore???
	sem3 = semget(key_sem3, 1, 0666 | IPC_CREAT | IPC_EXCL);
	shmid_num = shmget(key_shm1, 4096, IPC_CREAT | 0644);
	shmid_consumer = shmget(key_shm2, 4096, IPC_CREAT | 0644);
	
	
		
	if (shmid_num == -1)
	{
		perror("Error in create shared mem num");
		exit(-1);
	}

	if (shmid_consumer == -1)
	{
		perror("Error in creat shared mem buff");
		exit(-1);
	}
	
	
	if (sem1 == -1 || sem2 == -1)
	{
		sem1 = semget(key_sem1, 1, 0666 | IPC_CREAT); // producer semaphore??
	sem2 = semget(key_sem2, 1, 0666 | IPC_CREAT);// consumer semaphore???
	sem3 = semget(key_sem3, 1, 0666 | IPC_CREAT);
	}
	else
	{    
	semun.val = 1; /* initial value of the semaphore, Binary semaphore *//////////////USED FOR MUTEX LOCK AND UNLOCK AND THUS IS BINARY
	
	if (semctl(sem1, 0, SETVAL, semun) == -1)
	{
		perror("Error in semctl");
		exit(-1);
	}
	
	semun.val = Producer_mem_size;//COunting semaphore, and is used to stop the producer from producing in case the buffer is full
	//this sem denotes how many places are empty
	
	if (semctl(sem2, 0, SETVAL, semun) == -1)
	{
		perror("Error in semctl");
		exit(-1);
	}
	
	semun.val = 0;//COunting semaphore, and is used to stop the consumer from consuming in case the buffer is empty	
	//this sem denotes how many places are filled
	if (semctl(sem3, 0, SETVAL, semun) == -1)
	{
		perror("Error in semctl");
		exit(-1);
	}
	}
	
	while (1) {
		/* Insert into buffer */
		//sleep(1);
		down(sem3);//I am taking one from the buffer, so decrease number of filled slots.
		down(sem1);//you won't enter with me (mutex)
		
		printf("\nI am the consumer and I will consume one product right now\n");
		
		
		//if the loop became bigger than the size of the producer's shared memory it will quit
		
		num = reader(shmid_consumer,shmid_num);// num
		printf("The buffer currently has (before my consumption:%d\n",num);
		num--;
		
		
		writer(shmid_consumer,shmid_num,num);//put this in the sharedmemory	
		num2++;
		if (num2 > 14)
		{
		num2 = 0;
		}
		up(sem1);//You can enter now (mutex unlock)
		up(sem2);//I took one, so increase the empty
		
		//down(sem2);
		//This checks the signal handle I think?
		//pthread_cond_signal (&c_cons);
		signal(SIGINT,handler);
	}
	printf ("producer quiting\n");  fflush (stdout);
	
}
void writer(int shmid1,int shmid2,int num)//shmid 1 is the buffer
{
	int *shmaddr1 = (int*)shmat(shmid1, (void *)0, 0);
	int *shmaddr2 = (int*)shmat(shmid2,(void*)0,0);//takes num
	if (shmaddr1 == -1 || shmaddr2 == -1)
	{
		perror("Error in attach in writer");
		exit(-1);
	}
	
	else
	{
		printf("\nWriter: Shared memory attached at address1 %p\n", shmaddr1);
		
		printf("\nWriter: Shared memory attached at address2 %p\n", shmaddr2);
		*shmaddr2 = num;
		
		printf("\nI have just consumed a product, the number of elements in buffer now is %d, the element I consumed is %d\n", *shmaddr2, shmaddr1[num2]);
	}
	
	shmdt(shmaddr1);
	shmdt(shmaddr2);
}

int reader(int shmid1,int shmid2)
{
	int *shmaddr1 = (int*)shmat(shmid1, (void *)0, 0);
	int *shmaddr2 = (int*)shmat(shmid2, (void *)0, 0);// num
	
	if (shmaddr1 == -1 || shmaddr2 == -1)
	{
		perror("Error in attach in reader");
		exit(-1);
	}
		printf("\nReader: Shared memory attached at address1 %p\n", shmaddr1);
		printf("\nReader: Shared memory attached at address2 %p\n", shmaddr2);
		

		//printf("in the reader The last num wass %d, and the consumed value in array is %d \n", *shmaddr2, shmaddr1[num2]);
	
	return(*shmaddr2);
}	
void down(int sem)
{
	struct sembuf p_op;

	p_op.sem_num = 0;
	p_op.sem_op = -1;
	p_op.sem_flg = !IPC_NOWAIT;

	if (semop(sem, &p_op, 1) == -1)
	{
		perror("Error in down()");
		exit(-1);
	}
}

void up(int sem)
{
	struct sembuf v_op;

	v_op.sem_num = 0;
	v_op.sem_op = 1;
	v_op.sem_flg = !IPC_NOWAIT;

	if (semop(sem, &v_op, 1) == -1)
	{
		perror("Error in up()");
		exit(-1);
   	}
}

void handler(int signal)
{
	shmctl(shmid_num, IPC_RMID, (struct shmid_ds *)0);
	shmctl(shmid_consumer, IPC_RMID, (struct shmid_ds *)0);
	semctl(sem1,1,IPC_RMID,semun);
	semctl(sem2,1,IPC_RMID,semun);
	semctl(sem3,1,IPC_RMID,semun);
	exit(10);
	
}
