#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <algorithm>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;
const int NO_RC = 3; //No of rows/columns
const int BUTTON_WIDTH = SCREEN_WIDTH / 3;
const int BUTTON_HEIGHT = SCREEN_HEIGHT / 3;
const int TOTAL_BUTTONS = 9;

enum GWinLine
{
	ROW_ONE,
	ROW_TWO,
	ROW_THREE,
	COL_ONE,
	COL_TWO,
	COL_THREE,
	DIAG,
	ANTI_DIAG,
	NO_WIN
};

enum LButtonState
{
	BUTTON_STATE_MOUSE_OUT = 0,
	BUTTON_STATE_MOUSE_OVER_MOTION = 1,
	BUTTON_STATE_MOUSE_DOWN = 2,
	BUTTON_STATE_MOUSE_UP = 3,
	//BUTTON_STATE_AI_SET = 4,
	BUTTON_STATE_TOTAL = 4
};

class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		// LTexture operator=(LTexture &Texture );

		//Loads image at specified path
		bool loadFromFile( std::string path );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );

		//Deallocates texture
		void free();
		
		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL );

		void render( int x1, int y1, int x2, int y2);

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

class LButton
{
	public:

		static int turn;

		//constructor
		LButton();

		//Destructor
		// ~LButton();

		//Set topleftposition for the button
		void setPosition(int x, int y);

		//Handle mouse movement
		void handleEvent(SDL_Event* e);

		//rendering
		void render();

		static void changeTurn();

		bool isbuttonset();

		void setbutton( LButtonState state);

		void reset();

	private:
		//Top left position
		SDL_Point mPosition;

		LButtonState mCurrentState;

		LTexture* mTextures[2];

		int mButtonValue;

		bool mButtonState;

		// //Currently used global sprite
		// LButtonSprite mCurrentSprite;
};

class LBoard
{
	private:
		int turn;
		int board[NO_RC][NO_RC];
		GWinLine mWinLine;
		int moves;

	public:
		LBoard();

		GWinLine checkgame(int x, int y, int turn);

		int getturn();
		
		void changeturn();

		void handleEvent( SDL_Event* e);

		bool isnotposs();

		void render();

		void playgame();

		int minimax( int row, int col, bool player );

		void reset();
		
};

int LButton::turn = 0;

bool init();

bool loadMedia();

void close();

SDL_Texture* loadTexture( std::string path);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

LTexture gGrid;
LTexture gX;
LTexture gO;

LBoard gBoard;

LButton gButtons[ TOTAL_BUTTONS ];

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL ){
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else{
		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::render( int x, int y, SDL_Rect* clip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
		SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0xFF );        
    	SDL_RenderDrawRect( gRenderer, &renderQuad );
	}
	
	else{
	//Render to screen
		SDL_RenderCopy( gRenderer, mTexture, clip, &renderQuad );
	}
}

void LTexture::render( int x1, int y1, int x2, int y2 )
{
	SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0xFF, 0xFF);
	SDL_RenderDrawLine( gRenderer, x1, y1, x2, y2);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

LButton::LButton()
{
	mPosition.x = 0;
	mPosition.y = 0;
	mCurrentState = BUTTON_STATE_MOUSE_OUT;
	mTextures[ 0 ] = &gX;
	mTextures[ 1 ] = &gO;
	mButtonState = false;
}

void LButton::reset()
{
	mCurrentState = BUTTON_STATE_MOUSE_OUT;
	mButtonState = false;
}

void LButton::setPosition(int x, int y){
	mPosition.x = x;
	mPosition.y = y;
}

void LButton::handleEvent(SDL_Event* e){

	if( e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP )
	{
		//Get mouse position
		int x, y;
		SDL_GetMouseState( &x, &y );

		//Check if mouse is in button
		bool inside = true;

		// printf("Pos- x- %f, y- %f", x, y);

		//Mouse is left of the button
		if( x < mPosition.x )
		{
			inside = false;
		}
		//Mouse is right of the button
		else if( x > mPosition.x + BUTTON_WIDTH )
		{
			inside = false;
		}
		//Mouse above the button
		else if( y < mPosition.y )
		{
			inside = false;
		}
		//Mouse below the button
		else if( y > mPosition.y + BUTTON_HEIGHT )
		{
			inside = false;
		}

		if( !inside && mCurrentState != BUTTON_STATE_MOUSE_UP ){
			mCurrentState = BUTTON_STATE_MOUSE_OUT;
		}

		else if( mCurrentState != BUTTON_STATE_MOUSE_UP ){
			
			switch(e->type){

				case SDL_MOUSEMOTION:
					mCurrentState = BUTTON_STATE_MOUSE_OVER_MOTION;
					break;

				case SDL_MOUSEBUTTONDOWN:
					mCurrentState = BUTTON_STATE_MOUSE_DOWN;
					mButtonValue = turn;
					break;

				case SDL_MOUSEBUTTONUP:
					// mCurrentState = BUTTON_STATE_MOUSE_UP;
					// mButtonValue = turn;
					// printf("ButtonPosition- x- %d, y- %d", mPosition.x, mPosition.y );
					// mButtonState = true;
					// LButton::changeTurn();
					setbutton( BUTTON_STATE_MOUSE_UP );
					break;

			}

		}
	}
}

