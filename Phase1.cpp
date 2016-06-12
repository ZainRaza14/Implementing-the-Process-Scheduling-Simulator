#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include<fcntl.h>
#include <queue>
#include<cstring>
#include<sstream>
#include<time.h>
#include <pthread.h>
#include <stack>
using namespace std;
enum State{New,Ready,Running,Blocked,Terminated};
string RemoveCarriageReturns(string str)
{
    string tmp="";
    for(int i=0;i<str.length();i++)
    {
        if(str[i]!='\n' && (isalnum(str[i]) || (str[i]==' ') || (str[i]=='_')))
            tmp+=str[i];
        
    }
    return tmp;
}
string GetNumericValue(string str)
{
    string tmp="";
    for(int i=0;i<str.length();i++)
    {
        if(isdigit(str[i]))
            tmp+=str[i];
        
    }
    return tmp;
}

string RemoveSpaces(string str)
{
    string tmp="";
    for(int i=0;i<str.length();i++)
    {
        if(str[i]!=' ')
            tmp+=str[i];
        
    }
    return tmp;
}
string intToString(int i)
{
    std::stringstream ss;
    std::string s;
    ss << i;
    s = ss.str();

    return s;
}
int RunCommand(const char* strCommand)
{
    
	int iForkId, iStatus;
	iForkId = fork();
	if (iForkId == 0)	// This is the child 
	{
		
		iStatus = execl("/bin/sh","sh","-c", strCommand, (char*) NULL);
		exit(iStatus);	
				 
	}
	else if (iForkId > 0)	// Parent, no error
	{
	waitpid(iForkId,&iStatus,0);	
            iStatus = 0;
	}
	else	// Parent, with error (iForkId == -1)
	{
		iStatus = -1;
	}
	return(iStatus);
} 
string ExtractData(string Xpath,string filename,bool blanks)
{
    int tmp_stdout=dup(1);
    int fd[2];
    pipe(fd);
    //int file=open("./tmp",O_APPEND|O_WRONLY);

    dup2(fd[1],1);
 
    
     string tmp;
if(blanks==true)    
    tmp="xmlstarlet sel -t -v ";
else
     tmp="xmlstarlet sel -b -t -v ";
    
     tmp+="'" + Xpath + "' " + filename;
    const char* fullcmd=tmp.c_str();
    RunCommand(fullcmd);
    char * returnval=new char[200];
    read(fd[0],returnval,200);
   
    dup2(tmp_stdout,1);
    return returnval;
    
}
string ExtractAttributes(string Xpath,string attribute,string filename,bool blanks)
{
    int tmp_stdout=dup(1);
     int fd[2];
    pipe(fd);
    //int file=open("./tmp.txt",O_WRONLY);
    dup2(fd[1],1);
     string tmp;
if(blanks==true)    
{   tmp="xmlstarlet sel -t -m ";}
else
{
     tmp="xmlstarlet sel -b -t -m ";
}   
     tmp+="'" + Xpath + "' ";
     tmp+="-n -v '" + attribute + "' '" + filename + "'";
    const char* fullcmd=tmp.c_str();
    RunCommand(fullcmd);
   char * returnval=new char[50];
    read(fd[0],returnval,20);
   
    dup2(tmp_stdout,1);
    return returnval;
    
}
void * FillReadyQueue(void *ptr);
void * FillPriorityQueues(void *ptr);
void * MaintainQueues(void * ptr);
void * StartExecution(void * ptr);
struct Process
{
    string ID;
    string ArrivalTime;
    string Priority;
    string Type;
    string code;
    State state;
    time_t BurstTime;
    time_t WaitTime;
    string ExecutedCode;
    time_t TimeInBlockedFront;
    Process()
    {
        state=New;
        BurstTime=0;
        WaitTime=0;
        ExecutedCode="";
        TimeInBlockedFront=0;
    }
};
bool ExistInQueue(queue<Process> q,int PID);
class PRI
{
     friend class OS;
private:
    bool Preemptive;
    
     
public:
    int GetLastInstructionNumber(Process p)
     {
         int count=0;
         stringstream ss;
         ss<<p.ExecutedCode;
         string tmp;
         while(getline(ss,tmp,'\n'))
         {
             count++;
         }
         return count;
     }
     Process Current_Process;
    queue<Process> PriQueue[3];
    bool NextProcess()
    {
        int qsize=PriQueue[0].size();
        int qsize1=PriQueue[1].size();
        int qsize2=PriQueue[2].size();
        if(qsize!=0)
        {
            Current_Process=PriQueue[0].front();
            PriQueue[0].pop();  
            return true;
        }
        else if(qsize1!=0)
        {
            Current_Process=PriQueue[1].front();
            
            PriQueue[1].pop();
            return true;
        }
        else if(qsize2!=0)
        {
            Current_Process=PriQueue[2].front();
            
            PriQueue[2].pop();
            return true;
        }
        else
        {
          return false;     
        }
    }
    PRI()
    {
    
     
    }
    bool ProcessExistsInQueues(queue<Process> &_Keyboard,queue<Process> &_Screen,queue<Process> *&_Printer,queue<Process> *&_Disk,int Num_Printers,int Num_Disks)
    {
        if(_Keyboard.size()>0)
            return true;
        if(_Screen.size()>0)
            return true;
        for(int i=0;i<Num_Printers;i++)
        {
            if(_Printer[i].size()>0)
                return true;
        }
        for(int i=0;i<Num_Disks;i++)
        {
            if(_Disk[i].size()>0)
                return true;
        }
        return false;
        
    }
   
   
    void PrintPriorityQueues()
    {
        for(int i=0;i<3;i++)
        {
          
            queue<Process> tmp1=PriQueue[i];
            int qsize=tmp1.size();
            
            if(i==0)
                cout<<"High Priority Process: "<<endl;
            else if(i==1)
                cout<<"Medium Priority Process: "<<endl;
            else if(i==2)
                cout<<"Low Priority Process: "<<endl;
            for(int j=0;j<qsize;j++)
            {
                Process tmp=tmp1.front();
                tmp1.pop();
               cout<<"Process ID: "<<tmp.ID<<endl;
               cout<<"Process Arivaltime: "<<tmp.ArrivalTime<<endl;
               cout<<"Process Priority: "<<tmp.Priority<<endl;
               cout<<"Process Type: "<<tmp.Type<<endl;
               cout<<endl;
               
            }
            
        }
    }
    
};

