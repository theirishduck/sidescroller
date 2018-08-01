// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN

#include <windows.h>   // include all the windows headers
#include <windowsx.h>  // include useful macros
#include <mmsystem.h>  // for DirectDraw
#include <stdlib.h>    // for rand functions
#include <stdio.h>     
#include <math.h>

#include <ddraw.h>     // include DirectDraw
#include "ddutil.h"    // for DDLoadBitmap

#include "Megaman4.h"

// DEFINES ////////////////////////////////////////////////

// defines for windows

#define WINDOW_CLASS_NAME "WINCLASS1"

#define WINDOW_WIDTH        800   // size of game window
#define WINDOW_HEIGHT       600
#define WINDOW_BPP           16   // bits per pixel

#define GAME_SPEED           30   // speed of game (increase to go slower)

#define MEG_SPEED             6   // x speed of Megaman animation
#define MEG_DELAY             3   // delay between animation frames

#define JUMP_SPEED           18   // initial y speed of jump
#define GRAVITY_FRICTION      1   // gravity slow down or acceleration for jumps

#define TILE_SKY              0
#define TILE_CLOUD            1
#define TILE_HILL             2
#define TILE_GROUND           3

#define MAP_WIDTH            75  // total tiles horizontal width
#define MAP_HEIGHT           12  // total tiles vertical height

#define SCREEN_WIDTH         17  // screen tiles horizontal width (cols)
#define SCREEN_HEIGHT        12  // screen tiles vertical height  (rows)

#define XTOP_LEFT           -50  // top x,y corner of screen display
#define YTOP_LEFT             0
#define TILE_SIZE            50  // each tile is 50 x 50

// MACROS /////////////////////////////////////////////////

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// GLOBALS ////////////////////////////////////////////////

HWND game_window = NULL;           // global game window handle
HINSTANCE game_instance = NULL;    // global game instance handle

// DirectDraw variables

LPDIRECTDRAW          game_draw_main         = NULL;   // dd main object
LPDIRECTDRAWSURFACE   game_draw_surface      = NULL;   // dd primary surface
LPDIRECTDRAWSURFACE   game_draw_back         = NULL;   // dd back surface
DDSURFACEDESC         game_draw_desc;                  // dd surface description
DDSCAPS               game_draw_caps;                  // dd surface capabilities
LPDIRECTDRAWCLIPPER   game_draw_clip;                  // dd surface clipper

DDBLTFX back_fill;   // to clear back buffer to black

bool draw_ok;        // for whether DirectDraw initialized ok

int xbmp, ybmp;      // BMP x and y position
int xlimit, ylimit;  // BMP limits for x and y
int xmove, ymove;    // x and y vector movement

int xmid;            // x midpoint position where to begin scrolling
int xoffset;         // offset position for tile 0 drawn at pixel 0 + offset
int xtile;           // current x (column) position of tile map

bool do_scroll;      // for whether reached midpoint and scrolling
                     
int meg_pos;         // animation position (0 to 9)
int meg_dir;         // facing direction (0 or 10)
int meg_countdown;   // for countdown delay between frames

// for bitmap animations and backgrounds, bmp dd surfaces

LPDIRECTDRAWSURFACE hbmp[20];
LPDIRECTDRAWSURFACE hsky;
LPDIRECTDRAWSURFACE hcloud;
LPDIRECTDRAWSURFACE hhill;
LPDIRECTDRAWSURFACE hground;

RECT bmp_rect  = {0, 0, 53, 48};                // bmp anim rect is 53 x 48
RECT tile_rect = {0, 0, TILE_SIZE, TILE_SIZE};  // tile background rect
RECT temp_rect;                                 // temp rect for blitting

int Map_Level[MAP_HEIGHT][MAP_WIDTH] =

// SAMPLE MAP KEY: 0=SKY 1=CLOUD 2=HILL 3=GROUND

