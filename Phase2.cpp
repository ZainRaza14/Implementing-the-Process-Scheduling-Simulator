#include <iostream>
#include<dirent.h>
#include<string.h>
#include<pthread.h>
#include<cstdlib>
#include<queue>
#include<sstream>
#include<fcntl.h>
#include<semaphore.h>
using namespace std;
queue<string> directories;
void GetDirectories(char * path);
void GetFiles(queue<string> &fl,char * path);
void PrintQueue(queue<string> q);
void SearchFile(char * path,string str);
int NoThreads;
string SearchStr;
string dir;
sem_t sem;
pthread_mutex_t mutex;
void * tgrep(void *ptr);
pthread_t * t;
int main(int argc,char*argv[])
{
string arg1,arg2,arg3;
arg1=argv[2];arg2=argv[3];arg3=argv[4];
NoThreads=atoi(arg1.c_str());
SearchStr=arg2;
dir=arg3;
t=new pthread_t[NoThreads];
sem_init(&sem,0,NoThreads);
pthread_mutex_init(&mutex,NULL);
directories.push(dir);

for(int i=0;i<NoThreads;i++)
{
    pthread_create(&t[i],NULL,tgrep,NULL);
}
pthread_exit(NULL);
    return 0;
}

void * tgrep(void *ptr)
{
pthread_detach(pthread_self());
while(directories.size()!=0)
{
sem_wait(&sem);
pthread_mutex_lock(&mutex);

string d=directories.front();
directories.pop();
GetDirectories((char*)d.c_str());

pthread_mutex_unlock(&mutex);
queue<string> tmp;
GetFiles(tmp,(char*)d.c_str());

//cout<<"files Queue: "<<endl;
//PrintQueue(tmp);
while(tmp.size()!=0)
{
    string f=tmp.front();
    tmp.pop();
    SearchFile((char*)f.c_str(),SearchStr);
}

sem_post(&sem);

}
}
void GetDirectories(char * path)
{

    DIR *d=opendir(path);

    dirent *file;
    while((file=readdir(d))!=NULL)
          {
              if(file->d_name[0]!='.')
              {

              if(file->d_type==DT_DIR)
              {
                    string tmp=file->d_name;
                    string tmp2=path;
                    tmp2+=tmp;
                    tmp2+='/';
                    directories.push(tmp2);
              }

              }

          }
closedir(d);


}
void GetFiles(queue<string> &fl,char * path)
{

    DIR *d=opendir(path);

    dirent *file;
    while((file=readdir(d))!=NULL)
          {
              if(file->d_name[0]!='.')
              {

              if(file->d_type==DT_REG)
              {
                    string tmp=file->d_name;
                    string tmp2=path;
                    tmp2+=tmp;
                    fl.push(tmp2);
              }

              }

          }
closedir(d);

}
void PrintQueue(queue<string> q)
{
queue<string> tmp=q;
for(int i=0;i<q.size();i++)
{
        cout<<tmp.front()<<endl;
        tmp.pop();
}
}
void SearchFile(char * path,string str)
{
    //cout<<"searching: "<<path<<endl;
int file=open(path,O_RDONLY);
char * buff=new char[1000];
read(file,buff,100);
string buffer=buff;

int found=buffer.find(str);
if(found!=string::npos)
cout<<"'"<<str<<"' found in "<<path<<endl;
delete buff;
close(file);

}
