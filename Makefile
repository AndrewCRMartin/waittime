waittime : waittime.c
	cc -g3 -o $@ $<

clean :
	\rm -f waittime
