#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <string>

using namespace std;

const int SERVER_PORT = 6969;
const int EXPECTED_BYTES = 200;
const char* SERVER_IP = "127.0.0.1";

bool connectToServer(int& socketFd)
{
    socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if(socketFd == -1)
    {
        cerr << "Failed to create socket: "
             << strerror(errno) << endl;
        return false;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);

    if(inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr) <= 0)
    {
        cerr << "Invalid server address" << endl;
        close(socketFd);
        return false;
    }

    if(connect(
        socketFd,
        (struct sockaddr*)&serverAddress,
        sizeof(serverAddress)
    ) == -1)
    {
        cerr << "Failed to connect: "
             << strerror(errno) << endl;
        close(socketFd);
        return false;
    }

    return true;
}


bool sendAll(int socketFd, const vector<char>& data)
{
    int totalSent = 0;
    int totalToSend = data.size();
    std::cout << "bytes sent by the client are: " << std::endl;
    while(totalSent < totalToSend)
    {
        int remaining = totalToSend - totalSent;

        int bytesSent = send(
            socketFd,
            &data[totalSent],
            remaining,
            0
        );

        if(bytesSent == -1)
        {
            cerr << "send() failed: "
                 << strerror(errno) << endl;
            return false;
        }

        if(bytesSent == 0)
        {
            cerr << "send() returned 0" << endl;
            return false;
        }
        for(int i=totalSent;i<totalSent+bytesSent;i++)
        {
            std::cout<<data[i];
        }
        totalSent += bytesSent;
    }
    std::cout << std::endl;

    return true;
}


bool receiveAll(int socketFd, vector<char>& data, int expectedBytes)
{
    data.resize(expectedBytes);

    int totalReceived = 0;
    std::cout << "bytes written to client by server write() bytes: " << std::endl;
    while(totalReceived < expectedBytes)
    {
        int remaining = expectedBytes - totalReceived;

        int bytesReceived = recv(
            socketFd,
            &data[totalReceived],
            remaining,
            0
        );

        if(bytesReceived == -1)
        {
            cerr << "recv() failed: "
                 << strerror(errno) << endl;
            return false;
        }

        if(bytesReceived == 0)
        {
            cerr << "Server closed connection before sending "
                 << expectedBytes << " bytes" << endl;
            return false;
        }
        for(int i = totalReceived;i<totalReceived+bytesReceived;i++)
        {
            std::cout<<data[i];
        }

        totalReceived += bytesReceived;
    }
    std::cout << std::endl;

    return true;
}


bool verifyEcho(
    const vector<char>& sent,
    const vector<char>& received
)
{
    if(sent.size() != received.size())
        return false;

    return sent == received;
}


vector<char> createTestData(char value)
{
    return vector<char>(EXPECTED_BYTES, value);
}

vector<char> createTestDataFromString(const string& value)
{
    vector<char> data;
    data.reserve(EXPECTED_BYTES);

    while(static_cast<int>(data.size()) < EXPECTED_BYTES)
    {
        for(char currentChar : value)
        {
            if(static_cast<int>(data.size()) == EXPECTED_BYTES)
                break;

            data.push_back(currentChar);
        }
    }

    return data;
}

void printClientChunk(const vector<char>& data, int offset, int bytesSent)
{
    std::cout << "bytes sent by the client chunk are: " << std::endl;

    for(int i = offset; i < offset + bytesSent; i++)
    {
        std::cout << data[i];
    }

    std::cout << std::endl;
}

bool testBasicExchange()
{
    int socketFd;

    if(!connectToServer(socketFd))
        return false;

    vector<char> sentData = createTestDataFromString(
        "A38yr9feigbosidbgsoelh93u84ysihgnldkngsldbgirohsdioghbs,/[;]lsanjayyyyyyy"
    );

    if(!sendAll(socketFd, sentData))
    {
        close(socketFd);
        return false;
    }

    vector<char> receivedData;

    if(!receiveAll(socketFd, receivedData, EXPECTED_BYTES))
    {
        close(socketFd);
        return false;
    }

    close(socketFd);

    return verifyEcho(sentData, receivedData);
}


