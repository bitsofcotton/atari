/*
 * サンプルソース
 *
 * QT 上であたり判定の簡易版 with mask 処理
 */

/* qt 関連のインクルード win32api の場合は他のものになるかと思われます。 */
#include <qapplication.h>
#include <qobject.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qpen.h>
#include <qcolor.h>
#include <qtimer.h>
#include <qevent.h>

/* ANSI C 関連のインクルード (乱数以外は使用していません。)
#include <stdlib.h>
#include <stdio.h>*/

/* int 型のビット数 */
#define SZ_PAD (sizeof(int) * 8)

/* エネミー(疑似) の数 */
#define M_ENEM 500

/* エネミーのサイズ(最小分) */
#define EN_H   5
#define EN_W   5

/* エネミーのサイズ(変化分) */
#define M_H    10
#define M_W    10

/* ウィジェット(ウィンドウ) のサイズ */
#define W_H    480
#define W_W    640

/* 本当に適用されるか判らない FPS */
#define FPS    24

/* 本当に適当な時機&敵機の構造体 */
typedef struct {
  /* 現在位置と */
  int x;
  int y;
  /*
   * 前の位置と (改良するのであれば必要)
   *  (しかし、貫通判定は面倒なので出来れば fps を上げる方向で)
   */
  int bx;
  int by;
  /* 移動速度 */
  int dx;
  int dy;
  /* サイズ */
  int height;
  int width;
  /* 画像(と言いつつ、マスク処理をかねています */
  int pixmap[M_H * M_W];
} __attribute__((packed)) chara_t;

/* 全くいい加減なゲーム全体の設定(ゲームじゃない) */
typedef struct {
  chara_t myself;
  chara_t *enemy;
  int     n_enemy;
} pixs_t;

/* マスク関連の構造体 (面倒なだけ...?) */
typedef struct {
  unsigned int *data;
  int height;
  int width;
} __attribute__((packed)) mask_t;

/* 以上は C にも移植性の高い形で再現 */


/* とりあえず qt を使用するので... */
class MyWidget : public QWidget
{
public:
  /* コントラクタ */
  MyWidget();
  /* タイマを使用 */
  QTimer* timer;

protected:
  /* win32api でいう wm_paint をオーバーロード */
  void paintEvent(QPaintEvent* e);
  
  /* 仕様 */
  Q_OBJECT
  
  /* タイマーのループバック */
public slots:
  void timer_loopback();
};
