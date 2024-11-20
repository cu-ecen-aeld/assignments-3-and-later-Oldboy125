/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>





#define MYPORT "9000"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold


volatile sig_atomic_t flag = 0;
volatile int new_fd;
volatile int sockfd;


void get_ip(int ,char *);
void sigint_handler(int);
char* read_file_to_buffer(const char* filename);

int main(int argc, char *argv[]) {

	int status;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	struct addrinfo hints, *res;
	
	ssize_t valread;
	char *data = NULL;
    	size_t data_len = 0;
	char buf[30000] = { 0 };
	char ipstr[INET6_ADDRSTRLEN] = { 0 };
	const char *filename = "/var/tmp/aesdsocketdata";
	int newline_found = 0;
	int daemon_flag = 0;
	

	const char delimiters[] = "\n";
	FILE *fptr;
	
	
	// Check for the -d attribute
	for (int i = 1; i < argc; i++) 
	{
	if (strcmp(argv[i], "-d") == 0) 
		{
	    	daemon_flag = 1;
	    	break;
		}
	}
	

	openlog("socket program", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
	
	setlogmask(LOG_UPTO(LOG_NOTICE));
	
	//Define signal handler
	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigint_handler);
	
	

	//fptr = fopen("aesdsocketdata", "a");



	//memory reset a struct configuration
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	if ((status = getaddrinfo(NULL, MYPORT, &hints, &res)) != 0) 
	{
	    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	    syslog(LOG_ERR, "Exiting from the program: %s\n", gai_strerror(status));
	    exit(status);
	}

	// make a socket, bind it, and listen on it:

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	
	
	if ((status = bind(sockfd, res->ai_addr, res->ai_addrlen)) != 0) 
	{
	    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
	    syslog(LOG_ERR, "Exiting from the program: %s\n", gai_strerror(status));
	    exit(status);
	}
	
	
	if (daemon_flag)
	{
	    pid_t pid = fork();

	    if (pid < 0) {
		perror("fork failed");
		exit(EXIT_FAILURE);
	    }

	    if (pid > 0) {
		syslog(LOG_NOTICE, "Closing parent \n");
		// Close server
		// free the linked-list
		freeaddrinfo(res); 
		exit(EXIT_SUCCESS);
	    }

	    // Child process
	    if (setsid() < 0) {
		perror("setsid failed");
		exit(EXIT_FAILURE);
	    }

	    // Redirect standard files to /dev/null
	    freopen("/dev/null", "r", stdin);
	    freopen("/dev/null", "w", stdout);
	    freopen("/dev/null", "w", stderr);
	}
	
	
	listen(sockfd, BACKLOG);
	while (flag == 0)
	{
		printf("Waiting for connection\n");
		syslog(LOG_NOTICE, "Waiting for connection\n");
		// now accept an incoming connection:

		addr_size = sizeof their_addr;
		
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

		if (new_fd == -1) 
		{
		    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(new_fd));
		    syslog(LOG_ERR, "Exiting from the program: %s\n", gai_strerror(new_fd));
		    flag = 1;	
		}
		else
		{
		get_ip(new_fd,ipstr);
		
		printf("Accepted connection from %s\n", ipstr);
		syslog(LOG_NOTICE, "Accepted connection from %s\n", ipstr);

		//send(new_fd, "Communication opened\n", strlen("Communication opened\n"), 0);
		printf("Communication opened\n");
		//fprintf(fptr, "Communication opened\n");
		
		newline_found = 0;
		if (data != NULL) 
		{ 
			free(data); 
			data = NULL;
		} 
		
		data_len = 0;
		
		while (newline_found == 0)
		{
			valread = recv(new_fd, buf, sizeof(buf),0);
			
			printf("Received: %ld\n", valread);
			//buf[valread] = '\0';

			char *newline_ptr = strchr(buf, '\n');
			if (newline_ptr != NULL) 
			{
			    //*newline_ptr = '\0';
			    printf("Found new line: %s l\n", newline_ptr);
			    newline_found = 1;
			}
			
			size_t chunk_len = strlen(buf);
			char *new_data = realloc(data, data_len + chunk_len + 1);
			if (new_data == NULL) 
			{
			    perror("realloc failed");
			    free(data);
			}

			data = new_data;			
			strcpy(data + data_len, buf);
			data_len += chunk_len;
			newline_ptr = NULL;
			
				
			if (newline_found == 1)
				{
				//valread = read(new_fd, buf,10); // subtract 1 for the null // terminator at the end
				printf("Received: %s\n", data);
				char* token = strtok(data, delimiters);

				//printf("Write: %s\n", token);
				
				
				// Open a file in writing mode
				fptr = fopen(filename, "a");
				if (fptr == NULL)
				{
					fprintf(stderr,"Error %s\n", strerror(errno));
					syslog(LOG_ERR, "Error %s\n", strerror(errno));
					exit(status);
				}

				fprintf(fptr, "%s\n", token);
				fflush(fptr);
				//strcat(token, "\n");
				// Close the file
				fclose(fptr); 
				
				char *content = read_file_to_buffer(filename);

				if (content != NULL) 
				{
					printf("File Content:\n%s\n", content);
					
				} 
				else 
				{
					printf("Failed to read the file.\n");
				}
				send(new_fd, content, strlen(content), 0);
				//token = strtok(NULL, delimiters); 
				
				if (content != NULL) 
				{ 
					free(content); 
					content = NULL;
				} 
				
				if (token != NULL) 
				{ 
					token = NULL;
				} 
				}
			}
		}
		
		
	}
	
	printf("Closing\n");
	// Write some text to the file
	
	if (data != NULL) 
	{ 
		free(data); 
	} 
	remove("/var/tmp/aesdsocketdata");
	
	// Close connection
	close(new_fd); 
	syslog(LOG_NOTICE, "Closed connection from %s\n", ipstr);
	// Close server
	close(sockfd); 
	// free the linked-list
	freeaddrinfo(res); 
	// Close Log
	closelog();
  	return 0;
  	
}

