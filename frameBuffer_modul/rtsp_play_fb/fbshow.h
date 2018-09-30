#ifndef FBSHOW_H
#define FBSHOW_H
//#include<windows.h>
#include <linux/fb.h>
#include<stdio.h>

class FbShow
{
public:
    FbShow();
    ~FbShow();

    int GetCurrentbpp();

    void ShowRGB_16bit(const void* imgData, int width, int height, int posX, int posY);
    void ShowRGB_24bit(const void* imgData, int width, int height, int posX, int posY);

    void ShowBmp_16bit(const void* imgData, int width, int height, int posX, int posY);
    void ShowBmp_24bit(const void* imgData, int width, int height, int posX, int posY);

    int ShowBmp_16bit(const char* path, int posX, int posY);

    bool InitFb();
    void DeInintFb();

    void bitmpa_format_convert_rgb16(char *data, int width, int height);
    void bitmpa_format_convert_rgb24(char *data, int width, int height);
protected:

    void print_fix_screeninfo(struct fb_fix_screeninfo& info);
    void print_var_screeninfo(struct fb_var_screeninfo& info);



    void write_rgb16_img_to_fb(char* fbAddress,
                               int screen_width, int screen_height,
                               unsigned char *imgData,
                               int width, int height,
                               int posX, int posY);

    void write_rgb24_img_to_fb(char* fbAddress,
                               int screen_width, int screen_height,
                               unsigned char *imgData,
                               int width, int height,
                               int posX, int posY);

    size_t get_file_size(const char* filePath);

private:

    struct fb_fix_screeninfo fb_fix; //这两个用于获取framebuffer相关信息的
    struct fb_var_screeninfo fb_var;

    char* m_fbAddr;
    int m_screen_fbd;
};

#endif // FBSHOW_H
