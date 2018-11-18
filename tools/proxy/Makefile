
# Basic shortcut to folders
CLIENT = ../../client
COMMON = ../../common
SERVER = ../../server
MASTER = ../../master

#Set the files
OBJFILES =      $(MASTER)/i_net.cpp \
                tv/protocol.cpp \
                main.cpp \

# Program flags
FLAGS = -DUNIX



# Compiler settings
CC = g++
CFLAGS = -g

all:
        $(CC) $(CFLAGS) $(FLAGS) $(OBJFILES) -o proxy
		
