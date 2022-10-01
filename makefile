#makefile incompleto(apenas para testes parciais)

OBJ_dir = obj
OBJ = data.o test_data.o

test_data: $(OBJ)
	gcc $(addprefix $(OBJ_dir)/,$(OBJ)) -g -o bin/test_data 

%.o: src/%.c
	gcc $< -c -I include -g -o $(OBJ_dir)/$@