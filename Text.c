#include <os.h>
#include <SDL\SDL.h>
#include "structures.h"
#include "keys.h"
#include "general.h"
#include "widgets.h"
#include "Text.h"
#include "textinput.h"
#include "scrollbar.h"

// Création
Widget *wText(const char *text, int dRows, int maxChars)
{
	return wExText(text, dRows, maxChars, NULL, NULL);
}

Widget *wExText(const char *text, int dRows, int maxChars, nSDL_Font *f, const char *voidStr)
{
	Widget *w = malloc(sizeof(Widget));
	if (!w) return NULL;
	w->type = WIDGET_TEXT;
	w->displayBounds = 0;
	w->Hexpansive = 1;
	w->Vexpansive = 0;
	w->parent = NULL;
	w->construct = NULL;
	w->background = NULL;
	w->isDynamic = 1;
	w->isLayout = 0;
	w->callBack = NULL;
	w->freeArgsType = 0;

	w->bounds.w = 160;
	w->bounds.h = 10*dRows+6;
	w->bounds.x = (320-w->bounds.w)/2;
	w->bounds.y = (240-w->bounds.h)/2;
	
	w->draw	= DrawText;
	w->activate	= ActivateText;
	w->close		= CloseText;
	w->add = NULL;
	
	w->customArgs = NULL;
	w->freeCustomArgs = NULL;
	
	w->args = malloc(sizeof(TextArgs));
	if (!w->args) {
		free(w);
		return NULL;
	}
	TextArgs *args	 = w->args;
	args->maxChars		= maxChars;
	args->voidStr		= (char *) voidStr;
	args->font			= f;
	args->dRows			= dRows;
	args->cRow 			= 0;
	args->yRow			= 0;
	args->cCharInRow	= 0;
	args->isEditable	= 1;
	args->isActive		= 0;
	args->scrollbar	= 0;
	args->text			= malloc(maxChars + 1);
	
	if (!text)
		args->text[0] = 0;
	
	else {
		int x=0, y=0;
		do {  // on vérifie qu'il n'y a pas deux espaces consécutifs
			if (x && (text[x-1] == ' ' || text[x-1] == 21) && text[x] == ' ')
				continue;
			args->text[y++] = text[x];
		} while (text[x++] && y < maxChars);
	}
	
	args->text[maxChars] = 0;
	args->length			= strlen(args->text);
	args->cChar				= 0;
	args->body				= (TextBody) {NULL, 0};
	
	return w;
}

Widget *wUnEditableText(const char *text, int dRows)
{
	Widget *w = wExText(text, dRows, strlen(text), NULL, NULL);
	wText_SetUnEditable(w);
	return w;
}

Widget *wExUnEditableText(const char *text, int dRows, nSDL_Font *f, const char *voidStr)
{
	Widget *w = wExText(text, dRows, strlen(text), f, voidStr);
	wText_SetUnEditable(w);
	return w;
}




