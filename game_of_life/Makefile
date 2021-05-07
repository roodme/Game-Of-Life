# paths
LIB = ../lib
INCLUDE = ../include

# Files to read
FILES = 

# compiler
CC = gcc

DEFINES = -DSHOW_EVOLUTION

# Compile options. Το -I<dir> λέει στον compiler να αναζητήσει εκεί include files
CFLAGS = $(DEFINES) -Wall -Werror -g -I$(INCLUDE)
LDFLAGS = -lm

# Αρχεία .o
OBJS = life_gif.o lifestate.o  $(LIB)/k08.a $(LIB)/libbmp.a

# Το εκτελέσιμο πρόγραμμα
EXEC = life_gif

# Παράμετροι για δοκιμαστική εκτέλεση
ARGS = states/pattern.rle 100 16 1 50 example.gif


$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

# Για να φτιάξουμε το k08.a, τρέχουμε το make στο lib directory.
$(LIB)/k08.a:
	$(MAKE) -C $(LIB) k08.a

# το ίδιο για το libbmp.a
$(LIB)/libbmp.a:
	$(MAKE) -C $(LIB) libbmp.a

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC) $(ARGS)

valgrind: $(EXEC)
	valgrind --leak-check=full ./$(EXEC) $(ARGS)

time: $(EXEC)
	time ./$(EXEC) $(ARGS)