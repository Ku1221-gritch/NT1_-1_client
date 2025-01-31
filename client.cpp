#include <Novice.h>
#include <math.h>
#include <process.h>
#include <mmsystem.h>
#include <fstream>

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "winmm.lib")

using namespace std;

DWORD WINAPI Threadfunc(void*);

struct POS
{
	int x;
	int y;
};
POS pos1P, pos2P, old_pos2P;
RECT rect;
SOCKET sWait;
bool bSocket = false;
HWND hwMain;

const char kWindowTitle[] = "KAMATA ENGINEクライアント";

typedef struct {
	float x;
	float y;
}Vector2;

typedef struct {
	Vector2 center;
	float radius;
	float speed;
}Circle;

// キー入力結果を受け取る箱
Circle bigPlayer, smallPlayer;
Vector2 center = { 100,100 };
char keys[256] = { 0 };
char preKeys[256] = { 0 };
int smallColor = RED;
int bigColor = BLUE;
int redScore = 0;
int blueScore = 0;
bool hitFlag = false;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WSADATA wdData;
	static HANDLE hThread;
	static DWORD dwID;


	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	hwMain = GetDesktopWindow();

	bigPlayer.center.x = 200;
	bigPlayer.center.y = 360;
	bigPlayer.radius = 100;
	bigPlayer.speed = 8;

	smallPlayer.center.x = 1000;
	smallPlayer.center.y = 360;
	smallPlayer.radius = 50;
	smallPlayer.speed = 8;

	// winsock初期化
	WSAStartup(MAKEWORD(2, 0), &wdData);

	// データを送受信処理をスレッド（WinMainの流れに関係なく動作する処理の流れ）として生成。
	// データ送受信をスレッドにしないと何かデータを受信するまでRECV関数で止まってしまう。
	hThread = (HANDLE)CreateThread(NULL, 0, &Threadfunc, (LPVOID)&bigPlayer, 0, &dwID);

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		if (keys[DIK_UP] != 0) {
			bigPlayer.center.y -= bigPlayer.speed;
		}
		if (keys[DIK_DOWN] != 0) {
			bigPlayer.center.y += bigPlayer.speed;
		}
		if (keys[DIK_RIGHT] != 0) {
			bigPlayer.center.x += bigPlayer.speed;
		}
		if (keys[DIK_LEFT] != 0) {
			bigPlayer.center.x -= bigPlayer.speed;
		}

		///
		/// ↓更新処理ここから
		///

		//当たり判定の計算
		float distance =
			sqrtf((float)pow((double)bigPlayer.center.x - (double)smallPlayer.center.x, 2) +
				(float)pow((double)bigPlayer.center.y - (double)smallPlayer.center.y, 2));
		
		//接触したとき赤くなる
		if (distance <= bigPlayer.radius + smallPlayer.radius) {
			bigColor = RED;
			smallColor = BLUE;
			bigPlayer.center.x = 200;
			bigPlayer.center.y = 360;
			smallPlayer.center.x = 1000;
			smallPlayer.center.y = 360;
		}
		else if (distance >= bigPlayer.radius + smallPlayer.radius) {
			bigColor = BLUE;
			smallColor = RED;
		}

		if (hitFlag == true)
		{
			bigPlayer.center.x = 200;
			bigPlayer.center.y = 360;
			smallPlayer.center.x = 1000;
			smallPlayer.center.y = 360;
			hitFlag = false;
		}

		//プレイヤーが画面の外に出ないようにする
		if (bigPlayer.center.x + bigPlayer.radius > 1280)
		{
			hitFlag = true;
			blueScore += 1;
		}
		if (bigPlayer.center.x - bigPlayer.radius < 0)
		{
			bigPlayer.center.x = 0 + bigPlayer.radius;
		}
		if (bigPlayer.center.y + bigPlayer.radius > 720)
		{
			bigPlayer.center.y = 0 + bigPlayer.radius;
		}
		if (bigPlayer.center.y - bigPlayer.radius < 0)
		{
			bigPlayer.center.y = 720 - bigPlayer.radius;
		}

		if (smallPlayer.center.x + smallPlayer.radius > 1280)
		{
			smallPlayer.center.x = 1280 - smallPlayer.radius;
		}
		if (smallPlayer.center.x - smallPlayer.radius < 0)
		{
			hitFlag = true;
			redScore += 1;
		}
		if (smallPlayer.center.y + smallPlayer.radius > 720)
		{
			smallPlayer.center.y = 0 + smallPlayer.radius;
		}
		if (smallPlayer.center.y - smallPlayer.radius < 0)
		{
			smallPlayer.center.y = 720 - smallPlayer.radius;
		}

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		//各プレイヤーの描画
		Novice::DrawEllipse((int)bigPlayer.center.x, (int)bigPlayer.center.y, (int)bigPlayer.radius, (int)bigPlayer.radius, 0.0f, bigColor, kFillModeSolid);
		Novice::DrawEllipse((int)smallPlayer.center.x, (int)smallPlayer.center.y, (int)smallPlayer.radius, (int)smallPlayer.radius, 0.0f, smallColor, kFillModeSolid);

		//各スコアの描画
		if (blueScore <= 5 && redScore <= 5) {
			Novice::ScreenPrintf(0, 0, "blue Score:%d", blueScore);
			Novice::ScreenPrintf(0, 20, "red Score:%d", redScore);
			Novice::ScreenPrintf(900, 0, "blue Score Win Score 6");
			Novice::ScreenPrintf(900, 20, "red Score Win Score 6");
			Novice::ScreenPrintf(600, 0, "hit the opposite wall+1");
		}
		else if (blueScore >= 5) {
			Novice::ScreenPrintf(560, 300, "bluePlayer:win!!!");
		}
		else if (redScore >= 5) {
			Novice::ScreenPrintf(560, 320, "redPlayer:win!!!");
		}

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();

	// winsock終了
	WSACleanup();

	return 0;
}

