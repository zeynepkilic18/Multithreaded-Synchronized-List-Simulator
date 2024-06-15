/*multithread listtest file, test sycnrhonized list*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "simulator.h"

List *list;
#define NELEMENTS 5

pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

void printsurvivor(Survivor *s) {
    printf("info: %.25s\n", s->info);
    printf("Location: (%d, %d)\n", s->coord.x, s->coord.y);
}

void printsurvivor_withmsg(Survivor *s, char *msg) {
    char str[256];
    sprintf(str, "msg:%.100s, info: %.25s-location: (%d, %d)\n", 
            msg, s->info, s->coord.x, s->coord.y);
    printf("%.256s", str);
}

/*thread function that adds elements*/
void *add_elements(void *args) {
    printf("\n\nadding elements to the list\n");
    for (int i = 0; i < NELEMENTS; i++) {
        Survivor s;
        sprintf(s.info, "id:%d-aname", i);
        s.coord.x = rand() % 1000;
        s.coord.y = rand() % 100;

        pthread_mutex_lock(&list_mutex);
        Node *n = list->add(list, &s);
        pthread_mutex_unlock(&list_mutex);

        Survivor *sp = (Survivor *)n->data;
        printsurvivor_withmsg(&s, "added");
    }
    return NULL;
}

/*thread function that removes elements*/
void *remove_elements(void *args) {
    printf("\ntest by removing elements\n");
    Survivor s;

    for (int i = 0; i < NELEMENTS; i++) {
        pthread_mutex_lock(&list_mutex);
        if (list->pop(list, &s) != NULL) {
            pthread_mutex_unlock(&list_mutex);
            printsurvivor_withmsg(&s, "removed");
        } else {
            pthread_mutex_unlock(&list_mutex);
        }
    }
    return NULL;
}

int init_list() { 
    list = create_list(sizeof(Survivor), NELEMENTS); 
    return 0;
}

int init_threads(pthread_t *add_thread, pthread_t *remove_thread) {
    if (pthread_create(add_thread, NULL, add_elements, NULL) != 0) {
        perror("Failed to create add_thread");
        return -1;
    }
    if (pthread_create(remove_thread, NULL, remove_elements, NULL) != 0) {
        perror("Failed to create remove_thread");
        return -1;
    }
    return 0;
}

int destroy_threads(pthread_t add_thread, pthread_t remove_thread) {
    pthread_cancel(add_thread);
    pthread_cancel(remove_thread);
    pthread_join(add_thread, NULL);
    pthread_join(remove_thread, NULL);
    return 0;
}

int main() {
    pthread_t add_thread, remove_thread;

    init_list();
    if (init_threads(&add_thread, &remove_thread) != 0) {
        return -1;
    }

    for (int i = 0; i < NELEMENTS; i++) {
        pthread_mutex_lock(&list_mutex);
        list->printlist(list, (void (*)(void *))printsurvivor);
        pthread_mutex_unlock(&list_mutex);
        printf("from tail\n\n");
        pthread_mutex_lock(&list_mutex);
        list->printlistfromtail(list, (void (*)(void *))printsurvivor);
        pthread_mutex_unlock(&list_mutex);
        sleep(5); /*print list every x seconds*/
    }

    destroy_threads(add_thread, remove_thread);
    list->destroy(list);
    printf("Bye Bye\n");
    return 0;
}