class OS
{
private:
    string ConfigFile;
    Process * PCB;
    queue<Process> * printers;
    queue<Process> * disks;
    queue<Process> Keyboard;
    queue<Process> Screen;
    queue<Process> ready;
    queue<Process> Terminated;
    int NumProcesses;
    int * Pids;
    int NumPrinters;
    int NumDisks;
    string SchedulingPolicy;
    bool Preemptive;   
    time_t os_StartTime;
     pthread_t FillQueue;     
 pthread_t FillPriQueue;
  pthread_t MaintainQueuesThread;
 pthread_t ExecutionThread;  
     int FillThreadID;
     int FillPriID;
     int MaintainQueuesID;
     PRI Pri_Sched;
    void GetPids(int count)
        {
        
        string attr=ExtractAttributes("/ProjectInput/Processes/Process","@ID",ConfigFile,false);
            Pids= new int[count];
            int c=0;
            for(int i=0;i<attr.length();i++)
            {
                int tmp;
                string tmp2="";
                bool copy=0;
                if(isdigit(attr[i]))
                {
                    copy=1;
                    while(isdigit(attr[i]))
                    {
                        tmp2+=attr[i];
                        i++;
                    }
                    tmp=atoi(tmp2.c_str());
                    Pids[c]=tmp;
                    c++;
                }
            }
        }
    void GetConfiguration()
    {
        SchedulingPolicy= RemoveCarriageReturns(ExtractData("/ProjectInput/Configuration/SchedulingPolicy",ConfigFile,false));
        string preem=RemoveCarriageReturns(ExtractData("/ProjectInput/Configuration/Preemptive",ConfigFile,false));
        if(preem.compare("Yes")==0 || preem.compare("yes")==0 || preem.compare("YES")==0 )
             Preemptive=true;
        else
        Preemptive=false;
        
        NumDisks=atoi(RemoveCarriageReturns(ExtractData("/ProjectInput/Configuration/NumbeofDisks",ConfigFile,false)).c_str());
        NumPrinters=atoi(RemoveCarriageReturns(ExtractData("/ProjectInput/Configuration/NumberofPrinters",ConfigFile,false)).c_str());
        printers=new queue<Process>[NumPrinters];        
        disks=new queue<Process>[NumDisks];
        
    }
    void GetProcesses()
    {
        NumProcesses=atoi(ExtractData("/ProjectInput/Processes/@num",ConfigFile,false).c_str());
           PCB=new Process[NumProcesses];
                   GetPids(NumProcesses);
           for(int i=0;i<NumProcesses;i++)
           {              
            PCB[i].ID=intToString(Pids[i]);
            PCB[i].ArrivalTime=GetNumericValue(RemoveCarriageReturns(ExtractData("/ProjectInput/Processes/Process[@ID=" + PCB[i].ID + "]/ArrivalTime",ConfigFile,false)));
             PCB[i].Priority=RemoveSpaces(RemoveCarriageReturns(ExtractData("/ProjectInput/Processes/Process[@ID=" + PCB[i].ID + "]/Priority",ConfigFile,false)));
             PCB[i].Type=RemoveCarriageReturns(ExtractData("/ProjectInput/Processes/Process[@ID=" + PCB[i].ID + "]/Type",ConfigFile,false));
             PCB[i].code=ExtractData("/ProjectInput/Processes/Process[@ID=" + PCB[i].ID + "]/code",ConfigFile,false);
                                
           }
        
    }
    
