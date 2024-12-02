#include <windows.h>
#include <commdlg.h>  
#include <shobjidl.h>    // IFileDialog
#include <tchar.h>
#include <fstream>        // std::ifstream 用于文件操作
#include <sstream>        // std::stringstream 用于处理文件内容
#include <string>         // std::string 用于字符串处理
#include<vector>
#include"tmhead.hpp"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void OpenFileDialog(HWND hwnd);
void ReadFileContents(const TCHAR* filePath, HWND hwnd);

bool isreaded = false;
bool ischanged = false;
// 定义按钮的ID
#define ID_BUTTON_IMPORT 1
#define ID_BUTTON_RUN 2

TM tm;//声明图灵机
std::wstring input;
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
		800,
		600,
		NULL,
		NULL,
		hInstance,
		NULL);

	// 创建"导入文件"按钮
	CreateWindow(TEXT("button"), TEXT("导入文件"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, 50, 120, 30,
		hwnd, (HMENU)ID_BUTTON_IMPORT, hInstance, NULL);

	CreateWindow(TEXT("button"),
		TEXT("运行一步"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, 400, 120, 30,
		hwnd, (HMENU)ID_BUTTON_RUN, hInstance, NULL);



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
		hEdit = CreateWindow(TEXT("EDIT"), TEXT("123"),
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

		//绘制图灵机


		// 设置矩形的宽度和高度
		charWidth = 30;
		charHeight = 30;
		x = 20; // 起始X坐标
		y = 450; // 起始Y坐标

		// 遍历每个字符，并绘制矩形框及字符
		std::wstring temp(tm.belt.begin(),tm.belt.end());
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
			x += charWidth + 5;  // 字符间隔
		}
		x = 20; // 起始X坐标
		y = 480; // 起始Y坐标
		RECT charRect = { x+(5+charWidth)*tm.position, y, x + charWidth+(5 + charWidth) * tm.position, y + charHeight};
		Rectangle(hdc, charRect.left, charRect.top, charRect.right, charRect.bottom);
		std::wstring state;
		state += std::to_wstring(tm.getcurrentState());
		TCHAR* ttemp = const_cast<TCHAR*>(state.c_str());
		charRect = { x + (5 + charWidth) * tm.position, y, x + charWidth + (5 + charWidth) * tm.position+10, y + charHeight };
		DrawText(hdc, ttemp, 1, &charRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		charRect = { x + (5 + charWidth) * tm.position - 10, y, x + charWidth + (5 + charWidth) * tm.position, y + charHeight };
		DrawText(hdc, TEXT("q"), 1, &charRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		// 恢复原来的画笔
		SelectObject(hdc, hOldPen);
		DeleteObject(hPen);
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == ID_BUTTON_IMPORT)  // 判断是否点击了“导入文件”按钮
		{
			OpenFileDialog(hwnd);  // 弹出文件对话框
		}
		if (LOWORD(wParam) == ID_BUTTON_RUN)//运行图灵机
		{
			switch (tm.run())
			{
			case OVER:
				MessageBox(NULL, TEXT("字符串读取完成"), TEXT("完成"), MB_OK | MB_ICONINFORMATION);
				break;
			case FAULT:
				MessageBox(NULL, TEXT("字符串读取失败"), TEXT("是吧"), MB_OK | MB_ICONERROR);
				break;
			}
			InvalidateRect(hwnd, NULL, TRUE);
		}
		if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == 0)
		{
			// 当文本框内容改变时
			TCHAR buffer[1024];
			GetWindowText(hEdit, buffer, 1024);
			std::wstring temp(buffer);
			tm.belt.clear();
			tm.belt = wstring_to_string(temp);
			InvalidateRect(hwnd, NULL, TRUE);
		}
		if (LOWORD(wParam) == 1 && ischanged)
		{
			std::wstring newText = input;
			SetWindowText(hEdit, newText.c_str());

			ischanged = false;
		}
		return 0;

	case WM_DESTROY:
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
	int stateNum;
	int startNum;
	std::vector<int> finalNum;
	char blank;
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
