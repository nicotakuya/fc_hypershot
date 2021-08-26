// HyperShot test program
//  by takuya matsubara
// 対応カートリッジ Mapper0(PRG-ROM 32KB / CHR-RAM 8KB)
// *CHR-ROMではないのでご注意ください
#include "pattern.h"   // パターンテーブル
#include "fcsub.h"     // サブルーチン

const char colortable[] = {
    C_BLACK ,C_GRAY ,C_LIGHTGRAY ,C_WHITE,  // BG palette0
    C_BLACK ,C_BLUE ,C_RED ,C_WHITE,        // BG palette1
    C_BLACK ,C_BLUE ,C_RED ,C_WHITE,        // BG palette2
    C_BLACK ,C_BLUE ,C_RED ,C_WHITE,        // BG palette3
    C_BLACK ,C_BLUE ,C_RED ,C_YELLOW,       // SP palette0
    C_BLACK ,C_BLUE ,C_LIMEGREEN ,C_YELLOW, // SP palette1
    C_BLACK ,C_BLUE ,C_RED ,C_WHITE,        // SP palette2
    C_BLACK ,C_BLUE ,C_RED ,C_WHITE         // SP palette3
};

const char titlestr[]="TEST OF HYPERSHOT";

unsigned char button[2];
unsigned char button_back[2];
signed char jump[2];

#define JOYD0_BIT (1<<0)

#define P1RUN_BIT  (1<<1)
#define P1JUMP_BIT (1<<2)
#define P2RUN_BIT  (1<<3)
#define P2JUMP_BIT (1<<4)

#define PLAYER1 0
#define PLAYER2 1


//----read gamepad1+gamepad2+HYPERSHOT
void hypershot(void)
{
    unsigned char tmp1;
    unsigned char tmp2;
    unsigned char tempdata;
    char i;

    JOYPAD1 = 0x01;    // Load
    JOYPAD1 = 0x00;    // Latch
    for(i=0; i<8; i++){
        // read gamepad1
        tmp1 <<= 1;
        if( JOYPAD1 & JOYD0_BIT ) tmp1 |= 1;

        // read gamepad2
        tmp2 <<= 1;
        tempdata = JOYPAD2;
        if( tempdata & JOYD0_BIT ) tmp2 |= 1;
    }
    // read HyperShot
    if(tempdata & P1RUN_BIT ) tmp1 |= BUTTON_A;
    if(tempdata & P1JUMP_BIT) tmp1 |= BUTTON_B;
    if(tempdata & P2RUN_BIT ) tmp2 |= BUTTON_A;
    if(tempdata & P2JUMP_BIT) tmp2 |= BUTTON_B;
    button[PLAYER1] = tmp1;
    button[PLAYER2] = tmp2;
}

// move player
void player(char num,unsigned char x,unsigned char y)
{
    unsigned char btn_now;     // ボタン 現在地
    unsigned char btn_edge;    // ボタン エッジ

    btn_now = button[num];   // コントローラー
    btn_edge = (button_back[num] ^ btn_now) & btn_now;

    if(btn_edge & BUTTON_A){
        x += 2;  // 右
        if(x > 224)x=8;
        sp_anime(num);
    }

    if((y >= 144)&&((btn_edge & BUTTON_B)!=0)){
        jump[num] = -31;   // start jump
        pulse1(600);
    }
    if(jump[num] < 31){    // jumping
        jump[num]++;                          // 重力による加速

        // ボタンを押していない場合、速く落下
        if((btn_now & BUTTON_B)==0)jump[num]+=2;
    }
    y += (jump[num] >> 2);
    if(y > 144) y = 144; // 地面のめり込み防止
    sp_ofs(num,x,y);
    button_back[num] = btn_now;
}

// タイトル＆ゲーム開始
void title(void)
{
    char x,y,tilenum;

    ppu_enable(0);    // 表示を無効化
    bg_printstr(2,2,(char *)titlestr);
    for(y=20; y<30; y++){
        for(x=0; x<32; x++){
            tilenum = ((x+y)& 1)+1;     // タイル番号
            bg_printch(x,y   ,tilenum); // BG Aに描画
            bg_printch(x,y+32,tilenum); // BG Bに描画
        }
    }
    sp_append(64,120, 0,0); // プレイヤー1追加
    sp_append(96,120, 0,1); // プレイヤー2追加
    jump[0] = 0;      // ジャンプ移動量
    jump[1] = 0;      // ジャンプ移動量
    ppu_enable(1);    // 表示を有効化
}

//---- メイン関数
char NesMain()
{
    char i;
    char x,y;

    fc_init(); // 全機能の初期化
    ppu_pattern((unsigned char *)pattern,0,512);   // tileset
    ppu_palette((char *)colortable,0,8);           // palette

    title();      // 背景とキャラの初期化

    ppu_vsync();  // Vブランク待ち

    while (1){
        hypershot();
        for(i=0; i<16; i++){
            x = sp_getx(i);
            y = sp_gety(i);
            if(y==OUTSIDE)continue; // 画面外のスプライトをスキップ
            player(i,x,y);  // プレイヤー移動処理
        }
        ppu_vsync();  // Vブランク待ち
        sp_dmastart();
        bg_scroll(0,0);
    }
    return 0;
}




