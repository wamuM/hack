main: main.c fetcher.c cJSON.c
	gcc -o main main.c fetcher.c cJSON.c -lcurl
