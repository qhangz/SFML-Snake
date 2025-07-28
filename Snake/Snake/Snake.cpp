/*
	舞台的坐标范围[0][width-1]、[0][height-1]
	gameMode通过“空格键”控制游戏模式，连续和步进模式切换
	x键退出游戏
	+-键加减音乐音量，enter键控制音乐开关，空格键切换模式
	穿墙一闪而过的问题在DrawStep函数中完成，当前后蛇身节点不相邻时不绘制蛇身
	蛇头180度旋转的解决方法：当蛇头步进一个完整的放个之后再对下一次的方向变换做响应
*/

#include <SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include<iostream>
#include<Windows.h>

#define WINDOW_WIDTH 80     //窗口宽度
#define WINDOW_HEIGHT 25    //窗口高度
#define STAGE_WIDTH 20      //舞台宽度
#define STAGE_HEIGHT 20     //舞台高度
#define GRIDSIZE 25     //纹理尺寸
#define SCALE 0.5
#define MAXLENGTH 100   //蛇身最大长度
#define INFO_WIDTH 400
#define STEP 0.1

using namespace sf;

bool gameOver;
bool gameQuit;
bool gamePause;
bool MutexOnceInput;//确保步进一个方格后才能做下一次的方向变化

const int width = STAGE_WIDTH;
const int height = STAGE_HEIGHT;
int headX, headY, fruitX, fruitY, score;
int tailX[MAXLENGTH], tailY[MAXLENGTH];
int ntail;
int delay;
int GameMode;	//控制游戏动画显示模式
float stepX, stepY;

enum eDirection { STOP = 0,LEFT,RIGHT,UP,DOWN };
eDirection dir , dir_ing;

sf::RenderWindow window(sf::VideoMode(width*GRIDSIZE + GRIDSIZE + INFO_WIDTH, height*GRIDSIZE +GRIDSIZE), L"SFML Snake!");
Texture tBackground, tSnakeHead, tSnakeBody, tFruit;	//创建纹理对象
Sprite spBackground, spSnakeHead, spSnakeBody, spFruit;	//创建精灵对象
int headRotation;
Font font;
Text text;
SoundBuffer sbEat, sbDie;
Sound soundEat, soundDie;
Music bkMusic;
int soundVolume;	//背景音量
bool MusicOn;		//背景音乐开关
sf::Clock KeyClockTimer;	//避免在输入模块设计时，如果同时按多个键，存在蛇调头导致的可能，导致游戏立即结束。
int KeyClockDelay = 50;	//去抖动时间阈值，单位毫秒
bool dirChange = false;

//提示信息，函数参数接收文字板块在舞台中的原点坐标，通过initialX控制各行文字相对于原点x坐标的缩进
void Prompt_info(int x, int y)
{
	int initialX = STAGE_WIDTH, initialY = 0;
	int CharacterSize = 24;
	text.setCharacterSize(CharacterSize);
	text.setFillColor(Color(255, 255, 255, 255));
	text.setStyle(Text::Bold);

	/*SetConsoleTextAttribute(h, COLOR_TEXT);*/
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"■游戏说明：");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □蛇身自撞，游戏结束");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □蛇可穿墙");
	window.draw(text);
	initialY += CharacterSize * 2;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"■操作说明：");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □向上移动：↑W");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □向下移动：↓S");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □向左移动：←A");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □向右移动：→D");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □开始游戏：任意操作键");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □背景音乐开关：回车键");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □背景音量调节：+/-键");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □动画模式切换：空格键");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	if (GameMode == 1)
	{
		text.setFillColor(Color(60, 110, 255, 255));//blue
		text.setString(L"         步进移动");
	}
	else
	{
		text.setFillColor(Color(205, 160, 0, 255));//
		text.setString(L"         连续移动");
	}
	window.draw(text);
	text.setFillColor(Color(255, 255, 255, 255));	//White text
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    □退出游戏：x键退出");
	window.draw(text);
	initialY += CharacterSize * 3;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"☆ 作者：杭电数媒 ZQH");
	window.draw(text);
}

