#include <os.h>
#include <SDL\SDL.h>

#include "general.h"
#include "keys.h"
#include "structures.h"
#include "textInput.h"


TextBody getTextBody(nSDL_Font *f, int w, char *s)
{
	int x, y, z;
	char c;
	TextBody text = {NULL, 0};
	void *tmp;
	int nRows = 0;
	TextRow *rows = NULL;
	
	if (!s) return text;
	if (!s[0]) return text;
	if (w < 20) return text;
	
	
	// 1-- on sépare en lignes
	x = 0; // x = position dans la chaîne
	c = s[x];
	
	if (c == 21) {
		z = x;
		goto NEWLINE;
	}
	
	while (c) {
		y = x+1;
		
		do {
			c = s[y];
			z = y - 1;
			if (z > x) s[z] = ' ';
			while (c && c != ' ' && c != 21) c = s[++y];
			s[y++] = 0;
		} while (c && c!=21 && nSDL_GetStringWidth(f, s+x) < w);
		
		s[y-1] = c;
		
		
		
		if (!c) {
			
			if (nSDL_GetStringWidth(f, s+x) >= w) {
				
				if (z == x) {
					// si le dernier mot est trop grand,
					// on chope toutes ses lignes.
					// (eh oui il peut y en avoir plusieurs... :( )
					do {
						y = x+1;
						
						do {
							if (y > x+1) s[y-1] = c;
							c = s[y];
							s[y++] = 0;
						} while (nSDL_GetStringWidth(f, s+x) < w);
						s[y-1] = c;
						
						// on enregistre la ligne
						tmp = realloc(rows, (nRows+1)*sizeof(TextRow));
						if (!tmp) break;
						rows = tmp;
						rows[nRows].row = s+x;
						rows[nRows++].length = y-x-2;
						x = y-2;
					} while (nSDL_GetStringWidth(f, s+x) >= w);
					
					y = x + strlen(s+x) + 2;
					c = 0;
				}
				
				else {
					// on a deux mots, dont le premier est de bonne taille
					// mais le premier+second est trop grand
					// soit taille(second) < w et il ne fait qu'une ligne
					// soit taille(second) >= w et peut faire plusieurs lignes.
					
					// on enregistre le premier mot
					tmp = realloc(rows, (nRows+1)*sizeof(TextRow));
					if (!tmp) break;
					rows = tmp;
					rows[nRows].row = s+x;
					rows[nRows++].length = z-x;
					x = z + 1;
					
					// on enregistre le 2e
					while (nSDL_GetStringWidth(f, s+x) >= w) {
						y = x+1;
						
						do {
							if (y > x+1) s[y-1] = c;
							c = s[y];
							s[y++] = 0;
						} while (nSDL_GetStringWidth(f, s+x) < w);
						s[y-1] = c;
						
						// on enregistre la ligne
						tmp = realloc(rows, (nRows+1)*sizeof(TextRow));
						if (!tmp) break;
						rows = tmp;
						rows[nRows].row = s+x;
						rows[nRows++].length = y-x-2;
						x = y-2;
					}
					y = x + strlen(s+x) + 1;
					c = 0;
				}
			}
			
		}
		
		else if (c == 21) {
			s[y-1] = 0;
			if (z != x && nSDL_GetStringWidth(f, s+x) >= w) {
				// alors on traite l'espace (en Z) d'abord !
				s[y-1] = 21;
				goto SPACE;
			}
			
			z = y-1;
			
			if (nSDL_GetStringWidth(f, s+x) >= w) {
				while (nSDL_GetStringWidth(f, s+x) >= w) {
					y = x+1;
					
					do {
						if (y > x+1) s[y-1] = c;
						c = s[y];
						s[y++] = 0;
					} while (nSDL_GetStringWidth(f, s+x) < w);
					s[y-1] = c;
					
					// on enregistre la ligne
					tmp = realloc(rows, (nRows+1)*sizeof(TextRow));
					if (!tmp) break;
					rows = tmp;
					rows[nRows].row = s+x;
					rows[nRows++].length = y-x-2;
					x = y-2;
				}
			}
			s[z] = 21;
			
		  NEWLINE:
			// on enregistre
			tmp = realloc(rows, (nRows+1)*sizeof(TextRow));
			if (!tmp) break;
			rows = tmp;
			rows[nRows].row = s+x;
			rows[nRows++].length = z-x;
			
			while(s[z+1] == 21) {
				tmp = realloc(rows, (nRows+1)*sizeof(TextRow));
				if (!tmp) break;
				rows = tmp;
				rows[nRows].row = s+z+1;
				rows[nRows++].length = 0;
				z++;
			}
			
			if (s[z+1] == 0)  {
				tmp = realloc(rows, (nRows+1)*sizeof(TextRow));
				if (!tmp) break;
				rows = tmp;
				rows[nRows].row = s+z+1;
				rows[nRows++].length = 0;
				break;
			}
			
			
			x = z+1;
			c = 21;
			continue;
		}
		
		
		
		
	  SPACE:
		// on enregistre
		tmp = realloc(rows, (nRows+1)*sizeof(TextRow));
		if (!tmp) break;
		rows = tmp;
		rows[nRows].row = s+x;
			
		if (c && z == x) {
			// si le mot est plus grand que la largeur maximale, on doit le couper
			y = x+1;
			do {
				if (y > x+1) s[y-1] = c;
				c = s[y];
				s[y++] = 0;
			} while (nSDL_GetStringWidth(f, s+x) < w);
			s[y-1] = c;
			
			rows[nRows++].length = y-x-2;
			x = y-2;
		}
		
		else {
			// si le mot est normalement constitué
			rows[nRows++].length = (c? z:y-1) - x;
			x = (c? z+1:y);
		}
	}
	
	text.rows = rows;
	text.nRows = nRows;
	return text;
}

