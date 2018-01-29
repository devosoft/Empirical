
othello: AvidaGP-Othello.cc
	g++ AvidaGP-Othello.cc -O3 -DNDEBUG --std=c++14 -I ../source -o othelloGP

debug: AvidaGP-Othello.cc
	g++ AvidaGP-Othello.cc -g --std=c++14 -I ../source -o othelloDB

clean:
	rm -f *.o othelloGP othelloDB

compete: compete.cc
	g++ compete.cc -O3 -DNDEBUG --std=c++14 -I ../source -o compete