//游戏结束信息
void gameOver_info(int x,int y)
{
	int initialX = 0, initialY = 20;
	int CharacterSize = 48;
	text.setCharacterSize(CharacterSize);
	text.setFillColor(Color(255, 0, 0, 255));
	text.setStyle(Text::Bold);

	text.setPosition(x + initialX, y + initialY);
	text.setString(L"        游戏结束!!");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"   Y重新开始/n退出");
	window.draw(text);
}

void Initial()
{
	window.setFramerateLimit(60);	//每秒设置目标帧率
	
	//加载字体
	if (!font.loadFromFile("../data/fonts/ZCOOLKuaiLe-Regular.ttf"))
	{
		std::cout << "Font not find." << std::endl;
	}
	text.setFont(font);

	//加载纹理图片
	if (!tBackground.loadFromFile("../data/images/BK.png"))
	{
		std::cout << "No texture find for Background." << std::endl;
	}
	if (!tSnakeHead.loadFromFile("../data/images/sh01.png"))
	{
		std::cout << "No texture fond for SnakeHead." << std::endl;
	}
	if (!tSnakeBody.loadFromFile("../data/images/sb0102.png"))
	{
		std::cout << "No texture fond for SnakeBody." << std::endl;
	}
	if (!tFruit.loadFromFile("../data/images/sb0202.png"))
	{
		std::cout << "No texture fond for Fruit." << std::endl;
	}

	//加载音频
	if (!sbEat.loadFromFile("../data/Audios/Eat01.ogg"))
	{
		std::cout << "Eat01 not find." << std::endl;
	}
	if (!sbDie.loadFromFile("../data/Audios/Die01.ogg"))
	{
		std::cout << "Die01 not find." << std::endl;
	}
	if (!bkMusic.openFromFile("../data/Audios/BGM01.ogg"))
	{
		std::cout << "BGM01 not find." << std::endl;
	}

	//加载精灵对象的纹理
	spBackground.setTexture(tBackground);
	spSnakeHead.setTexture(tSnakeHead);
	spSnakeBody.setTexture(tSnakeBody);
	spFruit.setTexture(tFruit);

	//用setOrigin函数设置sprite对象的坐标原点
	spBackground.setOrigin(GRIDSIZE / SCALE / 2, GRIDSIZE / SCALE / 2);
	spSnakeHead.setOrigin(GRIDSIZE / SCALE / 2, GRIDSIZE / SCALE / 2);
	spSnakeBody.setOrigin(GRIDSIZE / SCALE / 2, GRIDSIZE / SCALE / 2);
	spFruit.setOrigin(GRIDSIZE / SCALE / 2, GRIDSIZE / SCALE / 2);

	//对精灵对象进行缩放操作
	spBackground.setScale(SCALE, SCALE);
	spSnakeHead.setScale(SCALE, SCALE);
	spSnakeBody.setScale(SCALE, SCALE);
	spFruit.setScale(SCALE, SCALE);

	//设置音频对应缓存
	soundEat.setBuffer(sbEat);	//音效读入缓冲
	soundDie.setBuffer(sbDie);	//音效读入缓冲
	bkMusic.play();				//背景音乐播放
	bkMusic.setLoop(true);		//背景音乐循环
	soundVolume = 50;
	MusicOn = true;
	//std::cout << "soundVolume:" << soundVolume << std::endl;

    //fruitFlash = false;
    gameOver = false;
	gameQuit = false;
	gamePause = false;
	MutexOnceInput = true;
    //isFullWidth = true;
    //isPause = false;

	GameMode = 2;
	stepX = 0.0;
	stepY = 0.0;

    //初始化
    srand(time(NULL));
    dir = STOP;
	dir_ing = STOP;
    headX = width / 2;
    headY = height / 2;
    fruitX = rand() % width;
    fruitY = rand() % height;
    score = 0;
	delay = 0;

    //蛇身初始化
    ntail = 1;
    for (int i = 0; i < MAXLENGTH; i++)
    {
        tailX[i] = 0;
        tailY[i] = 0;
    }
}

