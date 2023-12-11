#include <windows.h> // ���������� �������� � ��������� API
#include <time.h>
#include "resource.h" // ϳ��������� ����

// �������� ����:
HINSTANCE hInst; //���������� ��������
LPCTSTR szWindowClass = "QWERTY";
LPCTSTR szTitle = "������� ���� ������ �.�. ʲ�ʲ�-21-1";
HWND hWnd; // ��������� hWnd
PAINTSTRUCT ps;
RECT rt;
HFONT hfont; // ����� ������

static HWND btnNewGame, btn2vs2, btnExit, btnlvl1, btnlvl2, btnlvl3, btnBack; // ���������� ������
static HWND textMenu; // ���������� ������ 

int y1 = 0, y2 = 0; // �������� �-��������� ��� ��������� ���������
int xBall = 0, yBall = 0; // �������� ���������� �'���
int scoreRed = 0, scoreBlue = 0; // ˳������� ����
bool start2vs2 = false; // ���� ���������� ��� �� ����
bool startVsBot = false; // ���� ���������� ��� � �����
int pickLvl = 0; // ����� ���� ���������
bool rightPush = true; // �������� ������� �'��� ����-������
bool upDirection = true; // �������� ������� �'��� �����-����
int ricochetAngle = 0; // ��� ������� �'��� �������� (�� 0 �� 4)


// ��������� ���� �������
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// ===============================================================================================================

class Ball {
public:
	int xBall;
	int yBall;

	Ball(int x, int y) {
		this->xBall = x;
		this->yBall = y;
	}
};

class GamePlay {
public:
	int y1;
	int y2;
	int pickLvl;
	int ricochetAngle;
	int scoreRed;
	int scoreBlue;
	bool start2vs2;
	bool startVsBot;
	bool rightPush;
	bool upDirection;

	GamePlay(int y1, int y2, int pl, int ra, int sr, int sb, bool s2v2, bool svb, bool rp, bool ud) {
		this->y1 = y1;
		this->y2 = y2;
		this->pickLvl = pl;
		this->ricochetAngle = ra;
		this->scoreRed = sr;
		this->scoreBlue = sb;
		this->start2vs2 = s2v2;
		this->startVsBot = svb;
		this->rightPush = rp;
		this->upDirection = ud;
	}
};

Ball ball = Ball(0, 0);
GamePlay parameters = GamePlay(0, 0, 0, 0, 0, 0, false, false, true, true);


// ������� �������� ��������
int exitProg() {
	switch (MessageBox(hWnd, "�� �������?", "������� ��������", MB_YESNO | MB_ICONQUESTION)) {

	case IDYES:
		PostQuitMessage(0); // ����������� ��� �������� �������� ���������� �� ������� � �������� ����
		break;

	case IDNO:
		return 0;
	}
}

// ������� ��������� ��'���� �� ���
void paintObject(HDC hdc, int y1, int y2, int xBall, int yBall, int scoreRed, int scoreBlue) {

	// ����������� int � LPCSTR
	char r[2], b[2];
	_itoa_s(scoreRed, r, 10);
	_itoa_s(scoreBlue, b, 10);
	LPCSTR sr = r;
	LPCSTR sb = b;

	// ³��������� �������� ��� ��������� � ���'�� (��� ���������� ��������� ����������) 
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP memBM = CreateCompatibleBitmap(hdc, 1000, 600); // �������� �� ��� ����� �������� ����� ���������� ��������
	SelectObject(memDC, memBM); // �� �������� memDC ����� �������� memBM

	// ���������� ��� � ������ ������ �������� � ������ ����
	HBRUSH fon = CreateSolidBrush(RGB(0, 0, 0)); // ������������ ���� �������� ��� ����
	SelectObject(memDC, fon); // ϳ�������� ������� "fon" 
	Rectangle(memDC, 0, 0, 1000, 600); // ����������� ���
	DeleteObject(fon); // ��������� ��'��� "fon"

	// ³��������� ��������� ����
	HBRUSH net = CreateSolidBrush(RGB(255, 0, 255));
	SelectObject(memDC, net);
	Rectangle(memDC, 497, 0, 503, 542);
	DeleteObject(net);

	// ³��������� ��������, �� �������� �'��
	HBRUSH player1 = CreateSolidBrush(RGB(255, 0, 0));
	SelectObject(memDC, player1);
	Rectangle(memDC, 0, y1 + 200, 10, y1 + 300);
	DeleteObject(player1);

	HBRUSH player2 = CreateSolidBrush(RGB(0, 0, 255));
	SelectObject(memDC, player2);
	Rectangle(memDC, 973, y2 + 200, 983, y2 + 300);
	DeleteObject(player2);

	// ³��������� ���� �'��
	HBRUSH ball = CreateSolidBrush(RGB(255, 255, 255));
	SelectObject(memDC, ball);
	Ellipse(memDC, xBall + 490, yBall + 240, xBall + 510, yBall + 260);
	DeleteObject(ball);

	// ʳ������ ����
	SetTextColor(memDC, RGB(255, 0, 0));
	SetBkColor(memDC, RGB(0, 255, 0));
	TextOut(memDC, 0, 0, sr, 1);

	SetTextColor(memDC, RGB(0, 0, 255));
	TextOut(memDC, 975, 0, sb, 1);

	// ������� ���������� � ��������� ���'�� �� ���� �����
	BitBlt(hdc, 0, 0, 1000, 600, memDC, 0, 0, SRCCOPY);

	// ������� ������� ���'���
	DeleteDC(memDC);
	DeleteObject(memBM);
}

