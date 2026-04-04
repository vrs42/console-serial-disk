all:	csd loopback recvtst

clean:
	rm csd loopback recvtst

csd: csd.c boot.c pdp.c tty.c disk.c
	cc csd.c -o csd

loopback: loopback.c
	cc loopback.c -o loopback

recvtst: recvtst.c pdp.c
	cc recvtst.c -o recvtst
