#include <types.h>
#include <lib.h>
#include <test.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <synchprobs.h>

static void initialize_bowls(void);
static void cleanup_bowls(void);
static void cat_eat(unsigned int bowlnumber, int eat_time);
static void cat_sleep(int sleep_time);
static void mouse_eat(unsigned int bowlnumber, int eat_time);
static void mouse_sleep(int sleep_time);
static void cat_simulation(void *ptr, unsigned long catnumber);
static void mouse_simulation(void *ptr, unsigned long mousenumber);

static int NumBowls;  // number of food bowls
static int NumCats;   // number of cats
static int NumMice;   // number of mice
static int NumLoops;  // number of times each cat and mouse should eat

static int CatEatTime = 1;      // length of time a cat spends eating
static int CatSleepTime = 2;    // length of time a cat spends sleeping
static int MouseEatTime = 1;    // length of time a mouse spends eating
static int MouseSleepTime = 2;  // length of time a mouse spends sleeping

 static struct semaphore *CatMouseWait;

static volatile char *bowls;

 static volatile int eating_cats_count;

static volatile int eating_mice_count;
static struct semaphore *mutex;
static volatile time_t cat_total_wait_secs;
static volatile uint32_t cat_total_wait_nsecs;
static volatile int cat_wait_count;
static volatile time_t mouse_total_wait_secs;
static volatile uint32_t mouse_total_wait_nsecs;
static volatile int mouse_wait_count;

static struct semaphore *perf_mutex;

