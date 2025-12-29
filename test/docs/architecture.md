
## Webserv 設計メモ（Week1〜Week4の土台）

### 0. 目的（この設計の狙い）

* **non-blocking + select/poll 1ループ**で全fdを回す
* fdごとに状態を持ち、**read/write/CGIを状態遷移で処理**
* 最初は `struct + 関数分割`、動いたらクラス化

---

## 1. データ構造（最低限）

### 1) Connection（fdごとの状態）

```text
Connection
- int fd
- std::string in_buf      // recvしたデータ（HTTP解析前の生）
- std::string out_buf     // send待ちデータ
- size_t out_sent         // out_bufの送信済み位置
- ConnState state         // READING / WRITING / (後で) CGI / CLOSED
- time_t last_active      // (後で) timeout用
- (後で) request / response / cgi関連のハンドル
```

### 2) 接続テーブル

```text
std::map<int, Connection> conns;
```

### 3) fd管理（select版）

```text
fd_set master_read, master_write;
fd_set read_fds, write_fds;
int fd_max;
```

---

## 2. メインループ（1ループだけ）

```text
while (running) {
  read_fds  = master_read;
  write_fds = master_write;
  select(fd_max+1, &read_fds, &write_fds, NULL, timeout);

  if (listen_fd readable) accept_new_clients();

  for each fd ready:
    if (fd readable)  handle_read(fd)
    if (fd writable)  handle_write(fd)

  cleanup_closed_fds();
}
```

**ルール**

* recv/send は **FD_ISSET が true のときだけ**
* すべて non-blocking
* closeする時は必ず「FD_CLR + close + conns.erase」

---

## 3. 関数分割（Week1でここまで切ればOK）

### A) accept_new_clients()

* listen_fd を accept（EAGAINまで回すのが理想）
* client_fd を non-blocking
* conns[client_fd] を作る（state=READING）
* `FD_SET(client_fd, master_read)` する
* fd_max 更新

### B) handle_read(fd)

* `recv()` を読む（EAGAINなら終了）
* `size==0` → 相手が閉じた → close_connection(fd)
* `size>0` → `in_buf += data`、last_active更新
* Week1はここで **固定レスポンス**を作って out_buf に入れてOK

  * out_buf を積んだら `FD_SET(fd, master_write)`
  * Week1は `FD_CLR(fd, master_read)` して「一回読んだら返して閉じる」でもOK

### C) handle_write(fd)

* `send()`（EAGAINなら終了）
* 送れた分だけ out_sent を進める
* 全部送れたら：

  * `FD_CLR(fd, master_write)`
  * close_connection(fd)（Week1は close でOK）

### D) close_connection(fd)

* `FD_CLR(fd, master_read)`
* `FD_CLR(fd, master_write)`
* `close(fd)`
* `conns.erase(fd)`
* （必要なら fd_max を下げる）

---

## 4. Week2〜の拡張ポイント（設計の“差し込み口”）

Week1ができたら、次は **handle_read の中身**を差し替えるだけで進む。

### Week2：HTTP最小（静的GET）

* `in_buf` から「ヘッダ終端 `\r\n\r\n`」を検出
* request line と headers を最小で読む
* ルーティングして静的ファイルを out_buf に積む
* keep-alive はまだ無視して closeでもOK（後で）

### Week3：CGI runner（別モジュール）

* `CgiRunner::start(conn, request)`

  * fork/execve + pipes
* CGIの stdout/stderr の pipe も **non-blocking** で監視対象にする

  * 「fdの種類」が増えるので、`FdRole` を持つと綺麗

    * 例：client_fd / cgi_out_fd / cgi_in_fd など

### Week4：統合（locationでCGI）

* routerが「この拡張子はCGI」と判断
* conn.state = CGI
* CGI出力が揃ったら out_buf にして WRITINGへ

---

## 5. クラス化のタイミング（最短で安全）

Week1〜2で骨格が安定したら、この順でクラス化すると痛くない：

1. `Connection`（struct→class化してもOK）
2. `Server`（listen_fd + accept + config参照）
3. `EventLoop`（select/pollとfd登録）
4. `HttpParser`
5. `CgiRunner`

---

## 6. 今すぐ直しておくと後が楽な小ルール

* **mainにはロジックを書かない**：selectして呼ぶだけに寄せる
* **FD操作はclose_connectionに集約**（漏れが死ぬほど減る）
* **recv/sendのEAGAINはエラー扱いしない**（次のselectで続き）

---
