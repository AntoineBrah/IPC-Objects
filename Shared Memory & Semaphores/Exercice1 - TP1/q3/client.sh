g++ -c client.c ; g++ client.o calcul/calculC.o calcul/calculCC.o -o client -lpthread; rm client.o ; ./client $1; rm client
