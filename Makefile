LINK.o = $(LINK.cc)
CXXFLAGS=-Wall -Wextra -Werror -pedantic -std=c++17 -g -fsanitize=address

OBJ = resolver.o

resolver: ${OBJ}

clean:
	${RM} ${OBJ}
