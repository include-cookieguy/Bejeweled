#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
using namespace std;

enum State { BACKGROUND, GEMS, CURSOR }; //kieu du lieu liet ke
const int sizeItem = 53; //kich thuoc cua item
const int itemsX = 8, itemsY = 8; //so luong item
const int sizeX = 740, sizeY = 480; //size of background(picture) cta se lay dung voi pixel cua anh
const int FPS = 60; //chi so khung hinh tren moi giay (60 khung hinh/s)
const int ANIMATION_DELAY = 600; //toc do cua gems

struct piece{
	int x, y, col, row, kind, match;
};
piece grid[itemsX + 2][itemsY + 2];

void swapGems(piece p1, piece p2){
	swap(p1.col, p2.col);
	swap(p1.row, p2.row);
	grid[p1.row][p1.col] = p1;
	grid[p2.row][p2.col] = p2;
}
string convertToString(int x){
    string res = "";
    if (x == 0)
        res = "0";
    else{
        while (x != 0){
            res = res + (char) ('0' + x % 10);
            x /= 10;
        }
    }
    reverse(res.begin(), res.end());
    return res;
}
int main(int argc, char **argv){
	srand(time(0));
	///Error checks
	cout << "SDL_Init\n";
	SDL_Init(SDL_INIT_EVERYTHING); //khoi tao sdl

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); /*(tan so, output format, 2 la am thanh stereo(dung de nghe nhac)
                                                        - 1 la am thanh mono(tot hon cho xem phim), bytes dc su dung cho moi ouput */
	Mix_Music *backgroundSound = Mix_LoadMUS("music.mp3"); //Mix_Music tao 1 bien con tro de luu music, truyen music.mp3 vao bien con tro vua dc khoi tao
    Mix_PlayMusic(backgroundSound, -1); //(con tro da dc khoi tao de luu music, loops = -1 la vong lap vo han de phat nhac)

    TTF_Init();
    TTF_Font *font;
    font = TTF_OpenFont("TIMES.TTF",500);

	SDL_Window *win = SDL_CreateWindow("Bejeweled", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, sizeX, sizeY, 0); /*SDL_Window cung giong nhu Mix_Music, SDL_CreateWindow
	de tao 1 cua so voi vi tri, kich thuoc nhu sau (ten cua cua so, vi tri x, vi tri y, chieu rong cua cua so, 0 la man hinh bt, SDL_WINDOW_MAXimized la full screan, minimize la min screen) */
    //kiem tra xem cua so dc khoi tao chua
	if (win == NULL) {
		cout << "SDL_CreateWindow error\n";// trong truong hop cua so chua dc khoi tao
	}
	// renderer la thu dung de ve, ban than SDL_Window k the ve dc, phai gan 1 cai renderer vao, roi ve bang renderer nay

	SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED); /*  (SDL_Window *win(chinh la bien con tro win
    vua tao, index, flags) */
	if (renderer == NULL) {
		cout << "SDL_CreateRenderer error\n";
	}
	///Load bitmaps
	vector<string> Bitmaps;
	Bitmaps.push_back("img/background.png");
	Bitmaps.push_back("img/gems.png");
	Bitmaps.push_back("img/cursor.png");
	///Create textures from bitmaps
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)){
		cout << "SDL_image could not initialize! SDL_image Error:" << IMG_GetError() << endl;
	}
	vector<SDL_Texture *> Textures;
	for (auto bitmap : Bitmaps) {
		Textures.push_back(IMG_LoadTexture(renderer, bitmap.c_str()));
		if (Textures.back() == NULL){
			cout << bitmap.c_str() << " SDL_CreateTextureFromSurface error\n";
		}
		else
			cout << bitmap.c_str() << " SDL_CreateTextureFromSurface OK\n";
	}
	///Init start board
	PlayAgain:
	    {
	for (int i = 1; i <= itemsX; i++){
		for (int j = 1; j <= itemsY; j++){
			grid[i][j].kind = rand() % 7; //ngau nhien 1 loai gems vao 1 o
			grid[i][j].col = j; //cot cua gems
			grid[i][j].row = i; //dong cua gems
			grid[i][j].x = j * sizeItem; // toa do x cua gems
			grid[i][j].y = i * sizeItem; //toa do y cua gems
		}
	}
	int x0 = 0, y0 = 0, x, y;
	int click = 0, moveRemain = 5, limitedScore = 15, currentScore = 0;
	SDL_Point pos;
	bool isSwap = false, isMoving = false;
	///Loop game
	while (true) {
		SDL_Point offset = { 52, 24 }; //offset la khoang cach cua cac gems de fit voi bang
		SDL_Event e;
		if (SDL_PollEvent(&e)){
			if (e.type == SDL_QUIT) {
				break;
			}
			if (e.button.button == SDL_BUTTON_LEFT) {
				if (!isSwap && !isMoving) click++;
				pos.x = e.motion.x - offset.x;
				pos.y = e.motion.y - offset.y;
			}
		}
		///Mouse click
		if (click == 1)	{
			x0 = pos.x / sizeItem + 1;
			y0 = pos.y / sizeItem + 1;
		}
		if (click == 2)	{
			x = pos.x / sizeItem + 1;
			y = pos.y / sizeItem + 1;
			if (abs(x - x0) + abs(y - y0) == 1)	{
				swapGems(grid[y0][x0], grid[y][x]);
				isSwap = 1;
				click = 0;
				moveRemain--;
			}
			else
				click = 1;
		}
		///Match finding
		for (int i = 1; i <= itemsX; i++){
			for (int j = 1; j <= itemsY; j++){
				if (grid[i][j].kind == grid[i + 1][j].kind)
					if (grid[i][j].kind == grid[i - 1][j].kind)
						for (int n = -1; n <= 1; n++)
							grid[i + n][j].match = 1;
				if (grid[i][j].kind == grid[i][j + 1].kind)
					if (grid[i][j].kind == grid[i][j - 1].kind)
						for (int n = -1; n <= 1; n++)
							grid[i][j + n].match = 1;
			}
		}
		///Moving animation
		isMoving = false;
		for (int i = 1; i <= itemsX; i++)
			for (int j = 1; j <= itemsY; j++) {
				piece &p = grid[i][j];
				int dx = p.x - p.col*sizeItem;
				int dy = p.y - p.row*sizeItem;
				if (dx) p.x -= dx / abs(dx);
				if (dy) p.y -= dy / abs(dy);
				if (dx || dy) isMoving = true;
			}
		///Get score
		int score = 0;
		for (int i = 1; i <= itemsX; i++)
			for (int j = 1; j <= itemsY; j++)
				score += grid[i][j].match;
		///Second swap if no match
		if (!isMoving)
            currentScore += score;
		if (isSwap && !isMoving){
			if (!score)
				swapGems(grid[y0][x0], grid[y][x]);
			isSwap = 0;
		}
		///Update grid
		if (!isMoving){
			for (int i = itemsX; i>0; i--)
				for (int j = 1; j <= itemsY; j++)
					if (grid[i][j].match)
						for (int n = i; n>0; n--)
							if (!grid[n][j].match){
								swapGems(grid[n][j], grid[i][j]);
								break;
							}
			for (int j = 1; j <= itemsX; j++)
				for (int i = itemsY, n = 0; i>0; i--)
					if (grid[i][j].match){
						grid[i][j].kind = rand() % 7;
						grid[i][j].y = -sizeItem*n++;
						grid[i][j].match = 0;
					}
        }
		SDL_RenderClear(renderer);
		///Board texture
		SDL_Rect boardRect = { 0, 0, sizeX, sizeY };
        SDL_RenderCopy(renderer, Textures[BACKGROUND], nullptr, &boardRect);
        ///Gems
		for (int i = 1; i <= itemsX; i++)
			for (int j = 1; j <= itemsY; j++){
				piece p = grid[i][j];
				SDL_Rect rectGem = { p.x, p.y - offset.y, 49, 49 };
				SDL_Rect rectSprite = {p.kind * 49, 0, 49, 49 };
				SDL_RenderCopy(renderer, Textures[GEMS], &rectSprite, &rectGem);
		}
		///Cursor
		SDL_Rect rectCursor = { x0*sizeItem, y0*sizeItem - offset.y, 49, 49 };
		SDL_RenderCopy(renderer, Textures[CURSOR], nullptr, &rectCursor);
        ///Display moves and scores
        string moveAndScore;
        moveAndScore = "Moves:" + convertToString(moveRemain) + "     Score:" + convertToString(currentScore);
        SDL_Surface *temp;
        SDL_Texture *text;
        SDL_Color color = {0, 0, 255, 255};
        temp = TTF_RenderText_Solid(font, moveAndScore.c_str(), color);
        SDL_Rect r = {500, 350, 10 * moveAndScore.size() ,100};
        text = SDL_CreateTextureFromSurface(renderer, temp);
        SDL_RenderCopy(renderer, text, nullptr, &r);
        SDL_FreeSurface(temp);
        SDL_DestroyTexture(text);
        SDL_RenderPresent(renderer);
        ///Game result
        if (moveRemain == 0){
            SDL_Delay(500);
            r = {0, 0, sizeX, sizeY};
            if (currentScore >= limitedScore){
                temp = IMG_Load("img/win.jpg");
                text = SDL_CreateTextureFromSurface(renderer, temp);
                SDL_RenderCopy(renderer, text, NULL, &r);
            }
            else{
                temp = IMG_Load("img/lose.jpg");
                text = SDL_CreateTextureFromSurface(renderer, temp);
                SDL_RenderCopy(renderer, text, NULL, &r);
            }
            ///Display final score
            string finalScore;
            finalScore = "Your score: " + convertToString(currentScore);
            SDL_Color color = {0, 0, 255, 255};
            temp = TTF_RenderText_Solid(font, finalScore.c_str(), color);
            SDL_Rect r = {200, 250, 25 * finalScore.size(), 100};
            text = SDL_CreateTextureFromSurface(renderer, temp);
            SDL_RenderCopy(renderer, text, nullptr, &r);
            ///Display play again
            string playAgain;
            playAgain = "Play again?   YES   NO";
            temp = TTF_RenderText_Solid(font, playAgain.c_str(), color);
            SDL_Rect r1 = {150, 350, 20 * playAgain.size(), 100};
            text = SDL_CreateTextureFromSurface(renderer, temp);
            SDL_RenderCopy(renderer, text, nullptr, &r1);
            SDL_FreeSurface(temp);
            SDL_DestroyTexture(text);
            SDL_RenderPresent(renderer);
            while (true)
                {
                    SDL_Event e;
                    if (SDL_PollEvent(&e))
                    {
                        if(e.type == SDL_KEYDOWN)
                        {
                            if (e.key.keysym.sym == SDLK_y)
                            {
                                currentScore = 0;
                                moveRemain = 5;
                                goto PlayAgain;
                            }
                            else if (e.key.keysym.sym == SDLK_n)
                            {
                                SDL_DestroyRenderer(renderer);
                                SDL_DestroyWindow(win);
                                SDL_Quit();
                                break;
                            }
                        }
                    }
                }
            break;
        }
        ///Animation delay
        int start = SDL_GetTicks(); // ham sdl_getticks de lay so mili s tu khi khoi tao sdl
		if (ANIMATION_DELAY / FPS > SDL_GetTicks() - start)
			SDL_Delay(ANIMATION_DELAY / FPS - (SDL_GetTicks() - start));
	}
	}
	///Quit game
	while (true){
        SDL_Event e;
        	if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				break;
			}
        }
    }
	IMG_Quit();
	for (auto texture : Textures)
		SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	Mix_FreeMusic(backgroundSound);
	Mix_CloseAudio();
	SDL_Quit();
	cout << "SDL_Quit\n";

	return 0;
}
