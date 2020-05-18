OBJ_FILES_A := sim_a.o liftA.o bufferA.o
OBJ_FILES_B := sim_b.o liftB.o bufferB.o
FLAGS := -Wall -pthread -lrt -g
 
all: lift_sim_a lift_sim_b

lift_sim_a: $(OBJ_FILES_A)
	@gcc -pthread $^ -o $@

lift_sim_b: $(OBJ_FILES_B)
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
	@rm lift_sim_a
	@rm lift_sim_b

