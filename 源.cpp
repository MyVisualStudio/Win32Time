#include <Windows.h>
#include <wininet.h>
//#include <iostream>
//#include <tchar.h>
#include "tinyxml2.h"
#pragma comment(lib, "WinInet.lib")

#define CLIENT_WIDTH		400
#define CLIENT_HEIGHT		150

bool deleteAll = false;

HBRUSH brush = NULL;
//���ڴ�С
RECT rect = { 0, 0, CLIENT_WIDTH, CLIENT_HEIGHT };
//���
HDC dc;
HDC bmpDC0,bmpDC1;
//ʱ��
SYSTEMTIME _time;

char* str = new char[64];

char* month[] = { ("Jan"), ("Feb"), ("March"), ("Apr"), ("May"),
("June"), ("July"), ("Aug"), ("Sep"), ("Oct"), ("Nov"), ("Dec") };

char* week[] = {("Sun"),("Mon"), ("Tue"), ("Wed"), ("Thu"),
("Fri"), ("Sat") };

char temp0[4] = {0};//���
char temp1[4] = {0};//���
char temp_cur[4] = { 0 };//��ǰ
char city[16] = {0};//����

LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


const char* FixChar(const char* str)
{
	DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);    //����ԭʼASCII����ַ���Ŀ       
	wchar_t* pw = new wchar_t[dwNum];                              //����ASCII����ַ�������UTF8�Ŀռ�
	char text[64] = {};
	MultiByteToWideChar(CP_UTF8, 0, str, -1, pw, dwNum);           //��ASCII��ת����UTF8
	WideCharToMultiByte(CP_THREAD_ACP, 0, pw, dwNum, text, dwNum, NULL, NULL); //��UTF8��ת����ASCII

	delete[] pw;

	return text;
}

