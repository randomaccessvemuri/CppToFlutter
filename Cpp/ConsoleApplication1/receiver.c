#include <czmq.h>
#include <iostream>

#include <stdio.h>
#include <cjson/cJSON.h>


int main()
{
	zsock_t* receiver = zsock_new(ZMQ_SUB);
	zsock_set_subscribe(receiver, "");
	//zsock_set_subscribe(receiver, "");
	zsock_connect(receiver, "tcp://127.0.0.1:5555");

	char* str;

	printf("Waiting for message...\n");

	while (true)
	{
		str = zstr_recv(receiver);
		if (str != NULL)
		{
			break;
		}
	}	
	cJSON* json = cJSON_Parse(str);

	if (json == NULL)
	{
		printf("Error before: [%s]\n", cJSON_GetErrorPtr());
	}
	else
	{
		cJSON* sendTime = cJSON_GetObjectItemCaseSensitive(json, "sendTime");
		cJSON* data = cJSON_GetObjectItemCaseSensitive(json, "data");

		if (cJSON_IsNumber(sendTime) && (sendTime->valueint > 0))
		{
			printf("sendTime: %d\n", sendTime->valueint);
		}

		if (cJSON_IsArray(data))
		{
			cJSON* dataItem = NULL;
			int arr[50] = { 0 };
			int i = 0;

			cJSON_ArrayForEach(dataItem, data)
			{
				if (cJSON_IsNumber(dataItem))
				{
					arr[i] = dataItem->valueint;
					i++;
				}
			}

			for (int j = 0; j < 50; j++)
			{
				printf("%d ", arr[j]);
			}
		}
	}

	Sleep(10000);
	


	
}