void Input()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if ((event.type == sf::Event::Closed))
		{
			window.close();
			gameOver = true;
			gameQuit = true;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::Escape)
		{
			window.close();	
			gameOver = true;
			gameQuit = true;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::X)
		{
			window.close();
			gameOver = true;
			gameQuit = true;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::Space)
		{
			if (GameMode == 1)
			{
				GameMode = 2;
			}
			else
			{
				GameMode = 1;
			}
			std::cout << "GameMode:" << GameMode << std::endl;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::Add)
		{
			soundVolume += 5;
			bkMusic.setVolume(soundVolume);
			//std::cout << "soundVolume:" << soundVolume << std::endl;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::Subtract)
		{
			soundVolume -= 5;
			bkMusic.setVolume(soundVolume);
			//std::cout << "soundVolume:" << soundVolume << std::endl;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::Enter)
		{
			if (MusicOn == true)
			{
				bkMusic.stop();
				MusicOn = false;
			}
			else
			{
				bkMusic.play();
				MusicOn = true;
			}
		}
    }

		
	if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A))
	{
		if (dir != RIGHT)
		{
			dir = LEFT;
		}
	}
	else if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D))
	{
		if (dir != LEFT)
		{
			dir = RIGHT;
		}
	}
	else if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
	{
		if (dir != DOWN && dirChange == false)
		{
			dir = UP;
		}
	}
	else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
	{
		if (dir != UP && dirChange == false)
		{
			dir = DOWN;
		}
	}

}

void Logic()
{
	tailX[0] = headX;
	tailY[0] = headY;

	//update snack head
	switch (dir)
	{
	case UP:
		headY--;
		headRotation = 0;
		break;
	case DOWN:
		headY++;
		headRotation = 180;
		break;
	case LEFT:
		headX--;
		headRotation = -90;
		break;
	case RIGHT:
		headX++;
		headRotation = 90;
		break;
	default:
		break;
	}

	//穿墙
	if (headX >= width)headX = 0;
	else if (headX < 0)headX = STAGE_WIDTH - 1;
	if (headY >= height)headY = 0;
	else if (headY < 0)headY = STAGE_HEIGHT - 1;

	//撞墙则gameover
	/*if (headX > STAGE_WIDTH - 1 || headX < 0 || headY > STAGE_HEIGHT - 1 || headY < 0)
	{
		gameOver = true;
	}*/

	//蛇头与蛇身相撞则gameover
	for (int i = 1; i < ntail; i++)
	{
		if (tailX[i] == headX && tailY[i] == headY)
		{
			soundDie.play();
			gameOver = true;
		}
	}

	//get fruit
	if (headX == fruitX && headY == fruitY)
	{
		soundEat.play();	//播放eat音效
		score += 10;
		/*fruitX = rand() % STAGE_WIDTH;
		fruitY = rand() % STAGE_HEIGHT;*/
		int originX = fruitX;
		int originY = fruitY;
		bool flag = false;
		while (!flag)
		{
			fruitX = rand() % STAGE_WIDTH;
			fruitY = rand() % STAGE_HEIGHT;
			flag = true;
			for (int i = 1; i < ntail; i++)
			{
				if ((fruitX == tailX[i] && fruitY == tailY[i]) || (fruitX == originX && fruitY == originY))	//新生成的fruit在蛇身上
				{
					flag = false;
					break;
				}
			}
		}
		ntail++;
	}


	//update snack body
	int prevX = tailX[0];
	int prevY = tailY[0];
	int prev2X, prev2Y;

	//将蛇身位置挨个往后传（去除掉了最后一个位置的蛇尾
	for (int i = 1; i < ntail; i++)
	{
		prev2X = tailX[i];
		prev2Y = tailY[i];
		tailX[i] = prevX;
		tailY[i] = prevY;
		prevX = prev2X;
		prevY = prev2Y;
	}


}