void DrawTextRow(SDL_Surface *scr, nSDL_Font *f, int x, int y, TextRow *l)
{
	if (!l->length) return;
	char c = l->row[l->length];
	l->row[l->length] = 0;
	
	DrawClippedStr(scr, f, x, y, l->row);
	
	l->row[l->length] = c;
}

void DrawTextBody(SDL_Surface *scr, nSDL_Font *f, int x, int y, TextBody *b, int fRow, int nRows)
{
	if (nRows <= 0 || b->nRows <= 0 || fRow < 0 || fRow > b->nRows) return;
	int h = 10;
	int z;
	
	for (z = 0; z < min(nRows, b->nRows - fRow); z++)
		DrawTextRow(scr, f, x, y+h*z, &(b->rows[z+fRow]));
}



char enshift(char c)
{
	if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
	if (c == ',') return 39;
	if (c == '(') return '{';
	if (c == ')') return '}';
	if (c == '1') return '<';
	if (c == '3') return '>';
	if (c == '2') return '_';
	if (c == 250) return 168;
	if (c == '0') return 13;
	if (c == '.') return 14;
	if (c == '4') return 3;
	if (c == '5') return 4;
	if (c == '7') return 5;
	if (c == '8') return 6;
	if (c == '6') return 11;
	if (c == '9') return 12;
	
	return c;
}