static void
initialize_bowls()
	{
		int i;

		KASSERT(NumBowls > 0);
 
		bowls = kmalloc(NumBowls*sizeof(char));
		if (bowls == NULL) 
		{
		panic("initialize_bowls: unable to allocate space for %d bowls\n",NumBowls);
		}

		for(i=0;i<NumBowls;i++) {
		bowls[i] = '-';
	}
		eating_cats_count = eating_mice_count = 0;
			mutex = sem_create("bowl mutex",1);
	if (mutex == NULL) {
     panic("initialize_bowls: could not create mutex\n");
   }
   perf_mutex = sem_create("stats mutex",1);
   if (perf_mutex == NULL) {
     panic("initialize_bowls: could not create perf_mutex\n");
   }
   cat_total_wait_secs = 0;
   cat_total_wait_nsecs = 0;
   cat_wait_count = 0;
   mouse_total_wait_secs = 0;
   mouse_total_wait_nsecs = 0;
   mouse_wait_count = 0;
   return;
 }

 static void
 cleanup_bowls()
 {
   if (mutex != NULL) {
     sem_destroy( mutex );
     mutex = NULL;
   }
   if (perf_mutex != NULL) {
     sem_destroy( perf_mutex );
     perf_mutex = NULL;
   }
   if (bowls != NULL) {
     kfree( (void *) bowls );
     bowls = NULL;
   }
 }


 void
 cat_eat(unsigned int bowlnumber, int eat_time)
 {

 KASSERT(bowlnumber > 0);
   KASSERT((int)bowlnumber <= NumBowls);

   P(mutex);

   if (bowls[bowlnumber-1] == 'c') {

   panic("cat_eat: attempt to make two cats eat from bowl %d!\n",bowlnumber);
   }
   if (eating_mice_count > 0) {

   panic("cat_eat: attempt to make a cat eat while mice are eating!\n");
   }
   KASSERT(bowls[bowlnumber-1]=='-');
   KASSERT(eating_mice_count == 0);

   eating_cats_count += 1;
   bowls[bowlnumber-1] = 'c';

   DEBUG(DB_SYNCPROB,"cat starts to eat at bowl %d [%d:%d]\n",
         bowlnumber,eating_cats_count,eating_mice_count);
   V(mutex);  // end critical section

   clocksleep(eat_time);

   P(mutex);  // start critical section
   KASSERT(eating_cats_count > 0);
   KASSERT(bowls[bowlnumber-1]=='c');
   eating_cats_count -= 1;
   bowls[bowlnumber-1]='-';

   DEBUG(DB_SYNCPROB,"cat finished eating at bowl %d [%d:%d]\n",
         bowlnumber,eating_cats_count,eating_mice_count);
   V(mutex);  // end critical section
 
   return;
 }



 void
 cat_sleep(int sleep_time)
 {

 clocksleep(sleep_time);
   return;
 }


 void
 mouse_eat(unsigned int bowlnumber, int eat_time)
 {

 KASSERT(bowlnumber > 0);
   KASSERT((int)bowlnumber <= NumBowls);

   P(mutex);  // start critical section

   /* violate any simulation requirements */
   if (bowls[bowlnumber-1] == 'm') {

   panic("mouse_eat: attempt to make two mice eat from bowl %d!\n",bowlnumber);
   }
   if (eating_cats_count > 0) {

   panic("mouse_eat: attempt to make a mouse eat while cats are eating!\n");
   }
   KASSERT(bowls[bowlnumber-1]=='-');
   KASSERT(eating_cats_count == 0);

   eating_mice_count += 1;
   bowls[bowlnumber-1] = 'm';
 
   DEBUG(DB_SYNCPROB,"mouse starts to eat at bowl %d [%d:%d]\n",
         bowlnumber,eating_cats_count,eating_mice_count);
   V(mutex);  // end critical section
 
   clocksleep(eat_time);
   P(mutex); // start critical section
 
   KASSERT(eating_mice_count > 0);
   eating_mice_count -= 1;
   KASSERT(bowls[bowlnumber-1]=='m');
   bowls[bowlnumber-1]='-';
 
   DEBUG(DB_SYNCPROB,"mouse finishes eating at bowl %d [%d:%d]\n",
         bowlnumber,eating_cats_count,eating_mice_count);
   V(mutex);  // end critical section
   return;
 }


 void
 mouse_sleep(int sleep_time)
 {
   clocksleep(sleep_time);
   return;
 }


 static
 void
 cat_simulation(void * unusedpointer, 
                unsigned long catnumber)
 {
   int i;
   unsigned int bowl;
   time_t before_sec, after_sec, wait_sec;
   uint32_t before_nsec, after_nsec, wait_nsec;
 
   (void) unusedpointer;
   (void) catnumber;
 
 
   for(i=0;i<NumLoops;i++) {

   cat_sleep(CatSleepTime);

   bowl = ((unsigned int)random() % NumBowls) + 1;
 
     gettime(&before_sec,&before_nsec);
    cat_before_eating(bowl); /* student-implemented function */
     gettime(&after_sec,&after_nsec);

     cat_eat(bowl, CatEatTime);

     cat_after_eating(bowl); /* student-implemented function */


     getinterval(before_sec,before_nsec,after_sec,after_nsec,&wait_sec,&wait_nsec);
     P(perf_mutex);
     cat_total_wait_secs += wait_sec;
     cat_total_wait_nsecs += wait_nsec;
     if (cat_total_wait_nsecs > 1000000000) {
       cat_total_wait_nsecs -= 1000000000;
       cat_total_wait_secs ++;
     }
     cat_wait_count++;
     V(perf_mutex);
   }

   V(CatMouseWait); 
 }


 static
 void
 mouse_simulation(void * unusedpointer,
           unsigned long mousenumber)
 {
   int i;
   unsigned int bowl;
   time_t before_sec, after_sec, wait_sec;
   uint32_t before_nsec, after_nsec, wait_nsec;

   (void) unusedpointer;
   (void) mousenumber;

   for(i=0;i<NumLoops;i++) {
		mouse_sleep(MouseSleepTime);

		bowl = ((unsigned int)random() % NumBowls) + 1;

		gettime(&before_sec,&before_nsec);
     mouse_before_eating(bowl); /* student-implemented function */
     gettime(&after_sec,&after_nsec);

     mouse_eat(bowl, MouseEatTime);

     mouse_after_eating(bowl); /* student-implemented function */

     getinterval(before_sec,before_nsec,after_sec,after_nsec,&wait_sec,&wait_nsec);
     P(perf_mutex);
     mouse_total_wait_secs += wait_sec;
     mouse_total_wait_nsecs += wait_nsec;
     if (mouse_total_wait_nsecs > 1000000000) {
       mouse_total_wait_nsecs -= 1000000000;
       mouse_total_wait_secs ++;
     }
     mouse_wait_count++;
     V(perf_mutex);
   }
 
   V(CatMouseWait); 
 }


 int
 catmouse(int nargs,
          char ** args)
 {
   int catindex, mouseindex, error;
   int i;
   int mean_cat_wait_usecs, mean_mouse_wait_usecs;
   time_t before_sec, after_sec, wait_sec;
   uint32_t before_nsec, after_nsec, wait_nsec;
   int total_bowl_milliseconds, total_eating_milliseconds, utilization_percent;


   if ((nargs != 9) && (nargs != 5)) {
     kprintf("Usage: <command> NUM_BOWLS NUM_CATS NUM_MICE NUM_LOOPS\n");
     kprintf("or\n");
     kprintf("Usage: <command> NUM_BOWLS NUM_CATS NUM_MICE NUM_LOOPS ");
     kprintf("CAT_EATING_TIME CAT_SLEEPING_TIME MOUSE_EATING_TIME MOUSE_SLEEPING_TIME\n");
     return 1;  // return failure indication
   }
 

 NumBowls = atoi(args[1]);
   if (NumBowls <= 0) {
     kprintf("catmouse: invalid number of bowls: %d\n",NumBowls);
     return 1;
   }
   NumCats = atoi(args[2]);
   if (NumCats < 0) {
     kprintf("catmouse: invalid number of cats: %d\n",NumCats);
     return 1;
   }
   NumMice = atoi(args[3]);
   if (NumMice < 0) {
     kprintf("catmouse: invalid number of mice: %d\n",NumMice);
     return 1;
   }
   NumLoops = atoi(args[4]);
   if (NumLoops <= 0) {
     kprintf("catmouse: invalid number of loops: %d\n",NumLoops);
     return 1;
   }
 
   if (nargs == 9) {
     CatEatTime = atoi(args[5]);
     if (CatEatTime < 0) {
       kprintf("catmouse: invalid cat eating time: %d\n",CatEatTime);
       return 1;
     }
   
     CatSleepTime = atoi(args[6]);
     if (CatSleepTime < 0) {
       kprintf("catmouse: invalid cat sleeping time: %d\n",CatSleepTime);
       return 1;
     }
   
     MouseEatTime = atoi(args[7]);
     if (MouseEatTime < 0) {
       kprintf("catmouse: invalid mouse eating time: %d\n",MouseEatTime);
       return 1;
     }

     MouseSleepTime = atoi(args[8]);
     if (MouseSleepTime < 0) {
       kprintf("catmouse: invalid mouse sleeping time: %d\n",MouseSleepTime);
      return 1;

	  }
   }

   kprintf("Using %d bowls, %d cats, and %d mice. Looping %d times.\n",
           NumBowls,NumCats,NumMice,NumLoops);
   kprintf("Using cat eating time %d, cat sleeping time %d\n", CatEatTime, CatSleepTime);
   kprintf("Using mouse eating time %d, mouse sleeping time %d\n", MouseEatTime, MouseSleepTime);

   CatMouseWait = sem_create("CatMouseWait",0);
   if (CatMouseWait == NULL) {
     panic("catmouse: could not create semaphore\n");
   }

   initialize_bowls();
 
   catmouse_sync_init(NumBowls);
   gettime(&before_sec,&before_nsec);


   for (catindex = 0; catindex < NumCats; catindex++) {
     error = thread_fork("cat_simulation thread", NULL, cat_simulation, NULL, catindex);
     if (error) {
       panic("cat_simulation: thread_fork failed: %s\n", strerror(error));
     }
     if (catindex < NumMice) {
       error = thread_fork("mouse_simulation thread", NULL, mouse_simulation, NULL, catindex);
       if (error) {
         panic("mouse_simulation: thread_fork failed: %s\n",strerror(error));
       }
     } 
   }

   for(mouseindex = catindex; mouseindex < NumMice; mouseindex++) {
     error = thread_fork("mouse_simulation thread", NULL, mouse_simulation, NULL, mouseindex);
     if (error) {
       panic("mouse_simulation: thread_fork failed: %s\n",strerror(error));
     }
   }

   for(i=0;i<(NumCats+NumMice);i++) {
     P(CatMouseWait);
   }

   gettime(&after_sec,&after_nsec);

   getinterval(before_sec,before_nsec,after_sec,after_nsec,&wait_sec,&wait_nsec);
   total_bowl_milliseconds = (wait_sec*1000 + wait_nsec/1000000)*NumBowls;
   total_eating_milliseconds = (NumCats*CatEatTime + NumMice*MouseEatTime)*NumLoops*1000;
   if (total_bowl_milliseconds > 0) {
     utilization_percent = total_eating_milliseconds*100/total_bowl_milliseconds;
     kprintf("Bowl utilization: %d%%\n",utilization_percent);
   }
   sem_destroy(CatMouseWait);
   catmouse_sync_cleanup(NumBowls);
   cleanup_bowls();
   if (cat_wait_count > 0) {
     mean_cat_wait_usecs = (cat_total_wait_secs*1000000+cat_total_wait_nsecs/1000)/cat_wait_count;
     kprintf("Mean cat waiting time: %d.%d seconds\n",mean_cat_wait_usecs/1000000,mean_cat_wait_usecs%1000000);
   }
   if (mouse_wait_count > 0) {
     mean_mouse_wait_usecs = (mouse_total_wait_secs*1000000+mouse_total_wait_nsecs/1000)/mouse_wait_count;
     kprintf("Mean mouse waiting time: %d.%d seconds\n",mean_mouse_wait_usecs/1000000,mean_mouse_wait_usecs%1000000);
   }
 
   return 0;
 }
