/*
	��̨�����귶Χ[0][width-1]��[0][height-1]
	gameModeͨ�����ո����������Ϸģʽ�������Ͳ���ģʽ�л�
	x���˳���Ϸ
	+-���Ӽ�����������enter���������ֿ��أ��ո���л�ģʽ
	��ǽһ��������������DrawStep��������ɣ���ǰ������ڵ㲻����ʱ����������
	��ͷ180����ת�Ľ������������ͷ����һ�������ķŸ�֮���ٶ���һ�εķ���任����Ӧ
*/

#include <SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include<iostream>
#include<Windows.h>

#define WINDOW_WIDTH 80     //���ڿ��
#define WINDOW_HEIGHT 25    //���ڸ߶�
#define STAGE_WIDTH 20      //��̨���
#define STAGE_HEIGHT 20     //��̨�߶�
#define GRIDSIZE 25     //����ߴ�
#define SCALE 0.5
#define MAXLENGTH 100   //������󳤶�
#define INFO_WIDTH 400
#define STEP 0.1

using namespace sf;

bool gameOver;
bool gameQuit;
bool gamePause;
bool MutexOnceInput;//ȷ������һ��������������һ�εķ���仯

const int width = STAGE_WIDTH;
const int height = STAGE_HEIGHT;
int headX, headY, fruitX, fruitY, score;
int tailX[MAXLENGTH], tailY[MAXLENGTH];
int ntail;
int delay;
int GameMode;	//������Ϸ������ʾģʽ
float stepX, stepY;

enum eDirection { STOP = 0,LEFT,RIGHT,UP,DOWN };
eDirection dir , dir_ing;

sf::RenderWindow window(sf::VideoMode(width*GRIDSIZE + GRIDSIZE + INFO_WIDTH, height*GRIDSIZE +GRIDSIZE), L"SFML Snake!");
Texture tBackground, tSnakeHead, tSnakeBody, tFruit;	//�����������
Sprite spBackground, spSnakeHead, spSnakeBody, spFruit;	//�����������
int headRotation;
Font font;
Text text;
SoundBuffer sbEat, sbDie;
Sound soundEat, soundDie;
Music bkMusic;
int soundVolume;	//��������
bool MusicOn;		//�������ֿ���
sf::Clock KeyClockTimer;	//����������ģ�����ʱ�����ͬʱ��������������ߵ�ͷ���µĿ��ܣ�������Ϸ����������
int KeyClockDelay = 50;	//ȥ����ʱ����ֵ����λ����
bool dirChange = false;

//��ʾ��Ϣ�����������������ְ������̨�е�ԭ�����꣬ͨ��initialX���Ƹ������������ԭ��x���������
void Prompt_info(int x, int y)
{
	int initialX = STAGE_WIDTH, initialY = 0;
	int CharacterSize = 24;
	text.setCharacterSize(CharacterSize);
	text.setFillColor(Color(255, 255, 255, 255));
	text.setStyle(Text::Bold);

	/*SetConsoleTextAttribute(h, COLOR_TEXT);*/
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"����Ϸ˵����");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    ��������ײ����Ϸ����");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    ���߿ɴ�ǽ");
	window.draw(text);
	initialY += CharacterSize * 2;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"������˵����");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    �������ƶ�����W");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    �������ƶ�����S");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    �������ƶ�����A");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    �������ƶ�����D");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    ����ʼ��Ϸ�����������");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    ���������ֿ��أ��س���");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    �������������ڣ�+/-��");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    ������ģʽ�л����ո��");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	if (GameMode == 1)
	{
		text.setFillColor(Color(60, 110, 255, 255));//blue
		text.setString(L"         �����ƶ�");
	}
	else
	{
		text.setFillColor(Color(205, 160, 0, 255));//
		text.setString(L"         �����ƶ�");
	}
	window.draw(text);
	text.setFillColor(Color(255, 255, 255, 255));	//White text
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"    ���˳���Ϸ��x���˳�");
	window.draw(text);
	initialY += CharacterSize * 3;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"�� ���ߣ�������ý ZQH");
	window.draw(text);
}

