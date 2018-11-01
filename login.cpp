#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h> // data returned by the stat() function
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <vector>
using namespace std;

#define PATH_MAX 4096
#define FIFO_NAME "mystatFIFO"

static const size_t npos=-1;
void PrintStats(string my_path_string, int fd)
{
    char* my_path = new char[my_path_string.length() + 1] ; // convert string to char
    strcpy(my_path, my_path_string.c_str());
    char my_stats[800]="          --- STATS ---\n\n         File: ";

    struct stat st;
    if(0 != stat(my_path,&st) )
    {
        perror("[stat] error: ");
        return;
    }
    strcat(my_stats,my_path);
    strcat(my_stats,"\n         Type: ");
    switch(st.st_mode & S_IFMT)
    {
        case S_IFREG : strcat(my_stats,"Regular file\n");break;
        case S_IFIFO : strcat(my_stats,"FIFO\n"); break;
        case S_IFSOCK: strcat(my_stats,"Socket\n"); break;
        case S_IFLNK : strcat(my_stats,"Link\n");break;
        case S_IFBLK : strcat(my_stats,"Block device\n"); break;
        case S_IFCHR : strcat(my_stats,"Character device\n");break;
        case S_IFDIR : strcat(my_stats,"Directory\n"); break;
        default: strcat(my_stats,"Unknown file type\n");
    }

    int pos=0;
    string my_file_name;

    for(int p = 0; p < my_path_string.length(); p++)    // last '/' to find
        if(my_path_string[p]=='/')
            pos = p;
    
    bool isHidden = false;
    if(pos!=0){
        my_file_name = my_path_string.substr(pos+1); 
    }
    if(my_file_name[0] == '.')
            isHidden = true;

    strcat(my_stats,"       Hidden: ");
    if(isHidden == true) strcat(my_stats,"yes\n");
    else strcat(my_stats,"no\n");


    strcat(my_stats,"    Extension: ");

    if( isHidden == false)
            pos=0;
    else pos = pos +2;         // if the file is hidden, we jump over the first '.'

    int p = pos;
    while(p < my_path_string.length())
    {
        if(my_path_string[p]=='.')
        {
            pos = p;
            break;
        }
        p++;
    }

    if(pos!=0){
        string extension = my_path_string.substr(pos);
        char extension_char[25];
        strcpy(extension_char,extension.c_str()); // convert to char
        strcat(my_stats, extension_char);
    }
    else 
        strcat(my_stats,"-");
    strcat(my_stats,"\n");

    // These don't seem to work with strcat, but we can use cout to prin them
    //strcat(my_stats,"    Dimension: "); 
    //long long my_size = (long long)st.st_size;
    //cout << " bytes" << endl;

    char acc[15]="000/----------"; // acc string

    if(S_IFDIR & st.st_mode) acc[4]='d';
    if(S_IRUSR & st.st_mode) acc[5]='r';
    if(S_IWUSR & st.st_mode) acc[6]='w';
    if(S_IXUSR & st.st_mode) acc[7]='x';
    if(S_IRGRP & st.st_mode) acc[8]='r';
    if(S_IWGRP & st.st_mode) acc[9]='w';
    if(S_IXGRP & st.st_mode) acc[10]='x';
    if(S_IROTH & st.st_mode) acc[11]='r';
    if(S_IWOTH & st.st_mode) acc[12]='w';
    if(S_IXOTH & st.st_mode) acc[13]='x';

    strcat(my_stats,"  Permissions:");
    int x=5,y=6,z=7,i=0;
    bool ro = false;
    char who_char[30];
    while(i <= 2)
    {
        string who;
        if(i==0) who=" Owner ";
        else if(i==1) who = "\t       Group ";
        else who = "\t       Others";
        if(acc[x]=='-' && acc[y]=='-' && acc[z]=='-') 
        {
            strcpy(who_char,who.c_str());
            strcat(my_stats,who_char);
            strcat(my_stats," does not have any permissions.\n");
        }
        else
        {
            strcpy(who_char,who.c_str());
            strcat(my_stats,who_char);
            strcat(my_stats," can ");
            if(acc[x]!='-' && acc[y]!='-' && acc[z]!='-'){ strcat(my_stats, "read, write and execute the file.\n"); acc[i] = '7';}
            if(acc[x]!='-' && acc[y]!='-' && acc[z]=='-'){ strcat(my_stats, "read and write the file.\n"); acc[i] = '6'; }
            if(acc[x]!='-' && acc[y]=='-' && acc[z]!='-'){ strcat(my_stats, "read and execute the file.\n"); acc[i] = '5'; }
            if(acc[x]!='-' && acc[y]=='-' && acc[z]=='-'){ strcat(my_stats, "read the file.\n"); acc[i] = '4'; }
            if(acc[x]=='-' && acc[y]!='-' && acc[z]!='-'){ strcat(my_stats, "write and execute the file.\n"); acc[i] = '3'; }
            if(acc[x]=='-' && acc[y]!='-' && acc[z]=='-'){ strcat(my_stats, "write the file.\n"); acc[i] = '2'; }
            if(acc[x]=='-' && acc[y]=='-' && acc[z]!='-'){ strcat(my_stats, "execute the file.\n"); acc[i] = '1'; }
            cout << endl;
            x+=3; y+=3; z+=3;
        }
        i++;
    }
    if(acc[0] == '4' && acc[1] == '4' && acc[2] == '4') strcat(my_stats, "\t       File is READ-ONLY.\n");
    strcat(my_stats,"               ");
    strcat(my_stats,acc);
    strcat(my_stats,"\n");


    struct passwd *pwd;         // used for finding out the username of an ID
    pwd = getpwuid(st.st_uid);
    if(pwd != NULL)
    {
       strcat(my_stats,"   Owner name: " );
       strcat(my_stats, pwd->pw_name);
       strcat(my_stats, "\n");
    }
    // cout <<  "    Owner UID: ";
    // cout << (long)st.st_uid << endl;

    // cout << "    Group GID: " << (long)st.st_gid << endl;
    strcat(my_stats,"     Accessed: "); 
    strcat(my_stats,ctime(&st.st_atime));
    strcat(my_stats,"     Modified: "); 
    strcat(my_stats,ctime(&st.st_mtime));
    write(fd,my_stats,strlen(my_stats));
    close(fd);
	
}

