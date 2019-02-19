#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_TIME 1440 //number of seconds to simulate a day
#define MAX_VISITS 20 //max number of visits possible

struct thread_args {
  int flag;
  float arrivalTime;
  float costumingTime;
  int numVisits;
  int id;
};

struct costumingTeam {
  int id;
  bool isFree;
  float busyTime;
  float freeTime;
};

struct customer {
  int flag;   //0 for pirates, 1 for ninjas
  int id;
  int visits;
  pthread_t thread;
  int costumingTeam[MAX_VISITS];
  float arrivalTime[MAX_VISITS];
	float costumingTime[MAX_VISITS];
  float eventTime;
  float waitTime[MAX_VISITS];
  bool isDone;
  struct customer *next;
};

struct customer* initializeCustomers(int costuming, int pirates, int ninjas, float costumingTimeP, float costumingTimeN, float arrivalTimeP, float arrivalTimeN);
float randomizeTime(float upper);
void singleDaySim(int costuming, int pirates, int ninjas, float costumingTimeP, float costumingTimeN, float arrivalTimeP, float arrivalTimeN, struct customer *customerHead, struct costumingTeam *arrayTeams);
void sortQueue(int size, struct customer *queue);
bool willVisitAgain();

pthread_mutex_t shop_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pir = PTHREAD_COND_INITIALIZER;
pthread_cond_t nin = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[]) {
	int costuming, pirates, ninjas;
	float costumingTimeP, costumingTimeN, arrivalTimeP, arrivalTimeN;

	if (argc != 8) {		//test to see if the command line has the correct number of arguments
		printf("Usage: ./phase1_project3 #costumingteams #pirates #ninjas costumingTimePirates costumingTimeNinjas arrivalTimePirates arrivalTimeNinjas\n");
		return EXIT_FAILURE;
	} else {
		for (int i = 1; i < argc; i++){
			switch(i){
			case 1:
				costuming = atoi(argv[i]);
        if((costuming < 2) || (costuming > 4)){
          printf("# of costuming teams has to be between 2 and 4\n");
      		return EXIT_FAILURE;
        }
				break;
			case 2:
				pirates = atoi(argv[i]);
        if((pirates < 10) || (pirates > 50)){
          printf("# of pirates has to be between 10 and 50\n");
          return EXIT_FAILURE;
        }
				break;
			case 3:
				ninjas = atoi(argv[i]);
        if((ninjas < 10) || (ninjas > 50)){
          printf("# of ninjas has to be between 10 and 50\n");
          return EXIT_FAILURE;
        }
				break;
			case 4:
				costumingTimeP = atof(argv[i]);
				if((costumingTimeP < 0) || (costumingTimeP > MAX_TIME)){
					printf("costuming time for pirates has to be between 0 and 1440\n");
					return EXIT_FAILURE;
				}
				break;
			case 5:
				costumingTimeN = atof(argv[i]);
				if((costumingTimeN < 0) || (costumingTimeN > MAX_TIME)){
					printf("costuming time for ninjas has to be between 0 and 1440\n");
					return EXIT_FAILURE;
				}
				break;
			case 6:
				arrivalTimeP = atof(argv[i]);
				if((arrivalTimeP < 0) || (arrivalTimeP > MAX_TIME)){
					printf("Arrival time for pirates has to be between 0 and 1440\n");
					return EXIT_FAILURE;
				}
				break;
			case 7:
				arrivalTimeN = atof(argv[i]);
				if((arrivalTimeN < 0) || (arrivalTimeN > MAX_TIME)){
					printf("Arrival time for ninjas has to be between 0 and 1440\n");
					return EXIT_FAILURE;
				}
				break;
			}
		}
	}

	srand(time(NULL));

  struct costumingTeam* arrayTeams = malloc(costuming * sizeof(*arrayTeams));
  for(int i = 1; i <= costuming; i++){
    arrayTeams[i].id = i;
    arrayTeams[i].isFree = true;
  }

  struct customer *customerHead;
  customerHead = (struct customer*)malloc(sizeof(struct customer));
  customerHead = initializeCustomers(costuming, pirates, ninjas, costumingTimeP, costumingTimeN, arrivalTimeP, arrivalTimeN);

	singleDaySim(costuming, pirates, ninjas, costumingTimeP, costumingTimeN, arrivalTimeP, arrivalTimeN, customerHead, arrayTeams);
	return EXIT_SUCCESS;
}

