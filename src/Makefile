BIN         = ../bin
EW-FOR_BIN  = talker
ABIN        = angel
OBJECT-DIR  = ../junk
CC          = gcc
CFLAGS      = -O2
# Need this for Linux
LIBS        =
#LIBS        = -lmalloc -lsocket -lnsl
HDRS        = 
DEFS        =

OBJS        = $(OBJECT-DIR)/admin.o \
	      $(OBJECT-DIR)/admin2.o \
              $(OBJECT-DIR)/aliases.o \
	      $(OBJECT-DIR)/channel.o \
              $(OBJECT-DIR)/commands.o \
              $(OBJECT-DIR)/dynamic.o \
              $(OBJECT-DIR)/editor.o \
              $(OBJECT-DIR)/examine.o \
	      $(OBJECT-DIR)/gag.o \
	      $(OBJECT-DIR)/games.o \
              $(OBJECT-DIR)/globals.o \
              $(OBJECT-DIR)/glue.o \
              $(OBJECT-DIR)/items.o \
              $(OBJECT-DIR)/lists.o \
              $(OBJECT-DIR)/mail.o \
              $(OBJECT-DIR)/parse.o \
              $(OBJECT-DIR)/plists.o \
              $(OBJECT-DIR)/room.o \
              $(OBJECT-DIR)/session.o \
              $(OBJECT-DIR)/shortcut.o \
              $(OBJECT-DIR)/socket.o \
              $(OBJECT-DIR)/tag.o \
              $(OBJECT-DIR)/talking.o \
              $(OBJECT-DIR)/track.o
#      /usr/lib/debug/malloc.o

all:            $(EW-FOR_BIN) $(ABIN)
				@echo 'Made all'

mguard:         $(ABIN)


$(ABIN):   angel.c Makefile
				$(CC) $(CFLAGS) -o $(ABIN) $(DEFS) angel.c $(LIBS)

$(EW-FOR_BIN):      $(OBJS) Makefile
				$(CC) $(CFLAGS) -o $(EW-FOR_BIN) $(DEFS) $(OBJS) $(LIBS)

clean:
	rm ../junk/* $(EW-FOR_BIN) $(ABIN)

install:           $(EW-FOR_BIN) $(ABIN)
				mv $(EW-FOR_BIN) $(BIN)
				chmod 700 $(BIN)/$(EW-FOR_BIN)
				mv $(ABIN) $(BIN)
				chmod 700 $(BIN)/$(ABIN)
				@echo 'Made install'

program:           $(EW-FOR_BIN)
				@echo 'Made program'

$(OBJECT-DIR)/%.o:   config.h player.h
				$(CC) $(CFLAGS) $(DEFS) -c $*.c -o $(OBJECT-DIR)/$*.o

###
$(OBJECT-DIR)/glue.o: glue.c 
$(OBJECT-DIR)/socket.o: socket.c 
$(OBJECT-DIR)/globals.o: globals.c 
$(OBJECT-DIR)/plists.o: plists.c compaction.c
$(OBJECT-DIR)/parse.o: parse.c clist.h
$(OBJECT-DIR)/items.o: items.c items.h
$(OBJECT-DIR)/lists.o: lists.c 
$(OBJECT-DIR)/talking.o: talking.c social.h
$(OBJECT-DIR)/commands.o: commands.c 
$(OBJECT-DIR)/examine.o: examine.c 
$(OBJECT-DIR)/channel.o: channel.c channel.h
$(OBJECT-DIR)/admin.o: admin.c admin.h
$(OBJECT-DIR)/admin2.o: admin2.c
$(OBJECT-DIR)/aliases.o: aliases.c aliases.h
$(OBJECT-DIR)/room.o: room.c
$(OBJECT-DIR)/games.o: games.c
$(OBJECT-DIR)/gag.o: gag.c
$(OBJECT-DIR)/mail.o: mail.c
$(OBJECT-DIR)/tag.o: tag.c
$(OBJECT-DIR)/malloc.o: malloc.c
$(OBJECT-DIR)/editor.o: editor.c
$(OBJECT-DIR)/session.o: session.c
$(OBJECT-DIR)/shortcut.o: shortcut.c
$(OBJECT-DIR)/dynamic.o: dynamic.c dynamic.h
$(OBJECT-DIR)/track.o: track.c
