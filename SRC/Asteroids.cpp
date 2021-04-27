#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "Enemy.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include<iostream>
#include<fstream>
#include<vector>
#include<algorithm>


// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation *explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation *asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation* asteroid3_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid3", 64, 4096, 64, 64, "asteroid3_fs.png");
	Animation *spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");
	Animation *enemy_anim = AnimationManager::GetInstance().CreateAnimationFromFile("enemy", 128, 128, 128, 128, "enemy_fs.png");

	// Create a spaceship and add it to the world
	mGameWorld->AddObject(CreateSpaceship());

	// Create an enemy and add it to the world
	mGameWorld->AddObject(CreateEnemy());
	// Create some asteroids and add them to the world
	CreateAsteroids(10);

	//Create the GUI
	CreateGUI();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Start the game
	GameSession::Start();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	switch (key)
	{
	case ' ':
		mSpaceship->Shoot();
		break;
	default:
		break;
	}

	
	if (namePrompt->GetVisible()) {
		if (CharCheck(key)) {//Check if characters are valid by evaluating the ASCII value
			playerName += key; //If it is, add it to the string that will hold the player's name
			charCount++; //Increment the count
			namePrompt->SetText("Enter Name: " + playerName); //Update the name prompt with the user's input
		}
		if (charCount == 3) {
			namePrompt->SetVisible(false); //Once 3 characters have been verfied and added to the name string, hide the name prompt. This stops the keylistener from passing in characters to the nameprompt method to check.
			auto it = next(names.begin(), pos); //Create an iterator that points to the same position the player's high score was inserted into the vector of scores
			names.insert(it, playerName); //Insert the player's name at the same position the player's high score was
			names.pop_back(); //Remove the last name in the list. Their score is no longer top 5
			ShowHighScores(); //Now the high score list can be shown, where the updated lists will be reflected.
		}
	}
}

bool Asteroids::CharCheck(uchar input_char)
{
	// CHECKING FOR ALPHABET
	if ((input_char >= 65 && input_char <= 90) || (input_char >= 97 && input_char <= 122)) {
		return true;
	}
	else if (input_char >= 48 && input_char <= 57) { // CHECKING FOR DIGITS
		return true;
	}
	else { // OTHERWISE SPECIAL CHARACTER
		return false;
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is pressed start applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
	// If left arrow key is pressed start rotating anti-clockwise
	case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
	// If right arrow key is pressed start rotating clockwise
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
	// Default case - do nothing
	default: break;
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	switch (key)
	{
	// If up arrow key is released stop applying forward thrust
	case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
	// If left arrow key is released stop rotating
	case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
	// If right arrow key is released stop rotating
	case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
	// Default case - do nothing
	default: break;
	} 
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	//If the object destroyed was an enemy, wait 5 seconds and add a new one.
	if (object->GetType() == GameObjectType("Enemy"))
	{
		SetTimer(5000, CREATE_NEW_ENEMY);
	}
	
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		if (!object->isSplit()) { //If the asteroid hasn't split already...
			CreateSmallerAsteroids(object); //Call this method to create two smaller ones. Smaller asteriods will have their 'split' attribute set to true, and cannot split any further.
											//The asteriod itself is also passed in as its position is needed to determine where to spawn the two new asteriods.
		}
		mAsteroidCount--;
		if (mAsteroidCount <= 0) 
		{ 
			SetTimer(500, START_NEXT_LEVEL); 
		}
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		//When the player respawns, aggro the enemy on them again.
		mSpaceship->Reset();
		mGameWorld->AddObject(mSpaceship);
		mEnemy->SetAggro(true);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);
		mScoreLabel->SetVisible(false);
		CalculateHighScore();
	}

	if (value == CREATE_NEW_ENEMY)
	{
		//Teleport the enemy to a random location upon creation and set its hp to 5.
		mEnemy->Teleport();
		mEnemy->SetHp(5);
		mGameWorld->AddObject(mEnemy);
	}
}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