    public:
     friend void * FillReadyQueue(void *ptr);
    friend void * FillPriorityQueues(void *ptr);
    friend void * MaintainQueues(void * ptr);
    friend void * StartExecution(void * ptr);
        OS(string File)
        {
            ConfigFile=File;
            GetConfiguration();
            GetProcesses();
            Start_OS();
            if(SchedulingPolicy.compare("PS")==0 || SchedulingPolicy.compare("ps")==0 || SchedulingPolicy.compare("Ps")==0 )
            {
                 
                Pri_Sched.Preemptive=Preemptive; 
                     void * tmp=(void*)this; 
                        FillPriID=pthread_create(&FillPriQueue,NULL,FillPriorityQueues,tmp);
                        pthread_create(&ExecutionThread,NULL,StartExecution,tmp);
                        
            }
             void * tmp=(void*)this; 
                        MaintainQueuesID=pthread_create(&MaintainQueuesThread,NULL,MaintainQueues,tmp);
            
             
           
            
            
           
              //Pri_Sched.PrintPriorityQueues();
                  
        }
        
        
        void PrintAllProcess()
        {
            for(int i=0;i<NumProcesses;i++)
            {
               cout<<"Process ID: "<<PCB[i].ID<<endl;
               cout<<"Process Arivaltime: "<<PCB[i].ArrivalTime<<endl;
               cout<<"Process Priority: "<<PCB[i].Priority<<endl;
               cout<<"Process Type: "<<PCB[i].Type<<endl;
               //cout<<"Code: "<<PCB[i].code<<endl;
               cout<<endl;
                               
            }
            
        }
        void Start_OS()
        {
          
          time(&os_StartTime);
         void * tmp=(void*)this; 
        FillThreadID=pthread_create(&FillQueue,NULL,FillReadyQueue,tmp);  
     
        
                       
        }
        void PrintReadyQueue()
        {
            stack<Process> st1;
            stack<Process> st2;
            int qsize=ready.size();
            for(int i=0;i<qsize;i++)
            {
                Process tmp=ready.front();
                ready.pop();
               cout<<"Process ID: "<<tmp.ID<<endl;
               cout<<"Process Arivaltime: "<<tmp.ArrivalTime<<endl;
               cout<<"Process Priority: "<<tmp.Priority<<endl;
               cout<<"Process Type: "<<tmp.Type<<endl;
               cout<<endl;
               st1.push(tmp);
            }
            for(int i=0;i<qsize;i++)
            {
                st2.push(st1.top());
                st1.pop();
            }
             for(int i=0;i<qsize;i++)
            {
                ready.push(st2.top());
                st2.pop();
            }
            
        }
        void PrintPids()
       
