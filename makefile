
.PHONY: all clean Q1 Q2

all: Q1 Q2 

Q1:
	make -C Q1 all

Q2:
	make -C Q2 all

clean:
	make -C Q1 clean
	make -C Q2 clean