{
   { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
   { 0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,1,0,0,0 },  
   { 0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0 },
   { 0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0 },
   { 0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0 },
   { 0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0 },
   { 0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0 },
   { 0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0 },
   { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
   { 0,0,2,0,0,0,0,2,0,0,2,0,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,2,0,2,0,0,0,2,0,0,0,0,0,2,0,0,2,0,0,0,0,2,0,2,0,0,0,0,2,0,0,0,0,0,2,0,0,0,0,2,0,0,0,2,0,2,0,0,0 },
   { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 },
   { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 }
};



// FUNCTIONS ////////////////////////////////////////////////////////////

bool GameInit();
void GameMain();
void GameQuit();
void ScrollLeft();
void ScrollRight();

// WINPROC /////////////////////////////////////////////////////////////

LRESULT CALLBACK WinProc(HWND hwnd, 
                         UINT msg, 
                         WPARAM wparam, 
                         LPARAM lparam)
{  // this is the main message handler of the system

   HDC         hdc; // handle to a device context
   PAINTSTRUCT ps;  // used in WM_PAINT

   switch(msg) // what is the message
   {    
      case WM_CREATE: 
      {
         // do initialization stuff here
                
         return(0); // return success
      } break;

      case WM_PAINT: 
      {
         hdc = BeginPaint(hwnd, &ps); // validate the window
                  
         EndPaint(hwnd, &ps);

         return(0); // return success
      }  break;

      case WM_DESTROY: 
      {
         PostQuitMessage(0); // kill the application, sends a WM_QUIT message 
         
         return(0); // return success
      }  break;

      default:break;

   } // end switch

// process default messages 

return (DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

// WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hinstance,
                   HINSTANCE hprevinstance,
                   LPSTR lpcmdline,
                   int ncmdshow)
{

   WNDCLASSEX winclass; // this will hold the class we create
   HWND hwnd;           // generic window handle
   MSG msg;             // generic message
   
   // first fill in the window class structure

   winclass.cbSize        = sizeof(WNDCLASSEX);
   winclass.style         = CS_DBLCLKS | CS_OWNDC | 
                            CS_HREDRAW | CS_VREDRAW;
   winclass.lpfnWndProc   = WinProc;
   winclass.cbClsExtra    = 0;
   winclass.cbWndExtra    = 0;
   winclass.hInstance     = hinstance;
   winclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
   winclass.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);   
   winclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
   winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
   winclass.lpszMenuName  = NULL;
   winclass.lpszClassName = WINDOW_CLASS_NAME;
   
   // save the game instance handle
   game_instance = hinstance;
   
   // register the window class
   if (!RegisterClassEx(&winclass)) return(0);

   // create the window
   if (!(hwnd = CreateWindowEx(NULL,                     // extended style
                               WINDOW_CLASS_NAME,        // class
                               "Sidescroller example by Campbell Craig",  // title
                               WS_POPUP | WS_VISIBLE,    // use POPUP for full screen
                               0,0,           // initial game window x,y
                               WINDOW_WIDTH,  // initial game width
                               WINDOW_HEIGHT, // initial game height
                               NULL,          // handle to parent 
                               NULL,          // handle to menu
                               hinstance,     // instance of this application
                               NULL)))        // extra creation parms
   return(0);
   
   // save the game window handle
   game_window = hwnd;    
      
   draw_ok = GameInit();   // game initialization function called here
   if (!draw_ok) SendMessage (hwnd, WM_CLOSE, 0, 0); // ddraw error so close/quit
         
   // enter main event loop using PeekMessage() to retrieve messages
   
   while(TRUE)
   {
      
      // get initial tick count to keep game speed constant
      DWORD start_tick = GetTickCount();
            
      // is there a message in queue, if so get it
      if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
      { 
         // test if this is a quit
         if (msg.message == WM_QUIT) break;
          
         // translate any accelerator keys
         TranslateMessage(&msg);

         // send the message to WinProc
         DispatchMessage(&msg);

      } // end if
          
      GameMain();  // game main processing function called here
      
      // check for <ESC> key and send quit game
      if (KEYDOWN(VK_ESCAPE)) SendMessage (hwnd, WM_CLOSE, 0, 0);
      
      // wait until we hit correct game speed frame rate
      while ((GetTickCount() - start_tick) < GAME_SPEED);
      
   } // end while
   
   GameQuit();  // game quit function and clean up before exit called here
   
   return(msg.wParam); // return to Windows

} // end WinMain

// BEGIN GAME CODE ////////////////////////////////////////

///////////////////////////////////////////////////////////
//
// GAME INITIALIZATION
//
///////////////////////////////////////////////////////////

bool GameInit()
{

   int i;
   
   // set x and y screen limits
   xlimit = WINDOW_WIDTH  - 55;
   ylimit = WINDOW_HEIGHT - 55;

   // set initial position of Megaman bitmap
   xbmp = 100;
   ybmp = ylimit;
   
   // set midpoint where to begin scrolling
   xmid = WINDOW_WIDTH / 2;
   // set offset to pixel 0, set first x tile position, turn off scrolling
   xoffset = 0; xtile = 0; do_scroll = FALSE;
      
   xmove = 0; ymove = 0; // initial x and y vector movement zero
         
   meg_pos = 1;   // start at first animation position
   meg_dir = 0;   // facing to the right
   
   meg_countdown = MEG_DELAY;
   
   // temporary change to full screen mode -- initialize DirectDraw
   
   // first create base IDirectDraw interface
   if (FAILED(DirectDrawCreate(NULL, &game_draw_main, NULL)))
   {
      MessageBox(game_window, "DirectDraw could not be initialized -- Create Error","DirectDraw Error",MB_OK);
      return FALSE; // error returned
   }
   
   // set cooperation to full screen
   if (FAILED(IDirectDraw_SetCooperativeLevel(game_draw_main, game_window, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)))
   {
      MessageBox(game_window, "DirectDraw could not be initialized -- Cooperative Level Error","DirectDraw Error",MB_OK);
      return FALSE; // error returned
   }   
   
   // set display mode to specific resolution
   if (FAILED(IDirectDraw_SetDisplayMode(game_draw_main, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_BPP)))
   {
      MessageBox(game_window, "DirectDraw could not be initialized -- Screen Resolution Not Available","DirectDraw Error",MB_OK);
      return FALSE; // error returned
   }

   // create the primary surface with a back buffer
   ZeroMemory(&game_draw_desc, sizeof(game_draw_desc)); // clear out surface description
   game_draw_desc.dwSize = sizeof(game_draw_desc);
   game_draw_desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
   game_draw_desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
   game_draw_desc.dwBackBufferCount = 1;

   if (FAILED(IDirectDraw_CreateSurface(game_draw_main, &game_draw_desc, &game_draw_surface, NULL)))
   {
      MessageBox(game_window, "DirectDraw could not be initialized -- Primary Surface Error","DirectDraw Error",MB_OK);
      return FALSE; // error returned
   }

   // get the pointer to the back buffer
   game_draw_caps.dwCaps = DDSCAPS_BACKBUFFER;
   if (FAILED(IDirectDrawSurface_GetAttachedSurface(game_draw_surface, &game_draw_caps, &game_draw_back)))
   {
      MessageBox(game_window, "DirectDraw could not be initialized -- Back Surface Error","DirectDraw Error",MB_OK);
      return FALSE; // error returned
   }
   
   // set the clipper on left and right sides of screen
   
   IDirectDraw_CreateClipper(game_draw_main, 0, &game_draw_clip, NULL);
   IDirectDrawClipper_SetHWnd(game_draw_clip, 0, game_window);
   IDirectDrawSurface_SetClipper(game_draw_back, game_draw_clip);
      
   // load the BMP Megaman surface animations
   hbmp[0]  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG00),0,0);
   hbmp[1]  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG01),0,0);
   hbmp[2]  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG02),0,0);
   hbmp[3]  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG03),0,0);
   hbmp[4]  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG04),0,0);
   hbmp[5]  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG05),0,0);
   hbmp[6]  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG06),0,0);
   hbmp[7]  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG07),0,0);
   hbmp[8]  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG08),0,0);
   hbmp[9]  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG09),0,0);
   hbmp[10] = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG10),0,0);
   hbmp[11] = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG11),0,0);
   hbmp[12] = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG12),0,0);
   hbmp[13] = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG13),0,0);
   hbmp[14] = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG14),0,0);
   hbmp[15] = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG15),0,0);
   hbmp[16] = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG16),0,0);
   hbmp[17] = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG17),0,0);
   hbmp[18] = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG18),0,0);
   hbmp[19] = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_MEG19),0,0);
   
   // set black color key for transparency on each animation surface
   for (i = 0; i < 20; i++) DDSetColorKey(hbmp[i], RGB(0,0,0));
      
   // load the BMP tile backgrounds
      
   hsky     = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_SKY),0,0);
   hcloud   = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_CLOUD),0,0);
   hhill    = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_HILL),0,0);
   hground  = DDLoadBitmap(game_draw_main,MAKEINTRESOURCE(BMP_GROUND),0,0);
            
   ZeroMemory(&back_fill,sizeof(back_fill)); // set up back fill for clear background
   back_fill.dwSize = sizeof(back_fill);
   back_fill.dwFillColor = RGB(0,0,0); 
      
   MessageBox(game_window, "DirectDraw Initialized Okay -- Entering GameMain Loop","DirectDraw OK",MB_OK);

   return TRUE; // success
          
} // END OF GameInit