struct customer* initializeCustomers(int costuming, int pirates, int ninjas, float costumingTimeP, float costumingTimeN, float arrivalTimeP, float arrivalTimeN){
  int counter = 1;

  struct customer *customerHead;
  customerHead = (struct customer*)malloc(sizeof(struct customer));
  customerHead->flag = 0;
  customerHead->id = counter;
  customerHead->visits = 1;
  //customerHead->costumingTeam[0] = 0;
  customerHead->arrivalTime[0] = randomizeTime(arrivalTimeP);
  customerHead->costumingTime[0] = randomizeTime(costumingTimeP);
  customerHead->eventTime = customerHead->arrivalTime[0];
  customerHead->waitTime[0] = 0;
  customerHead->isDone = false;
  customerHead->next = NULL;
  counter++;

  struct customer *temp1;
  temp1 = (struct customer*)malloc(sizeof(struct customer));
  temp1 = customerHead;
  for(int i = 0; i < (pirates - 1); i++){
    struct customer *temp2;
    temp2 = (struct customer*)malloc(sizeof(struct customer));
    temp2->flag = 0;
    temp2->id = counter;
    temp2->visits = 1;
    //temp2->costumingTeam[0] = 0;
    temp2->arrivalTime[0] = randomizeTime(arrivalTimeP);
    temp2->costumingTime[0] = randomizeTime(costumingTimeP);
    temp2->eventTime = temp2->arrivalTime[0];
    temp2->waitTime[0] = 0;
    temp2->isDone = false;
    temp2->next = NULL;
    counter++;
    temp1->next = temp2;
    temp1 = temp2;
  }
  for(int i = 0; i < ninjas; i++){
    struct customer *temp2;
    temp2 = (struct customer*)malloc(sizeof(struct customer));
    temp2->flag = 1;
    temp2->id = counter;
    temp2->visits = 1;
    //temp2->costumingTeam[0] = 1;
    temp2->arrivalTime[0] = randomizeTime(arrivalTimeN);
    temp2->costumingTime[0] = randomizeTime(costumingTimeN);
    temp2->eventTime = temp2->arrivalTime[0];
    temp2->waitTime[0] = 0;
    temp2->isDone = false;
    temp2->next = NULL;
    counter++;
    temp1->next = temp2;
    temp1 = temp2;
  }
  return customerHead;
}

float randomizeTime(float upper){
  float a = (float)rand()/(float)(RAND_MAX/1);
  float b = (float)rand()/(float)(RAND_MAX/1);
  float r = sqrt(-2*log(a))*cos(2*M_PI*b);
  float final = r + upper;
  if(final < 0){
    final = final * (-1);
  }
  if(final > MAX_TIME){
    final =  randomizeTime(upper);
  }
  return final;
}