/* 通信スレッド関数 */
DWORD WINAPI Threadfunc(void*) {

	SOCKET sConnect;
	SOCKADDR_IN saConnect;

	char addr[20]; // IPアドレス用配列

	// ファイル読み込み (#include <fstream> が必要)
	std::ifstream ifs("ip.txt");
	ifs.getline(addr, sizeof(addr));

	WORD wPort = 8000;

	// ソケット作成
	sConnect = socket(PF_INET, SOCK_STREAM, 0);

	// コネクト用SOCKADDR_IN変数を初期化
	ZeroMemory(&saConnect, sizeof(SOCKADDR_IN));
	saConnect.sin_family = AF_INET;
	saConnect.sin_addr.s_addr = inet_addr(addr);
	saConnect.sin_port = htons(wPort);

	// サーバに接続要求
	if (connect(sConnect, (sockaddr*)(&saConnect), sizeof(saConnect)) == SOCKET_ERROR)
	{
		SetWindowText(hwMain, L"接続エラー");
		closesocket(sConnect);
		WSACleanup();

		return 1;
	}

	SetWindowText(hwMain, L"送信開始\r\n");


	while (1)
	{
		int     nRcv;

		// 2Pの座標情報を送信
		send(sConnect, (const char*)&bigPlayer, sizeof(Circle), 0);

		// サーバから1P座標情報を取得
		nRcv = recv(sConnect, (char*)&smallPlayer, sizeof(Circle), 0);

		// 通信が途絶えたらループ終了
		if (nRcv == SOCKET_ERROR)
		{
			break;
		}
	}

	//ソケットを閉じる
	closesocket(sConnect);

	return 0;
}





#if 0
// 通信スレッド関数
DWORD WINAPI threadfunc(void* ) {

	SOCKET sConnect;
	struct sockaddr_in saConnect, saLocal;
	DWORD dwAddr;
	// 送信バッファ
	Circle* lpBuf = new Circle;

	char addr[17];
	WORD wPort = 8000;

	ifstream ifs("ip.txt");
	ifs.getline(addr, 16);

	dwAddr = inet_addr(addr);

	sConnect = socket(PF_INET, SOCK_STREAM, 0);

	ZeroMemory(&saConnect, sizeof(sockaddr_in));
	ZeroMemory(&saLocal, sizeof(sockaddr_in));

	saLocal.sin_family = AF_INET;
	saLocal.sin_addr.s_addr = INADDR_ANY;
	saLocal.sin_port = 0;

	bind(sConnect, (LPSOCKADDR)&saLocal, sizeof(saLocal));

	saConnect.sin_family = AF_INET;
	saConnect.sin_addr.s_addr = dwAddr;
	saConnect.sin_port = htons(wPort);

	// サーバーに接続
	if (connect(sConnect, (sockaddr*)(&saConnect), sizeof(saConnect)) == SOCKET_ERROR)
	{
		closesocket(sConnect);

		WSACleanup();

		return 1;
	}

	SetWindowText(hwMain, L"送信開始\r\n");

	// 通信スタート
	send(sConnect, (char*)&a, sizeof(Circle), 0);

	while (1)
	{
		int     nRcv;

		// サーバから1P座標情報を取得
		nRcv = recv(sConnect, (char*)&b, sizeof(Circle), 0);

		// 通信が途絶えたらループ終了
		if (nRcv == SOCKET_ERROR)
		{
			break;
		}

		// 2Pの座標情報を送信
		send(sConnect, (const char*)&a, sizeof(Circle), 0);
	}

	closesocket(sConnect);

	// 送信データバッファ解放
	delete lpBuf;

	return 0;
}
#endif