// �������� �������/���������� �'���
void ballFlight(int &xBall, int &yBall, int y1, int y2, int &ricochetAngle, bool &rightPush, bool &upDirection) {
	// ����� ������� �������� �'���
	if (xBall > 465 && (abs(yBall - y2) < 51)) {
		rightPush = false;
		if (abs(yBall - y2) >= 0 && abs(yBall - y2) < 10) ricochetAngle = 0;
		else if (abs(yBall - y2) >= 10 && abs(yBall - y2) < 20) ricochetAngle = 1;
		else if (abs(yBall - y2) >= 20 && abs(yBall - y2) < 30) ricochetAngle = 2;
		else if (abs(yBall - y2) >= 30 && abs(yBall - y2) < 40) ricochetAngle = 3;
		else if (abs(yBall - y2) >= 40) ricochetAngle = 4;
	}

	else if (xBall < -480 && (abs(yBall - y1) < 51)) {
		rightPush = true;
		if (abs(yBall - y1) >= 0 && abs(yBall - y1) < 10) ricochetAngle = 0;
		else if (abs(yBall - y1) >= 10 && abs(yBall - y1) < 20) ricochetAngle = 1;
		else if (abs(yBall - y1) >= 20 && abs(yBall - y1) < 30) ricochetAngle = 2;
		else if (abs(yBall - y1) >= 30 && abs(yBall - y1) < 40) ricochetAngle = 3;
		else if (abs(yBall - y1) >= 40) ricochetAngle = 4;
	}

	// ����� ������� �'��� �� �������� �� ��������� ������� ����
	if (yBall > 280) upDirection = true;
	else if (yBall < -240) upDirection = false;

	// ��������� ������� ���� �'��� �� �-���������
	if (rightPush) xBall += 5;
	else xBall -= 5;

	// ��������� �������� ���� �'��� �� �-���������
	if (upDirection) {
		switch (ricochetAngle) {
		case 0: break;
		case 1:
			yBall -= 3;
			break;
		case 2:
			yBall -= 5;
			break;
		case 3:
			yBall -= 6;
			break;
		case 4:
			yBall -= 7;
			break;
		}
	}

	else {
		switch (ricochetAngle) {
		case 0: break;
		case 1:
			yBall += 3;
			break;
		case 2:
			yBall += 5;
			break;
		case 3:
			yBall += 6;
			break;
		case 4:
			yBall += 7;
			break;
		}
	}
}

// ������� ��������� �������� ��������
void playerControl(int &y1, int &y2, bool svb) {
	if (GetKeyState(38) < 0 && y2 > -200 && !svb) y2 -= 5;
	if (GetKeyState(40) < 0 && y2 < 240 && !svb) y2 += 5;
	if (GetKeyState('W') < 0 && y1 > -200) y1 -= 5;
	if (GetKeyState('S') < 0 && y1 < 240) y1 += 5;
}

