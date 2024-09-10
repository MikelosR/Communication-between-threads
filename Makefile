a1:
	@echo "Compile ProcessA1 ...";	
	gcc ProcessA.c read_thread.c write_thread.c -o runA -lpthread -Wno-int-to-pointer-cast

run1:
	./runA

b1:
	@echo "Compile ProcessB ...";	
	gcc ProcessB.c read_thread.c write_thread.c -o runB -lpthread -Wno-int-to-pointer-cast

run2:
	./runB $(ARGS)

clean:
	rm -f runA runB