//��Ϸ������Ϣ
void gameOver_info(int x,int y)
{
	int initialX = 0, initialY = 20;
	int CharacterSize = 48;
	text.setCharacterSize(CharacterSize);
	text.setFillColor(Color(255, 0, 0, 255));
	text.setStyle(Text::Bold);

	text.setPosition(x + initialX, y + initialY);
	text.setString(L"        ��Ϸ����!!");
	window.draw(text);
	initialY += CharacterSize;
	text.setPosition(x + initialX, y + initialY);
	text.setString(L"   Y���¿�ʼ/n�˳�");
	window.draw(text);
}

void Initial()
{
	window.setFramerateLimit(60);	//ÿ������Ŀ��֡��
	
	//��������
	if (!font.loadFromFile("../data/fonts/ZCOOLKuaiLe-Regular.ttf"))
	{
		std::cout << "Font not find." << std::endl;
	}
	text.setFont(font);

	//��������ͼƬ
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

	//������Ƶ
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

	//���ؾ�����������
	spBackground.setTexture(tBackground);
	spSnakeHead.setTexture(tSnakeHead);
	spSnakeBody.setTexture(tSnakeBody);
	spFruit.setTexture(tFruit);

	//��setOrigin��������sprite���������ԭ��
	spBackground.setOrigin(GRIDSIZE / SCALE / 2, GRIDSIZE / SCALE / 2);
	spSnakeHead.setOrigin(GRIDSIZE / SCALE / 2, GRIDSIZE / SCALE / 2);
	spSnakeBody.setOrigin(GRIDSIZE / SCALE / 2, GRIDSIZE / SCALE / 2);
	spFruit.setOrigin(GRIDSIZE / SCALE / 2, GRIDSIZE / SCALE / 2);

	//�Ծ������������Ų���
	spBackground.setScale(SCALE, SCALE);
	spSnakeHead.setScale(SCALE, SCALE);
	spSnakeBody.setScale(SCALE, SCALE);
	spFruit.setScale(SCALE, SCALE);

	//������Ƶ��Ӧ����
	soundEat.setBuffer(sbEat);	//��Ч���뻺��
	soundDie.setBuffer(sbDie);	//��Ч���뻺��
	bkMusic.play();				//�������ֲ���
	bkMusic.setLoop(true);		//��������ѭ��
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

    //��ʼ��
    srand(time(NULL));
    dir = STOP;
	dir_ing = STOP;
    headX = width / 2;
    headY = height / 2;
    fruitX = rand() % width;
    fruitY = rand() % height;
    score = 0;
	delay = 0;

    //�����ʼ��
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

	//��ǽ
	if (headX >= width)headX = 0;
	else if (headX < 0)headX = STAGE_WIDTH - 1;
	if (headY >= height)headY = 0;
	else if (headY < 0)headY = STAGE_HEIGHT - 1;

	//ײǽ��gameover
	/*if (headX > STAGE_WIDTH - 1 || headX < 0 || headY > STAGE_HEIGHT - 1 || headY < 0)
	{
		gameOver = true;
	}*/

	//��ͷ��������ײ��gameover
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
		soundEat.play();	//����eat��Ч
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
				if ((fruitX == tailX[i] && fruitY == tailY[i]) || (fruitX == originX && fruitY == originY))	//�����ɵ�fruit��������
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

	//������λ�ð������󴫣�ȥ���������һ��λ�õ���β
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
	

	//���Ʊ���
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			spBackground.setPosition(i * GRIDSIZE + detaX, j * GRIDSIZE + detaY);	//ָ�������λ��
			window.draw(spBackground);								//��������Ƶ�������
		}
	}

	//������
	spSnakeHead.setPosition(headX * GRIDSIZE + detaX - 2, headY * GRIDSIZE + detaY - 1);
	spSnakeHead.setRotation(headRotation);
	window.draw(spSnakeHead);
	for (int i = 1; i < ntail; i++)
	{
		spSnakeBody.setPosition(tailX[i] * GRIDSIZE + detaX - 2, tailY[i] * GRIDSIZE + detaY - 1);
		window.draw(spSnakeBody);
	}

	//����fruit
	spFruit.setPosition(fruitX * GRIDSIZE + detaX - 2, fruitY * GRIDSIZE + detaY - 1);
	window.draw(spFruit);

	if (gameOver) {
		gameOver_info(width / 8 * GRIDSIZE, height / 4 * GRIDSIZE);
	}

    window.display();	//����ʾ�����������ݣ���ʾ����Ļ�ϣ�sfml���õ���˫�������
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

	//���򰴼��¼�����ʱ�䶶��������dirChange�ϣ�ʵ��keyClockDelayʱ�����ܽ�����һ�η�����£�
	//���ʱ�仹�ǲ��ÿ��ư���������Ϸ֡�ʵ���ߣ�������Ӧ��ʱ���������û������׳�������
	//���Խ���Ϸ�߼�����Ϊ������ͷ����һ������֮������Ӧ��һ������任
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

	if (MutexOnceInput)//ȷ������һ��������������һ�εķ���仯
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
			MutexOnceInput = true;//ȷ������һ��������������һ�εķ���仯
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

	
	//��ǽ
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

		//ײǽ��gameover
		/*if (headX > STAGE_WIDTH - 1 || headX < 0 || headY > STAGE_HEIGHT - 1 || headY < 0)
		{
			gameOver = true;
		}*/
		//��ͷ��������ײ��gameover
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
			soundEat.play();	//����eat��Ч
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
					if ((fruitX == tailX[i] && fruitY == tailY[i]) || (fruitX == originX && fruitY == originY))	//�����ɵ�fruit��������
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

		//������λ�ð������󴫣�ȥ���������һ��λ�õ���β
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


	//���Ʊ���
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			spBackground.setPosition(i * GRIDSIZE + detaX, j * GRIDSIZE + detaY);	//ָ�������λ��
			window.draw(spBackground);								//��������Ƶ�������
		}
	}

	//������
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
		if (tailY[1] == headY && tailX[1] != headX)	//ˮƽ����
		{
			if (abs(headX - tailX[1]) == 1)//�������������ȥ�Ķ���
			{
				spSnakeBody.setPosition((tailX[1] + (headX - tailX[1]) * stepLength) * GRIDSIZE + detaX - 2, tailY[1] * GRIDSIZE + detaY - 1);
			}
		}
		if (tailY[1] != headY && tailX[1] == headX)	//��ֱ����
		{
			if (abs(headY - tailY[1]) == 1)//�������������ȥ�Ķ���
			{
				spSnakeBody.setPosition(tailX[1] * GRIDSIZE + detaX - 2, (tailY[1] + (headY - tailY[1]) * stepLength) * GRIDSIZE + detaY - 1);
			}
		}
		window.draw(spSnakeBody);
	}
	for (int i = 2; i < ntail; i++)
	{
		if (tailY[i] == tailY[i - 1] && tailX[i] != tailX[i - 1])	//ˮƽ����
		{
			if (abs(tailX[i - 1] - tailX[i]) == 1)//�������������ȥ�Ķ���
			{
				spSnakeBody.setPosition((tailX[i] + (tailX[i - 1] - tailX[i]) * stepLength) * GRIDSIZE + detaX - 2, tailY[i] * GRIDSIZE + detaY - 1);
			}
		}
		if (tailY[i] != tailY[i - 1] && tailX[i] == tailX[i - 1])	//��ֱ����
		{
			if (abs(tailY[i - 1] - tailY[i]) == 1)//�������������ȥ�Ķ���
			{
				spSnakeBody.setPosition(tailX[i] * GRIDSIZE + detaX - 2, (tailY[i] + (tailY[i - 1] - tailY[i]) * stepLength) * GRIDSIZE + detaY - 1);
			}
		}
		window.draw(spSnakeBody);
	}

	//����fruit
	spFruit.setPosition(fruitX * GRIDSIZE + detaX - 2, fruitY * GRIDSIZE + detaY - 1);
	window.draw(spFruit);

	if (gameOver) {
		gameOver_info(width / 8 * GRIDSIZE, height / 4 * GRIDSIZE);
	}

	window.display();	//����ʾ�����������ݣ���ʾ����Ļ�ϣ�sfml���õ���˫�������
}

int main()
{
    
    //sf::CircleShape shape(100.f);
    //shape.setFillColor(sf::Color::Green);
	HWND hwnd = GetConsoleWindow();
	::SendMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
	//�����������˴��󣬿���̨���ھͻ�����

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