int GetWeatherHttp()
{
	HINTERNET m_hSession = InternetOpenA(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	if (NULL == m_hSession)
	{
		InternetCloseHandle(m_hSession);
		return 0;
	}

	HINTERNET m_hConnect = InternetConnectA(m_hSession, "api.k780.com", 88, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (NULL == m_hConnect)
	{
		InternetCloseHandle(m_hConnect);
		InternetCloseHandle(m_hSession);
		return 0;
	}
//
//http://api.k780.com:88/?app=weather.today&weaid=chengdu&appkey=25228&sign=c72625a834531f2dae77ba8139b16b37&format=xml
	HINTERNET hOpenRequest = HttpOpenRequestA(m_hConnect, "GET", "/?app=weather.today&weaid=chengdu&appkey=25228&sign=c72625a834531f2dae77ba8139b16b37&format=xml", "HTTP/1.1", NULL,
		NULL, INTERNET_FLAG_DONT_CACHE, 1);

	if (NULL == hOpenRequest)
	{
		InternetCloseHandle(hOpenRequest);
		InternetCloseHandle(m_hConnect);
		InternetCloseHandle(m_hSession);
		return 0;
	}

	BOOL bRequest = HttpSendRequestA(hOpenRequest, NULL, 0, NULL, 0); //����http����

	if (bRequest)
	{
		char szBuffer[1024] = { 0 };
		char result[2048] = { 0 };
		int point = 0;
		DWORD dwByteRead = 0;
		
		FILE* fp = NULL;
		char text[32] = { 0 };
		sprintf(text, "����\\%d-%d.xml", _time.wYear, _time.wMonth);
		fopen_s(&fp, text, "a+");//д�뱾��

		sprintf(text, "\rDay# %d_@@_Time# %d:%0.2d %s\r", _time.wDay, _time.wHour, _time.wMinute, week[_time.wDayOfWeek]);
		fwrite(text, 1, strlen(text), fp);//���в�д��Head

		while (InternetReadFile(hOpenRequest, szBuffer, sizeof(szBuffer), &dwByteRead) && dwByteRead > 0)
		{
			memcpy(result+point, szBuffer, dwByteRead);
			point += dwByteRead;
			fwrite(szBuffer, dwByteRead, 1, fp);
			ZeroMemory(szBuffer, dwByteRead);
		}
		fclose(fp);

		tinyxml2::XMLDocument* pDoc = new tinyxml2::XMLDocument();
		pDoc->Parse(result);
		if (pDoc->Error())
			return 0;

		tinyxml2::XMLElement* pRoot = pDoc->RootElement();
		tinyxml2::XMLNode* pWeather = pRoot->FirstChildElement("result");
		
		strcpy(city, FixChar((pWeather->FirstChildElement("citynm"))->GetText()));
		strcpy(temp0, FixChar((pWeather->FirstChildElement("temp_low"))->GetText()));
		strcpy(temp1, FixChar((pWeather->FirstChildElement("temp_high"))->GetText()));
		strcpy(temp_cur, FixChar((pWeather->FirstChildElement("temp_curr"))->GetText()));
		//������ʾͼ��0
		char icon_url[64] = {};
		HBITMAP hbmp = NULL;
		strcpy(icon_url, FixChar((pWeather->FirstChildElement("weather_icon"))->GetText()));
		if (strcmp(icon_url, "null") != 0)
		{
			char* po = icon_url + 30;
			sprintf(icon_url, "%s.bmp", po);

			hbmp = (HBITMAP)LoadImageA(NULL, icon_url, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			if (bmpDC0 != NULL)
				DeleteDC(bmpDC0);
			bmpDC0 = NULL;
			bmpDC0 = CreateCompatibleDC(dc);
			if (hbmp)
				DeleteObject(SelectObject(bmpDC0, hbmp));
		}
		//������ʾͼ��1
		memset(icon_url, 64, 0);
		strcpy(icon_url, FixChar((pWeather->FirstChildElement("weather_icon1"))->GetText()));
		if (strcmp(icon_url, "null") != 0)
		{
			char* pi = icon_url + 30;
			sprintf(icon_url, "%s.bmp", pi);

			hbmp = (HBITMAP)LoadImageA(NULL, icon_url, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			if (bmpDC1 != NULL)
				DeleteDC(bmpDC1);
			bmpDC1 = NULL;
			bmpDC1 = CreateCompatibleDC(dc);
			if (hbmp)
				DeleteObject(SelectObject(bmpDC1, hbmp));
		}

		pDoc->Clear();
		delete pDoc;
	}

	InternetCloseHandle(m_hSession);
	InternetCloseHandle(m_hConnect);
	InternetCloseHandle(hOpenRequest);

	return 1;
}

int __stdcall WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR cmdLine,
	int nCmdShow)
{
	WNDCLASS wc;

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = TEXT("First");
	wc.lpszMenuName = NULL;
	wc.style = CS_VREDRAW | CS_HREDRAW;

	RegisterClass(&wc);

	//��ȡ���������С  
	RECT r;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
	r.left = r.right - CLIENT_WIDTH;
	r.top = r.bottom - CLIENT_HEIGHT;

//#ifdef WIN32
//	_tsetlocale(LC_ALL, _T(""));
//	::AllocConsole();
//	::freopen("conout$", "w", stdout);
//	::freopen("CONIN$", "r", stdin);
//	::freopen("CONOUT$", "w", stderr);
//#endif

	HWND hWnd = CreateWindow(wc.lpszClassName, TEXT(""),
		WS_POPUP,
		r.left, r.top, (r.right - r.left), (r.bottom - r.top),GetDesktopWindow(),
		0, hInstance, 0);

	//HWND h = FindWindow(("Progman"), NULL);
	//SetParent(hWnd, h);

	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED 
		| WS_EX_TOOLWINDOW);
	SetLayeredWindowAttributes(hWnd, RGB(249, 201, 201), 255, LWA_COLORKEY);
	
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, hWnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (!deleteAll)
	{
		deleteAll = true;
		KillTimer(hWnd, 1);
		delete[] str;
		DeleteObject(brush);
		if (bmpDC0)
			DeleteDC(bmpDC0);
		if (bmpDC1)
			DeleteDC(bmpDC1);
		ReleaseDC(hWnd, dc);
	}
	return TRUE;
}



LRESULT __stdcall WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			SetTimer(hWnd, 1, 996, NULL);//�趨ʱ��
			dc = GetDC(hWnd);
			brush = CreateSolidBrush(RGB(249, 201, 201));
			SetTextColor(dc, RGB(10, 15, 10));
			SetBkMode(dc, TRANSPARENT);
			HFONT hFont = CreateFontA(
				50, 18,    //�߶�50, ��ȡ20��ʾ��ϵͳѡ�����ֵ  
				0, 0,    //�ı���б����������б��Ϊ0  
				FW_HEAVY,    //����  
				0, 0, 0,        //��б�壬���»��ߣ����л���  
				DEFAULT_CHARSET,    //�ַ���  
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,        //һϵ�е�Ĭ��ֵ  
				DEFAULT_PITCH | FF_DONTCARE,
				("Monotype Corsiva")    //��������  
				);
			DeleteObject(SelectObject(dc, hFont));

			GetLocalTime(&_time);
			GetWeatherHttp();
		}
		return 0;

		case WM_TIMER:
		{
			if (wParam == 1)
			{         
				GetLocalTime(&_time);
				sprintf(str, "%0.2d:%0.2d:%0.2d %s %d\n%s %-2d %s\n     %s-%s@%s",
					_time.wHour,
					_time.wMinute,
					_time.wSecond,
					week[_time.wDayOfWeek],
					_time.wYear,
					month[_time.wMonth - 1],
					_time.wDay,
					city,
					temp0,
					temp1,
					temp_cur
					);
				 
					FillRect(dc, &rect, brush);
					DrawTextA(dc, str, strlen(str), &rect, DT_CENTER | DT_WORDBREAK | DT_MODIFYSTRING);
					if (bmpDC0)
						BitBlt(dc, 30, 104, 50, 46, bmpDC0, 0, 0, SRCCOPY);
					if (bmpDC1)
						BitBlt(dc, 82, 104, 50, 46, bmpDC1, 0, 0, SRCCOPY);

					if (0 == _time.wMinute && 0 == _time.wSecond)
						GetWeatherHttp();
			}
		
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			SendMessage(hWnd, WM_SYSCOMMAND, 0xF012, 0);//�����ƶ���ק�����ƶ�ָ��
		}
		return 0;

		case WM_DESTROY:
		{
			 PostQuitMessage(0);
			 if (!deleteAll)
			 {
				 deleteAll = true;
				 KillTimer(hWnd, 1);
				 delete[] str;
				 DeleteObject(brush);
				 if (bmpDC0)
					 DeleteDC(bmpDC0);
				 if (bmpDC1)
					 DeleteDC(bmpDC1);
				 ReleaseDC(hWnd, dc);
			 }
		}
		return 0;

		case WM_QUIT:
		{
			if (!deleteAll)
			{
				deleteAll = true;
				KillTimer(hWnd, 1);
				delete[] str;
				DeleteObject(brush);
				if (bmpDC0)
					DeleteDC(bmpDC0);
				if (bmpDC1)
					DeleteDC(bmpDC1);
				ReleaseDC(hWnd, dc);
			}
		}
		return 0;

	}
	
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
