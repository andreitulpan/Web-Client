#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

/* Functia reprezinta executarea unei cereri de register */
void register_cmd() {
    char *username = malloc(MAX * sizeof(char));
    char *password = malloc(MAX * sizeof(char));
    printf("username=");
    get_input(&username);
    printf("password=");
    get_input(&password);

    int sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    serialized_string = json_serialize_to_string_pretty(root_value);

    char *message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register", "application/json", &serialized_string, 1, NULL, 1, NULL);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    int code = check_response(response);
    if (code == 201) {
        printf("Contul a fost creat cu succes!\n");
    } else if (code == 400) {
        printf("Username-ul %s este deja utilizat!\n", username);
    } else {
        printf("Eroare la crearea user-ului!\n");
    }

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);

    close(sockfd);
    free(message);
    free(response);
    free(username);
    free(password);
}

/* Functia reprezinta executarea unei cereri de login */
void login_cmd(char **cookies) {
    char *username = malloc(MAX * sizeof(char));
    char *password = malloc(MAX * sizeof(char));
    printf("username=");
    get_input(&username);
    printf("password=");
    get_input(&password);

    int sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    serialized_string = json_serialize_to_string_pretty(root_value);

    char *message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login", "application/json", &serialized_string, 1, NULL, 1, NULL);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);

    int code = check_response(response);
    if (code == 200) {
        *cookies = get_cookie(response);
        printf("Te-ai logat cu succes!\n");
    } else if (code == 400) {
        printf("Username sau parola gresita\n");
    } else {
        printf("Eroare la logare!\n");
    }

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);

    close(sockfd);
    free(message);
    free(response);
    free(username);
    free(password);
}

/* Functia reprezinta executarea unei cereri de acces in librarie */
void enter_library(char *cookies, char **token) {
    int sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
    char *message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", NULL, &cookies, 1, NULL);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    *token = get_token(response);
    close(sockfd);
    free(message);
    free(response);
}

/* Functia reprezinta executarea unei cereri de afisare a tuturor cartilor */
void get_books(char *cookies, char *token) {
    int sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
    char *message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", NULL, NULL, 1, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    int code = check_response(response);
    if (code != 403) {
        char *booksList = compute_books(response);
        JSON_Value *json_value = json_parse_string(booksList);
        free(booksList);
        booksList = json_serialize_to_string_pretty(json_value);
        json_value_free(json_value);
        printf("%s\n", booksList);
        free(booksList);
    } else {
        printf("Nu aveti acces la biblioteca!\n");
    }
    close(sockfd);
    free(message);
    free(response);
}