void LButton::setbutton( LButtonState state )
{
	mCurrentState = state;
	mButtonValue = turn;
	mButtonState = true;
	LButton::changeTurn();
}

void LButton::render()
{
	//Show current button sprite
	SDL_Rect outlineRect = { mPosition.x, mPosition.y, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 3 };
	//Uint8 a = 255;

	switch( mCurrentState ){

		case BUTTON_STATE_MOUSE_OVER_MOTION:
			gGrid.render( mPosition.x, mPosition.y, &outlineRect );
			break;

		case BUTTON_STATE_MOUSE_DOWN:
			//a -= 64;
			//gX.setAlpha(a);
			mTextures[ mButtonValue ]->render(mPosition.x, mPosition.y );
			// gX.render( mPosition.x, mPosition.y, &outlineRect );
			break;

		case BUTTON_STATE_MOUSE_UP:
			mTextures[ mButtonValue ]->render(mPosition.x, mPosition.y );
			mTextures[ mButtonValue ]->render( mPosition.x, mPosition.y, &outlineRect );
			break;

		// case BUTTON_STATE_AI_SET:
		// 	mTextures[ mButtonValue ]->render(mPosition.x, mPosition.y );
		// 	mTextures[ mButtonValue ]->render( mPosition.x, mPosition.y, &outlineRect );
		// 	break;

		default:
			break;

	}
}

void LButton::changeTurn()
{
	LButton::turn = (LButton::turn == 0) ? 1 : 0;
}

bool LButton::isbuttonset()
{
	return (mButtonState);
}

LBoard::LBoard()
{
	moves = 0;
	turn = 0;
	mWinLine = NO_WIN;
	for (int i = 0; i < NO_RC; i++)
	{
		for (int j = 0; j < NO_RC; j++)
		{
			/* code */
			board[i][j] = -1;
		}
	}
}

void LBoard::reset()
{
	moves = 0;
	turn = 0;
	mWinLine = NO_WIN;
	for (int i = 0; i < NO_RC; i++)
	{
		for (int j = 0; j < NO_RC; j++)
		{
			/* code */
			board[i][j] = -1;
		}
	}
}

GWinLine LBoard::checkgame(int x, int y, int turn)
{
	board[ x ][ y ] = turn;
	//check row			
	for (int i = 0; i < NO_RC; i++)
	{
		/* code */
		if(board[x][i] != turn)
			break;
		if(i == NO_RC - 1)
		{	
			if( x == 0)
				return COL_ONE;
			else if( x == 1)
				return COL_TWO;
			else
				return COL_THREE;
		}					
	}

	//check column
	for (int i = 0; i < NO_RC; i++)
	{
		/* code */
		if(board[i][y] != turn)
			break;
		if(i == NO_RC - 1)
		{
			if( y == 0)
				return ROW_ONE;
			else if( y == 1)
				return ROW_TWO;
			else
				return ROW_THREE;
		}
	}

	if(x == y)
	{
		for (int i = 0; i < NO_RC; i++)
		{
			/* code */
			if(board[i][i] != turn)
				break;
			if(i == NO_RC - 1)
				return DIAG;
		}
	}

	if( x+y == NO_RC  - 1 )
	{
		for (int i = 0; i < NO_RC; i++)
		{
			/* code */
			if(board[ NO_RC - i - 1 ][ i ] != turn)
				break;
			if(i == NO_RC - 1)
				return ANTI_DIAG;
		}
	}
	return NO_WIN;			
}

int LBoard::getturn()
{
	return turn;
}

void LBoard::changeturn()
{
	turn = (turn==0) ? 1 : 0; 
}

bool LBoard::isnotposs()
{
	if( mWinLine != NO_WIN || moves == 9 )
		return true;

	return false;
}

