#include <iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include <unistd.h>
#include<cerrno>
#include<cstring>
#include<vector>
#include<csignal>


using namespace std;

const int FALLBACK_PORT_NUMBER = 6969;
const int MAX_QUEUE_SIZE = 10;

volatile sig_atomic_t SIGINT_SIGNAL = 0;

struct connectedClientsInfo{
    int fileDescriptor;
    sockaddr_in socketAddress;
    // socklen_t socketLength;
};
void signalHandler(int)
{
    SIGINT_SIGNAL = 1;
    return;
}
int createSocket(){
    // creating a scoket returns a file descriptor for the socket
    int fileDescriptor = socket(AF_INET,SOCK_STREAM,0);
    return fileDescriptor;
}

int bindSocket(int fileDescriptor,struct sockaddr* pointer,socklen_t socketLength){
    return bind(fileDescriptor,pointer,socketLength);
}

int main(int argc,char *argv[])
{
    struct sigaction sa{};
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);


    std::cout << "Redis from scratch\n";

    int fileDescriptor = createSocket();
    if(fileDescriptor == -1){
        int savedError = errno;
        cerr<<savedError<<endl;
        cout<<"Error creating TCP socket....................."<<endl;
        return 0;
    }
    // bind the socket onto a port for listening
    // use type sockaddr_in for IPv4 netwrok type
    sockaddr_in socketBindVars;
    socketBindVars.sin_family = AF_INET;

    int port_number = FALLBACK_PORT_NUMBER;
    if(argc > 1)
    {
        try{
            int portNumber = stoi(argv[1]);
            if(portNumber > 0 && portNumber <= 65535)
                port_number = portNumber;
            else    
                throw runtime_error("error");
        }
        catch(exception &e)
        {
            cout<<"running on desired port number failed, falling back to "<<port_number<<endl;
        }
    }

    // htons converts the port number to network byte order(big-endian)
    socketBindVars.sin_port = htons(port_number); 
    // INADDR_ANY  basically represents 0.0.0.0 ie listen from any netwrok interfaces
    socketBindVars.sin_addr.s_addr = htonl(INADDR_ANY);

    // socket length in bytes
    socklen_t socketLength = sizeof(socketBindVars);
    
    // type caste to (struct sockaddr*) so that its accepted by the bind function.
    int bindStatus = bindSocket(fileDescriptor,(struct sockaddr*)&socketBindVars,socketLength);
    if(bindStatus == -1){
        cout<<"Error binding the socket................"<<endl;
        cout<<"error message : "<<strerror(errno)<<endl;
        errno = 0;
        return 0;
    }
    else
        cout<<"Socket bound to port: "<<port_number<<endl;
    // now listen for connections
    int listeningStatus = listen(fileDescriptor,MAX_QUEUE_SIZE);
    if(listeningStatus == -1)
    {
        cout<<"Listening failed............error message : "<<strerror(errno)<<endl;
        errno = 0;
        return 0;
    }
    else
        cout<<"Listening on socket created and bound to port........"<<endl;
    // listen for a Ctrl+C and exit the server gracefully.
    // signal(SIGINT,(sighandler_t)signalHandler); // typecast to 'sighandler_t' type

    // accept connections on a while loop
    // initialise a vector of struct connectedClientInfo
    vector<connectedClientsInfo> clientsInfo;
    while (true){
        // pass in empty addr and socklen for the OS to fill them in
        sockaddr_in peerSocket;
        socklen_t peerSocketLength = sizeof(peerSocket);
        int newSocket = accept(fileDescriptor,(struct sockaddr *)&peerSocket,&peerSocketLength);
        bool skipClient = false;
        if(SIGINT_SIGNAL == 1)
            break;
        if(newSocket == -1)
        {
            if(errno == EINTR)
            {
                if(SIGINT_SIGNAL)
                    break;
                else
                    continue;
            }
            cout<<"Error accepting connection from client................."<<endl;
            cout<<"error is : "<<strerror(errno)<<endl;
        }
        else{
            cout<<"Accepted incoming connection........."<<endl;
            connectedClientsInfo currentClient;
            currentClient.fileDescriptor = newSocket;
            currentClient.socketAddress = peerSocket;
            // currentClient.socketLength = peerSocketLength;
            clientsInfo.push_back(currentClient);

            //read the contents from the connected client................................................................................
            // init a byte buffer
            char readBuffer[1024];
            int bytesExpected = 200;
            int bytesRemaining = bytesExpected;
            int totalBytesRead = 0;
            while(totalBytesRead < bytesExpected)
            {
                int bytesRead = read(newSocket,&readBuffer[totalBytesRead],bytesRemaining);
    
                if(bytesRead == -1)
                {
                    if(errno == EINTR)
                    {
                        if(SIGINT_SIGNAL)
                        {
                            skipClient = true;
                            break;
                        }
                        else
                            continue;
                    }
                    cout<<"Error reading bytes from connection........."<<strerror(errno)<<endl;
                    errno = 0;
                    skipClient = true;
                    // remove client connection
                    for(auto it=clientsInfo.begin();it<clientsInfo.end();it++)
                    {
                        int fd = (*it).fileDescriptor;
                        if(fd == newSocket)
                        {
                            clientsInfo.erase(it);
                            break;
                        }
                    }
                    // close the client connection
                    int status = close(newSocket);
                    if(status == 0)cout<<"close client connection successfully.........."<<endl;
                    else{
                        cout<<"error closing client connection : "<<strerror(errno)<<endl;
                        errno = 0;
                    }
                    break;
                }
                else if(bytesRead == 0){
                    
                    cout<<"EOF or no data to read currently,client has closed connection........."<<endl;
                    // close connection and remove the client from clientsInfo
                    for (auto p=clientsInfo.begin();p<clientsInfo.end();p++){
                        if((*p).fileDescriptor == newSocket)
                        {
                            clientsInfo.erase(p);
                            cout<<"removed the client from tracking since connection closed........"<<endl;
                            break;
                        }
                    }
                    //close connection
                    int status = close(newSocket);
                    if(status == 0)
                        cout<<"closed connection with client successfully.........."<<endl;
                    else
                        cout<<"error closing connection with client.........."<<endl;
                    skipClient = true;
                    break;
                }
                else
                    cout<<"Read "<<bytesRead<<" bytes from buffer"<<endl;
                for(int i = totalBytesRead;i<totalBytesRead+bytesRead;i++)
                {
                    std::cout<<readBuffer[i];
                }
                totalBytesRead += bytesRead;
                bytesRemaining = bytesExpected - totalBytesRead;
            }

            // process bytes which are read, some logic can come later................

            // write from buffer to client .........................................................................
            if(skipClient != true)
            {
                int totalBytesToWrite = totalBytesRead;
                int totalBytesWritten = 0;
                while(totalBytesWritten < totalBytesToWrite)
                {
                    int bytesRemainingToWrite = totalBytesToWrite - totalBytesWritten;
                    int bytesWritten = write(newSocket,&readBuffer[totalBytesWritten],bytesRemainingToWrite);
                    if(bytesWritten == -1)
                    {
                        if(errno == EINTR)
                        {
                            if(SIGINT_SIGNAL)
                            {
                                skipClient= true;
                                break;
                            }
                            else
                                continue;
                        }
                        cout<<"Error writing bytes from connection........."<<strerror(errno)<<endl;
                        errno = 0;
                        skipClient = true;
                        // remove client connection
                        for(auto it=clientsInfo.begin();it<clientsInfo.end();it++)
                        {
                            int fd = (*it).fileDescriptor;
                            if(fd == newSocket)
                            {
                                clientsInfo.erase(it);
                                break;
                            }
                        }
                        // close the client connection
                        int status = close(newSocket);
                        if(status == 0)cout<<"close client connection successfully.........."<<endl;
                        else{
                            int error = errno;
                            cout<<"error closing client connection : "<<strerror(errno)<<endl;
                            errno = 0;
                        }
                        break;
                    }
                    else if(bytesWritten == 0)
                    {
                        cout<<"Write returned 0 bytes........."<<endl;
                        break;
                    }
                    cout<<"Wrote "<<bytesWritten<<" bytes"<<endl;
                    for(int i = totalBytesWritten;i<totalBytesWritten+bytesWritten;i++)
                    {
                        std::cout<<readBuffer[i];
                    }
                    totalBytesWritten += bytesWritten;
                }
            }
        }
    }
    if(SIGINT_SIGNAL){
        // clear all the client connections
        for(auto it=clientsInfo.begin();it<clientsInfo.end();it++)
        {
            int clientFileDescriptor = (*it).fileDescriptor;
            int status = close(clientFileDescriptor);
            if(status == 0)cout<<"closed client connection ,fd: "<<clientFileDescriptor<<endl;
            else cout<<"failed to close connection with client fd: "<<clientFileDescriptor<<endl;
        }
        int status = close(fileDescriptor);
        if(status == 0)cout<<"closed listener socket"<<endl;
        else cout<<"error closing listener socket..........."<<endl;
    }
    return 0;
}