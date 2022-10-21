#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000
#define MAX 64

void get_input(char **input);

void get_input_book(char **input);

int check_response(char *response);

char* get_cookie(char *response);

char *get_token(char *response);

char *compute_books(char *response);

void error(const char *msg);

void compute_message(char *message, const char *line);

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

void close_connection(int sockfd);

void send_to_server(int sockfd, char *message);

char *receive_from_server(int sockfd);

char *basic_extract_json_response(char *str);

#endif
