all:	csd loopback recvtst pqscsd.bn

clean:
	rm csd loopback recvtst

csd: csd.c boot.c pdp.c tty.c disk.c pqscsd.h
	cc csd.c -o csd

loopback: loopback.c
	cc loopback.c -o loopback

recvtst: recvtst.c pdp.c
	cc recvtst.c -o recvtst

# Could use bin2bn here
pqscsd.bn: pqscsd.pa
	pal -d $< && mv pqscsd.bin $@

pqscsd.h: pqscsd.bn
	./bn2h $< >$@
