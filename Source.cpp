#include <windows.h> // підключення бібліотеки з функціями API
#include <time.h>
#include "resource.h" // Підключення меню

// Глобальні змінні:
HINSTANCE hInst; //Дескриптор програми
LPCTSTR szWindowClass = "QWERTY";
LPCTSTR szTitle = "Курсова СПро Сургай Р.В. КІУКІу-21-1";
HWND hWnd; // Глобальна hWnd
PAINTSTRUCT ps;
RECT rt;
HFONT hfont; // Стиль тексту

static HWND btnNewGame, btn2vs2, btnExit, btnlvl1, btnlvl2, btnlvl3, btnBack; // дескриптор кнопок
static HWND textMenu; // дескриптор тексту 

int y1 = 0, y2 = 0; // Значення У-координат для керування ракетками
int xBall = 0, yBall = 0; // Початкові координати м'яча
int scoreRed = 0, scoreBlue = 0; // Лічільник очок
bool start2vs2 = false; // Флаг розпочатка гри на двох
bool startVsBot = false; // Флаг розпочатка гри з ботом
int pickLvl = 0; // Змінна рівня складності
bool rightPush = true; // Напрямок польоту м'яча вліво-вправо
bool upDirection = true; // Напрямок польоту м'яча вверх-вниз
int ricochetAngle = 0; // Кут відбиття м'яча ракеткою (від 0 до 4)


// Попередній опис функцій
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


// Функція закриття програми
int exitProg() {
	switch (MessageBox(hWnd, "Ви впевнені?", "Закрити програму", MB_YESNO | MB_ICONQUESTION)) {

	case IDYES:
		PostQuitMessage(0); // Повідомлення про закриття програми передається на обробку в головний цикл
		break;

	case IDNO:
		return 0;
	}
}

// Функція малювання об'єктів на полі
void paintObject(HDC hdc, int y1, int y2, int xBall, int yBall, int scoreRed, int scoreBlue) {

	// Конвертація int в LPCSTR
	char r[2], b[2];
	_itoa_s(scoreRed, r, 10);
	_itoa_s(scoreBlue, b, 10);
	LPCSTR sr = r;
	LPCSTR sb = b;

	// Віртуальний контекст для малювання у пам'яті (для запобігання мерехтіння зображення) 
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP memBM = CreateCompatibleBitmap(hdc, 1000, 600); // Картинка на якій будем малювати через віртуальний контекст
	SelectObject(memDC, memBM); // На контексті memDC будем малювати memBM

	// Обновлюємо фон з кожним циклом ітерації у чорний колір
	HBRUSH fon = CreateSolidBrush(RGB(0, 0, 0)); // Встановлюємо колір пензлика для фону
	SelectObject(memDC, fon); // Підключаємо пензлик "fon" 
	Rectangle(memDC, 0, 0, 1000, 600); // Зафарбовуємо фон
	DeleteObject(fon); // Видаляємо об'єкт "fon"

	// Відрисовуємо фіолетову сітку
	HBRUSH net = CreateSolidBrush(RGB(255, 0, 255));
	SelectObject(memDC, net);
	Rectangle(memDC, 497, 0, 503, 542);
	DeleteObject(net);

	// Відрисовуємо панельки, які вібивають м'яч
	HBRUSH player1 = CreateSolidBrush(RGB(255, 0, 0));
	SelectObject(memDC, player1);
	Rectangle(memDC, 0, y1 + 200, 10, y1 + 300);
	DeleteObject(player1);

	HBRUSH player2 = CreateSolidBrush(RGB(0, 0, 255));
	SelectObject(memDC, player2);
	Rectangle(memDC, 973, y2 + 200, 983, y2 + 300);
	DeleteObject(player2);

	// Відрисовуємо білий м'яч
	HBRUSH ball = CreateSolidBrush(RGB(255, 255, 255));
	SelectObject(memDC, ball);
	Ellipse(memDC, xBall + 490, yBall + 240, xBall + 510, yBall + 260);
	DeleteObject(ball);

	// Кількість очок
	SetTextColor(memDC, RGB(255, 0, 0));
	SetBkColor(memDC, RGB(0, 255, 0));
	TextOut(memDC, 0, 0, sr, 1);

	SetTextColor(memDC, RGB(0, 0, 255));
	TextOut(memDC, 975, 0, sb, 1);

	// Копіюємо зображення з віртуальної пам'яті на нашу форму
	BitBlt(hdc, 0, 0, 1000, 600, memDC, 0, 0, SRCCOPY);

	// Очищуємо виділену пам'ять
	DeleteDC(memDC);
	DeleteObject(memBM);
}