/* Functia reprezinta executarea unei cereri de afisare a unei carti specifice */
void get_book(char *cookies, char *token) {
    char *id = malloc(MAX * sizeof(char));
    printf("id=");
    get_input(&id);

    char *address = malloc(MAX * sizeof(char));
    strcpy(address, "/api/v1/tema/library/books/");
    strcat(address, id);

    int sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
    char *message = compute_get_request("34.241.4.235", address, NULL, NULL, 1, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    int code = check_response(response);
    if (code == 200) {
        char *bookInfo = compute_books(response);
        JSON_Value *json_value = json_parse_string(bookInfo);
        free(bookInfo);
        bookInfo = json_serialize_to_string_pretty(json_value);
        printf("%s\n", bookInfo);
        free(bookInfo);
        json_value_free(json_value);
    } else if (code == 404) {
        printf("Cartea cu id-ul %s nu exista!\n", id);
    } else if (code == 403) {
        printf("Nu aveti acces la biblioteca!\n");
    } else {
        printf("A intervenit o eroare!\n");
    }
    close(sockfd);

    free(message);
    free(response);
    free(address);
    free(id);
}

/* Functia reprezinta executarea unei cereri de adaugare a unei carti */
void add_book(char *cookies, char *token) {
    char **in = (char **)malloc(5 * sizeof(char *));
    for (int i = 0; i < 5; i++) {
        in[i] = (char *)malloc(MAX * sizeof(char));
    }

    printf("title=");
    get_input_book(&in[0]);
    printf("author=");
    get_input_book(&in[1]);
    printf("genre=");
    get_input_book(&in[2]);
    printf("publisher=");
    get_input_book(&in[3]);
    printf("page_count=");
    get_input_book(&in[4]);

    int sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "title", in[0]);
    json_object_set_string(root_object, "author", in[1]);
    json_object_set_string(root_object, "genre", in[2]);
    json_object_set_string(root_object, "publisher", in[3]);
    json_object_set_number(root_object, "page_count", atoi(in[4]));
    serialized_string = json_serialize_to_string_pretty(root_value);
    char *message = compute_post_request("34.241.4.235", "/api/v1/tema/library/books", "application/json", &serialized_string, 1, NULL, 1, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    int code = check_response(response);
    if (code == 200)
        printf("Cartea %s s-a adaugat cu succes!\n", in[0]);
    else if (code  == 403)
        printf("Nu aveti acces la biblioteca!\n");
    else
        printf("A intervenit o eroare in adaugarea cartii!\n");
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    close(sockfd);

    for (int i = 0; i < 5; i++) {
        free(in[i]);
    }
    free(in);
    free(message);
    free(response);
}

/* Functia reprezinta executarea unei cereri de steregere a unei carti */
void delete_book(char *cookies, char *token) {
    char *id = malloc(MAX * sizeof(char));
    printf("id=");
    get_input(&id);

    char *address = malloc(MAX * sizeof(char));
    strcpy(address, "/api/v1/tema/library/books/");
    strcat(address, id);

    int sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);
    char *message = compute_delete_request("34.241.4.235", address, NULL, NULL, 1, token);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    int code = check_response(response);
    if (code == 200)
        printf("Cartea cu id-ul %s s-a sters cu succes!\n", id);
    else if(code == 403)
        printf("Nu aveti acces la biblioteca!\n");
    else
        printf("Cartea nu exista sau a intervenit o eroare!\n");
    close(sockfd);

    free(message);
    free(response);
    free(address);
    free(id);
}

/* Functia reprezinta executarea unei cereri de delogare*/
void logout_cmd(char **cookies, char **token) {
    if (cookies != NULL) {
        printf("Te-ai delogat cu succes!\n");
        free(*cookies);
        *cookies = NULL;
        if (*token != NULL) {
            free(*token);
            *token = NULL;
        }
    } else {
        printf("Nu esti logat!\n");
    }
}

int main() {
    char *cookies = NULL;   // Cookie-ul primit la login
    char *token = NULL;     // Token-ul de acces pentru biblioteca

    int running = 1;
    while (running) {
        char *command = malloc(MAX * sizeof(char));
        fgets(command, MAX, stdin);
        command = strtok(command, " \n");
        if (!strcmp(command, "register")) {
            if (cookies == NULL) {
                register_cmd();
            } else {
                printf("Pentru a crea un cont nou trebuie sa fii delogat!\n");
            }
        } else if (!strcmp(command, "login")) {
            if (cookies == NULL) {
                login_cmd(&cookies);
            } else {
                printf("Esti deja logat!\n");
            }
        } else if (!strcmp(command, "enter_library")) {
            if (cookies != NULL) {
                enter_library(cookies, &token);
                printf("Acces verificat in librarie!\n");
            } else {
                printf("Pentru a accesa biblioteca trebuie sa fii logat!\n");
            }
        } else if (!strcmp(command, "get_books")) {
            if (cookies != NULL)
                get_books(cookies, token);
            else
                printf("Nu sunteti logat!\n");
        } else if (!strcmp(command, "get_book")) {
            if (cookies != NULL)
                get_book(cookies, token);
            else
                printf("Nu sunteti logat!\n");
        } else if (!strcmp(command, "add_book")) {
            if (cookies != NULL)
                add_book(cookies, token);
            else
                printf("Nu sunteti logat!\n");
        } else if (!strcmp(command, "delete_book")) {
            if (cookies != NULL)
                delete_book(cookies, token);
            else
                printf("Nu sunteti logat!\n");
        } else if (!strcmp(command, "logout")) {
            if (cookies != NULL)
                logout_cmd(&cookies, &token);
            else
                printf("Pentru a te deloga trebuie sa fii mai intai logat!\n");
        } else if (!strcmp(command, "exit")) {
            printf("Se inchide clientul...\n");
            running = 0;
        } else {
            printf("Comanda nu exista!\n");
        }
        free(command);
    }

    if (cookies != NULL)
        free(cookies);
    if (token != NULL)
        free(token);

    return 0;
}