shared_ptr<GameObject> Asteroids::CreateEnemy()
{
	// Create a raw pointer to an enemy that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mEnemy = make_shared<Enemy>();
	mEnemy->SetBoundingShape(make_shared<BoundingSphere>(mEnemy->GetThisPtr(), 4.0f));
	shared_ptr<Shape> enemy_bullet_shape = make_shared<Shape>("enemy_bullet.shape");
	mEnemy->SetBulletShape(enemy_bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("enemy");
	shared_ptr<Sprite> enemy_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mEnemy->SetSprite(enemy_sprite);
	mEnemy->SetScale(0.1f);
	mEnemy->SetTarget(mSpaceship);
	// Return the enemy so it can be added to the world
	return mEnemy;
}


void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateSmallerAsteroids(shared_ptr<GameObject> object)
{	
	for (uint i = 0; i < 2; i++) //Only runs twice, since only two new asteroids are being created.
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid3");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		asteroid->SetPosition(object->GetPosition()); //Spawn these two smaller asteriods where the original was destroyed.
		asteroid->setSplit(true); //Makes sure the smaller asteriods cannot split again.
		mGameWorld->AddObject(asteroid);
		mAsteroidCount++; //Add one to the counter, since the original asteroid being destroyed took away one and we added two more just now.
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_TOP (to make space for the high scores)
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.8f));

	// Create new GUILabels and wrap them up in a shared_ptr
	scoreOne = shared_ptr<GUILabel>(new GUILabel("1st: "));
	scoreTwo = shared_ptr<GUILabel>(new GUILabel("2nd: "));
	scoreThree = shared_ptr<GUILabel>(new GUILabel("3rd: "));
	scoreFour = shared_ptr<GUILabel>(new GUILabel("4th: "));
	scoreFive = shared_ptr<GUILabel>(new GUILabel("5th: "));
	namePrompt = shared_ptr<GUILabel>(new GUILabel("Enter name: "));
	// Set the horizontal alignment of the labels to GUI_HALIGN_CENTER
	scoreOne->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	scoreTwo->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	scoreThree->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	scoreFour->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	scoreFive->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	namePrompt->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);

	// Set the vertical alignment of the labels to GUI_VALIGN_MIDDLE
	scoreOne->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	scoreTwo->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	scoreThree->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	scoreFour->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	scoreFive->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	namePrompt->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);

	// Set the visibility of the labels to false (hidden)
	scoreOne->SetVisible(false);
	scoreTwo->SetVisible(false);
	scoreThree->SetVisible(false);
	scoreFour->SetVisible(false);
	scoreFive->SetVisible(false);
	namePrompt->SetVisible(false);

	// Add the GUILabels to the GUIContainer  
	shared_ptr<GUIComponent> score_one_component
		= static_pointer_cast<GUIComponent>(scoreOne);
	shared_ptr<GUIComponent> score_two_component
		= static_pointer_cast<GUIComponent>(scoreTwo);
	shared_ptr<GUIComponent> score_three_component
		= static_pointer_cast<GUIComponent>(scoreThree);
	shared_ptr<GUIComponent> score_four_component
		= static_pointer_cast<GUIComponent>(scoreFour);
	shared_ptr<GUIComponent> score_five_component
		= static_pointer_cast<GUIComponent>(scoreFive);
	shared_ptr<GUIComponent> name_prompt_component
		= static_pointer_cast<GUIComponent>(namePrompt);

	mGameDisplay->GetContainer()->AddComponent(score_one_component, GLVector2f(0.5f, 0.7f));
	mGameDisplay->GetContainer()->AddComponent(score_two_component, GLVector2f(0.5f, 0.6f));
	mGameDisplay->GetContainer()->AddComponent(score_three_component, GLVector2f(0.5f, 0.5f));
	mGameDisplay->GetContainer()->AddComponent(score_four_component, GLVector2f(0.5f, 0.4f));
	mGameDisplay->GetContainer()->AddComponent(score_five_component, GLVector2f(0.5f, 0.3f));
	mGameDisplay->GetContainer()->AddComponent(name_prompt_component, GLVector2f(0.5f, 0.2f));
	
	//Initialise these global variables.
	charCount = 0;
	pos = 0;
}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0) 
	{ 
		//Teleport the enemy away to a random location if the player dies and pacify it until the player has respawned.
		mEnemy->Teleport();
		mEnemy->SetAggro(false);
		SetTimer(2000, CREATE_NEW_PLAYER);
	}
	else
	{
		SetTimer(500, SHOW_GAME_OVER);
	}
}