// ������� ��������� �������� �����
void funBot(int &y2, int xBall, int yBall, int pickLvl, bool rightPush) {
	// ���� �'�� �� ������� ���� ������ � �����
	if (rightPush) {
		// ���� ���������� �'��� �� ������� ���� ������ �������,
		// ��� ���������� ������� ������� �� �'��
		if ((y2 - yBall) > 20 && y2 >= -200 && y2 <= 240 && xBall > 0) {
			if (pickLvl == 1) y2 -= 3;
			else if (pickLvl == 2) y2 -= 4;
			else if (pickLvl == 3) y2 -= 5;
		}

		else if ((y2 - yBall) < 20 && y2 >= -200 && y2 <= 240 && xBall > 0) {
			if (pickLvl == 1) y2 += 3;
			else if (pickLvl == 2) y2 += 4;
			else if (pickLvl == 3) y2 += 5;
		}

		// ���������� ������� ���� �� ������ ����
		if (y2 < -200) y2 = -200;
		else if (y2 > 240) y2 = 240;
	}
}

// ������� ���� ���
void gameOver(HDC hdc, HWND bng, HWND b2v2, HWND be, int &y1, int &y2, int &xb, int &yb, int &sr, int &sb, int &pl, int &ra, bool &rp, bool &ud, bool &s2v2, bool &svb) {
	// ����� ��������/�������
	if (xBall > 490) {
		sr++;

		switch (MessageBox(hWnd, "�������� ����� ����� ���!",
			"��������", MB_OK | MB_ICONINFORMATION)) {
		case IDOK:
			y1 = y2 = xb = yb = ra = 0;
			rp = ud = true;
		}

		if (sr >= 10) {
			HBRUSH fon = CreateSolidBrush(RGB(255, 0, 0));
			SelectObject(hdc, fon);
			Rectangle(hdc, 0, 0, 1000, 600);
			DeleteObject(fon);
			s2v2 = svb = false;
			pl = sb = sr = 0;

			switch (MessageBox(hWnd, "������ �������� �����!",
				"��������", MB_OK | MB_ICONINFORMATION)) {
			case IDOK:
				HBRUSH fon = CreateSolidBrush(RGB(0, 0, 0));
				SelectObject(hdc, fon);
				Rectangle(hdc, 0, 0, 1000, 600);
				DeleteObject(fon);
				ShowWindow(be, SW_SHOW);
				ShowWindow(bng, SW_SHOW);
				ShowWindow(b2v2, SW_SHOW);
			}
		}
	}

	else if (xBall < -505) {
		sb++;

		switch (MessageBox(hWnd, "���� ����� ����� ���!",
			"��Ͳ�", MB_OK | MB_ICONINFORMATION)) {
		case IDOK:
			y1 = y2 = xb = yb = ra = 0;
			rp = ud = true;
		}

		if (sb >= 10) {
			HBRUSH fon = CreateSolidBrush(RGB(0, 0, 255));
			SelectObject(hdc, fon);
			Rectangle(hdc, 0, 0, 1000, 600);
			DeleteObject(fon);
			start2vs2 = false;
			startVsBot = false;
			pl = sb = sr = 0;

			switch (MessageBox(hWnd, "������ ���� �����!",
				"��Ͳ�", MB_OK | MB_ICONINFORMATION)) {
			case IDOK:
				HBRUSH fon = CreateSolidBrush(RGB(0, 0, 0));
				SelectObject(hdc, fon);
				Rectangle(hdc, 0, 0, 1000, 600);
				DeleteObject(fon);
				ShowWindow(be, SW_SHOW);
				ShowWindow(bng, SW_SHOW);
				ShowWindow(b2v2, SW_SHOW);
			}
		}
	}
}



// ============================================================================================================


