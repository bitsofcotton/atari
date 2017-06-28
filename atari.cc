#include "atari.hh"

/* それぞれの実体をここで宣言 */
chara_t enemy[M_ENEM + 1];
pixs_t  data;
mask_t  mask;

MyWidget* mywidget;

MyWidget::MyWidget()
{
  /* タイマーを自分に向かせて */
  timer = new QTimer(mywidget);
  connect(timer, SIGNAL(timeout()), this, SLOT(timer_loopback()));
  
  /* 1000/FPSms 周期で割り込み */
  timer->start((int)((double)1000 / FPS), FALSE);
  
  /* ウィジェットサイズを制限 */
  setFixedSize(W_W, W_H);
}

/* 簡易的(かなり)  な移動処理 */
void movechr(chara_t* chara)
{
  chara->bx = chara->x;
  chara->by = chara->y;
  chara->x += chara->dx;
  chara->y += chara->dy;
  
  /* みての通り画面外へ消える仕様あり */
  if(chara->x > W_W)
    {
      chara->x = W_W + (W_W - chara->x);
      chara->dx = -chara->dx;
    }
  else if(chara->x < 0)
    {
      chara->x = -chara->x;
      chara->dx = -chara->dx;
    }
  
  if(chara->y > W_H)
    {
      chara->y = W_H + (W_H - chara->y);
      chara->dy = -chara->dy;
    }
  else if(chara->y < 0)
    {
      chara->y = -chara->y;
      chara->dy = -chara->dy;
    }
}

/* 適当(適切かどうかは主観)なマクロ群 */
#define ISBIT(i,j,chara)				       \
  if(chara->pixmap[(i * chara->width + j) / SZ_PAD]	       \
     & (1 << ((i * chara->width + j) % SZ_PAD)))	       
#define EACHDOT(i,j,chara)			       \
  for(int i = 0; i < chara->height; i ++)	       \
    for(int j = 0; j < chara->width; j ++)

/* 自分も敵も描画する関数(それぞれのオブジェクト毎) */
void draw(QPainter* p, QPen col, chara_t* chara)
{
  /* 指定された色で */
  p->setPen(col);
  
  /* それぞれの自分のドットに対して */
  EACHDOT(i,j,chara)
    /* 自分の画像中にそのドットが存在していれば (透明でなければ) */
    ISBIT(i,j,chara)
    /* 何か別の色で描画 */
      p->drawPoint(chara->x + j, chara->y + i);
}

/*
 * オブジェクト毎にマスクを取り直す関数
 * 座標はディスプレイに依ります
 */
void mapWriteMask(mask_t* msk, chara_t* chara)
{
  /*
   * ビットシフトと連携することにより更に高速化可能です
   */
  /* それぞれのキャラクタのピクセルに対して */
  EACHDOT(i,j,chara)
    /* 存在すれば */
    ISBIT(i,j,chara)
    {
      /* 座標を変換して */
      int k = j + chara->x;
      int l = i + chara->y;
      
      /* マスクを取る */
      msk->data[(l * msk->width + k) / SZ_PAD]
	|= (1 << ((l * msk->width + k) % SZ_PAD));
    }
}

/*
 * とりあえず敵機全体をマスクとして取る関数
 * 本当、とりあえず...
 */
void remakeMask(mask_t* msk)
{
  /* マスクの諸ビットをクリアして */
  memset(msk->data, 0, msk->width * msk->height * sizeof(int) / SZ_PAD);
  
  /* それぞれの敵機をマスク */
  for(int i = 0; i < M_ENEM; i ++)
    mapWriteMask(msk, &enemy[i]);
}

/* マスクとキャラクタが交差しているか確かめる関数 */
int judgeMask(mask_t* msk, chara_t* chara)
{
  /* キャラクタのそれぞれのドットに対して */
  EACHDOT(i,j,chara)
    /* もしキャラクタのドットが透明でなければ */
    ISBIT(i,j,chara)
    {
      /* 座標を補正して */
      int k = j + chara->x;
      int l = i + chara->y;
      
      /* マスクの同座標がどうなっているかをみて */
      if((l * msk->width + k) < msk->height * msk->width
	 && (msk->data[(l * msk->width + k) / SZ_PAD]
	  & (1 << ((l * msk->width + k) % SZ_PAD))))
	/* 重なっていれば真を */
	return true;
    }
  /* そうでなければ偽を返す */
  return false;
}

/*
 * 
 */
