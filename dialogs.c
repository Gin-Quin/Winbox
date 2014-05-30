#include <os.h>
#include <SDL\SDL.h>

#include "Winbox.h"

// thème par défaut allégé afin d'accélérer l'exécution
static wTHEME *FastTheme()
{
	wTHEME *t = calloc(0, sizeof(wTHEME));
	if (!t) return NULL;
	
	SDL_PixelFormat *fmt = SDL_GetVideoSurface()->format;
	t->color1 = SDL_MapRGB(fmt, 239,251,247);
	t->color2 = SDL_MapRGB(fmt, 24,48,90);
	t->color3 = SDL_MapRGB(fmt, 66,113,206);
	t->color4 = SDL_MapRGB(fmt, 33,16,33);
	t->defbg = wBackgroundColor(SDL_MapRGB(fmt,99,162,181), SDL_MapRGB(fmt,214,235,235), BACKG_TRICOLOR);
	t->font_index = NSDL_FONT_THIN;
	t->font = nSDL_LoadFont(t->font_index, 33, 16, 33);
	
	t->button_bg = wBackgroundColor(SDL_MapRGB(fmt, 222,222,222), SDL_MapRGB(fmt, 16,52,132), BACKG_HGRAD);
	t->button_bg_selected = wBackgroundColor(SDL_MapRGB(fmt, 0,24,33), SDL_MapRGB(fmt, 74,77,74), BACKG_HGRAD);
	t->button_font = nSDL_LoadFont(NSDL_FONT_VGA, 0, 0, 0);
	t->button_font_selected = nSDL_LoadFont(NSDL_FONT_VGA, 255, 255, 255);
	
	t->window_c1 = SDL_MapRGB(fmt, 0,0,0);
	t->window_title_bg = wBackgroundColor(SDL_MapRGB(fmt, 0,0,0), SDL_MapRGB(fmt, 214,231,225), BACKG_HGRAD);
	t->window_title_font = nSDL_LoadFont(NSDL_FONT_TINYTYPE, 255, 255, 255);
	
	t->dialog_bg1 = wBackgroundColor(SDL_MapRGB(fmt, 222,222,222), SDL_MapRGB(fmt, 57,162,132), BACKG_HGRAD);
	t->dialog_bg2 = wBackgroundColor(SDL_MapRGB(fmt, 222,222,222), SDL_MapRGB(fmt, 148,73,82), BACKG_HGRAD);

	t->text_c1 = SDL_MapRGB(fmt, 198,211,222);
	t->request_c1 = SDL_MapRGB(fmt, 231,227,255);
	t->request_c2 = SDL_MapRGB(fmt, 239,239,239);
	t->request_c3 = SDL_MapRGB(fmt, 49,65,115);
	t->request_f1 = nSDL_LoadFont(NSDL_FONT_THIN, 0, 8, 41);
	t->request_f2 = nSDL_LoadFont(NSDL_FONT_TINYTYPE, 107, 146, 189);
	return t;
}


void wShowMsg(const char *title, const char *msg)
{
	Widget *dialog = wDialog1(title, "OK");
	Widget *text = wUnEditableText(msg, 4);
	wSetHeight(dialog, 92);
	wAddWidget(dialog, text);
	wExecConstruct(dialog, FastTheme());
}


int wInputMsg(const char *title, const char *msg)
{
	Widget *dialog = wDialog2(title, "Oui", "Non");
	Widget *text = wUnEditableText(msg, 4);
	wSetHeight(dialog, 92);
	wAddWidget(dialog, text);
	
	return wExecConstruct(dialog, FastTheme());
}


