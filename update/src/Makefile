VERSION     = 1.0
BIN         = ../bin
EXEC        = update
OUT         = ../junk
CC          = gcc
CFLAGS      = -O2 
#LIBS        = -lg
LIBS        = -lg -Wl,-qmagic


OBJS        =   $(OUT)/aliases.o \
		$(OUT)/compaction.o \
	     	$(OUT)/dynamic.o \
              	$(OUT)/editor.o \
		$(OUT)/email.o \
		$(OUT)/file.o \
              	$(OUT)/globals.o \
              	$(OUT)/glue.o \
		$(OUT)/items.o \
              	$(OUT)/lists.o \
              	$(OUT)/mail.o \
              	$(OUT)/plists.o \
              	$(OUT)/room.o \
              	$(OUT)/track.o \
              	$(OUT)/update.o \
		$(OUT)/validate.o

	
$(EXEC):      $(OBJS) Makefile
				$(CC) $(CFLAGS) -o $(EXEC) $(DEFS) $(OBJS) $(LIBS)

clean:
	rm ../junk/* $(EXEC) 

install:           $(EXEC) 
				mv $(EXEC) $(BIN)
				chmod 700 $(BIN)/$(EXEC)
				@echo 'Made install'

program:           $(EXEC)
				@echo 'Made update'

$(OUT)/%.o:   config.h player.h
				$(CC) $(CFLAGS) $(DEFS) -c $*.c -o $(OUT)/$*.o

###
$(OUT)/compaction.o: compaction.c
$(OUT)/dynamic.o: dynamic.c dynamic.h
$(OUT)/editor.o: editor.c
$(OUT)/email.o: email.c
$(OUT)/file.o: file.c file.h
$(OUT)/globals.o: globals.c 
$(OUT)/glue.o: glue.c 
$(OUT)/lists.o: lists.c 
$(OUT)/mail.o: mail.c
$(OUT)/plists.o: plists.c 
$(OUT)/room.o: room.c
$(OUT)/track.o: track.c
$(OUT)/update.o: update.c update.h 
$(OUT)/validate.o: validate.c
$(OUT)/items.o: items.c items.h
$(OUT)/aliases.o: aliases.c 

