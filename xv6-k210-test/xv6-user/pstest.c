#include "kernel/include/types.h"
#include "kernel/include/param.h"
#include "kernel/include/ps.h"
#include "xv6-user/user.h"

void print_usage()
{
	printf("\nUsages:\n");
	printf("Try \"-> / $ ps\"\n");
	printf("Or  \"-> / $ ps -o pid,ppid,comm,state,time,etime,vsz (7 fields: optional)\"\n");
}

int setflag(char* tmp, int n)
{
	int flag = n;
	if(strncmp(tmp,"pid",6) == 0)
	{
		flag |= PS_PID;
	}
	else if(strncmp(tmp,"ppid",6) == 0)
	{
		flag |= PS_PPID;
	}
	else if(strncmp(tmp,"comm",6) == 0)
	{
		flag |= PS_COMMAND;
	}
	else if(strncmp(tmp,"state",6) == 0)
	{
		flag |= PS_S;
	}
	else if(strncmp(tmp,"time",6) == 0)
	{
		flag |= PS_TIME;
	}
	else if(strncmp(tmp,"etime",6) == 0)
	{
		flag |= PS_ELAPSED;
	}
	else if(strncmp(tmp,"vsz",6) == 0)
	{
		flag |= PS_VSZ;
	}
	if(flag == n)
	{
		printf("error: unknown user-defined format specifier \"%s\"\n",tmp);
		print_usage();
		exit(0);
	}
	return flag;
}

int parse_arg(char *arg)
{
	int flag = 0;
	char* ptr;
	char tmp[6];
	while((ptr = strchr(arg,',')) != NULL)
	{
		int len = ptr-arg;
		strncpy(tmp,arg,len);
		tmp[len] = '\0';
		flag = setflag(tmp,flag);
		arg = ptr + 1;
	}
	flag = setflag(arg,flag);
	return flag;
}

void format_time(char* times, uint64 ticks)
{
	if(ticks)
		ticks /= TICKS_FREQ;
	int hh,mm,ss;
	char tmp[3];
	ss = ticks % 60;
	ticks /= 60;
	mm = ticks % 60;
	hh = ticks / 60;
	times[0] = '\0';
	itoa(hh,tmp);
	if(hh >= 10){
		strcat(times,tmp);
	}else{
		strcat(times,"0");
		strcat(times,tmp);
	}
	strcat(times,":");
	itoa(mm,tmp);
	if(mm >= 10){
		strcat(times,tmp);
	}else{
		strcat(times,"0");
		strcat(times,tmp);
	}
	strcat(times,":");
	itoa(ss,tmp);
	if(ss >= 10){
		strcat(times,tmp);
	}else{
		strcat(times,"0");
		strcat(times,tmp);
	}
}

void print_hint(int flag)
{
	if(flag & PS_PID)
	{
		printf("PID\t");
	}
	if(flag & PS_PPID)
	{
		printf("PPID\t");
	}
	if(flag & PS_COMMAND)
	{
		printf("COMMAND\t");
	}
	if(flag & PS_S)
	{
		printf("S\t");
	}
	if(flag & PS_TIME)
	{
		printf("TIME\t\t");
	}
	if(flag & PS_ELAPSED)
	{
		printf("ELAPSED\t\t");
	}
	if(flag & PS_VSZ)
	{
		printf("VSZ");
	}
	printf("\n");
}

void print_res(struct procinfo *p, int flag)
{
	if(flag & PS_PID)
	{
		printf("%d\t",p->pid);
	}
	if(flag & PS_PPID)
	{
		printf("%d\t",p->ppid);
	}
	if(flag & PS_COMMAND)
	{
		printf("%s\t",p->command);
	}
	if(flag & PS_S)
	{
		printf("%c\t",p->state);
	}
	if(flag & PS_TIME)
	{
		char tm[10];
		format_time(tm,p->times);
		printf("%s\t",tm);
		// printf("%d\t\t",p->times);
	}
	if(flag & PS_ELAPSED)
	{
		char tm[10];
		format_time(tm,p->etime);
		printf("%s\t",tm);
		// printf("%d\t\t",p->etime);
	}
	if(flag & PS_VSZ)
	{
		printf("%d",p->vsz/1024);
	}
	printf("\n");
}