// Dessin
void TextingProcess(Widget *w)
{
	TextArgs *args = w->args;
	wTHEME *theme = w->construct->theme;
	nSDL_Font *f = args->font? args->font : theme->font;
	int x=0, y=-1;
	char *c = NULL;
	args->length = strlen(args->text);
		
	if (!args->text || !args->text[0]) {
		args->body.rows = NULL;
		args->body.nRows = 0;
		args->cCharInRow = 0;
		args->cChar = 0;
		args->cRow = 0;
		args->yRow = 0;
		args->scrollbar = 0;
		if (!args->isEditable) w->isDynamic = 0;
		return;
	}
	
	
	free(args->body.rows);
	
	// on calcule les lignes du texte
	if (args->dRows * (w->bounds.w/5 + 1) < args->length) {
		// on est sûr qu'il y a besoin de scrollbar
		args->scrollbar = 1;
		args->body = getTextBody(f, w->bounds.w - 15, args->text);
	}
	else {
		args->scrollbar = 0;
		args->body = getTextBody(f, w->bounds.w - 8, args->text);
		if (args->body.nRows > args->dRows) {
			// il y a besoin de scrollbar
			free(args->body.rows);
			args->scrollbar = 1;
			args->body = getTextBody(f, w->bounds.w - 15, args->text);
		}
	}
	
	// on calcule la cCharInRow en fonction de cChar
	// et yRow en fonction de cRow
	if (args->body.nRows && args->body.rows) {
		// si on a une chaîne non-vide
		
		if (args->cChar == args->length) {
			// si on est au dernier caractère
			if (args->body.nRows < args->dRows) {
				args->yRow = 0;
				args->cRow = args->body.nRows - 1;
			}
			else {
				args->yRow = args->body.nRows - args->dRows;
				args->cRow = args->dRows - 1;
			}
			args->cCharInRow = args->body.rows[args->body.nRows-1].length;
		}
		
		else {
			c = NULL;
			x = 0;
			while (x < args->cChar) {
				if (c) x++;
				x += args->body.rows[++y].length;
				if (x < args->length) {
					c = args->body.rows[y].row + args->body.rows[y].length;
					if (c == args->body.rows[y+1].row) c = NULL;
				}
				else
					c = NULL;
			}
			
			if (y == -1) {
				args->yRow = 0;
				args->cRow = 0;
				args->cCharInRow = args->cChar;
			}
			else {
				x -= args->body.rows[y].length;
				args->cCharInRow = args->cChar - x;
				if (y > args->cRow) {
					// on augmente soit yRow, soit cRow si possible
					if (args->cRow == args->dRows-1) args->yRow = y - args->cRow;
					else args->cRow = y - args->yRow;
				}
				else {
					// si on a supprimmé une des premières lignes
					args->yRow = 0;
					args->cRow = y;
				}
				if (args->yRow && args->body.nRows < args->dRows) {
					// si on ajoute une des premières lignes
					args->yRow = 0;
					args->cRow = y;
				}
			}
		}
	}
	else {
		args->cCharInRow = 0;
		args->cChar = 0;
		args->cRow = 0;
		args->yRow = 0;
	}
	if (!args->isEditable && args->body.nRows <= args->dRows)
		w->isDynamic = 0;
}


void DrawingProcess(Widget *w)
{
	SDL_Surface *scr = w->construct->scr;
	wTHEME *theme = w->construct->theme;
	TextArgs *args = w->args;
	Uint32 c1 = theme->request_c1? theme->request_c1 : theme->color1;
	Uint32 c2 = theme->request_c2? theme->request_c2 : theme->color1;
	Uint32 c3 = theme->request_c3? theme->request_c3 : theme->color4;
	nSDL_Font *f = args->font? args->font : theme->font;
	if (args->isActive == 1) c1 = theme->text_c1? theme->text_c1 : theme->color1;
	
	if (args->isActive)
		DrawFillRectXY(scr, w->bounds.x+2, w->bounds.y+2, w->bounds.w-4, w->bounds.h-4, c1);
	else
		DrawFillRect(scr, &w->bounds, c2);
	
	if (args->scrollbar) {
		// dessin des scrollbars
		DrawVScrollBar(scr, theme, &w->bounds, 10*max(args->body.nRows, args->yRow + args->dRows)+6, 10*args->yRow);
		if (args->isActive < 2) {
			// on dessine en transparent
			SDL_Surface *s_im;
			s_im = SDL_CreateRGBSurface(SDL_SWSURFACE, w->bounds.w, w->bounds.h, 16, 0,0,0,0);
			DrawFillRect(s_im, NULL, args->isActive? c1 : c2);
			SDL_SetAlpha(s_im, SDL_SRCALPHA, 128);
			DrawSurface(s_im, NULL, scr, &w->bounds);
			SDL_FreeSurface(s_im);
			if (args->isActive) wDrawBounds(w);
		}
	}
	
	if (!args->isActive) DrawRectXY(scr, w->bounds.x+1, w->bounds.y+1, w->bounds.w-2, w->bounds.h-2, theme->color2);
	if (args->text[0])
		DrawTextBody(scr, f, w->bounds.x+3, w->bounds.y+4, &args->body, args->yRow, args->dRows);
	else if (args->voidStr)
		DrawClippedStr(scr, theme->request_f2, w->bounds.x+3, w->bounds.y+4, args->voidStr);
	
	
	// on dessine le curseur
	if (args->isActive == 2 && args->isEditable) {
		char *s = args->body.rows? args->body.rows[args->yRow + args->cRow].row : args->text;
		char c = *(s + args->cCharInRow);
		*(s + args->cCharInRow) = 0;
		int cw = nSDL_GetStringWidth(args->font? args->font:theme->font, s);
		*(s + args->cCharInRow) = c;
		DrawFillRectXY(scr, w->bounds.x+cw+3, w->bounds.y+3+10*args->cRow, 1, 9, c3);
	}
}