void MyStat(string file_name)
{
    pid_t pid,p[2];
    int wait_for_kid, fd;
    string file_name_2;
    char file_name_char[200], file_name_char2[200];
    char stats[800];

    if( pipe(p) == -1)
    {
        perror("error: pipe login");
        exit(1);
    }
    pid = fork();
    if(pid == -1)
    {
        perror("error: fork login");
        exit(2);
    }
    if(pid==0)
    {
        // child
        close(p[1]);    // reading only, so close write channel
        read(p[0], &file_name_char2, sizeof(file_name_char2)); // read file_name from parent through pipe
        close(p[0]);

        file_name_2 = string(file_name_char2);  // convert to string
        
        fd = open(FIFO_NAME,O_WRONLY);  // write stats through FIFO to parent
        PrintStats(file_name_2, fd);
        close(fd);
	
        wait_for_kid=2;
	    exit(wait_for_kid);
    }
    else if(pid)
    {
        // parent
        close(p[0]);

        strcpy(file_name_char,file_name.c_str()); // convert to char
        write(p[1],&file_name_char,sizeof(file_name_char)); // write file_name to child using pipe
	    close(p[1]);

        fd = open(FIFO_NAME,O_RDONLY);  // get stats through FIFO from child
        read(fd,stats,500);
        cout << stats << endl;

        wait(&wait_for_kid);
        close(fd);
        close(p[1]);
    }
}


