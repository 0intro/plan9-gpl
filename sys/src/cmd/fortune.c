/*
 * This file is part of the UCB release of Plan 9. It is subject to the license
 * terms in the LICENSE file found in the top-level directory of this
 * distribution and at http://akaros.cs.berkeley.edu/files/Plan9License. No
 * part of the UCB release of Plan 9, including this file, may be copied,
 * modified, propagated, or distributed except according to the terms contained
 * in the LICENSE file.
 */

#include <u.h>
#include <libc.h>
#include <bio.h>

char choice[2048];
char index[] = "/sys/games/lib/fortunes.index";
char fortunes[] = "/sys/games/lib/fortunes";

void
main(int argc, char *argv[])
{
	int i;
	long offs;
	uchar off[4];
	int ix, nix;
	int newindex, oldindex;
	char *p;
	Dir *fbuf, *ixbuf;
	Biobuf *f, g;

	newindex = 0;
	oldindex = 0;
	ix = offs = 0;
	if((f=Bopen(argc>1?argv[1]:fortunes, OREAD)) == 0){
		print("Misfortune!\n");
		exits("misfortune");
	}
	ixbuf = nil;
	if(argc == 1){
		ix = open(index, OREAD);
		if(ix>=0){
			ixbuf = dirfstat(ix);
			fbuf = dirfstat(Bfildes(f));
			if(ixbuf == nil || fbuf == nil){
				print("Misfortune?\n");
				exits("misfortune");
			}
			if(ixbuf->length == 0){
				/* someone else is rewriting the index */
				goto NoIndex;
			}
			oldindex = 1;
			if(fbuf->mtime > ixbuf->mtime){
				nix = create(index, OWRITE, 0666);
				if(nix >= 0){
					close(ix);
					ix = nix;
					newindex = 1;
					oldindex = 0;
				}
			}
		}else{
			ix = create(index, OWRITE, 0666);
			if(ix >= 0)
				newindex = 1;
		}
	}
	if(oldindex){
		seek(ix, truerand()%(ixbuf->length/sizeof(offs))*sizeof(offs), 0);
		read(ix, off, sizeof(off));
		Bseek(f, off[0]|(off[1]<<8)|(off[2]<<16)|(off[3]<<24), 0);
		p = Brdline(f, '\n');
		if(p){
			p[Blinelen(f)-1] = 0;
			strcpy(choice, p);
		}else
			strcpy(choice, "Misfortune!");
	}else{
NoIndex:
		Binit(&g, ix, 1);
		srand(truerand());
		for(i=1;;i++){
			if(newindex)
				offs = Boffset(f);
			p = Brdline(f, '\n');
			if(p == 0)
				break;
			p[Blinelen(f)-1] = 0;
			if(newindex){
				off[0] = offs;
				off[1] = offs>>8;
				off[2] = offs>>16;
				off[3] = offs>>24;
				Bwrite(&g, off, sizeof(off));
			}
			if(lrand()%i==0)
				strcpy(choice, p);
		}
	}
	print("%s\n", choice);
	exits(0);
}
