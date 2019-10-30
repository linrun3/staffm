#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#define AL 1
#define UL 2
#define QN  3
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
void AdminLogin(int sockfd);//管理员登录函数，将信息发送至服务器进行判断，若返回ok则能够调用管理员菜单
void AdminMenu(int sockfd);//管理员菜单，内含管理员功能函数
void UsrLogin(int sockfd);//用户登录函数
void Query(int sockfd);//管理员查找函数
void Mod(int sockfd);//管理员修改函数
void Add(int sockfd);//管理员进行用户添加函数
void Del(int sockfd);//管理员进行删除函数
void History(int sockfd);//历史查询函数
void UsrMenu(int sockfd);//用户菜单，有用户能执行的函数
void UsrQue(int sockfd);
void UsrUpdate(int sockfd);
char g_name[30];
int main(int argc, const char *argv[])
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(argc < 3)
    {
        printf("please input server addr\n");
        exit(-1);
    }
    if(sockfd < 0 )
    {
        perror("sockfd");
        exit(-1);
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    socklen_t addrlen = sizeof(addr);
    if(connect(sockfd,(struct sockaddr*)&addr,addrlen)<0)
    {
        perror("connect");
        exit(-1);
    }
    while(1)
    {       
        printf("*************************************************************\n********  1：管理员模式    2：普通用户模式    3：退出********\n*************************************************************\n");
        printf("请输入您的选择（数字）>>");
        int n;
        scanf("%d",&n);
        getchar();
        switch(n)
        {
            case 1: AdminLogin(sockfd);break;
            case 2: UsrLogin(sockfd);break;
            case 3: return 0 ;break;
            default:
                   printf("请重新输入\n");
        }
    }
    return 0;
}
void AdminLogin(int sockfd)
{
    Msg msg;
    msg.type = AL;
loop:
    printf("请输入用户名:");
    scanf("%s",msg.name);
    getchar();
    char tempname[20];
    strcpy(tempname,msg.name);
    printf("请输入密码：（6位）：");
    scanf("%s",msg.data);
    getchar();
    if(send(sockfd,&msg,sizeof(msg),0)<0)
    {
        printf("send fail\n");
        return ;
    }
    if(recv(sockfd,&msg,sizeof(msg),0)<0)
    {
        printf("recv fail\n");
    }
    if(strncmp(msg.data,"OK",2)==0)
    {
         strcpy(g_name,msg.name);
         AdminMenu(sockfd);
         return;// 当选项函数结束后退回主界面
    }
    else 
    printf("密码错误\n");
    goto loop;

}
void UsrLogin(int sockfd)
{
    Msg msg;
    msg.type = UL;
loop:
    printf("请输入用户名:");
    scanf("%s",msg.name);
    getchar();
    char tempname[20];
    strcpy(tempname,msg.name);
    printf("请输入密码：（6位）：");
    scanf("%s",msg.data);
    getchar();
    if(send(sockfd,&msg,sizeof(msg),0)<0)
    {
        perror("send fail\n");
        return ;
    }
    if(recv(sockfd,&msg,sizeof(msg),0)<0)
    {
        perror("recv fail\n");
    }
    if(strncmp(msg.data,"OK",2)==0)
    {
        printf("msg.data %s\n",msg.data); 
        strcpy(g_name,tempname);
        UsrMenu(sockfd);
        return;
    }
    else
    printf("密码错误\n");
    goto loop;
}

