#include  <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define WIDTH 320
#define HEIGHT 200

#define FIRE_W WIDTH
#define FIRE_H HEIGHT
#define FIRE_PALETTE_SIZE 37

static uint8_t fire_pixels[FIRE_W * FIRE_H];

//classic 37 color fire pallete
static const uint32_t fire_palette[FIRE_PALETTE_SIZE] = {
  0x070707, 0x1F0707, 0x2F0F07, 0x470F07, 0x571707, 0x671F07, 0x771F07,
  0x8F2707, 0x9F2F07, 0xAF3F07, 0xBF4707, 0xC74707, 0xDF4F07, 0xDF5707,
  0xDF5707, 0xD75F07, 0xD75F07, 0xD7670F, 0xCF6F0F, 0xCF770F, 0xCF7F0F,
  0xCF8717, 0xC78717, 0xC78F17, 0xC7971F, 0xBF9F1F, 0xBF9F1F, 0xBFA727,
  0xBFA727, 0xBFAF2F, 0xB7AF2F, 0xB7B72F, 0xB7B737, 0xCFCF6F, 0xDFDF9F,
  0xEFEFC7, 0xFFFFFF
};

// int must be atleast 16 bits. so it doesnt mandate an exact size for it. that's why we use uint32_t instead of int.
uint32_t framebuffer[WIDTH * HEIGHT];

void put_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
    {
        return;
    }
    framebuffer[y * WIDTH + x] = color;
}

void clear(uint32_t color)
{
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        framebuffer[i] = color;
    }
}

int main(void)
{
    SDL_Window *window ;
    SDL_Renderer *renderer ;
    SDL_Texture *texture ;
    SDL_Event event ;

    const double target_frame = 1.0 / 60.0; // Target frame time for 60 FPS

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    window = SDL_CreateWindow(
        "SDL Framebuffer", 
        WIDTH* 4 , 
        HEIGHT* 4, 
        0
    );

    if (!window)
    {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    renderer = SDL_CreateRenderer(
        window, 
        NULL
    );

    if (!renderer)
    {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_XRGB8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        WIDTH, 
        HEIGHT
    );

    if (!texture)
    {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_SetTextureScaleMode(
        texture, 
        SDL_SCALEMODE_NEAREST
    );

    uint8_t is_running = 1; 

    uint32_t frame = 0;

    //fire initialization
    for (int i = 0; i < FIRE_W * FIRE_H; i++)
    {
        fire_pixels[i] = 0;
    }

    //seed the bottom row with max intensity
    for (int i = 0; i< FIRE_W; i++)
    {
        fire_pixels[(FIRE_H - 1) * FIRE_W + i] = FIRE_PALETTE_SIZE - 1;
    }

    while (is_running)
    {
        uint64_t start = SDL_GetPerformanceCounter();
        
        //Poll for events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                //Stop running 
                is_running = 0;
            }
        }

        clear(0x2A2A2A); 
        
        //here we can draw to the framebuffer. For example, we can fill it with a color.
        
        //fire update
        for (uint32_t x = 0; x < FIRE_W; x++)
        {
            for (uint32_t y = 1; y < FIRE_H; y++)
            {
                uint32_t src = y * FIRE_W + x;
                uint32_t rand_idx = rand()%3;
                int32_t dst = (int32_t)src - FIRE_W;
                int32_t dst_x = (int32_t)(src%FIRE_W) - (int32_t)rand_idx + 1;
                if (dst<0 || dst_x<0 || dst_x>=FIRE_W)
                {
                    continue;
                }
                else
                {
                    dst = dst- (int32_t)(src%FIRE_W) + dst_x;
                    uint8_t src_val = fire_pixels[src];
                    uint8_t decay = rand_idx & 1;
                    fire_pixels[dst] = (src_val > decay) ? src_val - decay : 0;

                }
            }
        }

        //fire rendering
        for (uint32_t y = 0; y < FIRE_H; y++)
        {
            for (uint32_t x = 0; x < FIRE_W; x++)
            {
                uint8_t idx = fire_pixels[y * FIRE_W + x];
                put_pixel(x, y, fire_palette[idx]);
            }
        }


        //copy the contents of the framebuffer to the texture.
        SDL_UpdateTexture(
            texture, 
            NULL, 
            framebuffer, 
            //Pitch is the width of texture in bytes. 
            WIDTH * sizeof(uint32_t)
        );


        SDL_RenderClear(renderer);
        SDL_RenderTexture(
            renderer, 
            texture, 
            NULL, 
            NULL
        );
        SDL_RenderPresent(renderer);

        uint64_t end = SDL_GetPerformanceCounter();
        double elapsed =  (double)(end-start) / (double)SDL_GetPerformanceFrequency();

        //Cap the frame rate to 60 fps
        if (elapsed < target_frame)
        {
            SDL_Delay(((target_frame - elapsed) * 1000.0)); //sleep for a while 
        }
        frame++;
    }
    
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return EXIT_SUCCESS;
}