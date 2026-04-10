all:	csd loopback recvtst csd0.img

csd0.img: csd.dsk
	cp $< $@

clean:
	rm csd loopback recvtst

csd: csd.c boot.c pdp.c tty.c disk.c
	cc csd.c -o csd

loopback: loopback.c
	cc loopback.c -o loopback

recvtst: recvtst.c pdp.c
	cc recvtst.c -o recvtst

# Could use bin2bn here
pqscsd.bn: pqscsd.pa
	pal -d $< && mv pqscsd.bin $@

# We don't save foo.sv as pqscsd.sv.
# (The PQS8 build has a file by that name.)
pqscsd.sd: pqscsd.bn
	bn2sv $<
	sv2sd foo.sv
	mv foo.sd $@ && rm foo.sv

pqscsd.dd: pqscsd.sd
	dd if=$< of=$@ bs=8448 skip=1

csd.dsk: pqscsd.dd noboot
	cat pqscsd.dd noboot >$@