char enctrl(wCONSTRUCT *ctr, char c)
{
	if (c == '0') return ':';
	if (c == '.') return '!';
	if (c == 250) return '?';
	if (c == '1') return 174;
	if (c == '2') return '"';
	if (c == '3') return 175;
	if (c == '4') return '~';
	if (c == '5') return '\\';
	if (c == '6') return '&';
	if (c == '7') return '#';
	if (c == '8') return '@';
	if (c == '9') return 2;
	if (c == ',') return ';';
	if (c == '(') return '[';
	if (c == ')') return ']';
	if (c == 'l') return 156;
	if (c == 'c') return 135;
	if (c == 'n') return 164;
	if (c == 'f') return 159;
	
	char chars[30];
	int nchars;
	int x;
	
	if (c == 'a') {
		char tmp[6] = {133, 131, 132, 134, 160, 145};
		nchars = 6;
		for (x = 0; x < nchars; x++) chars[x] = tmp[x];
	}
	else if (c == 'e') {
		char tmp[4] = {130, 138, 136, 137};
		nchars = 4;
		for (x = 0; x < nchars; x++) chars[x] = tmp[x];
	}
	else if (c == 'i') {
		char tmp[5] = {140, 139, 141, 161, 173};
		nchars = 5;
		for (x = 0; x < nchars; x++) chars[x] = tmp[x];
	}
	else if (c == 'o') {
		char tmp[4] = {147, 148, 149, 162};
		nchars = 4;
		for (x = 0; x < nchars; x++) chars[x] = tmp[x];
	}
	else if (c == 'u') {
		char tmp[3] = {150, 151, 163};
		nchars = 3;
		for (x = 0; x < nchars; x++) chars[x] = tmp[x];
	}
	else if (c == 'y') {
		char tmp[2] = {152, 157};
		nchars = 2;
		for (x = 0; x < nchars; x++) chars[x] = tmp[x];
	}
	else if (c == 255) {
		nchars = 27;
		for (x = 0; x < nchars; x++) chars[x] = 224+x;
	}
	else
		return c;
	
	SDL_Surface *scr = ctr->scr;
	wTHEME *theme = ctr->theme;
	int z;
	x = nchars-1;
	SDL_Rect r = {160-5*nchars, 225, 10*nchars+2, 13};
	SDL_Rect tr = {0, 226, 10, 11};
	SDL_Surface *save = SDL_CreateRGBSurface(SDL_SWSURFACE, r.w, r.h, 16, 0, 0, 0, 0);
	SDL_BlitSurface(scr, &r, save, NULL);
	Uint32 black = SDL_MapRGB(scr->format, 0, 0, 0);
	Uint32 white = SDL_MapRGB(scr->format, 255, 255, 255);
	Uint32 bg = Darker(theme->color1, 10);
	nSDL_Font *f = nSDL_LoadFont(theme->font_index, 0, 0, 0);
	char k = 0;
	char ch[2] = "0";
	
	
	
	SDL_FillRect(scr, &r, white);
	DrawRect(scr, &r, black);
	
	for (z=0; z < nchars; z++) {
		ch[0]= chars[z];
		DrawClippedStr(scr, f, 162-5*nchars+10*z, tr.y + 2, ch);
	}
	
	SDL_Flip(scr);
	
	
	while (K_CTRL()) {
		k = K_getAlphaKey();
		if (!k) k = K_getNumericKey();
		if (!k) k = K_getPuncKey();
		if (K_PUNC()) k = 255;
		
		if (k == c) {
			// on redessine
			tr.x = 161-5*nchars+10*x;
			SDL_FillRect(scr, &tr, white);
			ch[0]= chars[x];
			DrawClippedStr(scr, f, tr.x+1, tr.y + 2, ch);
			
			x = (x+1)%nchars;
			tr.x = 161-5*nchars+10*x;
			SDL_FillRect(scr, &tr, bg);
			ch[0]= chars[x];
			DrawClippedStr(scr, f, tr.x+1, tr.y + 2, ch);
			
			SDL_Flip(scr);
			
			// on attend que la touche pressée soit relachée
			while (k == c) {
				k = K_getAlphaKey();
				if (!k) k = K_getNumericKey();
				if (!k) k = K_getPuncKey();
				if (K_PUNC()) k = 255;
			}
		}
	}
	
	SDL_BlitSurface(save, NULL, scr, &r);
	SDL_FreeSurface(save);
	nSDL_FreeFont(f);
	SDL_Flip(scr);
	
	return chars[x];
}




