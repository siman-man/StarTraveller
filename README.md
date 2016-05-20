You are given NStar stars in 2D space. 
あなたには二次元空間上にN個の星たちが与えられています

You have NShip space ships that can travel between stars. 
あなたはN個の宇宙船を持っており、それらは星間移動が可能です

The amount of energy used by a space ship to travel from one star to another is calculated as 
星間移動に必要な移動コストは各星間のユークリッド距離から計算されます

the Euclidean distance between these stars. 

There are NUfo unidentified flying objects (UFO) moving around in space. 
またN個のUFOが宇宙を飛び回っています

UFO's reduce the energy required to travel between two stars. 
UFOは2つの星間の移動コストを下げてくれます

The energy consumed by your space ship is multiplied by 0.001 when your ship travels 
UFOと同じ方向に移動した場合は、消費するコストが従来の0.001倍となります。

in the same direction at the same time as a UFO. 

For example, if you travel between star A(0,0) and B(10,0) it will cost you 10 energy. 
例えば(0,0)から(10,0)に移動するとコストは10ですが

However, if one UFO is flying from A to B at the same time, it will cost you 10*0.001=0.01 energy. 
もし同じタイミングでUFOが通過していた場合は10 * 0.001 = 0.01となります

If two UFO's are flying from A to B at the same time, it will cost you 10*0.001*0.001=0.00001 energy.
もし2つのUFOが飛んでいた場合は10 * 0.001 * 0.001 = 0.00001となります

Your task is to minimize the total energy used by your ships in order to visit every star at least once.
あなたの目的はこの全ての星を最低でも1回訪れる経路のコストを最小にすることです


### 調べる事

- [ ] UFOの動きはランダムなのか
- [ ] UFOは平均してどの程度星を訪れることができるのか
- [ ] Multiple Traveling Salesman Problemについて調べる

### 調査結果

- [x] UFOの動きはランダム
- [x] 停滞行動は可能

## 問題を理解する

### 未知のものは何か

* 全ての星に到達する最小コストの経路
* UFOの3ターン目以降の移動先

### 与えられているもの

* 宇宙船の初期座標 (1 - 10個)
* 星の座標 (100 - 2000個)
* UFOの2ターン後までの行動
* 星の集まりである銀河がいくつか定義されている (1 - 16個)
* 星は銀河を中心とした±300の位置に属する

* UFOの移動候補は10 + rnd(NStar/10);
  * かならず1マス以上動く
  * この候補の中空一番近い星に移動を行う

### 条件

* 盤面外に移動してはいけない
* (NStar*4)以上のターンを消費してはいけない

## 計画をたてる

### 似たような問題はあるか

* 巡回セールスマン問題
* Multiple Traveling Salesman Problem

## 考察

* k-meansによる銀河系の特定
  * N = 16でスタートしてクラスタ数がある一定数を下回った場合はその点を削除する
  * N = 1からスタートしてクラスタに属している星で距離が100を超えたものが存在した場合はNを増やす
* UFOと一緒に行動するとコストが0.001倍なので、なるべくUFOと一緒に行動したほうが良い
* 銀河系を回るのが一番コスト的には少なさそう
* 船同士はなるべくバラバラになるように
* (NStar*3)ターンまではほとんど通常行動を行わなくても良い
* 最後に残った星たちで巡回セールス問題を解き行動を終える
  * Nが少ない場合はDPで最適解が出せる

## コストがかかる場面について

### コスト大きい

* 宇宙船に乗り込むとき
* 宇宙船から他の星に移動するとき
* 星から星への移動 (補正ない)

### コスト小さい

* 宇宙船についていく
** UFOの平均移動距離 * 0.001のコストが毎ターンかかる

### ボトルネックはどこか

* 最後の循環移動がかなりボトルネック

### ボトルネックを減らすには

* 宇宙船に早い段階で乗り込んで、未到達の星をなるべく減らす
* 宇宙船に乗り込むコストと最後のコストの分岐点を考える

### 未到達の星を減らすには

* UFOだけでは到達しにくいポイントを探す
* 到達しにくいポイントについては近くに来た時に直接訪れる

* 最終的に使うコスト 巡回パスを生成した後に A -> B -> C となった場合 [A -> B + B -> C] この区間をコストとして消費
* UFOから直接向かう場合は、その距離分の損失となる

### UFOから降りることのデメリット

* 星間移動のコストが高くなる (1000倍)
* 消すとメリットのある星を後半ピックアップする
* メリットのある星に移動できる場合には多少コストを払ってでもその星にいくUFOに乗り換える
** 「乗り換えコスト」 < 「星の到達報酬」