void Draw()
{
    window.clear(Color::Color(130,130,130,255));
	Prompt_info(width * GRIDSIZE + GRIDSIZE, GRIDSIZE);

	int detaX = GRIDSIZE / SCALE / 2;
	int detaY = GRIDSIZE / SCALE / 2;
	

	//绘制背景
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			spBackground.setPosition(i * GRIDSIZE + detaX, j * GRIDSIZE + detaY);	//指定纹理的位置
			window.draw(spBackground);								//将纹理绘制到缓冲区
		}
	}

	//绘制蛇
	spSnakeHead.setPosition(headX * GRIDSIZE + detaX - 2, headY * GRIDSIZE + detaY - 1);
	spSnakeHead.setRotation(headRotation);
	window.draw(spSnakeHead);
	for (int i = 1; i < ntail; i++)
	{
		spSnakeBody.setPosition(tailX[i] * GRIDSIZE + detaX - 2, tailY[i] * GRIDSIZE + detaY - 1);
		window.draw(spSnakeBody);
	}

	//绘制fruit
	spFruit.setPosition(fruitX * GRIDSIZE + detaX - 2, fruitY * GRIDSIZE + detaY - 1);
	window.draw(spFruit);

	if (gameOver) {
		gameOver_info(width / 8 * GRIDSIZE, height / 4 * GRIDSIZE);
	}

    window.display();	//把显示缓冲区的内容，显示到屏幕上，sfml采用的是双缓冲机制
}

void InputStep()
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		if ((event.type == sf::Event::Closed))
		{
			window.close();
			gameOver = true;
			gameQuit = true;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::Escape)
		{
			window.close();
			gameOver = true;
			gameQuit = true;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::X)
		{
			window.close();
			gameOver = true;
			gameQuit = true;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::Space)
		{
			if (GameMode == 1)
			{
				GameMode = 2;
			}
			else
			{
				GameMode = 1;
			}
			std::cout << "GameMode:" << GameMode << std::endl;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::Add)
		{
			soundVolume += 5;
			bkMusic.setVolume(soundVolume);
			//std::cout << "soundVolume:" << soundVolume << std::endl;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::Subtract)
		{
			soundVolume -= 5;
			bkMusic.setVolume(soundVolume);
			//std::cout << "soundVolume:" << soundVolume << std::endl;
		}
		if (event.type == sf::Event::EventType::KeyReleased && event.key.code == sf::Keyboard::Enter)
		{
			if (MusicOn == true)
			{
				bkMusic.stop();
				MusicOn = false;
			}
			else
			{
				bkMusic.play();
				MusicOn = true;
			}
		}
	}

	//方向按键事件（将时间抖动设置在dirChange上，实现keyClockDelay时间后才能进行下一次方向更新）
	//这个时间还是不好控制啊，随着游戏帧率的提高，两次响应的时间间隔的设置还是容易出现问题
	//所以将游戏逻辑设置为，当蛇头步进一个各自之后再响应下一个方向变换
	/*if (KeyClockTimer.getElapsedTime().asMilliseconds() > KeyClockDelay)
	{
		dirChange = false;
	}
	if(event.type == sf::Event::KeyPressed)
	{
		if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A))
			if (dir != RIGHT && dirChange == false)
			{
				dir = LEFT;
				dirChange = true;
				KeyClockTimer.restart();
			}
		if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D))
			if (dir != LEFT && dirChange == false)
			{
				dir = RIGHT;
				dirChange = true;
				KeyClockTimer.restart();
			}
		if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
			if (dir != DOWN && dirChange == false)
			{
				dir = UP;
				dirChange = true;
				KeyClockTimer.restart();
			}
		if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
			if (dir != UP && dirChange == false)
			{
				dir = DOWN;
				dirChange = true;
				KeyClockTimer.restart();
			}
	}*/

	if (MutexOnceInput)//确保步进一个方格后才能做下一次的方向变化
	{
		if (event.type == sf::Event::KeyPressed)
		{
			if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A))
			{
				if (dir != RIGHT && dirChange == false)
				{
					dir = LEFT;
					MutexOnceInput = false;
					KeyClockTimer.restart();
				}
			}
			else if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D))
			{
				if (dir != LEFT && dirChange == false)
				{
					dir = RIGHT;
					MutexOnceInput = false;
					KeyClockTimer.restart();
				}
			}
			else if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W))
			{
				if (dir != DOWN && dirChange == false)
				{
					dir = UP;
					MutexOnceInput = false;
					KeyClockTimer.restart();
				}
			}
			else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))
			{
				if (dir != UP && dirChange == false)
				{
					dir = DOWN;
					MutexOnceInput = false;
					KeyClockTimer.restart();
				}
			}
		}
	}




}

