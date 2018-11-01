# Computer Networks Project 

### Custom Commands implemented:
- "**login: username**" - whose existence is validated by using a configuration file (_"userfile.txt"_)
  + the possibility to create a new account
- "**myfind file**" - a command that allows finding a file and displaying information associated with that file; the displayed information will contain the creation date, date of change, file size, file access rights, etc.
- "**mystat file**" - a command that allows you to view the attributes of a file
- "**quit**"

### Other project specifications:
- communication among processes is done using the following communication mechanisms: **pipes, fifos, and socketpairs**
- communication is done by executing commands read from the keyboard in the parent process and executed in child processes
- the result obtained from the execution of any command will be displayed on screen by the parent process
- **no function in the "exec()" family will be used to implement "myfind" or "mystat" commands** in order to execute some system commands that offer the same functionalities.

In order to compile the project:
```
g++ login.cpp -o login.exe
./login.exe
```


![alt text](https://image.ibb.co/inEcTL/start.jpg)

![alt text](https://image.ibb.co/b9OFF0/myfind.jpg)
