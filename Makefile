# ******************************************************************************
# FROM : inginious.info.ucl.ac.be/course/LINGI1341/envoyer-et-recevoir-des-donnees
# AUTHOR : Olivier Tilmans
# ******************************************************************************
# See gcc/clang manual to understand all flags
CFLAGS += -std=c99 # Define which version of the C standard to use
CFLAGS += -Wall # Enable the 'all' set of warnings
CFLAGS += -Werror # Treat all warnings as error
CFLAGS += -Wshadow # Warn when shadowing variables
CFLAGS += -Wextra # Enable additional warnings
CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing
CFLAGS += -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE # feature_test_macros for getpot and getaddrinfo

# We have no libraries to link against except libc, but we want to keep
# the symbols for debugging
LDFLAGS= -rdynamic -lz
# ******************************************************************************

all: clean receiver sender link
debug: CFLAGS += -g -DDEBUG -Wno-unused-parameter -fno-omit-frame-pointer
debug: clean sender
debug: clean receiver
sender: src/sender.o src/packet_interface.o src/transport_interface.o
	gcc -o sender src/sender.o src/packet_interface.o src/transport_interface.o $(CFLAGS) $(LDFLAGS)
receiver: src/receiver.o src/packet_interface.o src/transport_interface.o
	gcc -o receiver src/receiver.o src/packet_interface.o src/transport_interface.o $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	@rm -rf receiver sender src/*.o && cd LINGI1341-linksim && $(MAKE) clean

link:
	@cd LINGI1341-linksim && $(MAKE)

test:
	@./tests/tests.sh
