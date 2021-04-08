CFLAGS = -Wall -g

BINS=  fileRecovery

all: $(BINS)

fileRecovery: fileRecovery.c
	$(CC) $(CFLAGS) -o fileRecovery fileRecovery.c

clean:
	rm $(BINS)