///////////////////////////////////////////////////////////
//
// GAME MAIN LOOP AND PROCESSING
//
///////////////////////////////////////////////////////////

void GameMain()
{
      
   int r,c; // row and column of screen map
   int m;   // map tile symbol
   int x;   // x,y screen pixel position for tile drawing
   int y = YTOP_LEFT - TILE_SIZE;
         
   // clear the back buffer to black
   IDirectDrawSurface_Blt(game_draw_back,NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT, &back_fill);
  
   // draw the background tiles to back buffer      
	     
   for (r = 0; r < SCREEN_HEIGHT; r++) // do the ROW
   {
      y += TILE_SIZE; x = XTOP_LEFT - TILE_SIZE + xoffset;

      for (c = 0; c < SCREEN_WIDTH; c++) // do the COLUMN
      {
         x += TILE_SIZE;
	                   
         m = Map_Level[r][c + xtile]; // get the map symbol
         
         // set up temp rect and blit a tile to back buffer
                  
         SetRect(&temp_rect, x, y, x + TILE_SIZE, y + TILE_SIZE);
                  	           
         if (m == TILE_SKY) IDirectDrawSurface_Blt(game_draw_back,&temp_rect,hsky,&tile_rect,DDBLT_WAIT,NULL);
            
         if (m == TILE_CLOUD) IDirectDrawSurface_Blt(game_draw_back,&temp_rect,hcloud,&tile_rect,DDBLT_WAIT,NULL);

         if (m == TILE_HILL) IDirectDrawSurface_Blt(game_draw_back,&temp_rect,hhill,&tile_rect,DDBLT_WAIT,NULL);
                        
         if (m == TILE_GROUND) IDirectDrawSurface_Blt(game_draw_back,&temp_rect,hground,&tile_rect,DDBLT_WAIT,NULL);
            	           
      }  // end of for COLUMN (WIDTH)
	        
   }  // end of for ROW (HEIGHT)
     
   // draw Megaman animation frame to back buffer
   SetRect(&temp_rect, xbmp, ybmp, xbmp + 53, ybmp + 48);
      
   IDirectDrawSurface_Blt(game_draw_back,&temp_rect,hbmp[meg_pos + meg_dir],&bmp_rect,DDBLT_WAIT | DDBLT_KEYSRC, NULL);

   // check for keys and move man or scroll visible screen
   
   if (ymove != 0) // we are in the air
   {
      ybmp += ymove;
        
      if (ybmp > ylimit)
      {
         ybmp = ylimit; ymove = 0; // we hit the ground
      }
         
      if (ymove < 0)
      {
         ymove += GRAVITY_FRICTION;
         if (ymove >= 0) ymove = GRAVITY_FRICTION; // begin fall
      }
      else if (ymove > 0)
      {
         ymove += GRAVITY_FRICTION;
      }
      
   } // end of if in the air
      
   if (KEYDOWN(VK_RIGHT)) // moving right or scrolling left
   {
      meg_dir = 0;
      if (ymove == 0)
      {
         meg_countdown--;
         if (meg_countdown == 0)
         {
            meg_countdown = MEG_DELAY;
            meg_pos++; if (meg_pos > 9) meg_pos = 0;
         }
      }
      ScrollLeft();
   }
      	  
   else if (KEYDOWN(VK_LEFT)) // moving left or scrolling right
   {
      meg_dir = 10;
      if (ymove == 0)
      {
         meg_countdown--;
         if (meg_countdown == 0)
         {
            meg_countdown = MEG_DELAY;
            meg_pos++; if (meg_pos > 9) meg_pos = 0;
         }
      }
      ScrollRight();
   }
	  
   else if (ymove == 0)
   {
      meg_pos = 1;
   }
	  	  	  	     
   if (KEYDOWN(VK_CONTROL) && ymove == 0) // allow jump if not in the air already
   {
      meg_pos = 9;         // in the air animation frame
      ymove = -JUMP_SPEED; // jump up in negative y
   }

   // copy/flip back buffer to front buffer
   IDirectDrawSurface_Flip(game_draw_surface,NULL,DDFLIP_WAIT);
   
} // END OF GameMain