int judgeRectangle(chara_t* chara1, chara_t* chara2)
{
  // chara->x, chara->y, chara->height, chara->width
  // enemy[M_ENEM].x, enemy[M_ENEM].y,
  // enemy[M_ENEM].height, enemy[M_ENEM].width
  int dx, dy;
  dx = (chara2->x + chara2->width) - (chara1->x + chara1->width);
  dy = (chara2->y + chara2->height) - (chara1->y + chara1->height);
  
  if(abs(dx) < chara1->width + chara2->width
     && abs(dy) < chara1->height + chara2->height)
    return true;
  return false;
}

/* タイマーのループバック */
void MyWidget::timer_loopback()
{
  /* 割り込まれたら*/
  /* 再描画 */
  mywidget->repaint(TRUE);
}

/* 描画処理のオーバーロード */
void MyWidget::paintEvent(QPaintEvent* e)
{
  /* 素敵な qt 仕様 */
  QPainter p;
  p.begin(this);
  p.translate(0, 0);
  
  /* とりあえずそれぞれの機を動かして */
  for(int i = 0; i < M_ENEM + 1; i ++)
    movechr(&enemy[i]);
  
  /* 後で面倒にならないように描画しておく */
  for(int i = 0; i < M_ENEM; i ++)
    draw(&p, Qt::black, &enemy[i]);
  /* これは自分 */
  draw(&p, Qt::blue, &enemy[M_ENEM]);
  
  //  #define _JUDGE_WITH_RECTANGLE_
#define _JUDGE_WITH_MASK_
#ifdef _JUDGE_WITH_MASK_
  /* ちょっと非効率だが */
  /* 敵機のマスクを取って */
  remakeMask(&mask);
  /* 自分と交差していたら */
  if(judgeMask(&mask, &enemy[M_ENEM]))
    {
      /* 赤く自分(専用(謎)) */
      draw(&p, Qt::red, &enemy[M_ENEM]);
      
      /* 自分のマスクを取って */
      memset(mask.data, 0, mask.height * mask.width * sizeof(unsigned int) / SZ_PAD);
      mapWriteMask(&mask, &enemy[M_ENEM]);
      
      /* 敵に対して判定 */
      for(int i = 0; i < M_ENEM; i ++)
	if(judgeMask(&mask, &enemy[i]))
	  /* 交差していたらもちろん書く */
	  draw(&p, Qt::red, &enemy[i]);
    }
#elif _JUDGE_WITH_RECTANGLE_
  for(int i = 0; i < M_ENEM; i ++)
    if(judgeRectangle(&enemy[M_ENEM], &enemy[i]))
      {
	draw(&p, Qt::red, &enemy[i]);
	draw(&p, Qt::red, &enemy[M_ENEM]);
      }
#endif
  
  /* qt 仕様 */
  p.end();
}

/* qt 仕様 */
QApplication* qapp;

/* win32 の時は winmain なるあの引数のやたら長い関数が作成されます。 */
int main(int argc, char** argv)
{
  /* 時系列で線形合同な乱数列を初期化して(要は乱数が同じになるのを防ぐ) */
  srand(time(0));
  
  /* マスクの初期化 */
  mask.width  = W_W;
  mask.height = W_H;
  mask.data   = (unsigned int*)malloc(mask.width * mask.height * sizeof(unsigned int) / SZ_PAD);
  
  /* 機体の初期化 */
  for(int i = 0; i < M_ENEM + 1; i ++)
    {
      enemy[i].x      = rand() % W_W;
      enemy[i].y      = rand() % W_H;
      enemy[i].dx     = (rand() % 1 ? -1 : 1) * (rand() % 10);
      enemy[i].dy     = (rand() % 1 ? -1 : 1) * (rand() % 10);
      enemy[i].height = rand() % M_H + 5;
      enemy[i].width  = rand() % M_W + 5;
      for(unsigned int j = 0;
	  j < (enemy[i].height * enemy[i].width + SZ_PAD - 1) / SZ_PAD;
	  j ++)
	enemy[i].pixmap[j] = rand();
      /* 下はでバグ */
      //	enemy[i].pixmap[j] = 0xffffffff;
    };
  
  /* 全体を初期化 */
  data.n_enemy = M_ENEM;
  data.enemy  = enemy;
  data.myself = enemy[M_ENEM];
  
  /* 
   * qt 仕様... win32 の場合は何だったか...
   * CreateWindowEx か何かだった気がします
   */
  qapp = new QApplication(argc, argv);
  
  /*
   * ウィジェット(ウィンドウ)の表示
   * win32 の場合は恐らく while でメッセージをまわすループかと思われます。
   */
  mywidget = new MyWidget();
  qapp->setMainWidget(mywidget);
  mywidget->show();
  
  /*
   * ここだけ win32 と似ている...
   * アプリケーションを実行しメッセージループへと入る
   */
  return qapp->exec();
}