// Алгоритм польоту/переміщення м'яча
void ballFlight(int &xBall, int &yBall, int y1, int y2, int &ricochetAngle, bool &rightPush, bool &upDirection) {
	// Умова відбиття ракеткою м'яча
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

	// Умова відскока м'яча від нижнього та верхнього кордону поля
	if (yBall > 280) upDirection = true;
	else if (yBall < -240) upDirection = false;

	// Виявлення напрмок руху м'яча по Х-координаті
	if (rightPush) xBall += 5;
	else xBall -= 5;

	// Виявлення напрямок руху м'яча по У-координаті
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

// Функція керування ракеткою ігроками
void playerControl(int &y1, int &y2, bool svb) {
	if (GetKeyState(38) < 0 && y2 > -200 && !svb) y2 -= 5;
	if (GetKeyState(40) < 0 && y2 < 240 && !svb) y2 += 5;
	if (GetKeyState('W') < 0 && y1 > -200) y1 -= 5;
	if (GetKeyState('S') < 0 && y1 < 240) y1 += 5;
}

// Функція керування ракеткою ботом
void funBot(int &y2, int xBall, int yBall, int pickLvl, bool rightPush) {
	// Ящко м'яч на території бота летить в нього
	if (rightPush) {
		// Якщо координати м'яча та ракетки бота сильно відхилені,
		// бот намагається підвести ракетку під м'яч
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

		// Повернення ракетки бота на ігрове поле
		if (y2 < -200) y2 = -200;
		else if (y2 > 240) y2 = 240;
	}
}

// Функція кінця гри
void gameOver(HDC hdc, HWND bng, HWND b2v2, HWND be, int &y1, int &y2, int &xb, int &yb, int &sr, int &sb, int &pl, int &ra, bool &rp, bool &ud, bool &s2v2, bool &svb) {
	// Умова програшу/виграшу
	if (xBall > 490) {
		sr++;

		switch (MessageBox(hWnd, "Червоний ігрок забив гол!",
			"ЧЕРВОНИЙ", MB_OK | MB_ICONINFORMATION)) {
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

			switch (MessageBox(hWnd, "Переміг червоний ігрок!",
				"ЧЕРВОНИЙ", MB_OK | MB_ICONINFORMATION)) {
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

		switch (MessageBox(hWnd, "Синій ігрок забив гол!",
			"СИНІЙ", MB_OK | MB_ICONINFORMATION)) {
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

			switch (MessageBox(hWnd, "Переміг синій ігрок!",
				"СИНІЙ", MB_OK | MB_ICONINFORMATION)) {
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


// Основна програма
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	HWND hWnd;

	// Реєстрація класу вікна
	MyRegisterClass(hInstance);

	hInst = hInstance; //зберігає дескриптор додатка в змінній hInst
	hWnd = CreateWindow(szWindowClass, // ім’я класу вікна
		szTitle, // назва програми
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, // стиль вікна
		270, // положення по Х
		100, // положення по Y
		1000, // розмір по Х
		600, // розмір по Y
		NULL, // дескриптор батьківського вікна
		NULL, // дескриптор меню вікна
		hInstance, // дескриптор програми
		NULL); // параметри створення.

	HDC hdc = GetDC(hWnd); // Задати контекст вікна
	ShowWindow(hWnd, nCmdShow); //Показати вікно
	UpdateWindow(hWnd); //Оновити вікно

	// Головний цикл
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

				// Якщо обрана гра з ботом, запускаємо функцію
				if (startVsBot) funBot(y2, xBall, yBall, pickLvl, rightPush);
				gameOver(hdc, btnNewGame, btn2vs2, btnExit, y1, y2, xBall, yBall, scoreRed, scoreBlue, pickLvl, ricochetAngle, rightPush, upDirection, start2vs2, startVsBot);

				// Міняємо затримку циклу залежно від вибраної складності
				if (pickLvl == 1) Sleep(10);
				else if (pickLvl == 2 || start2vs2) Sleep(5);
				else if (pickLvl == 3) Sleep(1);
			}
		}
	}

	return msg.wParam;
}

// ========================================================================================================

// Стиль класу основго вікна програми
ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; //стиль вікна
	wcex.lpfnWndProc = (WNDPROC)WndProc; //віконна процедура
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance; //дескриптор програми
	wcex.hIcon = LoadIcon(NULL, IDI_ERROR); //визначення іконки
	wcex.hCursor = LoadCursor(NULL, IDC_CROSS); //визначення курсору
	wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW + 2); //установка фону
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1); //визначення меню
	wcex.lpszClassName = szWindowClass; //ім’я класу
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex); //реєстрація класу вікна
}



// =======================================================================================================



