/*
 * This class represents a person with name and age attributes.
 * It provides methods to access and modify these attributes.
 */

#pragma once
#include <vector>
#include<string>

#define FAULT -1
#define OVER 0
#define NOTOVER 1
class arror // 读头类
{
	

public:
	int position; // 读头的位置
	arror() : position(0) {}  // 初始化位置为0
	virtual void left() {} // 读头左移
	virtual void right() {} // 读头右移
	int getPosition() const { return position; } // 获取当前读头位置
	void setPosition(int newPos) { position = newPos; } // 设置读头位置
};

typedef struct
{
	int fstate; // 当前状态
	char fc; // 当前符号
	int astate; // 下一状态
	char ac; // 写入符号
	char vec; // 移动方向：左或右
} function;

class TM : public arror // 图灵机类
{
private:
	int stateNum;  // 状态数
	std::vector<function> functions;  // 状态转换规则
	std::vector<int> overstate;
	int start;  // 初始状态
	int currentState; // 当前状态
	int currentfunction; // 当前转换规则
	char blank = 'B';
	bool isover = false;
	bool isfault = false;
public:
	std::string belt; // 图带
	std::string originalBelt = belt; // 原始图带
	std::wstring StateSTR = L"未导入";
	std::wstring AlphaSTR = L"未导入";
	std::wstring StartSTR = L"未导入";
	std::wstring FinalSTR = L"未导入";
	std::wstring BlankSTR = L"未导入";
public:
	TM(int stateNum, std::string belt, std::vector<function> functions, std::vector<int> overstate, int start = 0,char blank = 'B')
		: stateNum(stateNum), belt(belt), functions(functions), start(start), currentState(start), overstate(overstate),blank(blank) {}
	TM():stateNum(0), belt(" "), functions(NULL), start(0), currentState(0), overstate(NULL) {}
	int getcurrentState()
	{
		return currentState;
	}
	friend void paint_function(HDC hdc);
	void right()override
	{
		if (belt.size() == position + 2)
			belt.insert(belt.end()-1, blank);
		position++;
	}
	void left()override
	{
		if (position == 0)
		{
			belt.insert(belt.begin(), blank);
			position++;
		}
		position--;
	}
	void reset()
	{
		position = 0;
		currentState = start;
		isover = false;
		isfault = false;
		belt.clear();
		belt = originalBelt;
	}
	int StateNum() const 
	{ 
		if(isover)
			return OVER;
		if (isfault)
			return FAULT;
		return NOTOVER;
	}
	int run()
	{
		if(isover)
			return OVER;
		if (isfault)
			return FAULT;
		char currentChar = belt[getPosition()];  // 获取当前图带字符
		function* transition = nullptr;
		int i = 0;
		// 查找当前状态和图带字符对应的转换规则
		for (auto& f : functions)
		{
			if (f.fstate == currentState && f.fc == currentChar)
			{
				transition = &f;
				currentfunction = i;
				break;
			}
			i++;
		}

		// 如果没有找到对应的转换规则，停止运行
		if (!transition)
		{
			isfault = true;
			return FAULT;
		}

		// 执行转换规则
		belt[getPosition()] = transition->ac;  // 写入新符号
		currentState = transition->astate;    // 更新状态

		// 根据方向移动读头
		if (transition->vec == 'L')
			left();
		else if (transition->vec == 'R')
			right();
		for (int i : overstate)
		{
			if (currentState == i)
			{
				isover = true;
				return OVER;
		}
				
		}
		return NOTOVER;
	}
};