void LBoard::render(){
	int x1, y1, x2, y2;

	switch (mWinLine)
	{
	case ROW_ONE:
		x1 = 0;
		y1 = 0 + SCREEN_WIDTH / 6;
		x2 = SCREEN_HEIGHT;
		y2 = 0 + SCREEN_WIDTH / 6;
		break;
	
	case ROW_TWO:
		x1 = 0;
		y1 = SCREEN_WIDTH / 3 + SCREEN_WIDTH / 6;
		x2 = SCREEN_HEIGHT;
		y2 = SCREEN_WIDTH / 3 + SCREEN_WIDTH / 6;
		break;
	
	case ROW_THREE:
		x1 = 0;
		y1 = SCREEN_WIDTH * 2 / 3 + SCREEN_WIDTH / 6;
		x2 = SCREEN_HEIGHT;
		y2 = SCREEN_WIDTH * 2 / 3 + SCREEN_WIDTH / 6;
		break;

	case COL_ONE:
		x1 = 0 + SCREEN_HEIGHT / 6;
		y1 = 0;
		x2 = 0 + SCREEN_HEIGHT / 6;
		y2 = SCREEN_WIDTH;
		break;
	
	case COL_TWO:
		x1 = SCREEN_HEIGHT / 3 + SCREEN_HEIGHT / 6;
		y1 = 0;
		x2 = SCREEN_HEIGHT / 3 + SCREEN_HEIGHT / 6;
		y2 = SCREEN_WIDTH;
		break;
	
	case COL_THREE:
		x1 = SCREEN_HEIGHT * 2 / 3 + SCREEN_HEIGHT / 6;
		y1 = 0;
		x2 = SCREEN_HEIGHT * 2 / 3 + SCREEN_HEIGHT / 6;
		y2 = SCREEN_WIDTH;
		break;
	
	case DIAG:
		x1 = 0;
		y1 = 0;
		x2 = SCREEN_HEIGHT;
		y2 = SCREEN_WIDTH;
		break;
	
	case ANTI_DIAG:
		x1 = 0;
		y1 = SCREEN_WIDTH;
		x2 = SCREEN_HEIGHT;
		y2 = 0;
		break;
	
	default:
		return;
		break;
	}

	gGrid.render( x1, y1, x2, y2);
}

void LBoard::handleEvent( SDL_Event* e)
{
	if( e->type == SDL_MOUSEBUTTONUP )
	{
		//Get mouse position
		int x, y;
		SDL_GetMouseState( &x, &y );

		if(x < SCREEN_HEIGHT / 3)
			x = 0;
		else if ( x < SCREEN_HEIGHT * 2 / 3 )
			x = 1;
		else
			x = 2;
		
		if(y < SCREEN_WIDTH / 3)
			y = 0;
		else if(y < SCREEN_WIDTH *2 / 3)
			y = 1;
		else
			y = 2;		
		
		if( !gButtons[ (y * 3) + x ].isbuttonset() )
		{
			// printf("x- %d, y- %d \n", x, y);
			moves++;
			mWinLine = checkgame( x, y, turn);
			// printf("mWinLine- %d \n", mWinLine);
			changeturn();
		}

	}
}

int LBoard::minimax( int row, int col, bool player )
{
	if( checkgame( row, col, player ) != NO_WIN )
	{
		board[ row ][ col ] = -1;
		if( !player )
			return -1;
		else 
			return 1;
	}

	int ret = 0;

	int flag = 1;

	// printf(" board- %d\n", board[row][col]);

	for( int curr_row = 0; curr_row < NO_RC; curr_row++)
	{
		for( int curr_col = 0; curr_col < NO_RC; curr_col++)
		{
			if( board[ curr_row ][ curr_col ] == -1 )
			{
				int minimaxval = minimax( curr_row, curr_col, !player );
				if( !player )
				{
					if(flag)
					{
						ret = minimaxval;
						flag = 0;
					}
					else
						ret = std::max( ret, minimaxval );
				}

				else
				{
					if(flag)
					{
						ret = minimaxval;
						flag = 0;
					}
					else
						ret = std::min( ret, minimaxval );
				}
				// if(row == 0 && col == 0 )
					// printf("ret- %d\n", ret);
			}				
		}
	}

	
	board[ row ][ col ] = -1;

	return ret;

}

