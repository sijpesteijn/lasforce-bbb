/*
 * global.c
 *
 *  Created on: Apr 3, 2016
 *      Author: gijs
 */

#include "../include/global.h"
#include "../include/jansson/jansson.h"
#include <syslog.h>

void populateListInfo(json_t *list_array, QueueItem *queueItem) {
	if (queueItem != NULL) {
		Command *command = queueItem->command;
		if (command->action == play) {
			Animation *animation = (Animation*)command->value;
			json_array_append( list_array, json_string(animation->name));
		} else if (command->action == stop) {
			json_array_append( list_array, json_string("stop"));
		}
		populateListInfo(list_array, queueItem->next);
	}
}

char* getQueueListJson(Queue *queue) {
	json_t *root = json_object();
	json_t *json_arr = json_array();
	populateListInfo(json_arr, queue->current);
	json_object_set_new(root, "cmd", json_string("list"));
	json_object_set_new(root, "list", json_arr);
	char *json = json_dumps(root, 0);
	json_decref(root);
	return json;
}

