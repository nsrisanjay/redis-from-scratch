#include <iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<cerrno>
#include<cstring>

using namespace std;

const int PORT_NUMBER = 8080;
const int MAX_QUEUE_SIZE = 10;

int createSocket(){
    // creating a scoket returns a file descriptor for the socket
    int fileDescriptor = socket(AF_INET,SOCK_STREAM,0);
    return fileDescriptor;
}

int bindSocket(int fileDescriptor,struct sockaddr* pointer,socklen_t socketLength){
    return bind(fileDescriptor,pointer,socketLength);
}

int main()
{
    std::cout << "Redis from scratch\n";

    int fileDescriptor = createSocket();
    if(fileDescriptor == -1){
        int savedError = errno;
        cerr<<savedError<<endl;
        cout<<"Error creating TCP socket....................."<<endl;
        return 0;
    }
    // bind the socket onto a port for listening
    sockaddr_in socketBindVars;
    socketBindVars.sin_family = AF_INET;
    // htons converts the port number to network byte order(big-endian)
    socketBindVars.sin_port = htons(PORT_NUMBER); 
    socketBindVars.sin_addr.s_addr = htonl(INADDR_ANY);

    socklen_t socketLength = sizeof(socketBindVars);
    
    // type caste to (struct sockaddr*) so that its accepted by the bind function.
    int bindStatus = bindSocket(fileDescriptor,(struct sockaddr*)&socketBindVars,socketLength);
    if(bindStatus == -1){
        cout<<"Error binding the socket................"<<endl;
        return 0;
    }else{
        cout<<"Socket bound to port: "<<PORT_NUMBER<<endl;
    }
    // now listen for connections
    int listeningStatus = listen(fileDescriptor,MAX_QUEUE_SIZE);
    if(listeningStatus == -1)
    {
        cout<<"Listening failed............"<<endl;
    }else{
        cout<<"Listening on socket created and bound to port........"<<endl;
        return 0;
    }
    // accept connections on a while loop
    while (true){
        // pass in empty addr and socklen for the OS to fill them in

        sockaddr_in peerSocket;
        socklen_t peerSocketLength = sizeof(peerSocket);
        int newSocket = accept(fileDescriptor,(struct sockaddr *)&peerSocket,&peerSocketLength);

        if(newSocket == -1)
        {
            cout<<"Error accepting connection from client................."<<endl;
        }else{
            cout<<"Accepted incoming connection........."<<endl;
        }
    }
    return 0;
}