void LBoard::playgame()
{
	bool player = true;

	int mvalue = -2	;

	int row = -1, col = -1;

	for( int curr_row = 0; curr_row < NO_RC; curr_row++)
	{
		for( int curr_col = 0; curr_col < NO_RC; curr_col++)
		{
			if( board[ curr_row ][ curr_col ] == -1 )
			{
				int curr_val = minimax( curr_row, curr_col, player );
				// printf("i- %d j- %d, curr_val- %d\n", curr_row, curr_col, curr_val);
				if(  curr_val > mvalue )
				{
					mvalue = curr_val;
					row = curr_row;
					col = curr_col;
				}
			}
		}
	}

	if( row != -1)
	{
		moves++;
		mWinLine =  checkgame( row, col, player );
		gButtons[ col*3 + row ].setbutton( BUTTON_STATE_MOUSE_UP );
		changeturn();
	}
}

bool init()
{
	bool success = true;
	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("Unable to initialize SDL, Error- %s\n", SDL_GetError());
		success = false;
	}
	else{
		gWindow = SDL_CreateWindow("Tic-Tac-Toe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL){
			printf("Unable to create window. Error- %s\n", SDL_GetError());
			success = false;
		}
		else{
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if( gRenderer == NULL ){
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else{
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) ){
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	bool success = true;

	if( !gGrid.loadFromFile( "grid.png" ) ){
		printf( "Failed to load sprite sheet texture!\n" );
		success = false;
	}

	if( !gX.loadFromFile( "x.png" ) ){
		printf( "Failed to load sprite sheet texture!\n" );
		success = false;
	}

	if( !gO.loadFromFile( "o.png" ) ){
		printf( "Failed to load sprite sheet texture!\n" );
		success = false;
	}
	if( success ){
		gButtons[ 0 ].setPosition( 0, 0 );
		gButtons[ 1 ].setPosition( SCREEN_WIDTH / 3 , 0);
		gButtons[ 2 ].setPosition( SCREEN_WIDTH * 2 / 3, 0 );
		gButtons[ 3 ].setPosition( 0, SCREEN_HEIGHT / 3);
		gButtons[ 4 ].setPosition( SCREEN_HEIGHT / 3, SCREEN_WIDTH / 3 );
		gButtons[ 5 ].setPosition( SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT / 3 );
		gButtons[ 6 ].setPosition( 0, SCREEN_HEIGHT * 2 / 3 );
		gButtons[ 7 ].setPosition( SCREEN_WIDTH / 3, SCREEN_HEIGHT * 2 / 3 );
		gButtons[ 8 ].setPosition( SCREEN_HEIGHT * 2 / 3, SCREEN_WIDTH * 2 / 3 );
	}

	return success;
}

void close()
{
	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

void reset()
{
	gBoard.reset();

	for( int i = 0; i < TOTAL_BUTTONS; ++i )
	{
		gButtons[ i ].reset();
	}
	
	LButton::turn = 0;
}
int main(int argc, char* args[])
{
	if(!init()){
		printf("Failed to initialize SDL!\n");
	}
	else{
		if(!loadMedia()){
			printf("Failed to load media!\n");
		}

		else{
			bool quit = false;
			SDL_Event e;

			while(!quit){

				

				if( gBoard.getturn() )
				{
					gBoard.playgame();
				}

				else
				{
					while(SDL_PollEvent(&e) != 0)
					{
						if(e.type == SDL_QUIT )
						{
							quit = true;
						}

						const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

						if( ( currentKeyStates[ SDL_SCANCODE_RCTRL ] || currentKeyStates[ SDL_SCANCODE_LCTRL ] ) && currentKeyStates[ SDL_SCANCODE_R ] )
						{
							reset();
						}
						//Handle button events
						gBoard.handleEvent( &e );

						for( int i = 0; i < TOTAL_BUTTONS; ++i )
						{
							gButtons[ i ].handleEvent( &e );
						}
					
					}

				}

				//SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

				gGrid.render(0, 0);

				//Render buttons
				for( int i = 0; i < TOTAL_BUTTONS; ++i )
				{
					gButtons[ i ].render();
				}
				
				gBoard.render();
				
				SDL_RenderPresent( gRenderer );

				// while(gBoard.iswin());
				if(gBoard.isnotposs())
				{	
					// printf("Enter\n");
					while( !quit )
					{
						while(SDL_PollEvent(&e) != 0)
						{
							if(e.type == SDL_QUIT )
								quit = true;

							const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

							if( ( currentKeyStates[ SDL_SCANCODE_RCTRL ] || currentKeyStates[ SDL_SCANCODE_LCTRL ] ) && currentKeyStates[ SDL_SCANCODE_R ] )
							{
								reset();
								break;
							}
						}
						quit = true;
					}
					quit  = false;
				}
			}
		}	
	}

	close();

	return 0;
}