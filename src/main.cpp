#include <iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include <unistd.h>
#include<cerrno>
#include<cstring>
#include<vector>

using namespace std;

const int PORT_NUMBER = 8080;
const int MAX_QUEUE_SIZE = 10;

struct connectedClientsInfo{
    int fileDescriptor;
    sockaddr_in socketAddress;
    // socklen_t socketLength;
};

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
    }
    else
        cout<<"Socket bound to port: "<<PORT_NUMBER<<endl;
    // now listen for connections
    int listeningStatus = listen(fileDescriptor,MAX_QUEUE_SIZE);
    if(listeningStatus == -1)
    {
        cout<<"Listening failed............"<<endl;
        return 0;
    }
    else
        cout<<"Listening on socket created and bound to port........"<<endl;
    // accept connections on a while loop
    // initialise a vector of struct connectedClientInfo
    vector<connectedClientsInfo> clientsInfo;
    while (true){
        // pass in empty addr and socklen for the OS to fill them in
        sockaddr_in peerSocket;
        socklen_t peerSocketLength = sizeof(peerSocket);
        int newSocket = accept(fileDescriptor,(struct sockaddr *)&peerSocket,&peerSocketLength);
        bool skipClient = false;
        if(newSocket == -1)
            cout<<"Error accepting connection from client................."<<endl;
        else{
            cout<<"Accepted incoming connection........."<<endl;
            connectedClientsInfo currentClient;
            currentClient.fileDescriptor = newSocket;
            currentClient.socketAddress = peerSocket;
            // currentClient.socketLength = peerSocketLength;
            clientsInfo.push_back(currentClient);

            //read the contents from the connected client
            // init a byte buffer
            char readBuffer[1024];
            int maxByteReads = 200;
            int bytesRead = read(newSocket,readBuffer,maxByteReads);

            if(bytesRead == -1)
            {
                cout<<"Error reading bytes from connection........."<<endl;
                skipClient = true;
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
            }else
                cout<<"Read "<<bytesRead<<" bytes from buffer"<<endl;
            // process bytes, some logic can come later
            char writeBuffer[1024] = {'a'};
            int maxByteWrites = 200;
            if(skipClient == false){
                int bytesWritten = write(newSocket,writeBuffer,200);
                if(bytesWritten == -1)
                    cout<<"Error writing bytes from connection........."<<endl;
                else if(bytesWritten == 0)
                    cout<<"EOF or no data to write currently........."<<endl;
                else
                   cout<<"Wrote "<<bytesWritten<<" bytes from buffer"<<endl;
            }

        }
    }
    return 0;
}