void DrawText(Widget *w)
{
	TextingProcess(w);
	DrawingProcess(w);
}




// Activation
int ActivateText(Widget *w)
{
	SDL_Surface *scr = w->construct->scr;
	TextArgs *args = w->args;
	args->isActive = 1;
	char c = 0;
	char k1=0, k2=0, k3=0, k4=0, k5=0, k6=0;
	BOOL ood = w->construct->onlyOneDynamic;
	int x = 0, y;
	char *s;
	int ok = ACTION_CONTINUE;
	
	if (ood) goto INLOOP;
	
	DrawingProcess(w);
	SDL_Flip(scr);
	
	while (!K_ESC()) {
		wait_key_pressed();
		
		
		if (K_UP() || K_DOWN() || K_LEFT() || K_RIGHT() || K_MENU() || K_TAB()
			|| K_SCRATCHPAD() || K_ENTER() || K_ESC()) {
				break;
			}
	
		else if (K_CLICK()) {
			
		  INLOOP:
			args->isActive = 2;
			if ((ok=wExecCallback(w, SIGNAL_CLICK)) != ACTION_CONTINUE)
				return ok;
			else
				TextingProcess(w);
			
			do {
				DrawingProcess(w);				
				SDL_Flip(scr);
				while (K_CLICK());
				
				// on chope une touche du keyboard
				if (k1 && (K_CTRL() || K_SHIFT()))
					while(K_getAlphaKey());
				if (k2 && (K_CTRL() || K_SHIFT()))
					while(K_getNumericKey());
				if (k3 && (K_CTRL() || K_SHIFT()))
					while(K_getPuncKey());
				if (k6 && (K_CTRL() || K_SHIFT()))
					while(K_PARAGRAPH());
				
				while (any_key_pressed() && !K_CTRL() && !K_SHIFT());
				while (!any_key_pressed());
				
				
				if ((K_ENTER() || K_CLICK())) {
					if ((ok=wExecCallback(w, SIGNAL_CLICK)) != ACTION_CONTINUE)
						return ok;
					
					if (ood) {
						if (K_ENTER()) goto QUIT;
					}
					
					else {
						TextingProcess(w);
						break;
					}
				}
				
				else if (K_MENU() || K_TAB() || K_DOC() || K_SCRATCHPAD() || K_ESC()) {
					if (ood) goto QUIT;
					break;
				}
				// on a chopé €-}
				
				
				k1 = K_getAlphaKey();
				k2 = K_getNumericKey();
				k3 = K_getPuncKey();
				k4 = K_DEL();
				k5 = K_PUNC() && K_CTRL();
				k6 = K_PARAGRAPH();
				
				
				
				if ((k1||k2||k3||k5||k6) && args->isEditable && args->length < args->maxChars) {
					if			(k1) c = k1;
					else if	(k2) c = k2;
					else if	(k3) c = k3;
					else if	(k5) c = 255;
					else if	(k6) c = 21;
					
					if (K_SHIFT())
						c = enshift(c);
					if (K_CTRL())
						c = enctrl(w->construct, c);
					
					if (c == ' ') {
						if (!args->cCharInRow) continue;
						if (args->cChar && args->text[args->cChar - 1] == ' ')
							continue;
						if (args->cChar < args->length && args->text[args->cChar] == ' ')
							goto RIGHT;
					}
					
					for (x = args->length-1; x >= args->cChar; x--)
						args->text[x+1] = args->text[x];
					args->text[args->cChar++]	= c;
					args->text[++args->length]	= 0;
					
					if ((ok=wExecCallback(w, SIGNAL_ACTION)) != ACTION_CONTINUE)
						return ok;
					TextingProcess(w);
				}
				
				
				else if (k4 && args->cChar && args->body.nRows) {
				if (!args->isEditable) continue;
				if (args->length > 1) {
						for (x = args->cChar-1; x < args->length; x++)
							args->text[x] = args->text[x+1];
					}
					else {
						args->text[0] = 0;
					}
					if (args->cChar) args->cChar--;
					args->length--;
					
					if (args->cChar && (args->text[args->cChar-1] == ' ' || args->text[args->cChar-1] == 21)) {
						// on enlève les espaces en trop
						while (args->text[args->cChar] == ' ') {
							for (x = args->cChar; x < args->length; x++)
								args->text[x] = args->text[x+1];
							args->length--;
						}
					}
					
					
					if ((ok=wExecCallback(w, SIGNAL_ACTION)) != ACTION_CONTINUE)
						return ok;
					TextingProcess(w);
				}
				
				
				else if (K_UP() && args->body.rows) {
					if (!args->isEditable && !args->yRow) continue;
					if (args->yRow + args->cRow == 0) {
						if (!args->isEditable) continue;
						// si on est à la première ligne, on va au premier caractère
						args->cChar = 0;
						args->cCharInRow = 0;
					}
					else {
						// sinon...
						if (!args->isEditable || !args->cRow)	args->yRow--;
						else												args->cRow--;
						x = args->body.rows[args->yRow + args->cRow].length;
						args->cChar -= x + 1;
						if (x < args->cCharInRow) {
							args->cChar -= args->cCharInRow - x;
							args->cCharInRow = x;
						}
						
						// on vérifie qu'on a bien modifié args->cChar
						y = args->yRow + args->cRow;
						s = args->body.rows[y].row + args->body.rows[y].length;
						if (s == args->body.rows[y+1].row) args->cChar++;
					}
				}
				
				
				else if (K_DOWN() && args->body.rows) {
					if (!args->isEditable && args->body.nRows <= args->dRows)
						continue;
					if (!args->isEditable && args->yRow + args->dRows >= args->body.nRows)
						continue;
					
					if (args->yRow + args->cRow == args->body.nRows - 1) {
						// si on est à la dernière ligne, on va au dernier caractère
						x = args->body.rows[args->yRow + args->cRow].length;
						args->cChar += x - args->cCharInRow;
						args->cCharInRow = x;
					}
					else {
						// sinon...
						x = args->body.rows[args->yRow + args->cRow].length;
						args->cChar += x + 1;
						if (!args->isEditable || args->cRow == args->dRows-1)	args->yRow++;
						else																	args->cRow++;
						x = args->body.rows[args->yRow + args->cRow].length;
						if (x < args->cCharInRow) {
							args->cChar -= args->cCharInRow - x;
							args->cCharInRow = x;
						}
						
						// on vérifie qu'on a bien modifié args->cChar
						y = args->yRow + args->cRow - 1;
						s = args->body.rows[y].row + args->body.rows[y].length;
						if (s == args->body.rows[y+1].row) args->cChar--;
					}
				}
				
				else if (K_LEFT() && args->body.rows) {
					if (!args->isEditable) continue;
					if (!args->cChar) continue;
					args->cChar--;
					if (!args->cCharInRow) {
						// 1. on vérifie qu'il faille bien modifier args->cChar
						y = args->yRow + args->cRow - 1;
						s = args->body.rows[y].row + args->body.rows[y].length;
						if (s == args->body.rows[y+1].row) args->cChar++;
						
						// 2. on change les lignes et la position du curseur dans la ligne
						if (!args->cRow)	args->yRow--;
						else					args->cRow--;
						args->cCharInRow = args->body.rows[args->yRow + args->cRow].length;
					}
					else
						args->cCharInRow--;
				}
				
				else if (K_RIGHT() && args->body.rows) {
				 RIGHT:
					if (!args->isEditable) continue;
					if (args->cChar == args->length) continue;
					args->cChar++; 
					if (args->cCharInRow == args->body.rows[args->yRow + args->cRow].length) {
						// 1. on vérifie qu'il faille bien modifier args->cChar
						y = args->yRow + args->cRow;
						s = args->body.rows[y].row + args->body.rows[y].length;
						if (s == args->body.rows[y+1].row) args->cChar--;
						
						// 2. on change les lignes et la position du curseur dans la ligne
						if (args->cRow == args->dRows-1)	args->yRow++;
						else										args->cRow++;
						args->cCharInRow = 0;
					}
					else
						args->cCharInRow++;
				}
				
				
				else if (!K_CTRL() && !K_SHIFT()) {
					if ((ok=wExecCallback(w, K_CLICK()||K_ENTER()? SIGNAL_CLICK : SIGNAL_KEY)) != ACTION_CONTINUE)
						return ok;
					TextingProcess(w);
					wait_no_key_pressed();
					continue;
				}
				
			} while (!K_ESC());
			
			args->isActive = 1;
			DrawingProcess(w);
			SDL_Flip(scr);
			while(K_ESC() || K_CLICK());
		}
		
		else if (any_key_pressed()) {
			if ((ok=wExecCallback(w, SIGNAL_KEY)) != ACTION_CONTINUE)
				return ok;
			DrawingProcess(w);
			SDL_Flip(scr);
			wait_no_key_pressed();
		}
	}
	
  QUIT:
	args->isActive = 0;
	return ok;
}