void RecursiveFind(char *path, string file_name, int fd)
{
    DIR *dir;
    struct dirent *de;
    struct stat st;
    char my_path[PATH_MAX];
    string dName, dName_hidden;

    if(stat(path,&st) !=0)
    {
        fprintf(stderr,"[stat] error for %s .\t", path);
        perror("error: ");
        return;
    }

    if( S_ISDIR(st.st_mode) )  // if path is directory
    {
        if((dir=opendir(path)) == NULL )
        {
            perror("error: cannot open directory. ");
            return; // keep going anyway
        }
        while( (de=readdir(dir)) != NULL)
        { 
            if( strcmp(de->d_name,".") && strcmp(de->d_name,"..") ) // ignore . and .. directories
            {
                sprintf(my_path,"%s/%s",path,de->d_name);

                dName = string(de->d_name); // convert char to string
		        string fullname = file_name;
                if(dName.find(fullname)!=npos)
                {
                    write(fd,my_path,strlen(my_path)); 
                }
		        fullname = "." + file_name;
                if(dName.find(fullname)!=npos)
                {
                    write(fd,my_path,strlen(my_path));
                }
                RecursiveFind(my_path, file_name, fd); // recursive call
            }
        }
        closedir(dir);
    }
}

void MyFind(string file_name)
{
    vector<string> files_list;
    pid_t pid;
    int sockp[2];
    int wait_for_kid, fd;
    char file_name_char[200], s[500]; // used in parent
    char file_name_char2[200]; // used in kid
    string file_name_string;
    string found_file_string;

    mknod(FIFO_NAME, S_IFIFO | 0666,0);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
    { 
        perror("Err... socketpair"); 
        exit(1); 
    }

    pid = fork();
    if(pid == -1)
    {
        perror("error: fork login");
        exit(2);
    }
    if(pid==0)
    {
        // child - socket 0
        close(sockp[1]);
        if (read(sockp[0], file_name_char2, 200) < 0) perror("[copil]Err..read"); // receive file_name through socket
        close(sockp[0]);
        
        char start_dir[23] = "/home/dana";
        struct stat st;
        /* stat() -> get file status
        stat(const char* path, struct stat buf)
        the function obtains information about the named file and writes it to the area pointed
        to by the buf argument */
        if(stat(start_dir,&st) != 0 )   
        {
            perror("[stat] error");
            exit(1);
        }
        if( S_ISDIR(st.st_mode) == 0)
        {
            fprintf(stderr,"error: %s is not a directory\n",start_dir);
            exit(2);
        }

        file_name_string = string(file_name_char2);         // convert file_name_char2 to string

        fd = open(FIFO_NAME,O_WRONLY);
        RecursiveFind(start_dir, file_name_string,fd);
        close(fd);
	    wait_for_kid=2;
	    exit(wait_for_kid);
    }
    else if(pid)
    {
        // parent - socket 1
        close(sockp[0]);
        
        strcpy(file_name_char,file_name.c_str()); // convert to char

        write(sockp[1],file_name_char, sizeof(file_name_char));   // send file_name to child through socket

        fd = open(FIFO_NAME,O_RDONLY);
        while(read(fd,s,500))
        {
            cout << "This file has been found:"<< s << endl;
            found_file_string = string(s);
            files_list.push_back(found_file_string);
        }

        for(int i=0;i<files_list.size();i++)
        {
            MyStat(files_list[i]);
        }
        wait(&wait_for_kid);   
        close(fd);
        close(sockp[1]);
    }
}

