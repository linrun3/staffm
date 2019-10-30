#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sqlite3.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#define AL 1
#define UL 2
#define QN 3
#define QA 4
#define MOD 5
#define ADD 6
#define HIST 7
#define DEL 8
#define UQ 9
#define UU 10
typedef struct sg
{
  int type;
  char name[20];
  char data[256];
  
}Msg;
void AdminLogin(int newfd);// 管理员登录判断，若可以登录则能进入doclient函数
void DoClient(int newfd);//内涵所有功能函数，根据相关类型进行调用，一直循环，直到客户端掉线或发出退出请求
void UsrLogin(int newfd);//用户登录判断
void AdminQueryName(int newfd);//管理员按名字查找函数
void AdminQueryALL(int newfd);//管理员查找所有人员函数
void Mod(int newfd);//修改函数
void HistInsert();//历史记录插入函数，当发生增删改时调用此函数，将时间用户行为插入到数据库
void Hist(int newfd);//历史记录显示函数
void Add(int newfd); //  添加函数
void Del(int newfd);
void UsrQue(int newfd);
void UsrUpdate(int newfd);
Msg msg;//全局结构体
sqlite3 *db;
char g_name[30];//当前用户名
int main(int argc, const char *argv[])
{
    if(argc <3)
    {
        printf("请输入ip地址及端口");
        exit(-1);
    }
    if(sqlite3_open("staff.db",&db)!= 0)
    {
        fprintf(stderr,"sqlite3 open %s",sqlite3_errmsg(db));
        return -1;
    }
    printf("the data base open success\n");
#if 0
    if(sqlite3_exec(db,"create table usrinfo(staffno integer,usertype integer,name text,passwd text,age integer,phone text,addr text,work text,date text,level integer,salary REAL);",NULL,NULL,&errmsg)!=0)
    {
        fprintf(stderr,"%s",errmsg);
    }
    if(sqlite3_exec(db,"create table historyinfo(time text,name text,words text);",NULL,NULL,&errmsg)!=0)
    {
        fprintf(stderr,"%s",errmsg);
    }
#endif
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0)
    {
        perror("socket");
        exit(-1);
    }
    /**
     * 设置地址端口重用
     */
    int optval = 1;
    socklen_t optlen = sizeof(optval);
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,optlen);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    socklen_t addrlen = sizeof(addr);
    if(bind(sockfd,(struct sockaddr*)&addr,addrlen)<0)
    {
        perror("bind fail");
        return -1;
    }
    if(listen(sockfd,5)<0)
    {
        perror("listen fail");
        return -1;
    }
    int newfd,pid;
    signal(SIGCHLD,SIG_IGN); //回收僵尸进程
    struct sockaddr_in cliaddr;
    struct in_addr addrip; //一个网络字节序到字符串的函数inet_ntoa中所需参数类型
    socklen_t clilen = sizeof(cliaddr); // 添加结构体用于保存客户端内容 
    while(1)
    {
        newfd = accept(sockfd,(struct sockaddr*)&cliaddr,&clilen);
        if(newfd<0)
        {
            printf("fail to accepti\n");
        }
        memcpy(&addrip,&cliaddr.sin_addr.s_addr,4);//将网络内容强行copy给struct in_addr类型
        printf("accept sockfd  %d\n ip address :%s\n",newfd,inet_ntoa(addrip));
        pid = fork();
        if(pid < 0)
        {
            printf("fail to fork\n");
        }
        if(pid == 0)
        {
            close(sockfd);
            DoClient(newfd);//子进程进行相关处理
        }
        else
        {
            close(newfd);
        }
    }

    return 0;
}
void DoClient(int newfd)
{
loop:    
    printf("do client\n");
        if(recv(newfd,&msg,sizeof(msg),0)<=0)
        {
            printf("client disconnecct\n");
            exit(-1);
        }
         printf("msg->type :%d\n",msg.type);
        if(msg.type == AL)
        {
            AdminLogin(newfd);
        }
        else if(msg.type == UL)
        {
            UsrLogin(newfd);        
        }
        else if(msg.type == QN)
        {
            AdminQueryName(newfd);
        }
        else if(msg.type == QA)
        {
            AdminQueryALL(newfd);
        }
        else if(msg.type == MOD)
        {
            Mod(newfd);
        }
        else if(msg.type == HIST)
        {
            Hist(newfd);
        }
        else if(msg.type == ADD)
        {
            Add(newfd);
        }
        else if(msg.type == DEL)
        {
            Del(newfd);
        }
        else if(msg.type == UQ)
        {
            UsrQue(newfd);
        }
        else if(msg.type == UU)
        {
            UsrUpdate(newfd);
        }
goto loop;
}
void AdminLogin(int newfd)
{
    
    char sql[400],*errmsg;
    int nrow,ncol;
    char **result;
    sprintf(sql,"select * from usrinfo where name = '%s' and passwd = '%s' and usertype = 0;",msg.name,msg.data);
    if(sqlite3_get_table(db,sql,&result,&nrow,&ncol,&errmsg)!=0)
    {
        fprintf(stderr,"%s",errmsg);
    }
    if(nrow ==1) 
    {   memset(msg.data,0,sizeof(msg.data));//发送确认信息
        strcpy(msg.data,"OK");
        printf("send ok \n");
        send(newfd,&msg,sizeof(msg),0);
        strcpy(g_name,msg.name);
        printf("admin %s  have log in\n",msg.name);
    }
    else 
    {   strcpy(msg.data,"wrong");
        send(newfd,&msg,sizeof(msg),0);
    }
}
void UsrLogin(int newfd)
{
     char sql[400],*errmsg;
    int nrow,ncol;
    char **result;
    sprintf(sql,"select * from usrinfo where name = '%s' and passwd = '%s';",msg.name,msg.data);
    if(sqlite3_get_table(db,sql,&result,&nrow,&ncol,&errmsg)!=0)
    {
        fprintf(stderr,"%s",errmsg);//若能够返回则说明用户存在
    }
    if(nrow == 1) 
    {   memset(msg.data,0,sizeof(msg.data));//发送确认信息
        strcpy(msg.data,"OK");
        printf("send ok\n");
        send(newfd,&msg,sizeof(msg),0);
        strcpy(g_name,msg.name);
        printf("user %s have log in\n",msg.name);
    }

}
void AdminQueryName(int newfd)
{
     char sql[400],*errmsg;
     int nrow,ncol;
     char **result;

    sprintf(sql,"select * from usrinfo where name = '%s' ;",msg.data);
    memset(msg.data,0,sizeof(msg.data));
    if(sqlite3_get_table(db,sql,&result,&nrow,&ncol,&errmsg)!=0)
    {
        printf("sqlite: %s\n",errmsg);
    }
    printf("%s\n",sql);
    int i,j,num=0;
    for(i=0;i<=nrow;i++)
    { for(j=0;j<ncol;j++)
        {
            memset(msg.data,0,sizeof(msg.data));
            strcpy(msg.data,result[num++]);
            send(newfd,&msg,sizeof(msg),0);
            printf("%s   ",msg.data);
        }
        printf("\n");
    }
    strcpy(msg.data,"all");//发送截止信息
    send(newfd,&msg,sizeof(msg),0);
    printf("send all\n");
    return;

}
void AdminQueryALL(int newfd)
{
    char sql[400],*errmsg;
     int nrow,ncol;
     char **result;

    sprintf(sql,"select * from usrinfo;");
    memset(msg.data,0,sizeof(msg.data));
    if(sqlite3_get_table(db,sql,&result,&nrow,&ncol,&errmsg)!=0)
    {
        printf("sqlite: %s\n",errmsg);
    }
    printf("%s\n",sql);
    int i,j,num=0;
            printf("send usrinfo\n");
    for(i=0;i<=nrow;i++)
    { for(j=0;j<ncol;j++)
        {
            memset(msg.data,0,sizeof(msg.data));
            strcpy(msg.data,result[num++]);
            send(newfd,&msg,sizeof(msg),0);
            printf("%s   ",msg.data);
        }
        printf("\n");
    }
    strcpy(msg.data,"all");//发送截止信息
    send(newfd,&msg,sizeof(msg),0);
    printf("send all\n");
    return;

}
void Mod(int newfd)
{
    char *errmsg;
    int num = atoi(msg.data);
    printf("mod num %d\n",num);
    recv(newfd,&msg,sizeof(msg),0);
    printf("%s\n",msg.data);
    if(sqlite3_exec(db,msg.data,NULL,NULL,&errmsg)!=0)
    {
       strcpy(msg.data,"fail");
       printf("send fail msg\n");
       send(newfd,&msg,sizeof(msg),0);
    }
    else 
    {
        strcpy(msg.data,"OK");
        printf("send success msg\n");
        send(newfd,&msg,sizeof(msg),0);
    }
    recv(newfd,&msg,sizeof(msg),0);
    HistInsert();

}
void HistInsert()
{
    time_t t;
	struct tm *tp;
    char *errmsg,sql[400];
	time(&t);
    
	//进行时间格式转换
	tp = localtime(&t);
    char date[100];
	sprintf(date, "%d-%d-%d %d:%d:%d", tp->tm_year + 1900, tp->tm_mon+1, tp->tm_mday, 
			tp->tm_hour, tp->tm_min , tp->tm_sec);
	sprintf(sql,"insert into historyinfo values('%s','%s','%s')",date,g_name,msg.data);
    sqlite3_exec(db,sql,NULL,NULL,&errmsg);
    
	return ;
}
void Hist(int newfd)
{
    char **result,*errmsg;
    int row,col,i,j,num=0;
    sqlite3_get_table(db,"select * from historyinfo;",&result,&row,&col,&errmsg);
    printf("send history info,nrow = %d column = %d\n",row,col);
    for(i=0;i<=row;i++)
    { for(j=0;j<col;j++)
        {
                strcpy(msg.data,result[num++]);
                send(newfd,&msg,sizeof(msg),0);
                printf("%s     ",msg.data);
        }
        printf("\n");
    }
    strcpy(msg.data,"all");
    send(newfd,&msg,sizeof(msg),0);
    printf("all sended\n");
}
void Add(int newfd)
{
    recv(newfd,&msg,sizeof(msg),0);
    printf("%s\n",msg.data);
    char *errmsg;
    sqlite3_exec(db,msg.data,NULL,NULL,&errmsg);
    recv(newfd,&msg,sizeof(msg),0);
    HistInsert();
}
void Del(int newfd)
{
    char *errmsg;
    printf("%s\n",msg.data);
    if(sqlite3_exec(db,msg.data,NULL,NULL,&errmsg)!=0)
    {
        printf("%s\n",errmsg);
        strcpy(msg.data,errmsg);
        send(newfd,&msg,sizeof(msg),0);
        return ;
    }
    strcpy(msg.data,"OK");
    send(newfd,&msg,sizeof(msg),0);
    recv(newfd,&msg,sizeof(msg),0);
    HistInsert();
}
void UsrQue(int newfd)
{

     char *errmsg;
     int nrow,ncol;
     char **result;
    if(sqlite3_get_table(db,msg.data,&result,&nrow,&ncol,&errmsg)!=0)
    {
        printf("sqlite: %s\n",errmsg);
    }
    printf("%s\n",msg.data);
    int i,j,num=0;
    for(i=0;i<=nrow;i++)
    { for(j=0;j<ncol;j++)
        {
            memset(msg.data,0,sizeof(msg.data));
            strcpy(msg.data,result[num++]);
            send(newfd,&msg,sizeof(msg),0);
            printf("%s   ",msg.data);
        }
        printf("\n");
    }
    strcpy(msg.data,"all");//发送截止信息
    send(newfd,&msg,sizeof(msg),0);
    printf("send all\n");
}
void UsrUpdate(int newfd)
{

}
