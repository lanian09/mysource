RM		= `which rm`
SRC		= t.c
OBJS 	= $(SRC:.c=.o)
TARGET	= TR 
TTT		= `if test -e /usr/lib64; then echo 1; else echo 0; fi`

#!-------------------------

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS)
	@echo "TEST MAKING..>>> ${TTT}."
	ifeq ( ${TTT}, 1 )
		@echo "EXIST"
	then
		@echo "NOT EXIST"
	fi

clean :
	$(RM) -rf $(OBJS)	