        {
            for(int i=0;i<NumProcesses;i++)
            {
                cout<<Pids[i]<<endl;
                
            }
            
        }
        void PrintAllQueues()
        {
            int qsize;
            if(SchedulingPolicy.compare("PS")!=0 || SchedulingPolicy.compare("PS")!=0 )
            {
                queue<Process> tmp_Ready=ready;
                cout<<"Ready Queue: "<<endl;
                qsize=tmp_Ready.size();
                for(int i=0;i<qsize;i++)
                {
                   cout<<"\tProcessID: "<<tmp_Ready.front().ID<<endl;
                   cout<<"\tArrivalTime: "<<tmp_Ready.front().ArrivalTime<<endl;
                   cout<<"\tProcess Priority: "<<tmp_Ready.front().Priority<<endl;
                   cout<<"\tProcess Type: "<<tmp_Ready.front().Type<<endl;
                   cout<<"\tProcess Burst Time: "<<tmp_Ready.front().BurstTime<<endl;
                   cout<<"\tProcess State: "<<tmp_Ready.front().state<<endl;
                   if(tmp_Ready.front().state==Blocked)
                       cout<<"\t\tProcess State: "<<tmp_Ready.front().TimeInBlockedFront<<endl; 
                   cout<<"\tProcess Wait Time: "<<tmp_Ready.front().WaitTime<<endl;
                   cout<<"\tProcess Executed Code:\n\t\t "<<tmp_Ready.front().ExecutedCode<<endl;
                   tmp_Ready.pop();
                }
            }
            
            queue<Process> tmp_Keyboard=Keyboard;
            cout<<"KeyBoard Queue: "<<endl;
            qsize=tmp_Keyboard.size();
            for(int i=0;i<qsize;i++)
            {
               cout<<"\tProcessID: "<<tmp_Keyboard.front().ID<<endl;
               cout<<"\tArrivalTime: "<<tmp_Keyboard.front().ArrivalTime<<endl;
               cout<<"\tProcess Priority: "<<tmp_Keyboard.front().Priority<<endl;
               cout<<"\tProcess Type: "<<tmp_Keyboard.front().Type<<endl;
               cout<<"\tProcess Burst Time: "<<tmp_Keyboard.front().BurstTime<<endl;
               cout<<"\tProcess State: "<<tmp_Keyboard.front().state<<endl;
               if(tmp_Keyboard.front().state==Blocked)
                   cout<<"\t\tProcess State: "<<tmp_Keyboard.front().TimeInBlockedFront<<endl; 
               cout<<"\tProcess Wait Time: "<<tmp_Keyboard.front().WaitTime<<endl;
               cout<<"\tProcess Executed Code:\n\t\t "<<tmp_Keyboard.front().ExecutedCode<<endl;
               tmp_Keyboard.pop();
               
            }
            
            
            queue<Process> tmp_Screen=Screen;
            cout<<"Screen Queue: "<<endl;
            qsize=tmp_Screen.size();
            for(int i=0;i<qsize;i++)
            {
               cout<<"\tProcessID: "<<tmp_Screen.front().ID<<endl;
               cout<<"\tArrivalTime: "<<tmp_Screen.front().ArrivalTime<<endl;
               cout<<"\tProcess Priority: "<<tmp_Screen.front().Priority<<endl;
               cout<<"\tProcess Type: "<<tmp_Screen.front().Type<<endl;
               cout<<"\tProcess Burst Time: "<<tmp_Screen.front().BurstTime<<endl;
               cout<<"\tProcess State: "<<tmp_Screen.front().state<<endl;
               if(tmp_Screen.front().state==Blocked)
                   cout<<"\t\tProcess State: "<<tmp_Screen.front().TimeInBlockedFront<<endl; 
               cout<<"\tProcess Wait Time: "<<tmp_Screen.front().WaitTime<<endl;
               cout<<"\tProcess Executed Code:\n\t\t "<<tmp_Screen.front().ExecutedCode<<endl;
               tmp_Screen.pop();
            }
            
            
            queue<Process> tmp_Terminated=Terminated;
            cout<<"Terminated Queue: "<<endl;
            qsize=tmp_Terminated.size();
            for(int i=0;i<qsize;i++)
            {
               cout<<"\tProcessID: "<<tmp_Terminated.front().ID<<endl;
               cout<<"\tArrivalTime: "<<tmp_Terminated.front().ArrivalTime<<endl;
               cout<<"\tProcess Priority: "<<tmp_Terminated.front().Priority<<endl;
               cout<<"\tProcess Type: "<<tmp_Terminated.front().Type<<endl;
               cout<<"\tProcess Burst Time: "<<tmp_Terminated.front().BurstTime<<endl;
               cout<<"\tProcess State: "<<tmp_Terminated.front().state<<endl;
               if(tmp_Terminated.front().state==Blocked)
                   cout<<"\t\tProcess State: "<<tmp_Terminated.front().TimeInBlockedFront<<endl; 
               cout<<"\tProcess Wait Time: "<<tmp_Terminated.front().WaitTime<<endl;
               cout<<"\tProcess Executed Code:\n\t\t "<<tmp_Terminated.front().ExecutedCode<<endl;
               tmp_Terminated.pop();
            }
           
            cout<<"Printer Queue: "<<endl;
            for(int i=0;i<NumPrinters;i++)
            {    queue<Process> tmp_Printer=printers[i];
                cout<<"\tPrinter "<<i<<": "<<endl;
                qsize=tmp_Printer.size();
                for(int i=0;i<qsize;i++)
                {
                   cout<<"\t\tProcessID: "<<tmp_Printer.front().ID<<endl;
                   cout<<"\t\tArrivalTime: "<<tmp_Printer.front().ArrivalTime<<endl;
                   cout<<"\t\tProcess Priority: "<<tmp_Printer.front().Priority<<endl;
                   cout<<"\t\tProcess Type: "<<tmp_Printer.front().Type<<endl;
                   cout<<"\t\tProcess Burst Time: "<<tmp_Printer.front().BurstTime<<endl;
                   cout<<"\t\tProcess State: "<<tmp_Printer.front().state<<endl;
                   if(tmp_Printer.front().state==Blocked)
                       cout<<"\t\t\tProcess State: "<<tmp_Printer.front().TimeInBlockedFront<<endl; 
                   cout<<"\t\tProcess Wait Time: "<<tmp_Printer.front().WaitTime<<endl;
                   cout<<"\t\tProcess Executed Code:\n\t\t "<<tmp_Printer.front().ExecutedCode<<endl;
                   tmp_Printer.pop();
                }
            }
            
            
            cout<<"Disk Queue: "<<endl;
            for(int i=0;i<NumDisks;i++)
            {    queue<Process> tmp_Disk=disks[i];
                cout<<"\tDisk "<<i<<": "<<endl;
                qsize= tmp_Disk.size();
                for(int i=0;i<qsize;i++)
                {
                   cout<<"\t\tProcessID: "<< tmp_Disk.front().ID<<endl;
                   cout<<"\t\tArrivalTime: "<< tmp_Disk.front().ArrivalTime<<endl;
                   cout<<"\t\tProcess Priority: "<< tmp_Disk.front().Priority<<endl;
                   cout<<"\t\tProcess Type: "<< tmp_Disk.front().Type<<endl;
                   cout<<"\t\tProcess Burst Time: "<< tmp_Disk.front().BurstTime<<endl;
                   cout<<"\t\tProcess State: "<< tmp_Disk.front().state<<endl;
                   if( tmp_Disk.front().state==Blocked)
                       cout<<"\t\t\tProcess State: "<< tmp_Disk.front().TimeInBlockedFront<<endl; 
                   cout<<"\t\tProcess Wait Time: "<< tmp_Disk.front().WaitTime<<endl;
                   cout<<"\t\tProcess Executed Code:\n\t\t "<< tmp_Disk.front().ExecutedCode<<endl;
                   tmp_Disk.pop();
                }
            }
            if(SchedulingPolicy.compare("PS")==0 || SchedulingPolicy.compare("ps")==0 )
            {
                cout<<"Priority Queues: "<<endl;
                Pri_Sched.PrintPriorityQueues();
            }
            
        }
        
        
        ~OS()
        {
        delete [] PCB;    
        
        }
        
};
void * MaintainQueues(void * ptr)
{
         OS *Object=(OS *)ptr;
         while(1)
         {
             if(Object->Keyboard.size()!=0)
             {
                 Object->Keyboard.front().WaitTime++;
                  Object->Keyboard.front().TimeInBlockedFront++;
                  if(Object->Keyboard.front().TimeInBlockedFront==3)
                  {
                      Object->Keyboard.front().TimeInBlockedFront=0;
                      Object->ready.push(Object->Keyboard.front());
                      Object->Keyboard.pop();                      
                  }
                                  
             }
             
             if(Object->Screen.size()!=0)
             {
                 Object->Screen.front().WaitTime++;
                  Object->Screen.front().TimeInBlockedFront++;
                  if(Object->Screen.front().TimeInBlockedFront==3)
                  {
                      Object->Screen.front().TimeInBlockedFront=0;
                      Object->ready.push(Object->Screen.front());
                      Object->Screen.pop();                      
                  }
                                  
             }
             
             for(int i=0;i<Object->NumPrinters;i++)
              {
             if(Object->printers[i].size()!=0)
             {
                 Object->printers[i].front().WaitTime++;
                  Object->printers[i].front().TimeInBlockedFront++;
                  if(Object->printers[i].front().TimeInBlockedFront==3)
                  {
                      Object->printers[i].front().TimeInBlockedFront=0;
                      Object->ready.push(Object->printers[i].front());
                      Object->printers[i].pop();                      
                  }
                                  
             }
             }
             
             for(int i=0;i<Object->NumDisks;i++)
              {
             if(Object->disks[i].size()!=0)
             {
                 Object->disks[i].front().WaitTime++;
                  Object->disks[i].front().TimeInBlockedFront++;
                  if(Object->disks[i].front().TimeInBlockedFront==3)
                  {
                      Object->disks[i].front().TimeInBlockedFront=0;
                      Object->ready.push(Object->disks[i].front());
                      Object->disks[i].pop();                      
                  }
                                  
             }
             }
             sleep(1);
}
         }