void singleDaySim(int costuming, int pirates, int ninjas, float costumingTimeP, float costumingTimeN, float arrivalTimeP, float arrivalTimeN, struct customer *customerHead, struct costumingTeam *arrayTeams){
  float time = 0;
  int size = pirates+ninjas;
  //int numVisits = 0;
	//float totalQueueLength = 0, averageQueueLength, grossRevenue = 0, goldePerVisit, totalProfits;
  struct customer* save = malloc(sizeof(*save));
  save = customerHead;
  struct customer* queue = malloc((size) * sizeof(*queue));
  struct thread_args *args[size];

  int counter = 0;
  for(int i = 0; i < size; i++){
    printf("%d\n", counter);
    counter++;
    args[i] = (struct thread_args *) malloc(sizeof(struct thread_args));
    args[i]->flag = customerHead->flag;
    args[i]->arrivalTime = customerHead->arrivalTime[0];
    args[i]->costumingTime = customerHead->costumingTime[0];
    args[i]->numVisits = customerHead->visits;
    args[i]->id = customerHead->id;
    pthread_create(&customerHead->thread, NULL, (void *) &singleDaySim, (void *) args[i]);
    queue[i] = *customerHead;
    customerHead = customerHead->next;
  }
  printf("si?\n");
  sortQueue(size, queue);
  printf("no?\n");
  customerHead = save;
  for(int i = 0; i < size; i++){
    printf("si holaaa\n");
    pthread_join(quethread, NULL);
    printf("no pikoss\n");
  }

  while(time <= MAX_TIME){
    if(size == 0){
      time = MAX_TIME+1;
    } else {
      time += (queue[0].eventTime - time);
      int visitNum = queue[0].visits - 1;
      if(queue[0].isDone == false){
        queue[0].eventTime += queue[0].costumingTime[visitNum];
        queue[0].isDone = true;
      } else {
        queue[0].eventTime = 2000;
        sortQueue(size, queue);
        size -= 1;
      }
      sortQueue(size, queue);
    }
  }
  for(int i = 0; i < (pirates+ninjas); i++){
    printf("queue %d: %f\n", i, queue[i].eventTime);
  }

  bool puto = willVisitAgain();
/*
  //Print statistics
  for(int j = 0; j < (pirates+ninjas-1); j++){
    float goldOwed = 0;
    if(customerHead->flag == 0){
      printf("Pirate #%d:\n", customerHead->id);
    } else if(customerHead->flag == 1){
      printf("Ninja #%d:\n", customerHead->id);
    }
    printf("Number of visits: %d\n", customerHead->visits);
    for(int i = 0; i < customerHead->visits; i++){
      printf("Visit #%d:\n", i+1);
      printf("\tCostuming time: %f\n", customerHead->costumingTime[i]);
      printf("\tWait time: %f\n", customerHead->waitTime[i]);
      totalQueueLength += customerHead->waitTime[i];
      if(customerHead->waitTime[i] <= 30){
        goldOwed += customerHead->costumingTime[i];
      }
      arrayTeams[customerHead->costumingTeam[i]].busyTime +=  customerHead->costumingTime[i];
    }
    printf("Total gold owed: %f\n\n", goldOwed);
    numVisits += customerHead->visits;
    grossRevenue += goldOwed;
    customerHead = customerHead->next;
  }

  averageQueueLength = (totalQueueLength)/(numVisits);
  goldePerVisit = (grossRevenue)/(numVisits);
  totalProfits = (grossRevenue)-(5*costuming);

  for(int i = 1; i <= costuming; i++){
    printf("Time costuming team %d was busy: %f\n", i, arrayTeams[i].busyTime);
    printf("Time costuming team %d was free: %f\n", i, arrayTeams[i].freeTime);
  }
	printf("Average queue length: %f\n", averageQueueLength);
	printf("Gross revenue: %f\n", grossRevenue);
	printf("Gold per visit: %f\n", goldePerVisit);
	printf("Total profits: %f\n", totalProfits);*/
}

void sortQueue(int size, struct customer *queue){
  for(int i = 0; i < (size-1); i++){
    for(int j = 0; j < (size-i-1);j++){
      if(queue[j].eventTime > queue[j+1].eventTime){
        struct customer *temp;
        temp = (struct customer*)malloc(sizeof(struct customer));
        temp->flag = queue[j].flag;
        temp->id = queue[j].id;
        temp->visits = queue[j].visits;
        temp->thread = queue[j].thread;
        temp->costumingTeam[0] = queue[j].costumingTeam[0];
        temp->arrivalTime[0] = queue[j].arrivalTime[0];
        temp->costumingTime[0] = queue[j].costumingTime[0];
        temp->eventTime = queue[j].eventTime;
        temp->waitTime[0] = queue[j].waitTime[0];
        temp->isDone = queue[j].isDone;
        temp->next = queue[j].next;
        queue[j] = queue[j+1];
        queue[j+1] = *temp;
        free(temp);
      }
    }
  }
}

bool willVisitAgain(){
  bool answer;
  int r = (rand() % (100))+1;
  if (r <= 25) {
    answer = true;
  } else {
    answer = false;
  }
  return answer;
}