void parse_res(char* str,struct procinfo *p)
{
	char tmp[20];
	char *ptr1 = strchr(str,'\n');
	char *ptr2 = strchr(ptr1,'\t');
	ptr1 += 1;
	//pid
	int len = ptr2-ptr1;
	strncpy(tmp,ptr1,len);
	tmp[len] = '\0';
	p->pid = atoi(tmp);
	ptr2 += 1;
	//name
	ptr1 = strchr(ptr2,'\t');
	len = ptr1-ptr2;
	strncpy(p->command,ptr2,len);
	p->command[len]='\0';
	ptr1 += 2;
	//state
	ptr2 = strchr(ptr1,'\t');
	len = ptr2 - ptr1;
	strncpy(tmp,ptr1,len);
	p->state = tmp[0];
	ptr2 += 1;
	//ppid
	ptr1 = strchr(ptr2,'\t');
	len = ptr1 - ptr2;
	strncpy(tmp,ptr2,len);
	tmp[len] = '\0';
	p->ppid = atoi(tmp);
	ptr1 += 1;
	//time
	uint64 utime,stime;
	ptr2 = strchr(ptr1,'\t');
	len = ptr2 - ptr1;
	strncpy(tmp,ptr1,len);
	tmp[len] = '\0';
	utime = atoi(tmp);
	ptr2 += 1;

	ptr1 = strchr(ptr2,'\t');
	len = ptr1 - ptr2;
	strncpy(tmp,ptr2,len);
	tmp[len] = '\0';
	stime = atoi(tmp);
	ptr1 += 1;

	p->times = utime + stime;

	ptr2 = strchr(ptr1,'\t');
	ptr2 += 1;
	ptr1 = strchr(ptr2,'\t');
	ptr1 += 1;
	//vsz
	ptr2 = strchr(ptr1,'\n');
	len = ptr2 - ptr1;
	strncpy(tmp,ptr1,len);
	tmp[len] = '\0';
	p->vsz = atoi(tmp);

	p->etime = getelapsed(p->pid);
}

int main(int argc, char *argv[])
{
	if(argc > 1 && argc != 3)
	{
		print_usage();
		exit(0);
	}
	int flag = 0x7f;
	if(argc == 3)
	{
		if(strncmp(argv[1],"-o",3) != 0)
		{
			print_usage();
			exit(0);
		}
		flag = parse_arg(argv[2]);
	}
	struct stat st;
	struct procinfo pi[NPROC];
	int cnt = 0;
	int fd;
	if((fd = open("/proc", 0)) < 0){
    	fprintf(2, "ls: cannot open %s\n", "/proc");
    	exit(0);
  	}
  	fstat(fd, &st);
  	char buf[512];
  	int n;
  	while(readdir(fd, &st) == 1){//逐一读取/proc目录下的目录项
  		int fd1;
      	if(st.name[0] > '0' && st.name[0] <= '9')
      	{
      		char path[20];
      		path[0] = '\0';
      		strcat(path,"/proc/");
      		strcat(path,st.name);
      		strcat(path,"/stat");
      		//读取各进程的文件信息
      		if((fd1 = open(path, 0)) < 0){ 
		      fprintf(2, "cat: cannot open %s\n", path);
		      exit(1);
    		}
    		if((n = read(fd1, buf, sizeof(buf))) > 0) {
			    parse_res(buf,&pi[cnt]);
			}
      		cnt++;
      	}
    }
	print_hint(flag);
	int i;
	for(i = 0; i < cnt; i++)
	{
		print_res(&pi[i],flag);
	}
	exit(0);
}