void * FillReadyQueue(void *ptr)
        {
               OS *Object=(OS *)ptr;
               int diff=0;
            while(1)
            {
                diff++;
                if((Object->ready.size())!=(Object->NumProcesses))
                {
                for(int i=0;i<(Object->NumProcesses);i++)
                {
                    if(atoi((Object->PCB[i].ArrivalTime.c_str()))==diff)
                    {
                        Object->ready.push(Object->PCB[i]);
                        
                    }
                    
                }
                }
                sleep(1);
                //time_t now=time(&now);
            
            }
        }


void * FillPriorityQueues(void *ptr)
        {
               OS *Object=(OS *)ptr;
            while(1)
            {
               
                int qsize=Object->ready.size();
                
                for(int i=0;i<qsize;i++)
                {
                    Process tmp;
                    if(Object->ready.front().Priority.compare("High")==0 || Object->ready.front().Priority.compare("HIGH")==0)
                    {
                        tmp=Object->ready.front();
                        Object->ready.pop();
                        if(!ExistInQueue(Object->Pri_Sched.PriQueue[0],atoi(tmp.ID.c_str())));
                        Object->Pri_Sched.PriQueue[0].push(tmp);
                        
                    }
                    else  if(Object->ready.front().Priority.compare("Medium")==0 || Object->ready.front().Priority.compare("MEDIUM")==0)
                    {
                         tmp=Object->ready.front();
                       Object->ready.pop();
                       if(!ExistInQueue(Object->Pri_Sched.PriQueue[1],atoi(tmp.ID.c_str())));
                        Object->Pri_Sched.PriQueue[1].push(tmp);
                        
                    }
                    else if(Object->ready.front().Priority.compare("Low")==0 || Object->ready.front().Priority.compare("LOW")==0)
                    {
                         tmp=Object->ready.front();
                       Object->ready.pop();
                        if(!ExistInQueue(Object->Pri_Sched.PriQueue[2],atoi(tmp.ID.c_str())));
                        Object->Pri_Sched.PriQueue[2].push(tmp);
                        
                    }
                        
                        
                    
                }
                
                sleep(1);
            
            }          
}
void * StartExecution(void * ptr)
{
         OS *Object=(OS *)ptr;
         while(1)
         {
         while(Object->Pri_Sched.NextProcess())
        {
            string Instruction;
            stringstream stream;
            stream<<Object->Pri_Sched.Current_Process.code;
            if(Object->Pri_Sched.Current_Process.state==Blocked)
            {
                for(int i=0;i<Object->Pri_Sched.GetLastInstructionNumber(Object->Pri_Sched.Current_Process)+1;i++)
                {
                    getline(stream,Instruction,'\n');

                }
            Object->Pri_Sched.Current_Process.state=Running;
            }
            bool Terminate=1; 
            while(getline(stream,Instruction,'\n'))
            {
                Terminate=1;
                if(Instruction.compare("COMPUTE")==0 || Instruction.compare("WRITE MEMORY")==0 || Instruction.compare("READ MEMORY")==0)
                {
                    Object->Pri_Sched.Current_Process.BurstTime++;
                    Object->Pri_Sched.Current_Process.ExecutedCode+=Instruction;
                Object->Pri_Sched.Current_Process.ExecutedCode+='\n';
               
                    
                }
                else if(Instruction.compare("WRITE SCREEN")==0)
                {
                    Object->Pri_Sched.Current_Process.state=Blocked;
                    Object->Pri_Sched.Current_Process.ExecutedCode+=Instruction;
                    Object->Pri_Sched.Current_Process.ExecutedCode+='\n';
                    Object->Screen.push(Object->Pri_Sched.Current_Process);
                    Terminate=0;
                    break;
                    
                }
                else if(Instruction.compare("READ KEYBOARD")==0)
                {
                    Object->Pri_Sched.Current_Process.state=Blocked;
                    Object->Pri_Sched.Current_Process.ExecutedCode+=Instruction;
                    Object->Pri_Sched.Current_Process.ExecutedCode+='\n';
                    Object->Keyboard.push(Object->Pri_Sched.Current_Process);
                     Terminate=0;
                    break;
                    
                }
                else if(Instruction.compare("WRITE PRINTER")==0)
                {
                    Object->Pri_Sched.Current_Process.state=Blocked;
                    Object->Pri_Sched.Current_Process.ExecutedCode+=Instruction;
                    Object->Pri_Sched.Current_Process.ExecutedCode+='\n';
                    int LeastLoad=0;
                    for(int i=0;i<Object->NumPrinters;i++)
                    {
                        if(Object->printers[i].size()<Object->printers[LeastLoad].size())
                        {
                            LeastLoad=i;
                        }
                    
                    
                    }
                    Object->printers[LeastLoad].push(Object->Pri_Sched.Current_Process);
                    Terminate=0;
                    break;
                     
                }
                 else if(Instruction.compare("WRITE DISK")==0 || Instruction.compare("READ DISK")==0 )
                {
                    Object->Pri_Sched.Current_Process.state=Blocked;
                    Object->Pri_Sched.Current_Process.ExecutedCode+=Instruction;
                    Object->Pri_Sched.Current_Process.ExecutedCode+='\n';
                    int LeastLoad=0;
                    for(int i=0;i<Object->NumDisks;i++)
                    {
                        if(Object->disks[i].size()<Object->disks[LeastLoad].size())
                        {
                            LeastLoad=i;
                        }
                   
                    }
                    Object->disks[LeastLoad].push(Object->Pri_Sched.Current_Process);
                    Terminate=0;
                    break;
                     
                }            
                                       
      
            }
            if(Terminate)
            {
            Object->Pri_Sched.Current_Process.state=Terminated;
            Object->Terminated.push(Object->Pri_Sched.Current_Process);
            }
         
              
           
        }
      
         sleep(1);
         }
         
}
bool ExistInQueue(queue<Process> q,int PID)
{
    bool exist=0;

            int qsize=q.size();
            for(int i=0;i<qsize;i++)
            {
                Process tmp=q.front();
                q.pop();
                if(tmp.ID.compare(intToString(PID))==0)
                    return true;
               
               
            }
           
            return false;
}

               
               
            
        

