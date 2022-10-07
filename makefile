#makefile incompleto(apenas para testes parciais)

OBJ_dir = obj
OBJ = data.o entry.o tree.o test_tree.o serialization.o

test: $(OBJ)
	gcc $(addprefix $(OBJ_dir)/,$(OBJ)) -g -o bin/test

%.o: source/%.c
	gcc $< -c -I include -g -o $(OBJ_dir)/$@