void get_ip(int socket, char *result)
{
	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[INET6_ADDRSTRLEN];
	int port;

	len = sizeof addr;
	getpeername(socket, (struct sockaddr*)&addr, &len);

	// deal with both IPv4 and IPv6:
	if (addr.ss_family == AF_INET) {
	    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
	    port = ntohs(s->sin_port);
	    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
	} else { // AF_INET6
	    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
	    port = ntohs(s->sin6_port);
	    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
	}

	printf("Peer IP address: %s\n", ipstr);
	printf("Peer port      : %d\n", port);
	strcpy(result, ipstr);

}


void sigint_handler(int signum)
{
    (void)signum;                    // Unused variable warning
    signal(SIGINT, sigint_handler);  // Reset signal handler 
    flag = 1;                         // Undefined behavior
    printf("Caught signal, exiting\n");
    shutdown(new_fd,SHUT_RDWR);
    shutdown(sockfd,SHUT_RDWR);
    syslog(LOG_NOTICE, "Caught signal, exiting");  
}

// Function to read all content of a file into a buffer
char* read_file_to_buffer(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    // Seek to the end of the file to determine its size
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Failed to seek to the end of the file");
        fclose(file);
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size == -1L) {
        perror("Failed to get the file size");
        fclose(file);
        return NULL;
    }

    // Allocate buffer to hold the file content
    char *buffer = (char*)malloc((file_size + 1) * sizeof(char));
    if (buffer == NULL) {
        perror("Failed to allocate memory for file content");
        fclose(file);
        return NULL;
    }

    // Seek back to the beginning of the file
    if (fseek(file, 0, SEEK_SET) != 0) {
        perror("Failed to seek back to the beginning of the file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    // Read the file content into the buffer
    size_t read_size = fread(buffer, sizeof(char), file_size, file);
    if (read_size != file_size) {
        perror("Failed to read the file content");
        free(buffer);
        fclose(file);
        return NULL;
    }

    // Null-terminate the buffer
    buffer[file_size] = '\0';

    fclose(file);
    return buffer;
}