int main(int argc, char** argv) {
   
 OS os("sched.config");
 sleep(14);
 os.PrintAllQueues();
  return 0;
}
//"xmlstarlet sel -t -v '/ProjectInput/Configuration/Preemptive' sched.config"
//xmlstarlet sel -t -v '/ProjectInput/Processes/@num' sched.config

/*
 FCFS
 * 
 * 
 * #include <iostream>
#include <queue>

using namespace std;

template <class t>
class FCFS
{
    private:

    queue <t> my_queue;

    public:


void new_process(t process)
  {
      my_queue.push(process);

  }
  void remove_process()
  {
      my_queue.pop();
  }


};

int main()
{
    FCFS <int> Q;    // the queue containing the processes

    Q.new_process(1);
    Q.remove_process();

}

 
 
 */

/*
 RR
 * 
 * void RR15(Process P[])
{
        int q = 15; //q is the time quantum
        int sp = 0; //time spent


        int i, k, time, j = 0;


        Process temp[50];

        bool f = false; //flag to indicate whether any process
                        //was scheduled as i changed from 0 n-1
                        //in the next for loop

        //while there are uncompleted processes

        for(i=0; j < 50; i=(i+1)%50)
        {
                //find the next uncompleted processes which has already
                //or just arived


                if(P[i].CPU_Burst>0 && sp>=P[i].Arrival_Time)
                {
                        f=true;
                        //if process requests for time less than quantum
                        if(P[i].CPU_Time <= q)
                                time = P[i].CPU_Burst;
                        else
                                time = q;

                        //schedule the process

                        P[i].Turn_Time += time, P[i].CPU_Burst....?

}
 
 
 
 */


