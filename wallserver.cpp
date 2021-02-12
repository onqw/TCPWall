#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <sstream>

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int option = 1;
     int queueSize = 20;
     int n;
     vector<string> wall;
     if (argc == 2) {
	  queueSize = atoi(argv[1]);
	  portno = 5514;
     } else if (argc == 3) {
	  queueSize = atoi(argv[1]);
	  portno = atoi(argv[2]);
     } else if (argc < 2) {
	 portno = 5514;
     }
     cout << "Wall server running on port " << portno << " with queue size " << queueSize << "." << endl;
     
     sockfd =  socket(AF_INET, SOCK_STREAM, 0);
     setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
     if (sockfd < 0) error("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;  
     serv_addr.sin_addr.s_addr = INADDR_ANY;  
     serv_addr.sin_port = htons(portno);

     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

     listen(sockfd,5);

     clilen = sizeof(cli_addr);

	while(true) {
	     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	     if (newsockfd < 0)  error("ERROR on accept");

	while(true) { 
     	bzero(buffer,256);
    	     send(newsockfd, "Wall Contents\n", 15, 0);
    	     send(newsockfd, "-------------\n", 15, 0);

	     if (!wall.empty()){
		     for(int i = 0; i < wall.size(); i++) {
			     string str = wall[i];
			     int strlen = str.length();
			     char *msg = &str[0]; 
			     send(newsockfd, msg, strlen, 0);
		     }
		     send(newsockfd, "\n", 2, 0);
	     } else {
		     send(newsockfd, "[NO MESSAGES - WALL EMPTY]\n", 28, 0);
		     send(newsockfd, "\n", 2, 0);
	     }
	     
	     send(newsockfd, "Enter command: ", 15, 0);

	     n = read(newsockfd,buffer,255);
	     string menuoption(buffer);
	     menuoption.pop_back();

	     char name[80];
	     char postmsg[80];
	     string quit = "quit";
	     string post = "post";
	     string clear = "clear";
	     string kill = "kill";
	     ostringstream oss;
	     ostringstream ossLen;
	     string postmsgstr = "";
	     string postname = "";
	     int maxLen = 80;

	     if (strcmp(menuoption.c_str(), post.c_str()) == 0) {
		     menuoption = "";
	     	     send(newsockfd, "Enter name: ", 12, 0);

	     	     n = read(newsockfd,name,80);
		     postname.assign(name, n);
		     postname.pop_back();
		     maxLen -= n + 1;

		     ossLen << "Post [Max length " << maxLen << "]: ";  
		     string lengthPost = ossLen.str();
		     int postLen = lengthPost.length();
		     const char *postDesc = lengthPost.c_str();

	     	     send(newsockfd, postDesc, postLen, 0);

	     	     n = read(newsockfd,postmsg,80);
		     postmsgstr.assign(postmsg, n);
		     oss << postname << ": " << postmsgstr; 
		     string finalmsg = oss.str();

		     if (n > maxLen + 1) {
			     send(newsockfd, "Error: message is too long!\n", 31, 0);
			     send(newsockfd, "\n", 2, 0);
		     } else {
	     	     	     wall.push_back(finalmsg);
			     send(newsockfd, "Successfully tagged the wall.\n", 31, 0);
			     send(newsockfd, "\n", 2, 0);
		     }

		     if (wall.size() > queueSize) {
			     wall.erase(wall.begin());
		     }
	     } else if (strcmp(menuoption.c_str(), clear.c_str()) == 0) {
	  	     send(newsockfd, "Wall cleared.\n", 15, 0);
		     send(newsockfd, "\n", 2, 0);
		     menuoption = "";
		     wall.clear();

	     } else if (strcmp(menuoption.c_str(), quit.c_str()) == 0) {
	  	     send(newsockfd, "Come back soon. Bye!", 20, 0);
		     menuoption = "";
		     shutdown(newsockfd, SHUT_RDWR);
		     if (close(newsockfd) < 0) {
		     }
		     break;
	     } else if (strcmp(menuoption.c_str(), kill.c_str()) == 0) {
	  	     send(newsockfd, "Closing socket and terminating server. Bye!", 43, 0);
		     menuoption = "";
		     shutdown(newsockfd, SHUT_RDWR);
		     close(newsockfd);
		     shutdown(sockfd, SHUT_RDWR);
		     close(sockfd);
		     return -1;
	     }

	     if (n < 0) error("ERROR reading from socket");
	     
	     menuoption = "";
	}
	}
     return 0; 
}
