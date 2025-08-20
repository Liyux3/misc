CC=gcc
EXE=add segfault loopf forever cmdarg relay hello sema

all: $(EXE)

add: add-loop.c
	$(CC) add-loop.c -o $@

segfault: segfault.c
	$(CC) segfault.c -o $@

loopf: loop.c
	$(CC) $< -o $@
    
forever: loopever.c
	$(CC) $< -o $@
    
cmdarg: cmdarg.c
	$(CC) $< -o $@
  
Relay: relay.c
	$(CC) $< -o $@

hello: hello.c
	$(CC) $< -o $@

sema: lab4-sema.c                                      
	gcc $^ -o $@ -pthread    

clean:
	rm $(EXE)
