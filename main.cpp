/*
 * This class represents a person with name and age attributes.
 * It provides methods to access and modify these attributes.
 */

#include <windows.h>
#include <commdlg.h>  
#include <shobjidl.h>    // IFileDialog
#include <tchar.h>
#include <fstream>        // std::ifstream 用于文件操作
#include <sstream>        // std::stringstream 用于处理文件内容
#include <string>         // std::string 用于字符串处理
#include <thread> 
#include<vector>
#include"tmhead.hpp"
#include"resource.h"

#define GREEN_COLOR RGB(0, 255, 0)
#define YELLOW_COLOR RGB(255, 255, 0)
#define RED_COLOR RGB(255, 0, 0)

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);



void OpenFileDialog(HWND hwnd);
void ReadFileContents(const TCHAR* filePath, HWND hwnd);

bool isreaded = false;
bool ischanged = false;
bool isrunning = false;
// 定义按钮的ID
#define ID_BUTTON_IMPORT 1
#define ID_BUTTON_RUN_ONCE 2
#define ID_BUTTON_RUN 3
#define ID_BUTTON_RESET 4
#define ID_BUTTON_STOP 5

TM tm;//声明图灵机
std::wstring input;
int wait_time = 500;
//字符串格式转换
std::string wstring_to_string(const std::wstring& wstr)
{
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	std::string str(bufferSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], bufferSize, NULL, NULL);
	return str;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{

	static TCHAR szAppName[] = TEXT("MyWindows");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_TM));
	if (!hMenu) {
		MessageBox(NULL, TEXT("加载菜单失败"), szAppName, MB_ICONERROR);
	}
	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("ERROR"), szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(
		szAppName,
		TEXT("图灵机模拟"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1200,
		600,
		NULL,
		hMenu,
		hInstance,
		NULL);
	SetClassLongPtr(hwnd, GCLP_HICON, (LONG)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)));
	// 创建"导入文件"按钮
	CreateWindow(TEXT("button"), TEXT("导入文件"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, 50, 120, 30,
		hwnd, (HMENU)ID_BUTTON_IMPORT, hInstance, NULL);

	CreateWindow(TEXT("button"),
		TEXT("运行一步"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, 400, 120, 30,
		hwnd, (HMENU)ID_BUTTON_RUN_ONCE, hInstance, NULL);

	CreateWindow(TEXT("button"),
		TEXT("运行"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		160, 400, 120, 30,
		hwnd, (HMENU)ID_BUTTON_RUN, hInstance, NULL);
	CreateWindow(TEXT("button"),
		TEXT("重置"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		440, 400, 120, 30,
		hwnd, (HMENU)ID_BUTTON_RESET, hInstance, NULL);
	CreateWindow(TEXT("button"),
		TEXT("停止"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		300, 400, 120, 30,
		hwnd, (HMENU)ID_BUTTON_STOP, hInstance, NULL);



	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		UpdateWindow(hwnd);
	}
	return msg.wParam;
}

int runtm_once(HWND hwnd)
{
	int state = tm.run();
	switch (state)
	{
	case OVER:
		MessageBox(NULL, TEXT("字符串读取结束"), TEXT("完成"), MB_OK | MB_ICONINFORMATION);
		break;
	case FAULT:
		MessageBox(NULL, TEXT("字符串读取失败"), TEXT("失败"), MB_OK | MB_ICONERROR);
		break;
	}
	InvalidateRect(hwnd, NULL, TRUE);
	return state;
}
void runtm(HWND hwnd)
{
	int state = NOTOVER;
	while (state == NOTOVER)
	{
		state = runtm_once(hwnd);
		Sleep(wait_time);
		if (isrunning == false)
			break;
	}
	isrunning = false;
}
std::thread tm_thread;
void paint_function(HDC hdc)//绘制状态专业函数
{
	int QWidth = 120;
	int QHeight = 30;
	int Qx = 750;
	int Qy = 50;
	int i = 0;
	HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 0));
	for (function f : tm.functions)
	{
		RECT rect = { Qx,Qy + i * QHeight,Qx + QWidth,Qy + (i + 1) * QHeight };
		if ((i * QHeight) > 420)
			rect = { Qx + 120,Qy + (i - 15) * QHeight,Qx + 120 + QWidth,Qy + (i + 1 - 15) * QHeight };
		if ((i * QHeight) > 840)
			rect = { Qx + 240,Qy + (i - 30) * QHeight,Qx + 240 + QWidth,Qy + (i + 1 - 30) * QHeight };
		Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
		if (tm.currentfunction == i)
		{
			switch (tm.StateNum())
			{
			case OVER:
				hBrush = CreateSolidBrush(GREEN_COLOR);
				break;
			case FAULT:
				hBrush = CreateSolidBrush(RED_COLOR);
				break;
			default:
				hBrush = CreateSolidBrush(YELLOW_COLOR);
			}
			FillRect(hdc, &rect, hBrush);
		}
		std::wstring str = L"Q(,)=(,,)";
		str.insert(2, std::to_wstring(f.fstate));
		str.insert(2, L"q");
		wchar_t c = static_cast<wchar_t>(f.fc);
		std::wstring temp;
		temp.push_back(c);
		str.insert(5, temp);
		str.insert(9, std::to_wstring(f.astate));
		str.insert(9, L"q");
		c = static_cast<wchar_t>(f.ac);
		temp.clear();
		temp.push_back(c);
		str.insert(12, temp);
		c = static_cast<wchar_t>(f.vec);
		temp.clear();
		temp.push_back(c);
		str.insert(14, temp);
		DrawText(hdc, str.c_str(), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		i++;//迭代器
	}
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit;
	int charWidth = 30;
	int charHeight = 30;
	int x = 10; // 起始X坐标
	int y = 10; // 起始Y坐标
	switch (message)
	{
	case WM_CREATE:
		// 创建文本框
		hEdit = CreateWindow(TEXT("EDIT"), TEXT("输入串"),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE,
			280, 350, 140, 30,
			hwnd, (HMENU)0, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		// 创建虚线画笔
		HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
		// 绘制矩形
		Rectangle(hdc, 10, 30, 720, 340);
		Rectangle(hdc, 10, 340, 720, 520);
		Rectangle(hdc, 720, 30, 1150, 520);
		// 绘制状态栏
		Rectangle(hdc, 140, 100, 700, 130);
		RECT r = { 20,100,140,130 };
		DrawText(hdc, TEXT("状态："), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		r = { 140, 100, 700, 130 };
		DrawText(hdc, tm.StateSTR.c_str(), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		Rectangle(hdc, 140, 150, 700, 180);
		r = { 20,150,140,180 };
		DrawText(hdc, TEXT("字母表："), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		r = { 140, 150, 700, 180 };
		DrawText(hdc, tm.AlphaSTR.c_str(), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		Rectangle(hdc, 140, 200, 700, 230);
		r = { 20,200,140,230 };
		DrawText(hdc, TEXT("初始状态："), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		r = { 140, 200, 700, 230 };
		DrawText(hdc, tm.StartSTR.c_str(), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		Rectangle(hdc, 140, 250, 700, 280);
		r = { 20,250,140,280 };
		DrawText(hdc, TEXT("最终状态："), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);

		r = { 140, 250, 700, 280 };
		DrawText(hdc, tm.FinalSTR.c_str(), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		Rectangle(hdc, 140, 300, 700, 330);
		r = { 20,300,140,330 };
		DrawText(hdc, TEXT("占位符号："), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);

		r = { 140, 300, 700, 330 };
		DrawText(hdc, tm.BlankSTR.c_str(), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		r = { 20,350,280,380 };
		DrawText(hdc, TEXT("输入符号：（可编辑）"), -1, &r,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		//绘制状态转移函数
		paint_function(hdc);

		//绘制图灵机


		// 设置矩形的宽度和高度
		charWidth = 30;
		charHeight = 30;
		x = 20; // 起始X坐标
		y = 450; // 起始Y坐标

		// 遍历每个字符，并绘制矩形框及字符
		std::wstring temp(tm.belt.begin(), tm.belt.end());
		TCHAR* Ttemp = const_cast<TCHAR*>(temp.c_str());
		for (int i = 0; Ttemp[i] != '\0'; ++i)
		{
			// 计算字符位置
			RECT charRect = { x, y, x + charWidth, y + charHeight };

			// 绘制矩形框
			Rectangle(hdc, charRect.left, charRect.top, charRect.right, charRect.bottom);

			// 绘制字符
			DrawText(hdc, &Ttemp[i], 1, &charRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			// 更新下一个字符的X坐标
			x += charWidth + 5;
		}
		x = 20; // 起始X坐标
		y = 480; // 起始Y坐标
		RECT charRect = { x + (5 + charWidth) * tm.position, y, x + charWidth + (5 + charWidth) * tm.position, y + charHeight };
		Rectangle(hdc, charRect.left, charRect.top, charRect.right, charRect.bottom);
		std::wstring state;
		state += std::to_wstring(tm.getcurrentState());
		TCHAR* ttemp = const_cast<TCHAR*>(state.c_str());
		charRect = { x + (5 + charWidth) * tm.position, y, x + charWidth + (5 + charWidth) * tm.position + 10, y + charHeight };
		DrawText(hdc, ttemp, 1, &charRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		charRect = { x + (5 + charWidth) * tm.position - 10, y, x + charWidth + (5 + charWidth) * tm.position, y + charHeight };
		DrawText(hdc, TEXT("q"), 1, &charRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		// 恢复原来的画笔
		SelectObject(hdc, hOldPen);
		DeleteObject(hPen);
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_KEYDOWN:
		if (wParam == VK_RETURN)
		{
			runtm_once(hwnd);
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDM_EXIT)
		{
			DestroyWindow(hwnd);
		}
		if (LOWORD(wParam) == IDM_ABOUT)
		{
			MessageBox(hwnd, TEXT("图灵机模拟器 V1.0.1\nCopyright (c) 2024 PopulusYang"), TEXT("关于"), MB_OK | MB_ICONINFORMATION);
		}
		if (LOWORD(wParam) == IDM_IMPORT)
		{
			OpenFileDialog(hwnd);
		}
		if (LOWORD(wParam) == IDM_HELP)
		{
			MessageBox(hwnd,
				TEXT("第一步：准备文件\n\t文件格式为：\n\t\t状态数\n\t\t字母表\n\t\t初始状态\n\t\t最终状态\n\t\t占位符号\n\t\t状态转移函数\n\t\t输入串(开头加上“/”)\n\n第二步：点击“导入文件”按钮，选择文件\n\n第三步：点击“运行”按钮，模拟图灵机运行（再次点击“运行”按钮，加速模拟）\n\n其他：\n\t点击“停止”按钮，停止模拟\n\t点击“重置”按钮，重置图灵机\n\t点击“运行”按钮，模拟图灵机运行\n\t点击“停止”按钮，中断模拟\n\t点击“重置”按钮，重置图灵机"),
				TEXT("帮助"), MB_OK | MB_ICONINFORMATION);
		}
		if (LOWORD(wParam) == ID_BUTTON_IMPORT)  // 判断是否点击了“导入文件”按钮
		{
			OpenFileDialog(hwnd);  // 弹出文件对话框
		}
		if (LOWORD(wParam) == ID_BUTTON_RUN_ONCE)//运行图灵机
			{
			if (isrunning) {
				MessageBox(hwnd, TEXT("正在运行"), TEXT("提示"), MB_OK | MB_ICONINFORMATION);
				break;
			}
			runtm_once(hwnd);
		}
		if (LOWORD(wParam) == ID_BUTTON_RUN)
		{
			if (isrunning) {
				wait_time = 10;
				break;
			}
			if (tm_thread.joinable())
				tm_thread.join();
			wait_time = 500;
			isrunning = true;
			tm_thread = std::thread(runtm, hwnd);
		}
		if (LOWORD(wParam) == ID_BUTTON_STOP)
		{
			isrunning = false;
			InvalidateRect(hwnd, NULL, TRUE);
			MessageBox(hwnd, TEXT("已停止"), TEXT("提示"), MB_OK | MB_ICONINFORMATION);
		}
		if (LOWORD(wParam) == ID_BUTTON_RESET)
		{
			if (isrunning) {
				isrunning = false;
			}
			if (tm_thread.joinable())
				tm_thread.join();
			tm.reset();
			InvalidateRect(hwnd, NULL, TRUE);
			MessageBox(hwnd, TEXT("已重置"), TEXT("提示"), MB_OK | MB_ICONINFORMATION);
			ischanged = true;
		}
		if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == 0)
		{
			// 当文本框内容改变时
			if (isrunning) {
				isrunning = false;
			}
			if (tm_thread.joinable())
				tm_thread.join();
			tm.reset();
			TCHAR buffer[1024];
			GetWindowText(hEdit, buffer, 1024);
			std::wstring temp(buffer);
			tm.belt.clear();
			tm.belt = wstring_to_string(temp);
			InvalidateRect(hwnd, NULL, TRUE);
		}
		if ((LOWORD(wParam) == 1 || LOWORD(wParam) == 102 || LOWORD(wParam) == 4) && ischanged)
		{
			std::wstring newText = input;
			SetWindowText(hEdit, newText.c_str());

			ischanged = false;
		}
		return 0;
	case WM_DESTROY:

		if (tm_thread.joinable()) {
			tm_thread.join(); // 等待线程结束
		}
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


// 使用 IFileDialog 打开文件对话框
void OpenFileDialog(HWND hwnd)
{
	// 初始化 COM 库
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		MessageBox(hwnd, TEXT("无法初始化 COM 库"), TEXT("错误"), MB_ICONERROR);
		return;
	}

	// 创建 IFileDialog 对象
	IFileDialog* pFileDialog = NULL;
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pFileDialog));
	if (FAILED(hr)) {
		MessageBox(hwnd, TEXT("无法创建文件对话框"), TEXT("错误"), MB_ICONERROR);
		CoUninitialize();
		return;
	}

	// 设置对话框的文件类型过滤器
	COMDLG_FILTERSPEC rgSpec[] = {
		{L"所有文件", L"*.*"},
		{L"文本文件", L"*.txt"},
	};
	pFileDialog->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

	// 显示对话框
	hr = pFileDialog->Show(hwnd);
	if (FAILED(hr)) {
		pFileDialog->Release();
		CoUninitialize();
		return;
	}

	// 获取用户选择的文件
	IShellItem* pItem;
	hr = pFileDialog->GetResult(&pItem);
	if (FAILED(hr)) {
		pFileDialog->Release();
		CoUninitialize();
		return;
	}

	// 获取文件路径
	LPWSTR pszFilePath = NULL;
	hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
	if (SUCCEEDED(hr)) {
		// 读取文件内容
		ReadFileContents(pszFilePath, hwnd);
		CoTaskMemFree(pszFilePath);
	}
	pItem->Release();
	pFileDialog->Release();
	CoUninitialize();
}
void ReadFileContents(const TCHAR* filePath, HWND hwnd)
{
	// 打开文件
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		MessageBox(hwnd, TEXT("无法打开文件"), TEXT("错误"), MB_ICONERROR);
		return;
	}

	// 用于存储按行读取的文件内容
	std::vector<std::wstring> lines;

	// 按行读取文件
	std::string line;
	while (std::getline(file, line))
	{
		// 将每行的内容从 std::string 转换为 std::wstring
		std::wstring wline(line.begin(), line.end());
		lines.push_back(wline);
	}

	// 关闭文件
	file.close();

	// 如果文件内容为空，显示提示
	if (lines.empty())
	{
		MessageBox(hwnd, TEXT("文件为空或无法读取"), TEXT("错误"), MB_ICONERROR);
		return;
	}
	int stateNum =0;
	int startNum =0;
	std::vector<int> finalNum;
	char blank = 'B';
	std::vector<function> functons;
	// 将每一行内容显示在一个 MessageBox 中
	for (size_t i = 0; i < lines.size(); ++i)
	{
		if (i == 0)
		{
			stateNum = (int)(lines[i][0] - '0');
		}
		if (i == 2)
		{
			startNum = (int)(lines[i][0] - '0');
		}
		if (i == 3)
		{
			for (WCHAR a : lines[i])
			{
				if (a > '0' && a < '9')
					finalNum.push_back((int)(lines[i][0] - '0'));
			}
		}
		if (i == 4)
		{
			blank = lines[i][0];
		}

		if ((int)(lines[i][0] - '0') == -1 && i > 5)
		{
			lines[i].erase(lines[i].begin());
			input = lines[i];
			ischanged = true;
			continue;
		}
		if (i >= 5)
		{
			std::string temp = wstring_to_string(lines[i]);
			static function tempf;
			tempf.fstate = (int)(temp[0] - '0');
			tempf.fc = temp[2];
			tempf.astate = (int)(temp[4] - '0');
			tempf.ac = temp[6];
			tempf.vec = temp[8];
			functons.push_back(tempf);
		}
	}
	tm = TM(stateNum, wstring_to_string(input), functons, finalNum, startNum, blank);
	tm.AlphaSTR = lines[1];
	tm.BlankSTR = lines[4];
	tm.FinalSTR = lines[3];
	tm.StartSTR = lines[2];
	std::wstring statestr;
	for (int i = 0; i <= stateNum; i++)
	{
		statestr += std::to_wstring(i);
		statestr += ' ';
	}
	tm.StateSTR = statestr;
	InvalidateRect(hwnd, NULL, TRUE);
}