// ������� ��������
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	HWND hWnd;

	// ��������� ����� ����
	MyRegisterClass(hInstance);

	hInst = hInstance; //������ ���������� ������� � ����� hInst
	hWnd = CreateWindow(szWindowClass, // ��� ����� ����
		szTitle, // ����� ��������
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, // ����� ����
		270, // ��������� �� �
		100, // ��������� �� Y
		1000, // ����� �� �
		600, // ����� �� Y
		NULL, // ���������� ������������ ����
		NULL, // ���������� ���� ����
		hInstance, // ���������� ��������
		NULL); // ��������� ���������.

	HDC hdc = GetDC(hWnd); // ������ �������� ����
	ShowWindow(hWnd, nCmdShow); //�������� ����
	UpdateWindow(hWnd); //������� ����

	// �������� ����
	while (TRUE) {
		if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) break;
			if (msg.message == WM_COMMAND && (start2vs2 || startVsBot)) {
				HBRUSH fon = CreateSolidBrush(RGB(0, 0, 0));
				SelectObject(hdc, fon); 
				Rectangle(hdc, 0, 0, 1000, 600);
				DeleteObject(fon);
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			if (start2vs2 || startVsBot) {
				paintObject(hdc, y1, y2, xBall, yBall, scoreRed, scoreBlue);
				playerControl(y1, y2, startVsBot);
				ballFlight(xBall, yBall, y1, y2, ricochetAngle, rightPush, upDirection);

				// ���� ������ ��� � �����, ��������� �������
				if (startVsBot) funBot(y2, xBall, yBall, pickLvl, rightPush);
				gameOver(hdc, btnNewGame, btn2vs2, btnExit, y1, y2, xBall, yBall, scoreRed, scoreBlue, pickLvl, ricochetAngle, rightPush, upDirection, start2vs2, startVsBot);

				// ̳����� �������� ����� ������� �� ������� ���������
				if (pickLvl == 1) Sleep(10);
				else if (pickLvl == 2 || start2vs2) Sleep(5);
				else if (pickLvl == 3) Sleep(1);
			}
		}
	}

	return msg.wParam;
}

// ========================================================================================================

// ����� ����� ������� ���� ��������
ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; //����� ����
	wcex.lpfnWndProc = (WNDPROC)WndProc; //������ ���������
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; //���������� ��������
	wcex.hIcon = LoadIcon(NULL, IDI_ERROR); //���������� ������
	wcex.hCursor = LoadCursor(NULL, IDC_CROSS); //���������� �������
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW + 2); //��������� ����
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); //���������� ����
	wcex.lpszClassName = szWindowClass; //��� �����
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex); //��������� ����� ����
}



// =======================================================================================================