///////////////////////////////////////////////////////////
//
// GAME QUIT AND CLEAN UP
//
///////////////////////////////////////////////////////////

void GameQuit()
{
   int i;
      
   // release/delete the bmp dd surfaces
   
   for (i = 0; i < 20; i++)
   {
      if (hbmp[i]) IDirectDrawSurface_Release(hbmp[i]);
   }
   
   if (hsky)    IDirectDrawSurface_Release(hsky);
   if (hcloud)  IDirectDrawSurface_Release(hcloud);
   if (hhill)   IDirectDrawSurface_Release(hhill);
   if (hground) IDirectDrawSurface_Release(hground);

   // release/delete the primary and back dd surfaces
   if (game_draw_back)    IDirectDrawSurface_Release(game_draw_back);
   if (game_draw_surface) IDirectDrawSurface_Release(game_draw_surface);

   // release/delete the IDirectDraw interface
   if (game_draw_main)
   {
      IDirectDraw_Release(game_draw_main); game_draw_main = NULL;
   } 
   
} // END OF GameQuit


///////////////////////////////////////
//
// Scroll Screen to the left
// or move Megaman to the right
//
///////////////////////////////////////

void ScrollLeft()
{

   if (do_scroll) // we are scrolling, Megaman reached mid
   {
      xoffset -= MEG_SPEED; // decrement offset for scroll screen left
      if (xoffset < 0)      // if offset out of range
      {
         xoffset += TILE_SIZE; xtile++; // wrap around offset, move to next tile
         if (xtile > MAP_WIDTH - SCREEN_WIDTH) // reached last right tile on map?
         {
            xtile = MAP_WIDTH - SCREEN_WIDTH; // if yes, stop at that tile
            xoffset = 0;
            xbmp = xmid + MEG_SPEED; // move Megaman just past mid
            do_scroll = FALSE;       // and turn off scrolling
         } // end if xtile outside map
      } // end if xoffset < 0
   } // end if do_scroll (we are scrolling)

   else // do_scroll FALSE (we are not scrolling)
   {
      // two cases for moving right
      // case 1: megaman to the left of mid
      if (xbmp < xmid)
      {
         xbmp += MEG_SPEED;  // move Megaman
         if (xbmp >= xmid)   // check if now reached mid
         {
            xbmp = xmid; do_scroll = TRUE; // if so, turn on scrolling
         }
      }
      else // case 2: Megaman to the right of mid
      {
         xbmp += MEG_SPEED; // move Megaman
         if (xbmp > xlimit) xbmp = xlimit; // reached edge of screen
      }
   } // end of else (we are not scrolling)

} // END OF ScrollLeft


