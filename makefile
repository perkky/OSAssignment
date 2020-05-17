OBJ_FILES_A := sim_a.o liftA.o bufferA.o
OBJ_FILES_B := sim_b.o liftB.o bufferB.o
FLAGS := -Wall -pthread -lrt -g
 
all: sim_a sim_b

sim_a: $(OBJ_FILES_A)
	@gcc -pthread $^ -o $@

sim_b: $(OBJ_FILES_B)
	@gcc -pthread -lrt $^ -o $@
	
sim_a.o: sim_a.c
	@gcc -c $(FLAGS) $< -o $@

sim_b.o: sim_b.c
	@gcc -c $(FLAGS) $< -o $@

liftB.o: lift.c
	@gcc -c -DPROCESS=1 $(FLAGS) $< -o $@

liftA.o: lift.c
	@gcc -c $(FLAGS) $< -o $@

bufferA.o: buffer.c
	@gcc -c $(FLAGS) $< -o $@

bufferB.o: buffer.c
	@gcc -c -DPROCESS=1 $(FLAGS) $< -o $@


.PHONY: clean
clean:
	@rm *.o
	@rm sim_a
	@rm sim_b