// FUNCTION: WndProc (HWND, unsigned, WORD, LONG)
// ³����� ���������. ������ � �������� �� �����������, �� ��������� � �������

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc = GetDC(hWnd);
	RECT rt;
	
	ReleaseDC(NULL, hdc);
	
	switch (message) {

	case WM_COMMAND: // ���������� ������ ����
		switch (LOWORD(wParam)) {
		
		case ID_INFO:
			MessageBox(hWnd, "������� ������ � ����\n������� �����: ʲ�ʲ�-21-1\n������ ��������� ³��������\n������: 36\n2022 ��\n�����: 1.0", 
				"��� ��������", MB_OK | MB_ICONINFORMATION);
			break;

		case ID_MAINMENU:
			if (start2vs2 || startVsBot) {
				switch (MessageBox(hWnd, "�� �������, �� ������ �������� ���?",
					"������� ����", MB_YESNO | MB_ICONINFORMATION)) {

				case IDYES:
					ShowWindow(btnExit, SW_SHOW);
					ShowWindow(btnNewGame, SW_SHOW);
					ShowWindow(btn2vs2, SW_SHOW);
					start2vs2 = startVsBot = false;
					rightPush = upDirection = true;
					y1 = y2 = xBall = yBall = scoreRed = scoreBlue = pickLvl = ricochetAngle = 0;
					break;
				}
			}
			break;
		};

		// ������� ������ "������� ��������"
		if (lParam == (LPARAM)btnExit) {
			exitProg();
		}

		// ������� ������ "�������� ���"
		if (lParam == (LPARAM)btnNewGame) {
			// ��������� ���� ������
			ShowWindow(btnExit, SW_HIDE);
			ShowWindow(btnNewGame, SW_HIDE);
			ShowWindow(btn2vs2, SW_HIDE);

			// �������� ������ ��� ������ ���������
			ShowWindow(btnlvl1, SW_SHOW);
			ShowWindow(btnlvl2, SW_SHOW);
			ShowWindow(btnlvl3, SW_SHOW);
			ShowWindow(btnBack, SW_SHOW);
		}

		// ������� ������ "������ �����"
		if (lParam == (LPARAM)btnlvl1) {
			ShowWindow(btnlvl1, SW_HIDE);
			ShowWindow(btnlvl2, SW_HIDE);
			ShowWindow(btnlvl3, SW_HIDE);
			ShowWindow(btnBack, SW_HIDE);

			startVsBot = true;
			pickLvl = 1;
		}

		// ������� ������ "������� �����"
		if (lParam == (LPARAM)btnlvl2) {
			ShowWindow(btnlvl1, SW_HIDE);
			ShowWindow(btnlvl2, SW_HIDE);
			ShowWindow(btnlvl3, SW_HIDE);
			ShowWindow(btnBack, SW_HIDE);

			startVsBot = true;
			pickLvl = 2;
		}

		// ������� ������ "�������� �����"
		if (lParam == (LPARAM)btnlvl3) {
			ShowWindow(btnlvl1, SW_HIDE);
			ShowWindow(btnlvl2, SW_HIDE);
			ShowWindow(btnlvl3, SW_HIDE);
			ShowWindow(btnBack, SW_HIDE);

			startVsBot = true;
			pickLvl = 3;
		}

		// ������� ������ "��� �� ����"
		if (lParam == (LPARAM)btn2vs2) {
			ShowWindow(btnExit, SW_HIDE);
			ShowWindow(btnNewGame, SW_HIDE);
			ShowWindow(btn2vs2, SW_HIDE);

			start2vs2 = true;
		}

		// ������� ������ "�����"
		if (lParam == (LPARAM)btnBack) {
			ShowWindow(btnExit, SW_SHOW);
			ShowWindow(btnNewGame, SW_SHOW);
			ShowWindow(btn2vs2, SW_SHOW);

			ShowWindow(btnlvl1, SW_HIDE);
			ShowWindow(btnlvl2, SW_HIDE);
			ShowWindow(btnlvl3, SW_HIDE);
			ShowWindow(btnBack, SW_HIDE);
		}
		break;

	case WM_CLOSE: // ��������� ��������� ���� � ���������� ����
		exitProg();
		break;

	case WM_CREATE: //����������� ��������� ��� �������� ����
		// ��������� ������ ��� �������� ���
		btnNewGame = CreateWindow("button", "�������� ���", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
			400, 150, 200, 50, hWnd, (HMENU)"ID_NEWGAME", NULL, NULL);

		// ��������� ������ ��� ��� �� ����
		btn2vs2 = CreateWindow("button", "��� �� ����", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
			400, 230, 200, 50, hWnd, (HMENU)"ID_2VS2", NULL, NULL);

		// ��������� ������ �������� ��������
		btnExit = CreateWindow("button", "������� ���", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
			400, 310, 200, 50, hWnd, (HMENU)"ID_EXITGAME", NULL, NULL);

		// ������ ��� ������ ���� ��������� �� ������ "�����"
		btnlvl1 = CreateWindow("button", "������", WS_CHILD | BS_PUSHBUTTON | SW_HIDE,
			400, 150, 200, 50, hWnd, (HMENU)"ID_LOW_LVL", NULL, NULL);
		btnlvl2 = CreateWindow("button", "�������", WS_CHILD | BS_PUSHBUTTON | SW_HIDE,
			400, 230, 200, 50, hWnd, (HMENU)"ID_MID_LVL", NULL, NULL);
		btnlvl3 = CreateWindow("button", "��������", WS_CHILD | BS_PUSHBUTTON | SW_HIDE,
			400, 310, 200, 50, hWnd, (HMENU)"ID_HARD_LVL", NULL, NULL);
		btnBack = CreateWindow("button", "�����", WS_CHILD | BS_PUSHBUTTON | SW_HIDE,
			450, 390, 100, 50, hWnd, (HMENU)"ID_BACK", NULL, NULL);
		break;

	case WM_PAINT: //������������ ����
		hdc = BeginPaint(hWnd, &ps); //������ ��������� ����
		GetClientRect(hWnd, &rt); //������� ���� ��� ���������
		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
		GetClientRect(hWnd, &rt);
		break;

	default:
		//������� ����������, �� �� �������� ������������
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}