// Fermeture
void CloseText(Widget *w)
{
	if (!w) return;
	if (w->args) {
		TextArgs *args = w->args;
		if (args->text) free(args->text);
		if (args->body.rows) free(args->body.rows);
		if (args->font && wIsFreedArg(w, WFONT)) wAddFontToConstruct(w->construct, args->font);
	}
}


// Méthodes
void wText_SetVoidStr(Widget *w, const char *voidStr)
{
	if (!w) return;
	if (w->type != WIDGET_TEXT) return;
	TextArgs *args = w->args;
	args->voidStr = (char *) voidStr;
}

void wText_SetText(Widget *w, const char *t)
{
	if (!w) return;
	if (w->type != WIDGET_TEXT) return;
	TextArgs *args = w->args;
	
	if (!args->isEditable && strlen(t) > args->maxChars) {
		free(args->text);
		args->maxChars = strlen(t);
		args->text = malloc(args->maxChars+1);
		strcpy(args->text, t);
	}
	else if (t) strncpy(args->text, t, args->maxChars);
	else args->text[0] = 0;
}

char *wText_GetText(Widget *w)
{
	if (!w) return NULL;
	if (w->type != WIDGET_TEXT) return NULL;
	TextArgs *args = w->args;
	return args->text;
}

char *wText_GetAllocatedText(Widget *w)
{
	if (!w) return NULL;
	if (w->type != WIDGET_TEXT) return NULL;
	TextArgs *args = w->args;
	char *t = malloc(strlen(args->text)+1);
	if (!t) return NULL;
	strcpy(t, args->text);
	
	return t;
}

void wText_SetFont(Widget *w, nSDL_Font *f)
{
	if (!w) return;
	if (w->type != WIDGET_TEXT) return;
	TextArgs *args = w->args;
	args->font = f;
}

void wText_SetEditable(Widget *w)
{
	if (!w) return;
	if (w->type != WIDGET_TEXT) return;
	TextArgs *args = w->args;
	args->isEditable = 1;
	w->isDynamic = 1;
}

void wText_SetUnEditable(Widget *w)
{
	if (!w) return;
	if (w->type != WIDGET_TEXT) return;
	TextArgs *args = w->args;
	args->isEditable = 0;
}

BOOL wText_IsEditable(Widget *w)
{
	if (!w) return 0;
	if (w->type != WIDGET_TEXT) return 0;
	TextArgs *args = w->args;
	return args->isEditable;
}


void wText_SetMaxChars(Widget *w, int maxChars)
{
	if (!w) return;
	if (w->type != WIDGET_TEXT) return;
	TextArgs *args = w->args;
	
	char *newText = realloc(args->text, maxChars+1);
	if (!newText) return;
	
	newText[maxChars] = 0;
	args->text = newText;
}