bool testPartialRead()
{
    int socketFd;

    if(!connectToServer(socketFd))
        return false;

    vector<char> data = createTestDataFromString(
        "A38yr9feigbosidbgsoelh93u84ysihgnldkngsldbgirohsdioghbs"
    );

    int firstChunk = 50;
    int secondChunk = 75;
    int thirdChunk = 75;

    int offset = 0;

    int bytesSent = send(
        socketFd,
        &data[offset],
        firstChunk,
        0
    );

    if(bytesSent != firstChunk)
    {
        close(socketFd);
        return false;
    }
    printClientChunk(data, offset, bytesSent);

    offset += firstChunk;

    usleep(100000);

    bytesSent = send(
        socketFd,
        &data[offset],
        secondChunk,
        0
    );

    if(bytesSent != secondChunk)
    {
        close(socketFd);
        return false;
    }
    printClientChunk(data, offset, bytesSent);

    offset += secondChunk;

    usleep(100000);

    bytesSent = send(
        socketFd,
        &data[offset],
        thirdChunk,
        0
    );

    if(bytesSent != thirdChunk)
    {
        close(socketFd);
        return false;
    }
    printClientChunk(data, offset, bytesSent);

    vector<char> receivedData;

    if(!receiveAll(socketFd, receivedData, EXPECTED_BYTES))
    {
        close(socketFd);
        return false;
    }

    close(socketFd);

    return verifyEcho(data, receivedData);
}


bool testUnexpectedDisconnect()
{
    int socketFd;

    if(!connectToServer(socketFd))
        return false;

    vector<char> partialData(50, 'C');

    int bytesSent = send(
        socketFd,
        partialData.data(),
        partialData.size(),
        0
    );

    if(bytesSent != static_cast<int>(partialData.size()))
    {
        close(socketFd);
        return false;
    }

    // Close before sending the expected 200 bytes.
    close(socketFd);

    // The important thing is that the server survives
    // and returns to accept().

    return true;
}

bool testMultipleClients()
{
    const int numberOfClients = 3;

    for(int i = 0; i < numberOfClients; i++)
    {
        int socketFd;

        if(!connectToServer(socketFd))
            return false;

        char testCharacter = 'D' + i;

        vector<char> sentData =
            createTestData(testCharacter);

        if(!sendAll(socketFd, sentData))
        {
            close(socketFd);
            return false;
        }

        vector<char> receivedData;

        if(!receiveAll(
            socketFd,
            receivedData,
            EXPECTED_BYTES
        ))
        {
            close(socketFd);
            return false;
        }

        close(socketFd);

        if(!verifyEcho(sentData, receivedData))
            return false;
    }

    return true;
}

bool testReconnect()
{
    // First connection

    int socketFd;

    if(!connectToServer(socketFd))
        return false;

    vector<char> firstData =
        createTestDataFromString(
            "A38yr9feigbosidbgsoelh93u84ysihgnldkngsldbgirohsdioghbs"
        );

    if(!sendAll(socketFd, firstData))
    {
        close(socketFd);
        return false;
    }

    vector<char> firstResponse;

    if(!receiveAll(
        socketFd,
        firstResponse,
        EXPECTED_BYTES
    ))
    {
        close(socketFd);
        return false;
    }

    close(socketFd);

    if(!verifyEcho(firstData, firstResponse))
        return false;


    // Second connection

    if(!connectToServer(socketFd))
        return false;

    vector<char> secondData =
        createTestDataFromString(
            "A38yr9feigbosidbgsoelh93u84ysihgnldkngsldbgirohsdioghbs"
        );

    if(!sendAll(socketFd, secondData))
    {
        close(socketFd);
        return false;
    }

    vector<char> secondResponse;

    if(!receiveAll(
        socketFd,
        secondResponse,
        EXPECTED_BYTES
    ))
    {
        close(socketFd);
        return false;
    }

    close(socketFd);

    return verifyEcho(secondData, secondResponse);
}
void runTest(
    const string& testName,
    bool (*testFunction)()
)
{
    std::cout << "Running: " << testName << " ... ";

    bool result = testFunction();

    if(result)
        std::cout << "PASS" << std::endl;
    else
        std::cout << "FAIL" << std::endl;
}


int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "       Redis Phase 1 TCP Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    runTest(
        "Basic TCP exchange",
        testBasicExchange
    );

    runTest(
        "Partial / fragmented read",
        testPartialRead
    );

    runTest(
        "Unexpected client disconnect",
        testUnexpectedDisconnect
    );

    runTest(
        "Multiple clients",
        testMultipleClients
    );

    runTest(
        "Disconnect and reconnect",
        testReconnect
    );

    std::cout << "========================================" << std::endl;

    return 0;
}