void LogicStep()
{
	bool updateFlag = false;

	tailX[0] = headX;
	tailY[0] = headY;

	//update snack head
	switch (dir_ing)
	{
	case UP:
		stepY -= STEP;
		if(stepY < -0.9999 || stepY >= 0.9999)
		{
			headY--;
			stepX = 0;
			stepY = 0;
			dir_ing = dir;
			headRotation = 0;
			updateFlag = true;
			MutexOnceInput = true;//确保步进一个方格后才能做下一次的方向变化
		}
		break;
	case DOWN:
		stepY += STEP;
		if (stepY < -0.9999 || stepY >= 0.9999)
		{
			headY++;
			stepX = 0;
			stepY = 0;
			dir_ing = dir;
			headRotation = 180;
			updateFlag = true;
			MutexOnceInput = true;
		}
		break;
	case LEFT:
		stepX -= STEP;
		if (stepX < -0.9999 || stepX >= 0.9999)
		{
			headX--;
			stepX = 0;
			stepY = 0;
			dir_ing = dir;
			headRotation = -90;
			updateFlag = true;
			MutexOnceInput = true;
		}
		break;
	case RIGHT:
		stepX += STEP;
		if (stepX < -0.9999 || stepX >= 0.9999)
		{
			headX++;
			stepX = 0;
			stepY = 0;
			dir_ing = dir;
			headRotation = 90;
			updateFlag = true;
			MutexOnceInput = true;
		}
		break;
	default:
		dir_ing = dir;
		MutexOnceInput = true;
		break;
	}

	
	//穿墙
	if (headX >= width)
	{
		headX = 0;
	}
	else if (headX < 0)
	{
		headX = STAGE_WIDTH - 1;
	}
	if (headY >= height)
	{
		headY = 0;
	}
	else if (headY < 0)
	{
		headY = STAGE_HEIGHT - 1;
	}

	if (updateFlag == true)
	{

		//撞墙则gameover
		/*if (headX > STAGE_WIDTH - 1 || headX < 0 || headY > STAGE_HEIGHT - 1 || headY < 0)
		{
			gameOver = true;
		}*/
		//蛇头与蛇身相撞则gameover
		for (int i = 1; i < ntail; i++)
		{
			if (tailX[i] == headX && tailY[i] == headY)
			{
				soundDie.play();
				gameOver = true;
			}
				
		}

		//get fruit
		if (headX == fruitX && headY == fruitY)
		{
			soundEat.play();	//播放eat音效
			score += 10;
			/*fruitX = rand() % STAGE_WIDTH;
			fruitY = rand() % STAGE_HEIGHT;*/
			int originX = fruitX;
			int originY = fruitY;
			bool flag = false;
			while (!flag)
			{
				fruitX = rand() % STAGE_WIDTH;
				fruitY = rand() % STAGE_HEIGHT;
				flag = true;
				for (int i = 1; i < ntail; i++)
				{
					if ((fruitX == tailX[i] && fruitY == tailY[i]) || (fruitX == originX && fruitY == originY))	//新生成的fruit在蛇身上
					{
						flag = false;
						break;
					}
				}
			}
			ntail++;
		}


		//update snack body
		int prevX = tailX[0];
		int prevY = tailY[0];
		int prev2X, prev2Y;

		//将蛇身位置挨个往后传（去除掉了最后一个位置的蛇尾
		for (int i = 1; i < ntail; i++)
		{
			prev2X = tailX[i];
			prev2Y = tailY[i];
			tailX[i] = prevX;
			tailY[i] = prevY;
			prevX = prev2X;
			prevY = prev2Y;
		}
	}


}

