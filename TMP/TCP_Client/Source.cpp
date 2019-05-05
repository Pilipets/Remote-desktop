#include<iostream>
#include<string>
#include<WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

int main()
{
	string ipAddress = "127.0.0.1";   //IP Address for the server
	int port = 54000;        //Listening port # on the server

	//Initialize WinSock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &wsData);
	if (wsResult != 0)
	{
		cerr << "Can't start Winsock, Err #" << wsResult << endl;
		return -1;
	}

	//Create a socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can't create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return -1;
	}

	//Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	//Connect to the server
	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can't connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	//Do-while loop to send and receive data
	char buf[4096];
	string userInput;

	do {
		//Prompt the user for some text
		cout << "> ";
		getline(cin, userInput);

		if (userInput.size() > 0) //Make sure the user has typed in something
		{
			//Send the text
			int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
			if (sendResult != SOCKET_ERROR)
			{
				//Wait for the response
				ZeroMemory(buf, 4096);
				int bytesReceived = recv(sock, buf, 4096, 0);
				if (bytesReceived > 0)
				{
					//Echo response to console
					cout << "SERVER> " << string(buf, 0, bytesReceived) << endl;
				}
			}
		}
	} while (userInput.size() > 0);

	//Gracefully close down everything
	closesocket(sock);
	WSACleanup();
	system("pause");
	return 0;
}