void wFastMsg(const char *msg, ...)
{
	char *p1 = (char *) msg;
	char *p2 = p1;
	int bufferSize = 512;
	int msgSize = 0;
	char *buffer = malloc(bufferSize);
	buffer[0] = 0;
	va_list v;
	va_start(v, msg);
	int W = 220;
	SDL_Surface *scr = SDL_GetVideoSurface();
	
	
	/* 1-- On remplace les %u, %i, %c et %s */
	while ((p2 = strchr(p1, '%'))) {
		*p2 = 0;
		msgSize += strlen(p1);
		if (msgSize > bufferSize) {
			bufferSize = msgSize+64;
			buffer = realloc(buffer, bufferSize);
		}
		strcat(buffer, p1);
		*p2++ = '%';
		
		// on ajoute l'élément suivant
		if (*p2 == 'u') {
			unsigned int u = va_arg(v, unsigned int);
			msgSize += 12;  // 12 chiffres majore largement tout entier
			if (msgSize > bufferSize) {
				bufferSize = msgSize+64;
				buffer = realloc(buffer, bufferSize);
			}
			sprintf(buffer + strlen(buffer), "%u", u);
		}
		
		else if (*p2 == 'i') {
			int i = va_arg(v, int);
			msgSize += 12;  // 12 chiffres majore largement tout entier
			if (msgSize > bufferSize) {
				bufferSize = msgSize+64;
				buffer = realloc(buffer, bufferSize);
			}
			sprintf(buffer + strlen(buffer), "%i", i);
		}
		
		else if (*p2 == 'c') {
			msgSize++;
			if (msgSize > bufferSize) {
				bufferSize = msgSize+64;
				buffer = realloc(buffer, bufferSize);
			}
			buffer[strlen(buffer)+1] = 0;
			buffer[strlen(buffer)] = (char) va_arg(v, int);
		}
		
		else if (*p2 == 's') {
			char *s = va_arg(v, char *);
			msgSize += strlen(s);
			if (msgSize > bufferSize) {
				bufferSize = msgSize+64;
				buffer = realloc(buffer, bufferSize);
			}
			strcat(buffer, s);
		}
		
		else {
			// on ajoute le symbole % et on continue à lire
			msgSize++;
			if (msgSize > bufferSize) {
				bufferSize = msgSize+64;
				buffer = realloc(buffer, bufferSize);
			}
			buffer[strlen(buffer)+1] = 0;
			buffer[strlen(buffer)] = '%';
			p2--;
		}
		
		p1 = p2+1;
	}
	va_end(v);
	
	// on rajoute la fin de la chaîne
	msgSize += strlen(p1);
	if (msgSize > bufferSize) {
		bufferSize = msgSize+64;
		buffer = realloc(buffer, bufferSize);
	}
	strcat(buffer, p1);
	
	
	
	
	
	/* 2-- On calcule les lignes */
	int nLines = 0;
	char *p0 = buffer;
	char *p3;
	p1 = p0-1;
	p2 = p0;
	char c;
	nSDL_Font *font = nSDL_LoadFont(NSDL_FONT_TINYTYPE, 35,0,0);
	
	while (p2) {
		p2 = strchr(p1+1, ' ');
		p3 = strchr(p1+1, '\n');
		if (!p2 && !p3) break;
		if (!p2) p2 = p3;
		else if (p3) p2 = min(p2, p3);
		// p2 est à présent le prochain 'séparateur'
		
		
		// on vérifie qu'il n'y ait pas de '\n'
		c = *p2;
		*p2 = 0;
// DrawFillRect(scr, NULL, RGB(255, 200, 200));
// nSDL_DrawString(scr, font, 0, 0, "nLines = %i\nwidth = %i", nLines, nSDL_GetStringWidth(font, p0));
// SDL_Flip(scr);
// wait_key_pressed();
		
		if (nSDL_GetStringWidth(font, p0) > W) { // si [p0->p2] dépasse, alors [p0->p1] forme une nouvelle ligne (<=> *p1 = 0)
			nLines++;
			
			if (p0 > p1) { // [p0->p2] forme une ligne trop grande
				p1 = p2+1;
				p0 = p1;
			}
			
			else {  // [p0->p1] forme une ligne de bonne taille
				// on pose p1 à 0
				*p1 = 0;
				
				// si p2 est un espace
				if (c == ' ') {
					*p2 = ' ';
					p0 = p1+1;
					p1 = p2;
				}
				
				// sinon
				else {
					nLines++;
					p1 = p2;
					p0 = p1+1;
				}
			}
		}
		
		else {
			if (c == ' ') {
				*p2 = ' ';
				p1 = p2;
			}
			
			else {
				nLines++;
				p1 = p2;
				p0 = p1+1;
			}
		}
	}
	
	// on ajoute la dernière ligne
	if (p0 > p1 || nSDL_GetStringWidth(font, p0) <= W) nLines++;
	else {
		nLines += 2;
		*p1 = 0;
	}
	
	
	/* 3-- On affiche le message */
	int x;
	SDL_Rect r = {0, 0, 226, 6+10*nLines};
	SDL_Rect clip;
	r.x = (scr->w - r.w) / 2;
	r.y = (scr->h - r.h) / 2;
	SDL_Surface *saveImage = NewSurface(r.w, r.h);
	DrawSurface(scr, &r, saveImage, NULL);
	SDL_GetClipRect(scr, &clip);
	SDL_SetClipRect(scr, &r);
	
	// on dessine le rectangle
	DrawFillRect(scr, &r, RGB(250,190,146));
	DrawRect(scr, &r, RGB(136,0,21));
	DrawRectXY(scr, r.x+1, r.y+1, r.w-2, r.h-2, RGB(207,22,22));
	
	
	// on affiche les lignes de texte
	p0 = buffer;
	for (x=0; x < nLines; x++) {
		DrawClippedStr(scr, font, (scr->w - nSDL_GetStringWidth(font,p0))/2, r.y+4+10*x, p0);
		p0 += strlen(p0)+1;
	}
	
	// on affiche et attend l'appui d'une touche
	SDL_SetClipRect(scr, &clip);
	SDL_Flip(scr);
	
	DrawSurface(saveImage, NULL, scr, &r);
	SDL_FreeSurface(saveImage);
	free(buffer);
	nSDL_FreeFont(font);
	
	wait_no_key_pressed();
	wait_key_pressed();
	wait_no_key_pressed();
	SDL_Flip(scr);
}