// FUNCTION: WndProc (HWND, unsigned, WORD, LONG)
// Віконна процедура. Приймає і обробляє всі повідомлення, що приходять в додаток

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc = GetDC(hWnd);
	RECT rt;
	
	ReleaseDC(NULL, hdc);
	
	switch (message) {

	case WM_COMMAND: // Формування команд меню
		switch (LOWORD(wParam)) {
		
		case ID_INFO:
			MessageBox(hWnd, "Курсова робота з СПро\nСтудент групи: КІУКІу-21-1\nСургай Ростислав Віталійович\nВаріант: 36\n2022 рік\nВерсія: 1.0", 
				"Про програму", MB_OK | MB_ICONINFORMATION);
			break;

		case ID_MAINMENU:
			if (start2vs2 || startVsBot) {
				switch (MessageBox(hWnd, "Ви впевнені, що хочете залишити гру?",
					"Головне меню", MB_YESNO | MB_ICONINFORMATION)) {

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

		// Обрабка кнопки "Закрити програму"
		if (lParam == (LPARAM)btnExit) {
			exitProg();
		}

		// Обробка кнопки "Одиночна гра"
		if (lParam == (LPARAM)btnNewGame) {
			// Приховуємо інші кнопки
			ShowWindow(btnExit, SW_HIDE);
			ShowWindow(btnNewGame, SW_HIDE);
			ShowWindow(btn2vs2, SW_HIDE);

			// Виводимо кнопки для вибору складності
			ShowWindow(btnlvl1, SW_SHOW);
			ShowWindow(btnlvl2, SW_SHOW);
			ShowWindow(btnlvl3, SW_SHOW);
			ShowWindow(btnBack, SW_SHOW);
		}

		// Обробка кнопки "Легкий рівень"
		if (lParam == (LPARAM)btnlvl1) {
			ShowWindow(btnlvl1, SW_HIDE);
			ShowWindow(btnlvl2, SW_HIDE);
			ShowWindow(btnlvl3, SW_HIDE);
			ShowWindow(btnBack, SW_HIDE);

			startVsBot = true;
			pickLvl = 1;
		}

		// Обробка кнопки "Середній рівень"
		if (lParam == (LPARAM)btnlvl2) {
			ShowWindow(btnlvl1, SW_HIDE);
			ShowWindow(btnlvl2, SW_HIDE);
			ShowWindow(btnlvl3, SW_HIDE);
			ShowWindow(btnBack, SW_HIDE);

			startVsBot = true;
			pickLvl = 2;
		}

		// Обробка кнопки "Складний рівень"
		if (lParam == (LPARAM)btnlvl3) {
			ShowWindow(btnlvl1, SW_HIDE);
			ShowWindow(btnlvl2, SW_HIDE);
			ShowWindow(btnlvl3, SW_HIDE);
			ShowWindow(btnBack, SW_HIDE);

			startVsBot = true;
			pickLvl = 3;
		}

		// Обробка кнопки "Гра на двох"
		if (lParam == (LPARAM)btn2vs2) {
			ShowWindow(btnExit, SW_HIDE);
			ShowWindow(btnNewGame, SW_HIDE);
			ShowWindow(btn2vs2, SW_HIDE);

			start2vs2 = true;
		}

		// Обробка кнопки "Назад"
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

	case WM_CLOSE: // Забороняє закривати вікно в системному меню
		exitProg();
		break;

	case WM_CREATE: //Повідомлення приходить при створенні вікна
		// Створюємо кнопку для одиночної гри
		btnNewGame = CreateWindow("button", "Одиночна гра", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
			400, 150, 200, 50, hWnd, (HMENU)"ID_NEWGAME", NULL, NULL);

		// Створення кнопки для гри на двох
		btn2vs2 = CreateWindow("button", "Гра на двох", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
			400, 230, 200, 50, hWnd, (HMENU)"ID_2VS2", NULL, NULL);

		// Створення кнопки закриття програми
		btnExit = CreateWindow("button", "Закрити гру", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
			400, 310, 200, 50, hWnd, (HMENU)"ID_EXITGAME", NULL, NULL);

		// Кнопки для вибору рівня складності та кнопка "Назад"
		btnlvl1 = CreateWindow("button", "Легкий", WS_CHILD | BS_PUSHBUTTON | SW_HIDE,
			400, 150, 200, 50, hWnd, (HMENU)"ID_LOW_LVL", NULL, NULL);
		btnlvl2 = CreateWindow("button", "Середній", WS_CHILD | BS_PUSHBUTTON | SW_HIDE,
			400, 230, 200, 50, hWnd, (HMENU)"ID_MID_LVL", NULL, NULL);
		btnlvl3 = CreateWindow("button", "Складний", WS_CHILD | BS_PUSHBUTTON | SW_HIDE,
			400, 310, 200, 50, hWnd, (HMENU)"ID_HARD_LVL", NULL, NULL);
		btnBack = CreateWindow("button", "Назад", WS_CHILD | BS_PUSHBUTTON | SW_HIDE,
			450, 390, 100, 50, hWnd, (HMENU)"ID_BACK", NULL, NULL);
		break;

	case WM_PAINT: //Перемалювати вікно
		hdc = BeginPaint(hWnd, &ps); //Почати графічний вивід
		GetClientRect(hWnd, &rt); //Область вікна для малювання
		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
		GetClientRect(hWnd, &rt);
		break;

	default:
		//Обробка повідомлень, які не оброблені користувачем
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}