void AdminMenu(int sockfd)
{
        
loop:
        printf("*************************************************************\n");
        printf("* 1：查询  2：修改 3：添加用户  4：删除用户  5：查询历史记录*\n");
        printf("* 6：退出													*\n");
        printf("*************************************************************\n");
        printf("请输入您的选择（数字）>>:");
        int n;
        scanf("%d",&n);
        getchar();
        switch(n)
        {
            case 1:printf("do AdminQuery\n");Query(sockfd);break;
            
            case 2:printf("do admin update\n");Mod(sockfd);break;
            case 3:printf("do admin add\n");Add(sockfd);break;
            case 4:printf("do admin delete\n");Del(sockfd);break;
            case 5:printf("do admin history");History(sockfd);break;
            case 6: return;break;
            default:
                   printf("请重新选择\n");
        }
                   goto loop;

}
void UsrMenu(int sockfd)
{
loop:
        printf("*************************************************************\n");
        printf("*************  1：查询  	2：修改		3：退出	 *************\n");
        printf("*************************************************************\n");
        printf("请输入您的选择（数字）>>");
        int n;
        scanf("%d",&n);
        getchar();
        if(n==1)
        {
            printf("do usrquery\n");
            UsrQue(sockfd);
        }
        else if(n==2)
        {
            printf("do usrupdate\n");
            UsrUpdate(sockfd);
        }
        else if(n==3)
        {
            return;
        }
        goto loop;
        
        
}
void Query(int sockfd)
{     
    Msg msg;
    int n;
loop:
    printf("*************************************************************\n");
    printf("*   ******* 1：按人名查找  	2：查找所有 	3：退出	 ********\n");
    printf("*************************************************************\n");
    printf("请输入您的选择（数字）>>");
    scanf("%d",&n);
    getchar();
    switch(n)
    {
        case 1:msg.type = QN;goto next;break;
        case 2:msg.type = QA;goto next;break;
        case 3:return;break;
        default:
               printf("请重新输入\n");
    }
    goto loop;
next:
    if(msg.type == QN)
    {
        printf("请输入您要查找的用户名：");
        scanf("%s",msg.data);
        getchar();
        send(sockfd,&msg,sizeof(msg),0);
        printf("工号	用户类型	 姓名	密码	年龄	电话	地址	职位	入职年月	等级	 工资\n");
        int cnt = 0;
        while(1)
        {
            memset(msg.data,0,sizeof(msg.data));
            if(recv(sockfd,&msg,sizeof(msg),0)>0)
            {
                if(strncmp(msg.data,"all",3)==0)
                {
                    printf("recv all \n");
                    break;
                }
                printf("%-10s",msg.data);
                cnt++;
                if(cnt == 11)
                {
                    printf("\n");
                    cnt = 0;
                }
            }
        }
    }
    if(msg.type == QA)
    {
        send(sockfd,&msg,sizeof(msg),0);
        printf("工号	用户类型	 姓名	密码	年龄	电话	地址	职位	入职年月	等级	 工资\n");
        int cnt = 0;
        memset(msg.data,0,sizeof(msg.data));
        while(1)
        {
            if(recv(sockfd,&msg,sizeof(msg),0)>0)
            {
                if(strncmp(msg.data,"all",3)==0)
                {
                    printf("recv all\n");
                    break;
                }
                printf("%-10s",msg.data);
                cnt++;
                if(cnt == 11)
                {
                    printf("\n");
                    cnt = 0;
                }
            }
        }
    }

    goto loop;    

}
void Mod(int sockfd)
{
    Msg msg;
    int num;
    int staffno;
    msg.type = MOD;
    char name[30];
    char value[30];
    printf("请输入您要修改职员的工号：");
    scanf("%s",msg.data);
    getchar();
    send(sockfd,&msg,sizeof(msg),0);
    staffno = atoi(msg.data);
loop:
    printf("*******************请输入要修改的选项********************\n");
    printf("******	1：姓名	  2：年龄	3：家庭住址   4：电话  ******\n");
    printf("******	5：职位	   6：工资  7：入职年月   8：评级  ******\n");
    printf("******	9：密码	 10：退出				  *******\n");
    printf("*************************************************************\n");
    printf("请输入您的选择（数字");
    scanf("%d",&num);
    getchar();
    int flag = 0;//区分是否需要加引号标记
    switch(num)
    {
        case 1:strcpy(name,"name");
               printf("请输入姓名：");
               scanf("%s",value);
               getchar();
               flag = 1;
               break;
        case 2:strcpy(name,"age");
               printf("请输入年龄："); 
               scanf("%s",value);
               getchar();
               break;
        case 3:strcpy(name,"addr");
               printf("请输入家庭住址：");
               scanf("%s",value);
               getchar();
               flag = 1;
               break;
        case 4:strcpy(name,"phone");
               printf("请输入电话：");
               scanf("%s",value);
               getchar();
               flag = 1 ;
               break;
        case 5:strcpy(name,"work");
               printf("请输入工作：");
               scanf("%s",value);
               getchar();
               flag = 1; 
               break;
        case 6:strcpy(name,"salary");
               printf("请输入薪水：");
               scanf("%s",value);
               getchar();
               break;
        case 7:strcpy(name,"date");
               printf("请输入日期：");
               scanf("%s",value);
               getchar();
               flag = 1;
               break;
        case 8:strcpy(name,"level");break;
               printf("请输入等级：");
               scanf("%s",value);
               getchar();
               break;
        case 9:strcpy(name,"passwd");
               printf("请输入密码：");
               scanf("%s",value);
               getchar();
               flag = 1; 
               break;
        case 10:return;break;
        default:printf("请重新选择\n");
                goto loop;
                break;
    }
    if(!flag)
    sprintf(msg.data,"update usrinfo set %s=%s where staffno =%d ",name,value,staffno);
    else 
    sprintf(msg.data,"update usrinfo set %s='%s' where staffno =%d ",name,value,staffno);

    send(sockfd,&msg,sizeof(msg),0);
    recv(sockfd,msg.data,sizeof(msg.data),0);
    if(strncmp(msg.data,"OK",0)==0)
    {
        printf("修改成功\n");
    }
    else 
    {
        printf("修改失败 \n");
    }
    sprintf(msg.data,"管理员%s修改了工号为%d的%s为%s",g_name,staffno,name,value);
    send(sockfd,&msg,sizeof(msg),0);

}
void Add(int sockfd)
{
    Msg msg; 
    msg.type = ADD;
loop:
    send(sockfd,&msg,sizeof(msg),0);
loop2:
    printf("***************热烈欢迎新员工***************\n");
    printf("请输入工号");
    int staffno;
    scanf("%d",&staffno);
    getchar();
    printf("您输入的工号%d 是一旦录入无法更改是否录入?Y/N",staffno);
    char confirm;
    scanf("%c",&confirm);
    getchar();
    if(confirm=='Y'||confirm=='y')
    {
        printf("请输入用户名");
        char name[20];
        scanf("%s",name);
        getchar();
        printf("请输入用户密码");
        char passwd[20];
        scanf("%s",passwd);
        getchar();
        printf("请输入年龄");
        int age;
        scanf("%d",&age);
        getchar();
        printf("请输入电话");
        char phone[20];
        scanf("%s",phone);
        getchar();
        printf("请输入住址");
        char addr[30];
        scanf("%s",addr);
        getchar();
        printf("请输入职位");
        char work[30];
        scanf("%s",work);
        getchar();
        printf("请输入入职日期");
        char date[20];
        scanf("%s",date);
        getchar();
        printf("请输入评级");
        int level;
        scanf("%d",&level);
        getchar();
        printf("请输入薪水");
        float salary;
        scanf("%f",&salary);
        getchar();
        printf("请输入是否为管理员Y/N");
        char type;
        int usrtype;
        scanf("%c",&type);
        getchar();
        if(type == 'Y'||type=='y')
        {
            usrtype = 1;
        }
        else 
        {
            usrtype = 0;
        }
        sprintf(msg.data,"insert into usrinfo values(%d,%d,'%s','%s',%d,'%s','%s','%s','%s',%d,%f)",staffno,usrtype,name,passwd,age,phone,addr,work,date,level,salary);
        send(sockfd,&msg,sizeof(msg),0);
        sprintf(msg.data,"管理员%s添加了工号为%d的用户%s",g_name,staffno,name);
        send(sockfd,&msg,sizeof(msg),0);
        printf("添加完成,是否继续添加Y/N\n");
        char con;
        scanf("%c",&con);
        getchar();
        if(con=='N'||con=='n')
        {
            return;
        }
    }
    else 
    {
        goto loop2;
    }
    goto loop;

}
void History(int sockfd)
{
    Msg msg;
    msg.type = HIST;
    send(sockfd,&msg,sizeof(msg),0);
    int cnt = 0;
    while(1)
    {   
        memset(msg.data,0,sizeof(msg.data));
        if(recv(sockfd,&msg,sizeof(msg),0)>0)
        {
            if(strncmp(msg.data,"all",3)==0)
            {      
                printf("all recv\n");
                break;
            }
                printf("%s-----",msg.data);
                cnt ++;
            if(cnt == 3)
            {
                printf("\n");
                cnt = 0;
            }
        }
    }
}
void Del(int sockfd)
{
    Msg msg;
    msg.type = 8;
    printf("请输入要删除的工号:");
    int staffno;char name[20];
    scanf("%d",&staffno);
    printf("请输入要删除的姓名：");
    scanf("%s",name);
    sprintf(msg.data,"delete from usrinfo where staffno = %d and name = '%s';",staffno,name);
    send(sockfd,&msg,sizeof(msg),0);
    recv(sockfd,&msg,sizeof(msg),0);
    if(strncmp(msg.data,"OK",2)!=0)
    {
        printf("失败: %s\n",msg.data);
        return;
    }
    sprintf(msg.data,"管理员%s删除了工号为%d的用户%s",g_name,staffno,name);
    send(sockfd,&msg,sizeof(msg),0);
    printf("数据库修改成功，删除工号为%d的用户\n",staffno);
}
void UsrQue(int sockfd)
{
    Msg msg;
    msg.type = UQ;
    sprintf(msg.data,"select * from usrinfo where name = '%s';",g_name);
    send(sockfd,&msg,sizeof(msg),0);
    int cnt=0;
    printf("工号	用户类型	 姓名	密码	年龄	电话	地址	职位	入职年月	等级	 工资\n");
    while(1)
        {
            memset(msg.data,0,sizeof(msg.data));
            if(recv(sockfd,&msg,sizeof(msg),0)>0)
            {
                if(strncmp(msg.data,"all",3)==0)
                {
                    printf("recv all \n");
                    break;
                }
                printf("%-10s",msg.data);
                cnt++;
                if(cnt == 11)
                {
                    printf("\n");
                    cnt = 0;
                }
            }
        }
        
    
}
void UsrUpdate(int sockfd)
{
    
    Msg msg;
    int num;
    msg.type = MOD;
    char name[30];
    char value[30];
    send(sockfd,&msg,sizeof(msg),0);
loop:
    printf("*******************请输入要修改的选项********************\n");
    printf("******	1：姓名	  2：年龄	3：家庭住址   4：电话  ******\n");
    printf("******	5：职位	   6：工资  7：入职年月   8：评级  ******\n");
    printf("******	9：密码	 10：退出				  *******\n");
    printf("*************************************************************\n");
    printf("请输入您的选择（数字");
    scanf("%d",&num);
    getchar();
    switch(num)
    {
        case 1:strcpy(name,"name");
               printf("请输入姓名：");
               scanf("%s",value);
               getchar();
               break;
        case 2:strcpy(name,"age");
               printf("请输入年龄："); 
               scanf("%s",value);
               getchar();
               break;
        case 3:strcpy(name,"addr");
               printf("请输入家庭住址：");
               scanf("%s",value);
               getchar();
               break;
        case 4:strcpy(name,"phone");
               printf("请输入电话：");
               scanf("%s",value);
               getchar();
               break;
        case 5:strcpy(name,"work");
               printf("请输入工作：");
               scanf("%s",value);
               getchar();
               break;
        case 6:strcpy(name,"salary");
               printf("请输入薪水：");
               scanf("%s",value);
               getchar();
               break;
        case 7:strcpy(name,"date");
               printf("请输入日期：");
               scanf("%s",value);
               getchar();
               break;
        case 8:strcpy(name,"level");break;
               printf("请输入等级：");
               scanf("%s",value);
               getchar();
               break;
        case 9:strcpy(name,"passwd");
               printf("请输入密码：");
               scanf("%s",value);
               getchar();
               break;
        case 10:return;break;
        default:printf("请重新选择\n");
                goto loop;
                break;
    }
    sprintf(msg.data,"update usrinfo set %s=%s where name ='%s';",name,value,g_name);
    send(sockfd,&msg,sizeof(msg),0);
    recv(sockfd,msg.data,sizeof(msg.data),0);
    if(strncmp(msg.data,"OK",0)==0)
    {
        printf("修改成功\n");
    }
    else 
    {
        printf("修改失败 \n");
    }
    sprintf(msg.data,"姓名为%s的员工将%s为%s",g_name,name,value);
    send(sockfd,&msg,sizeof(msg),0);

}