void DrawStep()
{
	window.clear(Color::Color(130, 130, 130, 255));
	Prompt_info(width * GRIDSIZE + GRIDSIZE, GRIDSIZE);

	int detaX = GRIDSIZE / SCALE / 2;
	int detaY = GRIDSIZE / SCALE / 2;


	//绘制背景
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			spBackground.setPosition(i * GRIDSIZE + detaX, j * GRIDSIZE + detaY);	//指定纹理的位置
			window.draw(spBackground);								//将纹理绘制到缓冲区
		}
	}

	//绘制蛇
	float stepLength;
	stepLength = stepX + stepY;
	if (stepLength < 0)
	{
		stepLength = -stepLength;
	}
	spSnakeHead.setPosition((headX + stepX) * GRIDSIZE + detaX - 2, (headY + stepY) * GRIDSIZE + detaY - 1);
	spSnakeHead.setRotation(headRotation);
	window.draw(spSnakeHead);

	if (ntail >= 2)
	{
		if (tailY[1] == headY && tailX[1] != headX)	//水平跟随
		{
			if (abs(headX - tailX[1]) == 1)//解决蛇身秒闪过去的动画
			{
				spSnakeBody.setPosition((tailX[1] + (headX - tailX[1]) * stepLength) * GRIDSIZE + detaX - 2, tailY[1] * GRIDSIZE + detaY - 1);
			}
		}
		if (tailY[1] != headY && tailX[1] == headX)	//竖直跟随
		{
			if (abs(headY - tailY[1]) == 1)//解决蛇身秒闪过去的动画
			{
				spSnakeBody.setPosition(tailX[1] * GRIDSIZE + detaX - 2, (tailY[1] + (headY - tailY[1]) * stepLength) * GRIDSIZE + detaY - 1);
			}
		}
		window.draw(spSnakeBody);
	}
	for (int i = 2; i < ntail; i++)
	{
		if (tailY[i] == tailY[i - 1] && tailX[i] != tailX[i - 1])	//水平跟随
		{
			if (abs(tailX[i - 1] - tailX[i]) == 1)//解决蛇身秒闪过去的动画
			{
				spSnakeBody.setPosition((tailX[i] + (tailX[i - 1] - tailX[i]) * stepLength) * GRIDSIZE + detaX - 2, tailY[i] * GRIDSIZE + detaY - 1);
			}
		}
		if (tailY[i] != tailY[i - 1] && tailX[i] == tailX[i - 1])	//竖直跟随
		{
			if (abs(tailY[i - 1] - tailY[i]) == 1)//解决蛇身秒闪过去的动画
			{
				spSnakeBody.setPosition(tailX[i] * GRIDSIZE + detaX - 2, (tailY[i] + (tailY[i - 1] - tailY[i]) * stepLength) * GRIDSIZE + detaY - 1);
			}
		}
		window.draw(spSnakeBody);
	}

	//绘制fruit
	spFruit.setPosition(fruitX * GRIDSIZE + detaX - 2, fruitY * GRIDSIZE + detaY - 1);
	window.draw(spFruit);

	if (gameOver) {
		gameOver_info(width / 8 * GRIDSIZE, height / 4 * GRIDSIZE);
	}

	window.display();	//把显示缓冲区的内容，显示到屏幕上，sfml采用的是双缓冲机制
}

int main()
{
    
    //sf::CircleShape shape(100.f);
    //shape.setFillColor(sf::Color::Green);
	HWND hwnd = GetConsoleWindow();
	::SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
	//程序运行至此处后，控制台窗口就会隐藏

	do
	{
		Initial();
		
		while (window.isOpen() && gameOver == false)
		{
			
			//Input();
			switch (GameMode)
			{
			case 1:
				Input();
				delay++;
				if (!gamePause || !gameOver)
				{
					if (delay % 10 == 0)
					{
						delay = 0;
						Logic();
					}
				}

				Draw();
				break;
			case 2:
				InputStep();
				LogicStep();
				DrawStep();
				break;
			}
		}
		//GameOver_info();
		if (gameQuit == true)
		{
			break;
		}
		while (gameOver)
		{
			Event e;
			while (window.pollEvent(e))
			{
				if (e.type == Event::Closed)
				{
					window.close();
					gameOver = false;
					gameQuit = false;
				}
				if (e.type == Event::EventType::KeyReleased && e.key.code == Keyboard::Y)
				{
					gameOver = false;
				}
				if (e.type == Event::EventType::KeyReleased && e.key.code == Keyboard::N) {
					gameOver = false;
					gameQuit = true;
				}
			}
		}

	} while (!gameQuit);
   

    return 0;
}












