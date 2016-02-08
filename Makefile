LasForce: 
	gcc -Wall -g -I./include/ -o lasforce-c ./src/lasforce.c
	
clean:
	$(RM) lasforce-bbb