///////////////////////////////////////
//
// Scroll Screen to the right
// or move Megaman to the left
//
///////////////////////////////////////

void ScrollRight()
{

   if (do_scroll) // we are scrolling, Megaman reached mid
   {
      xoffset += MEG_SPEED;        // increment offset for scroll screen right
      if (xoffset > TILE_SIZE - 1) // if offset out of range
      {
         xoffset -= TILE_SIZE; xtile--; // wrap around offset, move to next tile
         if (xtile < 0) // reached last left tile on map?
         {
            xtile = 0; // if yes, stop at that tile
            xoffset = 0;
            xbmp = xmid - MEG_SPEED; // move Megaman just past mid
            do_scroll = FALSE;       // and turn off scrolling
         } // end if xtile outside map
      } // end if xoffset < 0
   } // end if do_scroll (we are scrolling)

   else // do_scroll FALSE (we are not scrolling)
   {
      // two cases for moving left
      // case 1: Megaman to the right of mid
      if (xbmp > xmid)
      {
         xbmp -= MEG_SPEED; // move Megaman
         if (xbmp <= xmid)  // check if now reached mid
         {
            xbmp = xmid; do_scroll = TRUE; // if so, turn on scrolling
         }
      }
      else // case 2: Megaman to the left of mid
      {
         xbmp -= MEG_SPEED; // move Megaman
         if (xbmp < XTOP_LEFT + TILE_SIZE) xbmp = XTOP_LEFT + TILE_SIZE; // reached edge
      }
   } // end of else (we are not scrolling)

} // END OF ScrollRight