void ChooseCommand()
{
    char input_char[200];
    string input;
    string file_name;
    size_t pos;

    cout << "You have been granted acccess to these custom commands:\n\n"; cout << "myfind\n"; cout << "mystat\n";  cout << "quit\n"; 
    cout << "\nType <info> to find out what every command does.\n"; cout << "Or you can write a command to see how it works!\n\n"; 
    cout << "<myfind> <file_name> (e.g. myfind ex1.c) \n"; cout << "<mystat> <file_name> (e.g. mystat fork.exe) \n\n\n";
	
    while(true)
    {
        cin.getline(input_char,200);
	    //convert to string
	    input = string(input_char);
        pos = input.find(" ") +1;
        file_name = input.substr(pos);

        if(input == "info")
        {
             cout << "---MYFIND---\n";
             cout << "Allows finding a file and displaying information associated with that file. " <<
                     "The displayed information will contain: the creation date, date of change, file size, file access rights, etc.\n";
             cout << "SYNTAX: myfind file\n\n";
             cout << "---MYSTAT---\n";
             cout << "Allows you to view the attributes of a file. \n";
             cout << "SYNTAX: mystat file\n\n";
             cout << "---QUIT---\n";
             cout << "Log out and exit the program.\n\n";
        }
        else if(input.substr(0,6)=="myfind")
        {            
            MyFind(file_name);
        }
        else if(input.substr(0,6) == "mystat")
        {            
            MyStat(file_name);

        }
	    else if(input == "clear")
	    {
            system("clear");
	    }
        else if(input == "quit")
        {
            cout << "You exited the program. Write .login.exe to start the program again. \n";
		    break;
        }
    }
}

bool SaveToUserList(string username)
{
    system("clear");
    ifstream f;
    string username_fromlist;
    bool found = false;
    f.open("userlist.txt");

    while(getline(f,username_fromlist))
    {
        if(username_fromlist == username)
        {
            cout << "\nWelcome back, " << username << " !\n\n";
            return 1;
        }
    }

    cout << "\nUsername is not in user list.\n\n";
    cout << "Do you want to create a new user? ('y' or 'n')";

    string option;
    while(option!="y" && option!="n")
    { 
         cin >> option;
    }     
    if(option == "y")
    {
        // write username in userlist.txt
        cout << "Use this username to login: \n" << username << endl;
        ofstream g;
        g.open("userlist.txt", fstream::app);   // app -> no overwrite
        g << username << '\n';
        g.close();
    }    
    return 0;
    f.close();
}

string GetUsername()
{
    cout << "Hello! In order to continue, please type 'login' and your username.\n";
    string login, username;

    cin >> login >> username;
    while(true)
    {
        if(login=="login")
            break;
        else
        {
            cout << "Please type <login> <username>\n";
            cin >> login >> username;
        }
    }
    return username;
}

int main()
{  
    pid_t pid, p[2];
    int wait_for_kid;
    int sockp[2];
    string username;
    bool hasAccount = false, hasAccount2=false;     // one used in parent, the other one in child process
    char username_char[200], username_char2[200];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
    { 
        perror(" [error] socketpair"); 
        exit(1); 
    }
    if( pipe(p) == -1)
    {
        perror("error: pipe login");
        exit(1);
    }
    pid = fork();
    if(pid == -1)
    {
        perror("error: fork login");
        exit(2);
    }
    if(pid==0)
    {
        // child process - socket0
        close(sockp[1]);
        close(p[0]); //close read
        if (read(sockp[0], username_char2, 200) < 0) perror("[copil]Err..read"); 
            cout << username_char2 << endl;
        close(sockp[0]);

        string username2 = string(username_char2);
        hasAccount = SaveToUserList(username2);
        write(p[1],&hasAccount,sizeof(hasAccount));

        close(p[1]);
        wait_for_kid = 3;
        exit(wait_for_kid);
    }
    else if(pid)
    {
        // parent process -socket1
        close(sockp[0]);
        close(p[1]);
        username = GetUsername();
        strcpy(username_char,username.c_str());

        write(sockp[1],username_char, sizeof(username_char));
        wait(&wait_for_kid);   

        read(p[0], &hasAccount, sizeof(hasAccount));
        if(hasAccount==1)
        {
            ChooseCommand();
		    exit(0);
        }
        else
        {
            cout << "You exited the program.\n";
            exit(0);
        }
        close(sockp[1]);
        close(p[0]);            // close write chanel
    }
    return 0;
}