### Simple mini-shell for Linux
---
### Roadmap
Minimal functional (MVP):  
✅ Parse commands to args(strtok())  
✅ Create child process (fork()) and execute command (execvp())  
✅ Waiting for completion (waitpid())  

Additional functional:  
✅ Processing multiple commands in a line (ls; pwd; whoami)  
✅ Background processes support (&)  
✅ Signal processing (Ctrl+C, Ctrl+Z)

Advanced level:  
🔹 Pipe support ( | )  
🔹 I/O redirect (>, <, >>)  
Own commands:  
🔹 cd  
✅ exit  
🔹 history  

---
The project was created with the aim of understanding Linux programming    
