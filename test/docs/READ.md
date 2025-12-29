# Webserv

A minimal HTTP/1.1 web server written in C++98.

This project implements a subset of HTTP features required by the
42 school Webserv subject, focusing on correctness, robustness,
and incremental development.

## Architecture Overview

The server is composed of the following logical layers:

- Socket Layer
  - socket(), bind(), listen(), accept()
  - Non-blocking file descriptors

- Event Loop
  - select() based I/O multiplexing
  - Tracks listening socket and client sockets

- Connection Management
  - One Connection object per client socket
  - Stores read buffer, write buffer, state

- HTTP Layer
  - Request parsing (start-line, headers, body)
  - Response building (status line, headers, body)

- Configuration Layer
  - Parses configuration file
  - Provides server/location settings

- Application Logic
  - Static file serving
  - Error pages
  - CGI execution

## Core Data Structures

### Connection

Represents a single client connection.

Responsibilities:
- Owns the client socket fd
- Stores partially received data
- Tracks request parsing state
- Holds pending response to send

### Server

Represents a listening socket.

Responsibilities:
- Holds configuration
- Accepts new connections
- Dispatches connections to event loop

## Event Loop Flow

1. Initialize listening socket(s)
2. Add listening fd(s) to master fd_set
3. Enter main loop:
   - Copy master set to read_fds / write_fds
   - Call select()
   - If listening fd is readable:
       - accept() new client
       - add client fd to master set
   - If client fd is readable:
       - recv() data
       - append to connection buffer
       - if full request received → parse
   - If client fd is writable:
       - send() pending response
       - close connection if finished

## 2ヶ月プラン（CGI込み、レビュー提出優先で現実的に）

ポイントは「Week4の時点で *CGIが一度は動く* を作る」。ここが遅れると後半で崩れる。

### Week 1：I/O基盤を“要件準拠”に寄せる

* **fdは全部 non-blocking**（listen/client/CGIパイプも含む）
* select/poll を **“1個だけ”**で read/write 両方監視できる形にする 
* クライアントごとの状態（READING / PROCESSING / WRITING）を持つ
* **「readableじゃないのにrecvしない」「writableじゃないのにsendしない」**を徹底（ここが0点条件）

成果物：複数クライアントが同時に来ても落ちず、読み/書きが状態遷移で回る“骨格”。

### Week 2：HTTP最低限（静的GETが返る）

* リクエストの終端検出（まずは `\r\n\r\n`）
* GETでファイル返却、基本ステータス（200/404/405/500くらい）
* default error page（無い場合は内蔵）

成果物：ブラウザ/curlで静的サイトが返る（Mandatoryの「fully static website」へ前進）

### Week 3：CGI runner を“単体で”完成させる（統合は後）

ここが肝。サーバに埋め込む前に、CGI実行部分だけ完成させる。

* `fork + execve + pipe + dup2` で

  * CGI stdin ← request body
  * CGI stdout → 親が回収
* env構築（最低限でも、REQUEST_METHOD / QUERY_STRING / CONTENT_LENGTH / CONTENT_TYPE / SCRIPT系 / SERVER系）
* **実行ディレクトリ（chdir）**対応 
* **タイムアウト**（「request should never hang indefinitely」）

成果物：コマンドライン実験でもいいから、**GET CGIが動いてstdoutが取れる**。

### Week 4：サーバへCGI統合（GET CGIを通す）

* configで拡張子→CGI扱い（例：`.py`） 
* `/cgi-bin/test.py?x=1` が動く（QUERY_STRING含む）
* CGI stdoutをHTTPレスポンスとして返す（最初は簡易でもOK）

成果物：**“CGI対応と言える状態”**に到達。

### Week 5：POST + upload（ここもMandatory）

* **POSTをCGIへstdinで渡す**（Content-Length必須）
* **upload**：configで許可/保存先指定、ファイルを保存できる 

  * ここは実装が2通りある：

    1. `multipart/form-data` をパースして保存（重い）
    2. “課題の想定テスト”に合わせてシンプルなアップロード方式＋テストファイル用意（現実的）
* DELETE実装（ファイル削除など）

### Week 6：chunked対応（CGI注意点の実装）

* PDFに明記：**chunkedはサーバがunchunkして、CGIへはEOF終端のbodyとして渡す** 
* CGI出力も **Content-Length無ければEOF終端**扱い 

### Week 7：config拡充・複数ポート・ルール類

* 複数port listen 
* routeごとの allowed methods / redirect / autoindex / default file / root / error pages / body size 

### Week 8：耐久・テスト・レビュー準備

* stress test（落ちない、リソース回収、ゾンビ無し）
* テスト用configとデフォルトファイルを揃える（評価で“全部動く”を見せるため）

---
