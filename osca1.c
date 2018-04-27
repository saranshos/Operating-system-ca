#include<stdio.h>
#include<conio.h>
void main()
{
char c='P';
int p[3]={1,2,3};
float at[3]={0.0,0.4,1.0};
float bst[3]={8.0,4.0,1.0};
float tat[3],mtime[3],bstc[3];
float t=0.0,avg=0.0,min;
int i,j,ind,inde;
clrscr();
printf("Process\t\tArival Time\t\tBurst Time\n");
for(i=0;i<3;i++)
{
printf("%c%d\t\t%f\t\t%f\n",c,p[i],at[i],bst[i]);
}
printf("\n\nProcess\t\tTurn Around Time FCFS\n");
for(i=0;i<3;i++)
{
t=t+bst[i];
tat[i]=t-at[i];
printf("%c%d\t\t%f\n",c,p[i],tat[i]);
}
for(i=0;i<3;i++)
{
avg=tat[i]+avg;
}
printf("The average turnaround time of FCFS algorithm will be:- %d\n\n",avg);



t=0.0;
for(i=0;i<3;i++)
{
mtime[i]=at[i];
}
for(i=0;i<3;i++)
{
bstc[i]=bst[i];
}
printf("Process\t\tTurn Around Time SJF\n");
for(i=0;i<3;i++)
{


min=mtime[0];
for(j=0;j<3;j++)
{
if(min>mtime[j])
{
min=mtime[j];
ind=j;
}
}

min=bst[0];
for(j=0;j<ind;j++)
{
if(min>bstc[j])
{
min=bstc[j];
inde=j;
}
}

t=t+bst[inde];
tat[i]=t-at[i];
mtime[ind]=1000.0;
bstc[inde]=1000.0;
printf("%c%d\t\t%f\n",c,p[i],tat[i]);
}
for(i=0;i<3;i++)
{
avg=tat[i]+avg;
}
printf("The average turnaround time of SJF algorithm will be:- %d\n\n",avg);


t=0.0;
for(i=0;i<3;i++)
{
mtime[i]=at[i]+(1-at[i]);
}
for(i=0;i<3;i++)
{
bstc[i]=bst[i];
}
printf("Process\t\tTurn Around Time SJF\n");
for(i=0;i<3;i++)
{


min=mtime[0];
for(j=0;j<3;j++)
{
if(min>mtime[j])
{
min=mtime[j];
ind=j;
}
}

min=bst[0];
for(j=0;j<ind;j++)
{
if(min>bstc[j])
{
min=bstc[j];
inde=j;
}
}

t=t+bst[inde];
tat[i]=t-at[i];
mtime[ind]=1000.0;
bstc[inde]=1000.0;
printf("%c%d\t\t%f\n",c,p[i],tat[i]);
}
for(i=0;i<3;i++)
{
avg=tat[i]+avg;
}
printf("The average turnaround time of SJF algorithm if cpu is idle for 1 unit will be:- %d\n\n",avg);

getch();
}