wMENU *wPopup(wMENU *m, int *n)
{
	return wPopupXY(m, -1, -1, n);
}


wMENU *wPopupXY(wMENU *m, int x, int y, int *n)
{
	*n = -1;
	if (!m) return NULL;
	int ok;
	int b = 0;
	wMENU *r = NULL;
	
	Widget *list = wBoxList(m, min(8, m->nItems), 0);
	if (x >= 0 && y >= 0) wSetSpot(list, x, y);
	wSetWidth(list, 180);
	wSetFreedArgs(list, WMENU);  // c'est à l'utilisateur de fermer le menu
	
	
	int CB_Click(Widget *w, int signal)
	{
		if (w && signal == SIGNAL_CLICK) {
			b=1;
			return ACTION_EXIT;
		}
		return ACTION_CONTINUE;
	}
	
	wConnect(list, CB_Click);
	
	ok = wActivateConstruct(list, NULL)-1;
	if (!ok || b) r = wList_GetCurrentItem(list, n);
	wCloseConstruct(list);
	return r;
}





int wOpenBox(const char *path, const char *pattern, char *fileName)
{
	int ok;
	nSDL_Font *font = nSDL_LoadFont(NSDL_FONT_VGA, 10, 32, 70);
	BOOL b = pattern && pattern[0] && strcmp(pattern, "*");
		
	// Déclaration des widgets
	Widget *mainWidget = wDialog2("Open", "Open", "Cancel");
	Widget *body = wBasicLayout(0);
	Widget *label = wExLabel("", ALIGN_LEFT, font);
	Widget *explorer = wBoxExplorer((char *) path, (char *) pattern, b? 10:12);
	Widget *typeFile = NULL;
	
	wAddWidget(mainWidget, body);
	wAddWidget(body, label);
	wAddWidget(body, explorer);
	if (b) {
		typeFile = wRequest("Files :", pattern, REQUEST_ALL, 32);
		wAddWidget(body, typeFile);
	}

	
	wSetWidth(mainWidget, 300);
	wSetHeight(mainWidget, 210);
	wSetDynamism(typeFile, 0);
	
	// Fonctions Callbacks
	int CB_ChangeLabel(Widget *w, int signal)
	{
		if (w == explorer && signal == SIGNAL_ACTION) {
			wList_GetCurrentPath(explorer, fileName);
			if (!fileName[0]) strcpy(fileName, "/");
			char *p = fileName;
			
			// on vérifie que fileName n'est pas trop grand
			if (nSDL_GetStringWidth(font, fileName) > label->bounds.w-4) {
				do p = strchr(++p, '/');
				while (p && nSDL_GetStringWidth(font, p)+nSDL_GetStringWidth(font, "... ") > label->bounds.w-4);
				
				if (p) {
					memmove(fileName+4, p, strlen(p)+1);
					fileName[0] = '.', fileName[1] = '.', fileName[2] = '.', fileName[3] = ' ';
				}
			}
			
			// on change le texte du label
			wLabel_SetText(label, fileName);
			wDrawWidget(label);
		}
		return ACTION_CONTINUE;
	}
	
	wConnect(explorer, CB_ChangeLabel);
	
	
	// on active la construction
	wDrawConstruct(mainWidget, NULL);
	CB_ChangeLabel(explorer, SIGNAL_ACTION);
	ok = wActivateConstruct(mainWidget, NULL);
	if (ok) {
		wList_GetCurrentPath(explorer, fileName);
		if (fileName[0] && fileName[strlen(fileName)-1] != '/') strcat(fileName, "/");
		strcat(fileName, wList_GetCurrentStr(explorer));
	}
	else fileName[0] = 0;
	
	wCloseConstruct(mainWidget);
	return ok;
}



