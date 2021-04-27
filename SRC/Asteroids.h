#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"
#include<vector>

class GameObject;
class Spaceship;
class Enemy;
class GUILabel;


class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char *argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

	bool CharCheck(uchar input_char);

	vector<string> split(const string& s);

	

private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<Enemy> mEnemy;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;
	shared_ptr<GUILabel> namePrompt;

	shared_ptr<GUILabel> scoreOne;
	shared_ptr<GUILabel> scoreTwo;
	shared_ptr<GUILabel> scoreThree;
	shared_ptr<GUILabel> scoreFour;
	shared_ptr<GUILabel> scoreFive;

	uint mLevel;
	uint mAsteroidCount;

	shared_ptr<GameObject> CreateSpaceship();
	shared_ptr<GameObject> CreateEnemy();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	void CreateSmallerAsteroids(shared_ptr<GameObject> object);
	shared_ptr<GameObject> CreateExplosion();
	void CalculateHighScore();
	void ShowHighScores();
	
	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;
	const static uint CREATE_NEW_ENEMY = 3;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;

	int charCount;
	string playerName;

	vector<string> names;
	vector<int> scores;

	int pos;
};

#endif