vector<string> Asteroids::split(const string& s) {
	vector<string> v;
	auto space = [](char c) {return c == ' ';}; //A lambda function to define whether a character is a space or not.
	auto start = find_if_not(s.cbegin(), s.cend(), space);//Find the first non space.
	while (start != s.cend()) {//Check to see the end hasnt been reached
		auto end = find_if(start, s.cend(), space);//Find the next space.
		v.push_back(string(start, end));//Put the characters between the spaces into a string and push it onto the vector.
		start = find_if_not(end, s.cend(), space);//Find the next space.
	}
	return v;
}

void Asteroids::CalculateHighScore() 
{	
	//Goes through the list of scores to see if the current score is a high score
	ifstream f; //The file to be opened
	string line; //The line to be read
	int i = 0; //amount of times to loop

	f.open("scores.txt"); //Opens the file

	if (f.is_open()) {
		while (getline(f, line) && i < 5) { //Loop runs 5 times to get the high scores, reading the file line by line
			auto v = split(line); //Splits the line read into the name and the score
			names.push_back(v[0]); //Pushes the name onto the vector of names
			scores.push_back(stoi(v[1])); //Converts each line to an int and pushes it onto the vector of scores
			i++;
		}
	}
	f.close(); //Closes the file


	for (auto it = begin(scores); it != end(scores);++it) { //Cycle through the list of scores and see if the current score is greater than any of the others.
		if ((mScoreKeeper.getScore()) >= *it) { //If the player's score is bigger than or equal to any score found in the list...
			pos = distance(scores.begin(), it); //Gives the index which will be replaced in the list of names
			scores.push_back(mScoreKeeper.getScore()); //Add it to the list of scores
			sort(scores.begin(), scores.end(), greater<int>()); //Sort the list in ascending order
			scores.pop_back(); //Removes the lowest score as it is no longer a high score
			namePrompt->SetVisible(true); //Make the name prompt show up which will cause the listener to pass on the next 3 characters pressed...
			break;
		}
	}

	if (namePrompt->GetVisible()) {
		//This means the program is prompting the user to enter their name, which means they got a high score, so nothing should be done here.
	}
	else {
		ShowHighScores(); //If not, the user's score was not a high score, and the list can be shown without any changes being made.
	}
}

void Asteroids::ShowHighScores() {
	// Format each high score label using an string-based stream
	std::ostringstream msg_stream_one;
	msg_stream_one << "1st: " << names.at(0) << " " << scores.at(0);
	std::string first = msg_stream_one.str();
	scoreOne->SetText(first);

	std::ostringstream msg_stream_two;
	msg_stream_two << "2nd: " << names.at(1) << " " << scores.at(1);
	std::string second = msg_stream_two.str();
	scoreTwo->SetText(second);

	std::ostringstream msg_stream_three;
	msg_stream_three << "3rd: " << names.at(2) << " " << scores.at(2);
	std::string third = msg_stream_three.str();
	scoreThree->SetText(third);

	std::ostringstream msg_stream_four;
	msg_stream_four << "4th: " << names.at(3) << " " << scores.at(3);
	std::string fourth = msg_stream_four.str();
	scoreFour->SetText(fourth);

	std::ostringstream msg_stream_five;
	msg_stream_five << "5th: " << names.at(4) << " " << scores.at(4);
	std::string fifth = msg_stream_five.str();
	scoreFive->SetText(fifth);

	scoreOne->SetVisible(true);
	scoreTwo->SetVisible(true);
	scoreThree->SetVisible(true);
	scoreFour->SetVisible(true);
	scoreFive->SetVisible(true);

	//Save the scores
	ofstream f2;
	f2.open("scores.txt");

	for (int i = 0;i < 5;++i) {
		f2 << names.at(i) << " " << scores.at(i) << '\n';
	}
	f2.close();
}
shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}