int wSaveBox(const char *path, char *fileName)
{
	int ok;
	nSDL_Font *font = nSDL_LoadFont(NSDL_FONT_VGA, 10, 32, 70);
	
	// Déclaration des widgets
	Widget *mainWidget = wDialog2("Save", "Save", "Cancel");
	Widget *body = wBasicLayout(0);
	Widget *label = wExLabel("", ALIGN_LEFT, font);
	Widget *explorer = wBoxExplorer((char *) path, NULL, 10);
	Widget *req = wRequest(NULL, fileName, REQUEST_VAR, 31);
	
	wSetWidth(mainWidget, 300);
	wSetHeight(mainWidget, 210);
	
	wAddWidget(mainWidget, body);
	wAddWidget(body, label);
	wAddWidget(body, explorer);
	wAddWidget(body, req);
	wSetActiveWidget(req);
	
	
	// Fonctions Callback
	int CB_Explorer(Widget *w, int signal)
	{
		if (w != explorer) return ACTION_CONTINUE;
		
		if (signal == SIGNAL_CLICK) {
			// on modifie req
			wRequest_SetText(req, wList_GetCurrentStr(explorer));
			wDrawWidget(req);
		}
		
		else if (signal == SIGNAL_ACTION) {
			// on modifie label
			wList_GetCurrentPath(explorer, fileName);
			if (!fileName[0]) strcpy(fileName, "/");
			char *p = fileName;
			
			// on vérifie que fileName n'est pas trop grand
			if (nSDL_GetStringWidth(font, fileName) > label->bounds.w-4) {
				do {
					p++;
					p = strchr(p, '/');
				} while (p && nSDL_GetStringWidth(font, p)+nSDL_GetStringWidth(font, "... ") > label->bounds.w-4);
				
				if (p) {
					memmove(fileName+4, p, strlen(p)+1);
					fileName[0] = '.', fileName[1] = '.', fileName[2] = '.', fileName[3] = ' ';
				}
			}
			
			// on change le texte du label
			wLabel_SetText(label, fileName);
			fileName[0] = 0;
			wDrawWidget(label);
		}
		
		return ACTION_CONTINUE;
	}
	
	wConnect(explorer, CB_Explorer);
	
	
	// on active le construct
	wDrawConstruct(mainWidget, NULL);
	CB_Explorer(explorer, SIGNAL_ACTION);  // on définit le texte du label
	ok = wActivateConstruct(mainWidget, NULL);
	if (ok) {
		wList_GetCurrentPath(explorer, fileName);
		if (fileName[0] && fileName[strlen(fileName)-1] != '/') strcat(fileName, "/");
		strcat(fileName, wRequest_GetText(req));
	}
	
	wCloseConstruct(mainWidget);
	return ok;
}












