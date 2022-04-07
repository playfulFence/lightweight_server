# Lightweight server in C

## BUT FIT, Kirill Mikhailov (xmikha00)

---

### 1. Description

> The task is to implement a server in C/C++ language communicating via HTTP protocol, which will be able to provide various informaiton about the system. The server will listen on the specified port and return the required information according to *URL*
> The server will properly handle HTTP headers and generate correct HTTP responses. The answer type will be text/plain

### 2. How to execute the project

> Compile it by `make` command in your directory and then run the fillowing way :
> `./hinfosvc (port_number)`

### 3. Functionality of server

- **Obtaining of domain name**
   > Upon receiving the ***hostname*** request, server returns the domain name
   Example :`curl http://servername:port/hostname`

- **Obtaining CPU information**
   > If the server recieves the ***cpu-name*** request, it will return the name of its CPU
   Example : `curl http://servername:port/cpu-name`

- **Actual server's CPU load**
   > If a ***load*** request was sent, the server will return information about the load of its CPU
   Example : `curl http://servername:port/load`

### 4. Error codes

- **1 - error with arguments**
  > Wrong format or number of arguments

- **2 - error in inner help functions**
  > Inner error in getCPUname/getHostname/getUsageCPU

- **3 - error with socket setting**
  > Inner error with setting sockets, *bind* or *listen* functions
  
### 5. Features

- **For macOS compile with *-DMAC_OS_VER* flag**
