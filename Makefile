CC = gcc
CFLAGS = -Wall -O2
LIBS = -lpthread

all: ultra_server ultra_client water_server water_client

ultra_server: ultra_server.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

ultra_client: ultra_client.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

water_server: water_server.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

water_client: water_client.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f ultra_server ultra_client water_server water_client

.PHONY: all clean 