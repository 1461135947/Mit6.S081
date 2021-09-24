#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 如果是普通文件则打印文件名;如果是目录则打印目录中的所有文件
void print(char *path){
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    char ** copy;
    int  index=0;
    //检索文件状态 
    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    //打印文件名
    printf("%s\n", path);
    switch(st.type){
    case T_DIR:
        
        copy=malloc(sizeof(char *)*100);
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            //跳过iNode计数为0的节点以及"."和".."节点
            if(de.inum == 0)
                continue;
            if(strcmp(de.name,".")==0||strcmp(de.name,"..")==0)
                    continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            copy[index]=malloc(strlen(buf)+1);
            memcpy(copy[index],buf,strlen(buf)+1);
            index++;
        }
         
        //递归调用；并且在此之前释放资源
        close(fd);
        for(int i=0;i<index;i++){
            print(copy[i]);
            free(copy[i]);
        }
        free(copy);
        return;
    }
    close(fd);
}
// 查找文件路径的最后一个文件名
char *getFileName(char *path){
    for(char *p=path+strlen(path);p!=path;p--){
        if(*p=='/'){
            return p+1;
        }
    }
    return path;
}
void find(char *path,char *fileName)
{
    char buf[512], *p;
    int fd;
    char ** copy;
    int  index=0;
    struct dirent de;
    struct stat st;
    //检索文件状态 
    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // 如果名字匹配则递归打印
    if(strcmp(getFileName(path),fileName)==0){
        close(fd);
        print(path);
        return ;
    }

    switch(st.type){
    case T_DIR:
        
        copy=malloc(sizeof(char *)*100);
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            //跳过iNode计数为0的节点以及"."和".."节点
            if(de.inum == 0)
                continue;
            if(strcmp(de.name,".")==0||strcmp(de.name,"..")==0)
                    continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            copy[index]=malloc(strlen(buf)+1);
            memcpy(copy[index],buf,strlen(buf)+1);
            index++;
        }
         
        //递归调用；并且在此之前释放资源
        close(fd);
        for(int i=0;i<index;i++){
            find(copy[i],fileName);
            free(copy[i]);
        }
        free(copy);
        return;
    }
    close(fd);
}
int main(int argc, char *argv[])
{
    if (argc != 3) 
    {
        fprintf(2, "usage: find <path> <name>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}
